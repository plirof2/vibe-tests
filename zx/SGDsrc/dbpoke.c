/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBPOKE.C                                                                                                        */
/* Description  : Game pokes handler                                                                                              */
/* Version type : Program module                                                                                                  */
/* Last changed : 15-04-1998  7:00                                                                                                */
/* Update count : 26                                                                                                              */
/* OS type      : PC (DOS)                                                                                                        */
/* Compiler     : Watcom C32 Optimizing Compiler Version 10.x                                                                     */
/* Linker       : Watcom C Linker                                                                                                 */
/*                                                                                                                                */
/*                Copyleft (C) 1995-2001 ThunderWare Research Center, written by Martijn van der Heide.                           */
/*                                                                                                                                */
/*                This program is free software; you can redistribute it and/or                                                   */
/*                modify it under the terms of the GNU General Public License                                                     */
/*                as published by the Free Software Foundation; either version 2                                                  */
/*                of the License, or (at your option) any later version.                                                          */
/*                                                                                                                                */
/*                This program is distributed in the hope that it will be useful,                                                 */
/*                but WITHOUT ANY WARRANTY; without even the implied warranty of                                                  */
/*                MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                                                   */
/*                GNU General Public License for more details.                                                                    */
/*                                                                                                                                */
/*                You should have received a copy of the GNU General Public License                                               */
/*                along with this program; if not, write to the Free Software                                                     */
/*                Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                                     */
/*                                                                                                                                */
/**********************************************************************************************************************************/

#define __DBPOKE_MAIN__

#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <direct.h>
#include "wbench32/wbench32.h"
#include "dbdbase.h"
#include "dbfile.h"
#include "dbconfig.h"
#include "dbscreen.h"
#include "dbmain.h"
#include "dbpoke.h"

#pragma pack                ()

/**********************************************************************************************************************************/
/* Define the static variables                                                                                                    */
/**********************************************************************************************************************************/

#define F_NAME      'N'
#define F_LASTNAME  'Y'
#define F_MORE      'M'
#define F_LASTPOKE  'Z'
#define MAXPOKNAME  30

static struct PokeEntry_s                                                                                       /* One poke entry */
{
  struct PokeEntry_s   *Next;
  struct PokeEntry_s   *Previous;
  byte                  Bank;
  word                  Address;
  word                  Set;                                                            /* byte for direct, 256+ for request user */
  byte                  Org;
};

static struct GamePokes_s                                                                                 /* One poke description */
{
  struct GamePokes_s   *Next;
  struct GamePokes_s   *Previous;
  char                  Name[MAXPOKNAME + 1];
  struct PokeEntry_s   *Pokes;
}                      *GamePokes = NULL;

static char            *_InvalidZ80File = "Invalid Z80 file %s";
static bool             Auto48Convert;
static bool             Auto128Convert;

static int              ErrorPoke;
static int              ErrorEntry;
static comboent         SelectedPoke       = 0;
static comboent         SelectedEntry      = 0;

static struct ComboInfo PokesWindow;
static struct ComboInfo EntriesWindow;

/**********************************************************************************************************************************/
/* Define the things needed when patching a game                                                                                  */
/**********************************************************************************************************************************/

static struct WindowInfo   MessageWindow = {220, 200, 200, 50, 200, 50, 200, 50, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE,
                                            NULL, WINF_EXCLUSIVE | WINF_WADJUST | WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE |
                                            WINF_XMSBUFFER, NULL, "Building Patched Game File"};
static struct MouseShape  *OldMouseShape;
static struct PokeEntry_s *PokeSetup = NULL;
static int                 FhIn = -1;
static int                 FhOut = -1;

/**********************************************************************************************************************************/
/* Define the static functions                                                                                                    */
/**********************************************************************************************************************************/

static void CleanupPatch        (void);
static void DestroyPokes        (void);
static void DestroyEntries      (struct PokeEntry_s *PokeEntries);
static bool SetupPokesWindow    (void);
static bool SetupEntriesWindow  (comboent SelectedLine);
static bool ReadGamePokes       (void);
static bool WriteGamePokes      (void);

static int  HandleAddPoke       (struct WindowInfo *Window, struct ButtonInfo *Button);
static int  HandleDelPoke       (struct WindowInfo *Window, struct ButtonInfo *Button);
static bool HandleUpdatePoke    (void);
static int  HandleAddEntry      (struct WindowInfo *Window, struct ButtonInfo *Button);
static int  HandleDelEntry      (struct WindowInfo *Window, struct ButtonInfo *Button);
static bool HandleUpdateEntry   (void);

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE INTERNAL POKE LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

static void CleanupPatch (void)

/**********************************************************************************************************************************/
/* Pre   : All used variables must be initialized and reflect the current situtaion.                                              */
/* Post  : All temporary items have been destroyed. If the output file was still open, it has been removed after closing.         */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  if (FhIn != -1)                                                                                /* Input file was already open ? */
  {
    close (FhIn);
    FhIn = -1;
  }
  if (FhOut != -1)                                                                               /* Patch file was already open ? */
  {
    close (FhOut);
    FhOut = -1;
    unlink (PokedGamePath);                                                                            /* Remove the failed patch */
  }
  if (PokeSetup != NULL)                                                                                   /* Destroy pokes array */
  {
    DestroyEntries (PokeSetup);
    PokeSetup = NULL;
  }
  DestroyWindow (&MessageWindow);
  SetMouseShape (OldMouseShape);
  return;
}

static void DestroyEntries (struct PokeEntry_s *PokeEntries)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : All memory taken by the given PokeEntry array has been released.                                                       */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct PokeEntry_s *OEntry;

  OEntry = PokeEntries;
  while (OEntry->Next != NULL)
  {
    OEntry = OEntry->Next;
    free (OEntry->Previous);
  }
  free (OEntry);
}

static void DestroyPokes (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : All memory taken by the `GamePokes' database has been released.                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *OPokes = NULL;

  if (GamePokes == NULL)
    return;
  OPokes = GamePokes;
  while (OPokes->Next != NULL)
  {
    DestroyEntries (OPokes->Pokes);
    OPokes = OPokes->Next;
    free (OPokes->Previous);
  }
  DestroyEntries (OPokes->Pokes);
  free (OPokes);
  GamePokes = NULL;
}

static bool SetupPokesWindow (void)

/**********************************************************************************************************************************/
/* Pre   : The PokesWindow must have been created.                                                                                */
/* Post  : All poke descriptions are inserted into a reinitialized PokesWindow and the entries have been redrawn.                 */
/*         The return value is FALSE if an out-of-memory error occured (which is not reported here).                              */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *OPokes;
  struct ComboEntry   NEntry;
  bool                AllOk = TRUE;

  OPokes = GamePokes;
  NEntry.Selected = 0x00;
  ReInitComboWindow (&PokesWindow, 16);
  while (OPokes != NULL)
  {
    strcpy (NEntry.Text, OPokes->Name);
    NEntry.Identifier = (dword)OPokes;
    if (!InsertComboEntry (&PokesWindow, &NEntry, NULL, -1))
      AllOk = FALSE;
    OPokes = OPokes->Next;
  }
  PrintComboEntries (&PokesWindow);
  return (AllOk);
}

static bool SetupEntriesWindow (comboent SelectedLine)

/**********************************************************************************************************************************/
/* Pre   : The EntriesWindow must have been created. `SelectedLine' holds the poke description number.                            */
/* Post  : All poke values belonging to the given description are inserted into a reinitialized EntriesWindow and the entries     */
/*         have been redrawn. The return value is FALSE if an out-of-memory error occured (which is not reported here).           */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct PokeEntry_s *OEntry;
  struct ComboEntry   NEntry;
  bool                AllOk = TRUE;
  char                ABank[2];
  char                ASet[4];

  if (PokesWindow.NumEntries == -1)
    OEntry = NULL;
  else
  {
    GetComboEntry (&PokesWindow, &NEntry, SelectedLine);
    OEntry = ((struct GamePokes_s *)NEntry.Identifier)->Pokes;
  }
  NEntry.Selected = 0x00;
  ReInitComboWindow (&EntriesWindow, 16);
  while (OEntry != NULL)
  {
    if (OEntry->Bank < 8)
      sprintf (ABank, "%u", OEntry->Bank);
    else
      strcpy (ABank, "-");
    if (OEntry->Set < 256)
      sprintf (ASet, "%3u", OEntry->Set);
    else
      strcpy (ASet, "ASK");
    sprintf (NEntry.Text, "%s  %5u,%s (%3u)", ABank, OEntry->Address, ASet, OEntry->Org);
    NEntry.Identifier = (dword)OEntry;
    if (!InsertComboEntry (&EntriesWindow, &NEntry, NULL, -1))
      AllOk = FALSE;
    OEntry = OEntry->Next;
  }
  PrintComboEntries (&EntriesWindow);
  return (AllOk);
}

static bool ReadGamePokes (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The pokes file of the currently selected game has been read into memory if it existed.                                 */
/*         The return value is FALSE if any read/memory error occurs.                                                             */
/* Import: TestPokesFileExists.                                                                                                   */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *NPokes;
  struct GamePokes_s *OPokes = NULL;
  struct PokeEntry_s *NEntry;
  struct PokeEntry_s *OEntry = NULL;
  int                 Fh;
  int       register  RCnt;
  char                NameId = F_NAME;
  char                PokeId;
  char                RdBuffer[300];
  dword               Length = 0;
  byte                NumPokes;
  bool                More;

  ErrorPoke = ErrorEntry = 0;
  Auto48Convert = Auto128Convert = FALSE;                                                                /* Assume valid bankings */
  AutoBank (0, "");                                                                             /* Determine 48K or 128K snapshot */
  if (TestPokesFileExists (FALSE))
  {
    if ((Fh = open (PokesPath, O_TEXT | O_RDONLY)) == -1)
      return (FALSE);
    OPokes = GamePokes;
    if (FileType == FT_SLT && SLTMap.PokesIncluded)
    {
      if (lseek (Fh, SLTMap.TableEnd + SLTMap.OffsetPokes, SEEK_SET) == -1)
      { close (Fh); return (FALSE); }
      while (Length < SLTMap.HeaderPokes.Length)
      {
        ErrorPoke ++;
        ErrorEntry = 0;
        if ((NPokes = (struct GamePokes_s *)malloc (sizeof (struct GamePokes_s))) == NULL)
        { close (Fh); return (FALSE); }
        NPokes->Next = NULL;
        NPokes->Pokes = NULL;
        if (OPokes == NULL)
        {
          GamePokes = NPokes;
          NPokes->Previous = NULL;
        }
        else
        {
          OPokes->Next = NPokes;
          NPokes->Previous = OPokes;
        }
        RCnt = -1;
        More = TRUE;
        while (++ RCnt < 299 && More)
          if (read (Fh, RdBuffer + RCnt, 1) == -1)        { close (Fh); return (FALSE); }
          else
            if (!*(RdBuffer + RCnt))                                                                           /* End of string ? */
              More = FALSE;
        if (More)                                         { close (Fh); return (FALSE); }                /* String way too long ? */
        RdBuffer[RCnt > MAXPOKNAME ? MAXPOKNAME : RCnt] = '\0';
        strcpy (NPokes->Name, RdBuffer);
        if (read (Fh, &NumPokes, 1) == -1)                { close (Fh); return (FALSE); }
        while (NumPokes > 0)
        {
          ErrorEntry ++;
          if ((NEntry = (struct PokeEntry_s *)malloc (sizeof (struct PokeEntry_s))) == NULL)
          { close (Fh); return (FALSE); }
          NEntry->Next = NULL;
          if (OEntry == NULL)
          {
            NPokes->Pokes = NEntry;
            NEntry->Previous = NULL;
          }
          else
          {
            OEntry->Next = NEntry;
            NEntry->Previous = OEntry;
          }
          if (read (Fh, &NEntry->Address, 2) == -1)       { close (Fh); return (FALSE); }
          if (read (Fh, &NEntry->Bank, 1) == -1)          { close (Fh); return (FALSE); }
          if (read (Fh, &NEntry->Set, 1) == -1)           { close (Fh); return (FALSE); }
          if (read (Fh, &NEntry->Org, 1) == -1)           { close (Fh); return (FALSE); }
          if (NEntry->Bank & 0x10)                                                                       /* Requires user input ? */
            NEntry->Set = 256;
          if (NEntry->Bank & 0x08)                                                                             /* Ignore paging ? */
            if (Snap128K)
            {
              NEntry->Bank = AutoBank (NEntry->Address, "");                                       /* Auto-convert 128K snapshots */
              Auto128Convert = TRUE;
            }
            else
              NEntry->Bank = 8;
          else
            if (Snap128K)
              NEntry->Bank &= 0x07;                                                                        /* Keep only page part */
            else
            {
              NEntry->Bank = 8;                                                                     /* Auto-convert 48K snapshots */
              Auto48Convert = TRUE;
            }
        }
      }
    }
    else
    {
      while (NameId == F_NAME)
      {
        ErrorPoke ++;
        ErrorEntry = 0;
        if (read (Fh, &NameId, 1) == -1)                  { close (Fh); return (FALSE); }
        if (NameId == F_NAME)                                                                              /* Not the endmarker ? */
        {
          if ((NPokes = (struct GamePokes_s *)malloc (sizeof (struct GamePokes_s))) == NULL)
          { close (Fh); return (FALSE); }
          NPokes->Next = NULL;
          NPokes->Pokes = NULL;
          if (OPokes == NULL)
          {
            GamePokes = NPokes;
            NPokes->Previous = NULL;
          }
          else
          {
            OPokes->Next = NPokes;
            NPokes->Previous = OPokes;
          }
          if (read (Fh, NPokes->Name, MAXPOKNAME) == -1)  { close (Fh); return (FALSE); }
          RCnt = MAXPOKNAME;
          while (-- RCnt >= 0 && NPokes->Name[RCnt] == ' ')
            ;
          NPokes->Name[++ RCnt] = '\0';
          if (read (Fh, RdBuffer, 1) == -1)               { close (Fh); return (FALSE); }
          if (RdBuffer[0] != '\n')                        { close (Fh); return (FALSE); }
          OEntry = NULL;
          PokeId = F_MORE;
          while (PokeId == F_MORE)
          {
            ErrorEntry ++;
            if ((NEntry = (struct PokeEntry_s *)malloc (sizeof (struct PokeEntry_s))) == NULL)
            { close (Fh); return (FALSE); }
            NEntry->Next = NULL;
            if (OEntry == NULL)
            {
              NPokes->Pokes = NEntry;
              NEntry->Previous = NULL;
            }
            else
            {
              OEntry->Next = NEntry;
              NEntry->Previous = OEntry;
            }
            if (read (Fh, &PokeId, 1) == -1)              { close (Fh); return (FALSE); }
            if (read (Fh, RdBuffer, 3) == -1)             { close (Fh); return (FALSE); }
            RdBuffer[3] = '\0';
            NEntry->Bank = atoi (RdBuffer);
            if (!Snap128K && NEntry->Bank < 8)
            {
              NEntry->Bank = 8;                                                                     /* Auto-convert 48K snapshots */
              Auto48Convert = TRUE;
            }
            if (read (Fh, RdBuffer, 1) == -1)             { close (Fh); return (FALSE); }
            if (RdBuffer[0] != ' ')                       { close (Fh); return (FALSE); }
            if (read (Fh, RdBuffer, 5) == -1)             { close (Fh); return (FALSE); }
            RdBuffer[5] = '\0';
            NEntry->Address = atoi (RdBuffer);
            if (Snap128K && NEntry->Bank >= 8)                                                     /* Auto-convert 128K snapshots */
            {
              NEntry->Bank = AutoBank (NEntry->Address, "");
              Auto128Convert = TRUE;
            }
            if (read (Fh, RdBuffer, 1) == -1)             { close (Fh); return (FALSE); }
            if (RdBuffer[0] != ' ')                       { close (Fh); return (FALSE); }
            if (read (Fh, RdBuffer, 3) == -1)             { close (Fh); return (FALSE); }
            RdBuffer[3] = '\0';
            NEntry->Set = atoi (RdBuffer);
            if (read (Fh, RdBuffer, 1) == -1)             { close (Fh); return (FALSE); }
            if (RdBuffer[0] != ' ')                       { close (Fh); return (FALSE); }
            if (read (Fh, RdBuffer, 3) == -1)             { close (Fh); return (FALSE); }
            RdBuffer[3] = '\0';
            NEntry->Org = atoi (RdBuffer);
            if (read (Fh, RdBuffer, 1) == -1)             { close (Fh); return (FALSE); }
            if (RdBuffer[0] != '\n')                      { close (Fh); return (FALSE); }
            OEntry = NEntry;
          }
          OPokes = NPokes;
        }
      }
    }
    close (Fh);
    return (TRUE);
  }
  return (FALSE);
}

static bool WriteGamePokes (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The pokes file of the currently selected game has been written (back) to disk. All poke descriptions without actual    */
/*         pokes attached have been skipped. If an empty file has been written, the file is removed before returning.             */
/*         The return value is TRUE if the write was done (after possibly asking the user).                                       */
/* Import: TestPokesFileExists.                                                                                                   */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *NPokes;
  struct PokeEntry_s *NEntry;
  int                 Fh;
  char                Buffer[80];
  char                TypeId;
  bool                AtLeastOne = FALSE;

  TestPokesFileExists (TRUE);
  if (PokesDirExists)
    if (!TestFileOverwrite (PokesPath))
    {
      if ((Fh = open (PokesPath, O_TEXT | O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1)
      {
        ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Could not create file:\n%s", strerror (errno));
        return (FALSE);
      }
      NPokes = GamePokes;
      while (NPokes != NULL)
      {
        TypeId = F_NAME;
        while (NPokes != NULL && NPokes->Pokes == NULL)                                       /* Only a name, no pokes attached ? */
          NPokes = NPokes->Next;                                                                                       /* Skip it */
        if (NPokes != NULL)                                                                             /* Still anything to do ? */
        {
          AtLeastOne = TRUE;
          sprintf (Buffer, "%c%-30s\n", TypeId, NPokes->Name);
          write (Fh, Buffer, strlen (Buffer));
          NEntry = NPokes->Pokes;
          while (NEntry != NULL)
          {
            if (NEntry->Next == NULL)
              TypeId = F_LASTPOKE;
            else
              TypeId = F_MORE;
            sprintf (Buffer, "%c%3u %5u %3u %3u\n", TypeId, NEntry->Bank, NEntry->Address, NEntry->Set, NEntry->Org);
            write (Fh, Buffer, strlen (Buffer));
            NEntry = NEntry->Next;
          }
          NPokes = NPokes->Next;
        }
      }
      TypeId = F_LASTNAME;
      write (Fh, &TypeId, 1);
      close (Fh);
      if (!AtLeastOne)                                                                                     /* No entries at all ? */
        unlink (PokesPath);                                                                                  /* Delete empty file */
      return (TRUE);
    }
  return (FALSE);
}

static int HandleAddPoke (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Add' button has been pressed (or released).                                                                       */
/* Post  : A new poke entry has been created.                                                                                     */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *OPokes;
  struct GamePokes_s *NPokes;
  struct ComboEntry   NEntry;
  struct ComboEntry   CEntry;

  if (!(Button->Flags & BF_PRESSED))
  {
    NEntry.Text[0] = '\0';
    if (InputRequester (-1, -1, NULL, NULL, NEntry.Text, MAXPOKNAME + 1, TRUE, FALSE, "Add Poke Description",
                        "Give the description of the new poke"))
    {
      if ((NPokes = (struct GamePokes_s *)malloc (sizeof (struct GamePokes_s))) == NULL)
        FatalError (1, _ErrOutOfMemory, sizeof (struct GamePokes_s));
      strcpy (NPokes->Name, NEntry.Text);
      NPokes->Next = NULL;
      NPokes->Pokes = NULL;
      if (GamePokes == NULL)
      {
        GamePokes = NPokes;
        NPokes->Previous = NULL;
      }
      else
      {
        GetComboEntry (&PokesWindow, &CEntry, PokesWindow.NumEntries);
        OPokes = (struct GamePokes_s *)CEntry.Identifier;                                                      /* (Last in chain) */
        OPokes->Next = NPokes;
        NPokes->Previous = OPokes;
      }
      NEntry.Identifier = (dword)NPokes;
      NEntry.Selected = 0x00;
      if (!InsertComboEntry (&PokesWindow, &NEntry, NULL, -1))
        FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
      SelectedPoke = PokesWindow.NumEntries;
      SelectedEntry = 0;
      SetupEntriesWindow (SelectedPoke);
      SelectEntry (&PokesWindow, SelectedPoke);
      SelectEntry (&EntriesWindow, SelectedEntry);
      HandleAddEntry (Window, Button);                                                                 /* Auto-insert first entry */
      return (WINH_EDITEND);                                                             /* Signal: (un)ghost appropriate buttons */
    }
  }
  return (WINH_NONE);
}

static int HandleDelPoke (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Delete' button has been pressed (or released).                                                                    */
/* Post  : The current poke has been removed.                                                                                     */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *OPokes;
  struct PokeEntry_s *OEntry;
  struct ComboEntry   NEntry;
  
  Window = Window;
  if (!(Button->Flags & BF_PRESSED) && PokesWindow.NumEntries >= 0)
  {
    GetComboEntry (&PokesWindow, &NEntry, SelectedPoke);
    if (DBConfig.ConfirmDelete)                                                               /* Confirmation needed: handle it ? */
      if (QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Delete",
                             "Are you sure you want to delete the poke\n\"%s\"?", NEntry.Text))
        return (WINH_NONE);
    OPokes = (struct GamePokes_s *)NEntry.Identifier;
    OEntry = OPokes->Pokes;
    if (OEntry != NULL)                                                                                    /* Remove poke entries */
    {
      while (OEntry->Next != NULL)
      {
        OEntry = OEntry->Next;
        free (OEntry->Previous);
      }
      free (OEntry);
    }
    if (SelectedPoke == 0)                                                                                /* Removing first one ? */
    {
      GamePokes = OPokes->Next;
      OPokes->Next->Previous = NULL;
    }
    else
    {
      OPokes->Previous->Next = OPokes->Next;                                                                            /* Unlink */
      OPokes->Next->Previous = OPokes->Previous;
    }
    free (OPokes);
    DeleteComboEntry (&PokesWindow, SelectedPoke);
    if (SelectedPoke > PokesWindow.NumEntries)
      SelectedPoke --;
    SelectedEntry = 0;
    SetupEntriesWindow (SelectedPoke);
    SelectEntry (&PokesWindow, SelectedPoke);
    SelectEntry (&EntriesWindow, SelectedEntry);
    return (WINH_EDITEND);                                                               /* Signal: (un)ghost appropriate buttons */
  }
  return (WINH_NONE);
}

static bool HandleUpdatePoke (void)

/**********************************************************************************************************************************/
/* Pre   : A poke description has been double-clicked.                                                                            */
/* Post  : The current poke description has been updated.                                                                         */
/*         The return value is TRUE if the value has been changed.                                                                */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *OPokes;
  char               *CEntry;
  struct ComboEntry   NEntry;

  GetComboEntry (&PokesWindow, &NEntry, SelectedPoke);
  OPokes = (struct GamePokes_s *)NEntry.Identifier;
  CEntry = NEntry.Text;
  if (InputRequester (-1, -1, NULL, NULL, CEntry, MAXPOKNAME + 1, TRUE, FALSE, "Update Poke Description",
                      "Give the new description of the poke"))
    if (strcmp (CEntry, OPokes->Name))                                                                      /* Actually changed ? */
    {
      PutComboEntry (&PokesWindow, &NEntry, SelectedPoke, TRUE);
      strcpy (OPokes->Name, CEntry);
      SelectEntry (&PokesWindow, SelectedPoke);
      return (TRUE);
    }
  return (FALSE);
}

static int HandleAddEntry (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Insert' button has been pressed (or released).                                                                    */
/* Post  : A new poke entry has been inserted.                                                                                    */
/* Import: AutoBank.                                                                                                              */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *OPokes;
  struct PokeEntry_s *OEntry;
  struct PokeEntry_s *NEntry;
  struct ComboEntry   NCEntry;
  char                ABank[2];
  char                AAddress[6];
  char                ASet[4];
  char                AOrg[4];
  struct WindowInfo   CWindow  = {190, 220, 160, 160, 160, 160, 160, 160, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                  WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "Add Poke"};
  struct ButtonInfo   BBank    = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 40, 1, DBLACK, DWHITE, NULL,
                                  1, NULL, NULL};
  struct ButtonInfo   BAddress = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 60, 5, DBLACK, DWHITE, NULL,
                                  2, NULL, NULL};
  struct ButtonInfo   BSet     = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 80, 3, DBLACK, DWHITE, NULL,
                                  3, NULL, NULL};
  struct ButtonInfo   BAskSet  = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 97, 80, 6, DBLACK, DWHITE, NULL,
                                  8, "\007", NULL};
  struct ButtonInfo   BOrg     = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 100, 3, DBLACK, DWHITE, NULL,
                                  4, NULL, NULL};
  struct ButtonInfo   BFetch   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_GHOSTED, 100, 100, 6, DBLACK, DWHITE, NULL,
                                  5, "@Fetch", NULL};
  struct ButtonInfo   BCancel  = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 10, 135, 8, DBLACK, DWHITE, NULL,
                                  6, "@Cancel", NULL};
  struct ButtonInfo   BOk      = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_GHOSTED, 96, 135, 8, DBLACK, DWHITE, NULL,
                                  7, "@Ok", NULL};
  struct ButtonInfo  *ButtonP;
  bool                Finished;
  bool                Cancelled;
  bool                Accepted = FALSE;
  byte               *MemoryImage;
  word                ImageSize;
  byte                RBank;
  word                RAddress;
  bool                ChangeOKButton;

  Window = Window;
  BBank.Contents = ABank;
  BAddress.Contents = AAddress;
  BSet.Contents = ASet;
  BOrg.Contents = AOrg;
  if (!(Button->Flags & BF_PRESSED))
  {
    AAddress[0] = ASet[0] = '\0';
    if (IsSnapshot (FileType))
      AOrg[0] = '\0';
    else
      strcpy (AOrg, "0");
    if (!Snap128K)                                                                                                  /* 48K Game ? */
    {
      BBank.Flags |= BF_GHOSTED;                                                                                 /* Then no banks */
      strcpy (ABank, "-");
    }
    else
      ABank[0] = '\0';
    AddButton (&CWindow, &BAddress, FALSE);
    AddButton (&CWindow, &BSet, FALSE);
    AddButton (&CWindow, &BAskSet, FALSE);
    AddButton (&CWindow, &BOrg, FALSE);
    AddButton (&CWindow, &BFetch, FALSE);
    AddButton (&CWindow, &BCancel, FALSE);
    AddButton (&CWindow, &BOk, FALSE);
    AddButton (&CWindow, &BBank, FALSE);
    DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
    Wprintf (&CWindow, 1, TEXT_CENTER, "Setup new poke parameters");
    Wprintf (&CWindow,  4, 2, "{+1Bank}N");
    Wprintf (&CWindow,  6, 2, "{+1Address}N");
    Wprintf (&CWindow,  8, 2, "{+1Value}N");
    Wprintf (&CWindow, 10, 2, "{+1Original}N");
    PushKey (&CWindow, K_ENTER, 0x0000);                                            /* Put <RETURN> in the keyboard queue, so the */
                                                                                    /* address box will be started up immediately */
    while (!Accepted)
    {
      Finished = FALSE;
      Cancelled = TRUE;
      while (!Finished)
      {
        ChangeOKButton = FALSE;
        switch (HandleWindow (&CWindow, &ButtonP))
        {
          case WINH_CLOSE   : Finished = TRUE; break;
          case WINH_RNOHAND : switch (ButtonP->ReactNum)
                              {
                                case 5 : RAddress = (word)atoi (AAddress);                                             /* (Fetch) */
                                         RBank = AutoBank (RAddress, ABank);
                                         if (GetMemoryImage (FullPath, RBank, &MemoryImage, &ImageSize))
                                         {
                                           sprintf (AOrg, "%u", *(MemoryImage + RAddress - 16384));
                                           UpdateButtonText (&CWindow, &BOrg, BF_FITTXT);
                                           ChangeOKButton = TRUE;
                                         }
                                         else
                                           SoundErrorBeeper (BEEP_SHORT);
                                         break;
                                case 6 : Finished = TRUE; break;                                                      /* (Cancel) */
                                case 7 : Finished = TRUE; Cancelled = FALSE; break;                                       /* (Ok) */
                                case 8 : if (strcmp (ASet, "ASK"))                                                   /* Set cycle */
                                         {
                                           strcpy (ASet, "ASK");
                                           UpdateButtonText (&CWindow, &BSet, BF_FITTXT);
                                           BSet.Flags |= BF_GHOSTED;
                                         }
                                         else
                                         {
                                           strcpy (ASet, "0");
                                           BSet.Flags &= ~BF_GHOSTED;
                                           UpdateButtonText (&CWindow, &BSet, BF_FITTXT);
                                         }
                                         ChangeOKButton = TRUE;
                                         break;
                              }
                              break;
          case WINH_EDITEND : if (ButtonP->ReactNum == 2)                                                            /* (Address) */
                              {
                                if (Snap128K)                                                                      /* 128K Game ? */
                                {
                                  if (ABank[0] == '\0')                                                     /* Bank not set yet ? */
                                  {
                                    RAddress = (word)atoi (AAddress);
                                    RBank = AutoBank (RAddress, ABank);
                                    sprintf (ABank, "%u", RBank);
                                    UpdateButtonText (&CWindow, &BBank, BF_FITTXT);
                                  }
                                }
                                if (IsSnapshot (FileType))
                                {
                                  if (AAddress[0] != '\0' && atol (AAddress) >= 16384 && atol (AAddress) < 65536)
                                  {
                                    BFetch.Flags &= ~BF_GHOSTED;
                                    RAddress = (word)atoi (AAddress);                               /* Try fetching the Org value */
                                    RBank = AutoBank (RAddress, ABank);
                                    if (GetMemoryImage (FullPath, RBank, &MemoryImage, &ImageSize))
                                    {
                                      sprintf (AOrg, "%u", *(MemoryImage + RAddress - 16384));
                                      UpdateButtonText (&CWindow, &BOrg, BF_FITTXT);
                                    }
                                  }
                                  else
                                    BFetch.Flags |= BF_GHOSTED;
                                  UpdateButtonText (&CWindow, &BFetch, 0);
                                }
                              }
                              ChangeOKButton = TRUE;
                              break;
        }
        if (ChangeOKButton)
        {
          if (ABank[0] != '\0' && AAddress[0] != '\0' && ASet[0] != '\0' && AOrg[0] != '\0')
            BOk.Flags &= ~BF_GHOSTED;
          else
            BOk.Flags |= BF_GHOSTED;
          UpdateButtonText (&CWindow, &BOk, 0);
        }
        MouseStatus ();
        if (GlobalKey == K_ESC)
          Finished = TRUE;
      }
      if (Cancelled)
        Accepted = TRUE;
      else
        if ((ABank[0] != '-' || atoi (ABank) < 8) &&
            atol (AAddress) >= 16384 && atol (AAddress) < 65536 &&
            (!strcmp (ASet, "ASK") || atol (ASet) < 256) &&
            atol (AOrg) < 256)
          Accepted = TRUE;
      if (!Accepted)
        ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - One or more values are out of range");
    }
    DestroyWindow (&CWindow);
    if (!Cancelled)
    {
      if ((NEntry = (struct PokeEntry_s *)malloc (sizeof (struct PokeEntry_s))) == NULL)
        FatalError (1, _ErrOutOfMemory, sizeof (struct PokeEntry_s));
      NEntry->Next = NULL;
      if (Snap128K)
        NEntry->Bank = atoi (ABank);
      else
        NEntry->Bank = 8;
      NEntry->Address = atoi (AAddress);
      if (strcmp (ASet, "ASK"))
        NEntry->Set = atoi (ASet);
      else
        NEntry->Set = 256;
      NEntry->Org = atoi (AOrg);
      GetComboEntry (&PokesWindow, &NCEntry, SelectedPoke);
      OPokes = (struct GamePokes_s *)NCEntry.Identifier;
      if (OPokes->Pokes == NULL)                                                                                  /* First poke ? */
      {
        OPokes->Pokes = NEntry;
        NEntry->Previous = NULL;
      }
      else
      {
        GetComboEntry (&EntriesWindow, &NCEntry, EntriesWindow.NumEntries);
        OEntry = (struct PokeEntry_s *)NCEntry.Identifier;
        OEntry->Next = NEntry;
        NEntry->Previous = OEntry;
      }
      NCEntry.Identifier = (dword)NEntry;
      NCEntry.Selected = 0x00;
      if (NEntry->Bank < 8)
        sprintf (ABank, "%u", NEntry->Bank);
      else
        strcpy (ABank, "-");
      if (NEntry->Set < 256)
        sprintf (ASet, "%3u", NEntry->Set);
      sprintf (NCEntry.Text, "%s  %5u,%s (%3u)", ABank, NEntry->Address, ASet, NEntry->Org);
      if (!InsertComboEntry (&EntriesWindow, &NCEntry, NULL, -1))
        FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
      SelectedEntry = EntriesWindow.NumEntries;
      SelectEntry (&EntriesWindow, SelectedEntry);
      return (WINH_EDITEND);                                                             /* Signal: (un)ghost appropriate buttons */
    }
  }
  return (WINH_NONE);
}

static int HandleDelEntry (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Remove' button has been pressed (or released).                                                                    */
/* Post  : The current poke entry has been removed.                                                                               */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *OPokes;
  struct PokeEntry_s *OEntry;
  struct ComboEntry   NEntry;
  
  Window = Window;
  if (!(Button->Flags & BF_PRESSED) && EntriesWindow.NumEntries >= 0)
  {
    GetComboEntry (&EntriesWindow, &NEntry, SelectedEntry);
    if (DBConfig.ConfirmDelete)                                                               /* Confirmation needed: handle it ? */
      if (QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Delete",
                             "Are you sure you want to delete the poke\n\"%s\"?", NEntry.Text))
        return (WINH_NONE);
    OEntry = (struct PokeEntry_s *)NEntry.Identifier;
    GetComboEntry (&PokesWindow, &NEntry, SelectedPoke);
    OPokes = (struct GamePokes_s *)NEntry.Identifier;
    if (SelectedEntry == 0)                                                                               /* Removing first one ? */
    {
      OPokes->Pokes = OEntry->Next;
      OEntry->Next->Previous = NULL;
    }
    else
    {
      OEntry->Previous->Next = OEntry->Next;                                                                            /* Unlink */
      OEntry->Next->Previous = OEntry->Previous;
    }
    free (OEntry);
    DeleteComboEntry (&EntriesWindow, SelectedEntry);
    if (SelectedEntry > EntriesWindow.NumEntries)
      SelectedEntry --;
    SelectEntry (&EntriesWindow, SelectedEntry);
    return (WINH_EDITEND);                                                               /* Signal: (un)ghost appropriate buttons */
  }
  return (WINH_NONE);
}

static bool HandleUpdateEntry (void)

/**********************************************************************************************************************************/
/* Pre   : A poke entry has been double-clicked.                                                                                  */
/* Post  : The current poke entry has been updated.                                                                               */
/* Import: AutoBank.                                                                                                              */
/**********************************************************************************************************************************/

{
  struct PokeEntry_s *OEntry;
  char               *CEntry;
  struct ComboEntry   NEntry;
  char                OldCEntry[30];
  char                ABank[2];
  char                AAddress[6];
  char                ASet[4];
  char                AOrg[4];
  struct WindowInfo   CWindow  = {190, 220, 160, 160, 160, 160, 160, 160, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                  WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "Update Poke"};
  struct ButtonInfo   BBank    = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 40, 1, DBLACK, DWHITE, NULL,
                                  1, NULL, NULL};
  struct ButtonInfo   BAddress = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 60, 5, DBLACK, DWHITE, NULL,
                                  2, NULL, NULL};
  struct ButtonInfo   BSet     = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 80, 3, DBLACK, DWHITE, NULL,
                                  3, NULL, NULL};
  struct ButtonInfo   BAskSet  = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 97, 80, 6, DBLACK, DWHITE, NULL,
                                  8, "\007", NULL};
  struct ButtonInfo   BOrg     = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 100, 3, DBLACK, DWHITE, NULL,
                                  4, NULL, NULL};
  struct ButtonInfo   BFetch   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 100, 100, 6, DBLACK, DWHITE, NULL,
                                  5, "@Fetch", NULL};
  struct ButtonInfo   BCancel  = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 10, 135, 8, DBLACK, DWHITE, NULL,
                                  6, "@Cancel", NULL};
  struct ButtonInfo   BOk      = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 96, 135, 8, DBLACK, DWHITE, NULL,
                                  7, "@Ok", NULL};
  struct ButtonInfo  *ButtonP;
  bool                Finished;
  bool                Cancelled;
  bool                Accepted = FALSE;
  byte               *MemoryImage;
  word                ImageSize;
  byte                RBank;
  word                RAddress;
  bool                ChangeOKButton;

  BBank.Contents = ABank;
  BAddress.Contents = AAddress;
  BSet.Contents = ASet;
  BOrg.Contents = AOrg;
  GetComboEntry (&EntriesWindow, &NEntry, SelectedEntry);
  OEntry = (struct PokeEntry_s *)NEntry.Identifier;
  CEntry = NEntry.Text;
  strcpy (OldCEntry, CEntry);
  if (Snap128K)
    sprintf (ABank, "%u", OEntry->Bank);
  else
  {
    sprintf (ABank, "-");
    BBank.Flags |= BF_GHOSTED;
  }
  sprintf (AAddress, "%u", OEntry->Address);
  if (OEntry->Set < 256)
    sprintf (ASet, "%u", OEntry->Set);
  else
    strcpy (ASet, "ASK");
  sprintf (AOrg, "%u", OEntry->Org);
  if (!IsSnapshot (FileType))                                                                                 /* Not a snapshot ? */
    BFetch.Flags |= BF_GHOSTED;                                                               /* Then we cannot retrieve anything */
  AddButton (&CWindow, &BCancel, FALSE);
  AddButton (&CWindow, &BOk, FALSE);
  AddButton (&CWindow, &BBank, FALSE);
  AddButton (&CWindow, &BAddress, FALSE);
  AddButton (&CWindow, &BSet, FALSE);
  AddButton (&CWindow, &BAskSet, FALSE);
  AddButton (&CWindow, &BOrg, FALSE);
  AddButton (&CWindow, &BFetch, FALSE);
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
  Wprintf (&CWindow, 1, TEXT_CENTER, "Update poke parameters");
  Wprintf (&CWindow,  4, 2, "{+1Bank}N");
  Wprintf (&CWindow,  6, 2, "{+1Address}N");
  Wprintf (&CWindow,  8, 2, "{+1Value}N");
  Wprintf (&CWindow, 10, 2, "{+1Original}N");
  if (OEntry->Set >= 256)
    BSet.Flags |= BF_GHOSTED;
  while (!Accepted)
  {
    Finished = FALSE;
    Cancelled = TRUE;
    while (!Finished)
    {
      ChangeOKButton = FALSE;
      switch (HandleWindow (&CWindow, &ButtonP))
      {
        case WINH_CLOSE   : Finished = TRUE; break;
        case WINH_RNOHAND : switch (ButtonP->ReactNum)
                            {
                              case 5 : RAddress = (word)atoi (AAddress);                                               /* (Fetch) */
                                       RBank = AutoBank (RAddress, ABank);
                                       if (GetMemoryImage (FullPath, RBank, &MemoryImage, &ImageSize))
                                       {
                                         sprintf (AOrg, "%u", *(MemoryImage + RAddress - 16384));
                                         UpdateButtonText (&CWindow, &BOrg, BF_FITTXT);
                                         ChangeOKButton = TRUE;
                                       }
                                       else
                                         SoundErrorBeeper (BEEP_SHORT);
                                       break;
                              case 6 : Finished = TRUE; break;                                                        /* (Cancel) */
                              case 7 : Finished = TRUE; Cancelled = FALSE; break;                                         /* (Ok) */
                              case 8 : if (strcmp (ASet, "ASK"))                                                     /* Set cycle */
                                       {
                                         strcpy (ASet, "ASK");
                                         UpdateButtonText (&CWindow, &BSet, BF_FITTXT);
                                         BSet.Flags |= BF_GHOSTED;
                                       }
                                       else
                                       {
                                         strcpy (ASet, "0");
                                         BSet.Flags &= ~BF_GHOSTED;
                                         UpdateButtonText (&CWindow, &BSet, BF_FITTXT);
                                       }
                                       ChangeOKButton = TRUE;
                                       break;
                            }
                            break;
        case WINH_EDITEND : if (ButtonP->ReactNum == 2)                                                              /* (Address) */
                            {
                              if (Snap128K)                                                                        /* 128K Game ? */
                              {
                                if (ABank[0] == '\0')
                                {
                                  RAddress = (word)atoi (AAddress);
                                  RBank = AutoBank (RAddress, ABank);
                                  sprintf (ABank, "%u", RBank);
                                  UpdateButtonText (&CWindow, &BBank, BF_FITTXT);
                                }
                              }
                              if (IsSnapshot (FileType))
                              {
                                if (AAddress[0] != '\0' && atol (AAddress) >= 16384 && atol (AAddress) < 65536)
                                {
                                  BFetch.Flags &= ~BF_GHOSTED;
                                  RAddress = (word)atoi (AAddress);                                 /* Try fetching the Org value */
                                  RBank = AutoBank (RAddress, ABank);
                                  if (GetMemoryImage (FullPath, RBank, &MemoryImage, &ImageSize))
                                  {
                                    sprintf (AOrg, "%u", *(MemoryImage + RAddress - 16384));
                                    UpdateButtonText (&CWindow, &BOrg, BF_FITTXT);
                                  }
                                }
                                else
                                  BFetch.Flags |= BF_GHOSTED;
                                UpdateButtonText (&CWindow, &BFetch, 0);
                              }
                            }
                            ChangeOKButton = TRUE;
                            break;
      }
      if (ChangeOKButton)
      {
        if (ABank[0] != '\0' && AAddress[0] != '\0' && ASet[0] != '\0' && AOrg[0] != '\0')
          BOk.Flags &= ~BF_GHOSTED;
        else
          BOk.Flags |= BF_GHOSTED;
        UpdateButtonText (&CWindow, &BOk, 0);
      }
      MouseStatus ();
      if (GlobalKey == K_ESC)
        Finished = TRUE;
    }
    if (Cancelled)
      Accepted = TRUE;
    else
      if ((ABank[0] != '-' || atoi (ABank) < 8) &&
          atol (AAddress) >= 16384 && atol (AAddress) < 65536 &&
          (!strcmp (ASet, "ASK") || atol (ASet) < 256) &&
          atol (AOrg) < 256)
        Accepted = TRUE;
    if (!Accepted)
      ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - One or more values are out of range");
  }
  DestroyWindow (&CWindow);
  if (!Cancelled)
  {
    if (Snap128K)
      OEntry->Bank = atoi (ABank);
    else
      OEntry->Bank = 8;
    OEntry->Address = atoi (AAddress);
    if (strcmp (ASet, "ASK"))
      OEntry->Set = atoi (ASet);
    else
      OEntry->Set = 256;
    OEntry->Org = atoi (AOrg);
    if (OEntry->Bank < 8)
      sprintf (ABank, "%u", OEntry->Bank);
    else
      strcpy (ABank, "-");
    if (OEntry->Set < 256)
      sprintf (ASet, "%3u", OEntry->Set);
    sprintf (CEntry, "%s  %5u,%s (%3u)", ABank, OEntry->Address, ASet, OEntry->Org);
    SelectEntry (&EntriesWindow, SelectedEntry);
    return ((bool)(strcmp (OldCEntry, CEntry) == 0) ? FALSE : TRUE);
  }
  return (FALSE);
}

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE EXPORTED POKE LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

byte AutoBank (word Address, char *Bank)

/**********************************************************************************************************************************/
/* Pre   : `Address' is the address to resolve, `Bank' is the current bank setting.                                               */
/* Post  : The correct (default) bank belonging to this address has been returned, depending on the snapshot type.                */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Snap128K = FALSE;                                                                                        /* Assume 48K snapshot */
  if (FileType != FT_Z80 && FileType != FT_SLT && CurrentEntry.Memory == 48)                      /* Only Z80 knows about banking */
    return (0);
  if (Bank[0] != '\0' && Bank[0] != '-')                                                           /* Bank has been set already ? */
  {
    Snap128K = TRUE;                                                                                /* (Only TRUE for 128K snaps) */
    return (atoi (Bank));
  }
  if (FileType == FT_Z80 || FileType == FT_SLT)
  {
    if (Z80Version == 145)                                                                              /* No banks, single image */
      return (0);
    if (Z80Version == 201)
      Snap128K = (Z80201SnapshotHeader.HWMode >= 3 ? TRUE : FALSE);
    else                                                                                                         /* (version 300) */
      Snap128K = (Z80201SnapshotHeader.HWMode >= 4 ? TRUE : FALSE);
  }
  else
    Snap128K = TRUE;                                                                          /* (Or we won't have come this far) */
  if (Snap128K)
    if (Address < (word)32768)
      return (5);
    else if (Address < (word)49152)
      return (2);
    else
      return (0);
  else                                                                                 /* Z80 v2.01 and up use 'banking' as well! */
    if (Address < (word)32768)
      return (5);
    else if (Address < (word)49152)
      return (1);
    else
      return (2);
}

bool TestPokesFileExists (bool Create)

/**********************************************************************************************************************************/
/* Pre   : `Create' is TRUE if the pokes directory should be created if not found.                                                */
/* Post  : A test has been made whether the pokes file (and directory) exists. All related variables (PokesDir, PokesPath,        */
/*         PokePathExists and PokesFound) have been initialized.                                                                  */
/*         If the pokes directory did not exist yet and `Create' was TRUE, it has been created if the user allowed it.            */
/*         The return value is TRUE if the pokes file has been found.                                                             */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  int      Fh;
  unsigned Dummy;
  char     PathBuffer[MAXDIR];

  PokesFound = FALSE;
  PokesDirExists = FALSE;
  if (FileType == FT_SLT && SLTMap.PokesIncluded)                                         /* SLT files have pokes as an extension */
  {
    PokesFound = TRUE;
    PokesDirExists = TRUE;
    MakeFullPath (PokesPath, GamePath, CurrentEntry.PCName);
    return (PokesFound);
  }
  if (DBConfig.AllFilesInOneDir)
    strcpy (PokesDir, GamePath);
  else
    MakeFullPath (PokesDir, GamePath, POKESDIR);
  if (!RequestFloppy (PokesDir[0], CurrentEntry.DiskId))
    return (FALSE);
  HWEAbort = TRUE;
  HWEOccured = FALSE;
  _dos_setdrive (ToNumber (PokesDir[0]), &Dummy);
  getcwd (PathBuffer, MAXDIR);
  PokesDirExists = !(chdir (PokesDir + 2));
  if (HWEOccured)
    PokesDirExists = FALSE;
  HWEAbort = FALSE;
  strcpy (PokesPath, PokesDir);
  strcat (PokesPath, "\\");
  strcat (PokesPath, CurrentEntry.PCName);
  strcpy (PokesPath + strlen (PokesPath) - 3, POKESEXT);
  if (PokesDirExists)
  {
    if ((Fh = open (PokesPath, O_BINARY | O_RDONLY)) != -1)
    {
      close (Fh);
      PokesFound = TRUE;
    }
  }
  else if (Create)
  {
    if (!QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm create directory",
        "The POKES directory\n    %s\ndoes not exist yet.\nDo you want to create it now ?", PokesDir))
      if (mkdir (PokesDir))
        ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Could not create directory:\n%s", strerror (errno));
      else
        PokesDirExists = TRUE;
  }
  chdir (PathBuffer);
  _dos_setdrive (CurDrv, &Dummy);
  return (PokesFound);
}

void EditGamePokes (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The pokes file for the currently selected games has been edited and written back.                                      */
/* Import: ReadGamePokes, WriteGamePokes, SetupPokesWindow, SetupEntriesWindow, DestoyPokes.                                      */
/**********************************************************************************************************************************/

{
  char               WHeader[D_NAME + 30];
  struct WindowInfo  CWindow = {120, 155, 382, 208, 382, 208, 382, 208, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, NULL};
  struct ButtonInfo  BAddP   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 18, 141, 8, DBLACK, DWHITE, NULL,
                                1, "@Add", HandleAddPoke};
  struct ButtonInfo  BDelP   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 78, 141, 8, DBLACK, DWHITE, NULL,
                                2, "@Delete", HandleDelPoke};
  struct ButtonInfo  BAddE   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 246, 141, 8, DBLACK, DWHITE, NULL,
                                3, "@Insert", HandleAddEntry};
  struct ButtonInfo  BDelE   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 306, 141, 8, DBLACK, DWHITE, NULL,
                                4, "Re@move", HandleDelEntry};
  struct ButtonInfo  BCancel = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 70, 180, 8, DBLACK, DWHITE, NULL,
                                5, "@Cancel", NULL};
  struct ButtonInfo  BOk     = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 260, 180, 8, DBLACK, DWHITE, NULL,
                                6, "@Save", NULL};
  struct ButtonInfo *ButtonP;
  comboent           SelectedLine;
  char               MessageBuffer[20];
  bool               Finished     = FALSE;
  bool               Cancelled    = TRUE;
  bool               PokesChanged = FALSE;
  bool               PokesExisted = TRUE;

  CWindow.Header = WHeader;
  if (!ReadGamePokes ())
    if (PokesFound)                                                                                        /* Read/memory error ? */
    {
      if (ErrorEntry > 0)
        sprintf (MessageBuffer, ", entry %d", ErrorEntry);
      else
        MessageBuffer[0] = '\0';
      ErrorRequester (-1, -1, NULL, NULL, NULL, "Error loading poke file\n%s\nwhile reading poke %d%s",
                      PokesPath, ErrorPoke, MessageBuffer);
      DestroyPokes ();
      return;
    }
  PokesExisted = PokesFound;                                                                            /* Remember initial state */
  sprintf (WHeader, "Edit Game Pokes: [%s]", CurrentEntry.Name);
  AddButton (&CWindow, &BCancel, FALSE);
  AddButton (&CWindow, &BOk, TRUE);
  AddButton (&CWindow, &BAddP, FALSE);
  AddButton (&CWindow, &BDelP, FALSE);
  AddButton (&CWindow, &BAddE, FALSE);
  AddButton (&CWindow, &BDelE, FALSE);
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
  WDrawShadowBox (&CWindow, 1, 0, 16, 36, "Description");
  WDrawShadowBox (&CWindow, 1, 38, 16, 62, "Pokes");
  MakeComboWindow (&PokesWindow, &CWindow, COMT_SELECT, COMF_NOBUFFER | COMF_VSLIDER | COMF_KEYSELECT | COMF_SYNCSLIDER,
                   CWindow.StartX + 16, CWindow.StartY + 30, 15, 64, MAXPOKNAME + 5, MAXPOKNAME, 10, NULL, FALSE,
                   0, NULL, DBLACK, DWHITE, 0, 0);
  MakeComboWindow (&EntriesWindow, &CWindow, COMT_SELECT, COMF_NOBUFFER | COMF_VSLIDER | COMF_SYNCSLIDER,
                   CWindow.StartX + 244, CWindow.StartY + 30, 15, 64, 19, 18, 10, NULL, FALSE,
                   0, NULL, DBLACK, DWHITE, 0, 0);
  SelectedPoke = 0;
  SelectedEntry = 0;
  SetupPokesWindow ();
  SetupEntriesWindow (SelectedPoke);
  SelectEntry (&PokesWindow, SelectedPoke);
  SelectEntry (&EntriesWindow, SelectedEntry);
  if (PokesWindow.NumEntries == -1)                                                                          /* No pokes at all ? */
  {
    BDelP.Flags |= BF_GHOSTED;
    BAddE.Flags |= BF_GHOSTED;
    BOk.Flags |= BF_GHOSTED;
    UpdateButtonText (&CWindow, &BDelP, 0);
    UpdateButtonText (&CWindow, &BAddE, 0);
    UpdateButtonText (&CWindow, &BOk, 0);
  }
  if (EntriesWindow.NumEntries == -1)
  {
    BDelE.Flags |= BF_GHOSTED;
    UpdateButtonText (&CWindow, &BDelE, 0);
  }
  if (Auto48Convert || Auto128Convert)                                                             /* Snapshot has been changed ? */
  {
    ErrorRequester (-1, -1, NULL, WBPics[2], "Auto Conversion Succeeded",
                    "Your snapshot seems to be converted from %s to %s.\n"
                    "Your pokes have been adjusted accordingly.\n"
                    "These changes are written back when you press the 'Save' button",
                    Auto48Convert ? "128K" : "48K", Auto48Convert ? "48K" : "128K");
    PokesChanged = TRUE;                                                                                  /* Signal: save on exit */
  }
  while (!Finished)
  {
    switch (HandleWindow (&CWindow, &ButtonP))
    {
      case WINH_CLOSE     : Finished = TRUE; break;
      case WINH_RNOHAND   : switch (ButtonP->ReactNum)
                            {
                              case 5 : Finished = TRUE; break;                                                        /* (Cancel) */
                              case 6 : Finished = TRUE; Cancelled = FALSE; break;                                         /* (Ok) */
                            }
                            break;
      case WINH_EDITEND   : PokesChanged = TRUE;                                                            /* (Add/Del handlers) */
                            if (PokesWindow.NumEntries == -1)                                                /* No pokes at all ? */
                            {
                              BDelP.Flags |= BF_GHOSTED;
                              BAddE.Flags |= BF_GHOSTED;
                              if (!PokesExisted)                                           /* Allow full delete if a file existed */
                                BOk.Flags |= BF_GHOSTED;
                            }
                            else
                            {
                              BDelP.Flags &= ~BF_GHOSTED;
                              BAddE.Flags &= ~BF_GHOSTED;
                              BOk.Flags &= ~BF_GHOSTED;
                            }
                            if (EntriesWindow.NumEntries == -1)
                              BDelE.Flags |= BF_GHOSTED;
                            else
                              BDelE.Flags &= ~BF_GHOSTED;
                            UpdateButtonText (&CWindow, &BDelP, 0);
                            UpdateButtonText (&CWindow, &BAddE, 0);
                            UpdateButtonText (&CWindow, &BOk, 0);
                            UpdateButtonText (&CWindow, &BDelE, 0);
                            break;
    }
    switch (HandleComboWindow (&PokesWindow, &SelectedLine))
    {
      case WINH_LINEFIRST : SelectedPoke = SelectedLine;
                            SelectedEntry = 0;
                            SetupEntriesWindow (SelectedPoke);
                            SelectEntry (&EntriesWindow, SelectedEntry);
                            break;
      case WINH_LINESEL   : if (HandleUpdatePoke ())
                              PokesChanged = TRUE;
                            break;
    }
    if (HandleComboWindow (&EntriesWindow, &SelectedEntry) == WINH_LINESEL)
      if (HandleUpdateEntry ())
        PokesChanged = TRUE;
    MouseStatus ();
    if (GlobalKey == K_ESC)
      Finished = TRUE;
  }
  if (!Cancelled && PokesChanged)
    WriteGamePokes ();
  DestroyComboWindow (&PokesWindow, 0);
  DestroyComboWindow (&EntriesWindow, 0);
  DestroyWindow (&CWindow);
  DestroyPokes ();
}

bool InsertGamePokes (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The user has been presented a list of all known poke descriptions for the current game (as entered with EdPoke).       */
/*         After selecting the requested poke settings and hitting the `Play' button, a patched version of the game has been      */
/*         created in the TMP directory. If any error occured or the user aborted, FALSE is returned and any partly patched game  */
/*         has been removed.                                                                                                      */
/* Import: CleanupPatch.                                                                                                          */
/**********************************************************************************************************************************/

{
  struct Z80145SnapshotHeader_s New145Header;
  struct BlockInfo_s
  {
    word  Length;
    byte  Bank;
  }                   BlockInfo;
  struct PokeEntry_s *OPEntry   = NULL;
  struct PokeEntry_s *NPEntry;
  struct PokeEntry_s *OEntry;
  struct GamePokes_s *OPokes;
  struct WindowInfo   CWindow = {195, 125, 226, 180, 226, 180, 226, 180, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                 WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "Play Poked Game"};
  struct ButtonInfo   BCancel = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 45, 152, 8, DBLACK, DWHITE, NULL, 5, "@Cancel", NULL};
  struct ButtonInfo   BOk     = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 129, 152, 8, DBLACK, DWHITE, NULL, 6, "@Play", NULL};
  struct ButtonInfo  *ButtonP;
  struct ComboEntry   NEntry;
  comboent            SelectedLine;
  char                MessageBuffer[20];
  bool                Finished     = FALSE;
  bool                Cancelled    = TRUE;
  bool                Accepted;
  dword               FileLength;
  dword               Offset;
  word                NRead;
  word                NewSize;
  byte                SLTMarker[6] = { 0, 0, 0, 'S', 'L', 'T' };

  if (!ReadGamePokes ())
  {
    if (PokesFound)                                                                                        /* Read/memory error ? */
    {
      if (ErrorEntry > 0)
        sprintf (MessageBuffer, ", entry %d", ErrorEntry);
      else
        MessageBuffer[0] = '\0';
      ErrorRequester (-1, -1, NULL, NULL, NULL, "Error loading poke file\n%s\nwhile reading poke %d%s",
                      PokesPath, ErrorPoke, MessageBuffer);
      DestroyPokes ();
      return (FALSE);
    }
  }
  else
  {
    AddButton (&CWindow, &BCancel, FALSE);
    AddButton (&CWindow, &BOk, TRUE);
    DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
    Wprintf (&CWindow, 1, TEXT_CENTER, "Select the pokes to be used");
    MakeComboWindow (&PokesWindow, &CWindow, COMT_SELECT, COMF_NOBUFFER | COMF_VSLIDER | COMF_KEYSELECT | COMF_SYNCSLIDER,
                     CWindow.StartX + 16, CWindow.StartY + 30, 15, 64, MAXPOKNAME + 5, MAXPOKNAME, 10, NULL, FALSE,
                     0, NULL, DBLACK, DWHITE, 0x02, 0);
    SelectedPoke = 0;
    SetupPokesWindow ();
    SelectEntry (&PokesWindow, SelectedPoke);
    while (!Finished)
    {
      switch (HandleWindow (&CWindow, &ButtonP))
      {
        case WINH_CLOSE     : Finished = TRUE; break;
        case WINH_RNOHAND   : switch (ButtonP->ReactNum)
                              {
                                case 5 : Finished = TRUE; break;                                                      /* (Cancel) */
                                case 6 : Finished = TRUE; Cancelled = FALSE; break;                                       /* (Ok) */
                              }
                              break;
      }
      HandleComboWindow (&PokesWindow, &SelectedLine);
      MouseStatus ();
      if (GlobalKey == K_ESC)
        Finished = TRUE;
    }

    if (Cancelled)
    {
      DestroyComboWindow (&PokesWindow, 0);
      DestroyWindow (&CWindow);
      DestroyPokes ();
      return (FALSE);
    }
    else
    {
      MakeFullPath (PokedGamePath, TmpDirectory, CurrentEntry.PCName);
      OPokes = GamePokes;
      SelectedLine = 0;                                                                           /* (Index into the combo table) */
      while (OPokes != NULL)                                                        /* Setup a single array, containing all pokes */
      {
        OEntry = OPokes->Pokes;
        GetComboEntry (&PokesWindow, &NEntry, SelectedLine);
        Finished = NEntry.Selected == 0x02 ? TRUE : FALSE;
        while (OEntry != NULL && OEntry->Address >= 16384)                                  /* All (valid) pokes in a description */
        {
          if ((NPEntry = (struct PokeEntry_s *)malloc (sizeof (struct PokeEntry_s))) == NULL)
            FatalError (1, _ErrOutOfMemory, sizeof (struct PokeEntry_s));
          NPEntry->Next    = NULL;
          if (OEntry->Bank >= 8)                                                                   /* Resolve Z80 v2.01+ bankings */
            NPEntry->Bank = AutoBank (OEntry->Address, "");
          else
            NPEntry->Bank = OEntry->Bank;
          NPEntry->Address = OEntry->Address;
          if (Finished)                                                                                  /* Use the `Set' value ? */
          {
            if (OEntry->Set < 256)                                                                               /* Given value ? */
              NPEntry->Set = OEntry->Set;
            else
            {
              Accepted = FALSE;
              MessageBuffer[0] = '\0';
              while (!Accepted)
              {
                if (InputRequester (-1, -1, NULL, "@Use Org", MessageBuffer, 4, FALSE, FALSE, "Set Poke Value",
                                    "{CGive the value for\n\"%s\" (%u,%u)}C", NEntry.Text,
                                    OEntry->Address, OEntry->Org))
                  if (MessageBuffer[0] != '\0' && atol (MessageBuffer) < 256)
                  {
                    Accepted = TRUE;
                    NPEntry->Set = atoi (MessageBuffer);
                  }
                  else
                    ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - The value is out of range\n        Use values 0 through 255");
                else                                                                                               /* (Cancelled) */
                {
                  Accepted = TRUE;
                  NPEntry->Set = OEntry->Org;                                                /* Use original value (disable poke) */
                }
              }
            }
          }
          else
            NPEntry->Set = OEntry->Org;                                                                 /* (No, disable the poke) */
          NPEntry->Set = Finished ? OEntry->Set : OEntry->Org;                                                /* Turn on or off ? */
          if (OPEntry == NULL)
          {
            PokeSetup = NPEntry;
            NPEntry->Previous = NULL;
          }
          else
          {
            OPEntry->Next = NPEntry;
            NPEntry->Previous = OPEntry;
          }
          OPEntry = NPEntry;
          OEntry = OEntry->Next;
        }
        SelectedLine ++;
        OPokes = OPokes->Next;
      }
      DestroyPokes ();
      DestroyComboWindow (&PokesWindow, 0);
      DestroyWindow (&CWindow);
      if (PokeSetup == NULL)
        return (FALSE);
      OldMouseShape = CurrentMouseShape;
      SetMouseShape (DefaultWaitingMouseShape);
      DrawWindow (&MessageWindow, NULL, 0, FALSE);
      Wprintf (&MessageWindow, 1, TEXT_CENTER, "Building patched game file");
      Wprintf (&MessageWindow, 3, TEXT_CENTER, CurrentEntry.PCName);
      if ((FhIn = open (FullPath, O_BINARY | O_RDONLY)) == -1)
      { ErrorRequester (-1, -1, NULL, NULL, NULL, "Cannot open game file\n%s\n- %s", FullPath, strerror (errno));
        CleanupPatch (); return (FALSE); }
      if ((FhOut = open (PokedGamePath, O_BINARY | O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1)
      { ErrorRequester (-1, -1, NULL, NULL, NULL, "Cannot create patched game file\n%s\n- %s", PokedGamePath, strerror (errno));
        CleanupPatch (); return (FALSE); }
      FileLength = filelength (FhIn);
      if (FileType == FT_Z80 || FileType == FT_SLT)
      {
        if (read (FhIn, &New145Header, sizeof (struct Z80145SnapshotHeader_s)) == -1)
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
          CleanupPatch (); return (FALSE); }
        Offset = sizeof (struct Z80145SnapshotHeader_s);
        New145Header.Stat1 |= 0x20;                                                            /* Signal: block is crunched (now) */
        if (write (FhOut, &New145Header, sizeof (struct Z80145SnapshotHeader_s)) == -1)
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, PokedGamePath, strerror (errno));
          CleanupPatch (); return (FALSE); }
        if (Z80Version == 145)
        {
          if ((NRead = read (FhIn, _BufferCrunched, (word)(FileLength - Offset))) == -1)                         /* Read old file */
          { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
            CleanupPatch (); return (FALSE); }
          if (Z80145SnapshotHeader.Stat1 & 0x20)                                                       /* Old block is crunched ? */
          {
            if (!DecrunchZ80Block (_BufferCrunched, NRead, _BufferDecrunched, (word)49152))
            { ErrorRequester (-1, -1, "@Close", NULL, NULL, _InvalidZ80File, FullPath);
              CleanupPatch (); return (FALSE); }
          }
          else if (FileLength - Offset != (word)49152)
          { ErrorRequester (-1, -1, "@Close", NULL, NULL, "Wrong filesize of file %s", FullPath, strerror (errno));
            CleanupPatch (); return (FALSE); }
          else
            memcpy (_BufferDecrunched, _BufferCrunched, (word)49152);
          OPEntry = PokeSetup;
          while (OPEntry != NULL)                                                                              /* Apply the pokes */
          {
            *(_BufferDecrunched + (word)OPEntry->Address - 16384) = OPEntry->Set;
            OPEntry = OPEntry->Next;
          }
          DestroyEntries (PokeSetup);
          PokeSetup = NULL;
          CrunchZ80Block (_BufferDecrunched, (word)49152, _BufferCrunched, &NewSize);                       /* Write updated file */
          if (write (FhOut, _BufferCrunched, NewSize) != NewSize)
          { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, PokedGamePath, strerror (errno));
            CleanupPatch (); return (FALSE); }
        }
        else
        {
          if (write (FhOut, &Z80201SnapshotHeader, sizeof (struct Z80201SnapshotHeader_s)) == -1)
          { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, PokedGamePath, strerror (errno));
            CleanupPatch (); return (FALSE); }
          Offset += sizeof (struct Z80201SnapshotHeader_s);
          if (Z80Version == 300)
          {
            if (write (FhOut, &Z80300SnapshotHeader, sizeof (struct Z80300SnapshotHeader_s)) == -1)
            { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, PokedGamePath, strerror (errno));
              CleanupPatch (); return (FALSE); }
            Offset += sizeof (struct Z80300SnapshotHeader_s);
          }
          if (lseek (FhIn, Offset, SEEK_SET) == -1)
          { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno));
            CleanupPatch (); return (FALSE); }
          if ((NRead = read (FhIn, &BlockInfo, 3)) == -1)
          { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
            CleanupPatch (); return (FALSE); }
          while (NRead == 3)                                                                          /* Handle all memory blocks */
          {
            if (BlockInfo.Length != 0x0000)                                                                 /* Not the SLT mark ? */
            {
              if ((NRead = read (FhIn, _BufferCrunched, BlockInfo.Length)) != BlockInfo.Length)
              { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
                CleanupPatch (); return (FALSE); }
              if (!DecrunchZ80Block (_BufferCrunched, NRead, _BufferDecrunched, 16384))
              { ErrorRequester (-1, -1, "@Close", NULL, NULL, _InvalidZ80File, FullPath);
                CleanupPatch (); return (FALSE); }
              OPEntry = PokeSetup;
              while (OPEntry != NULL)                                                                          /* Apply the pokes */
              {
                if (OPEntry->Bank + 3 == BlockInfo.Bank)
                  *(_BufferDecrunched + ((word)OPEntry->Address % 16384)) = OPEntry->Set;
                OPEntry = OPEntry->Next;
              }
              CrunchZ80Block (_BufferDecrunched, 16384, _BufferCrunched, &NewSize);                         /* Write updated file */
              BlockInfo.Length = NewSize;
              if (NewSize == 0xFFFF)                                                           /* Block could not be compressed ? */
                NewSize = 16384;                                               /* Keep the original length after writing the mark */
              if (write (FhOut, &BlockInfo, 3) != 3)
              { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, PokedGamePath, strerror (errno));
                CleanupPatch (); return (FALSE); }
              if (write (FhOut, _BufferCrunched, NewSize) != NewSize)
              { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, FullPath, strerror (errno));
                CleanupPatch (); return (FALSE); }
              if ((NRead = read (FhIn, &BlockInfo, 3)) == -1)
              { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
                CleanupPatch (); return (FALSE); }
            }
            else
              NRead = 0;
          }
          DestroyEntries (PokeSetup);
          PokeSetup = NULL;
        }
        if (FileType == FT_SLT)
        {
          if (write (FhOut, &SLTMarker, 6) != 6)                                                    /* Write SLT extension marker */
          { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, PokedGamePath, strerror (errno));
            CleanupPatch (); return (FALSE); }
          Offset = SLTMap.TableStart;
          if (lseek (FhIn, Offset, SEEK_SET) == -1)
          { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno));
            CleanupPatch (); return (FALSE); }
          while (Offset < FileLength)                                                                           /* Write SLT tail */
          {
            NewSize = (word)(FileLength - Offset >= 256 ? 256 : FileLength - Offset);
            if ((NRead = read (FhIn, _BufferCrunched, NewSize)) != NewSize)
            { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
              CleanupPatch (); return (FALSE); }
            if (write (FhOut, _BufferCrunched, NewSize) != NewSize)
            { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, PokedGamePath, strerror (errno));
              CleanupPatch (); return (FALSE); }
            Offset += (word)NewSize;
          }
        }
      }
      else if (FileType == FT_SNA)
      {
        if (read (FhIn, &SNASnapshotHeader, sizeof (struct MMSnapshotHeader_s)) == -1)                    /* Copy over SNA header */
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
          CleanupPatch (); return (FALSE); }
        Offset = sizeof (struct MMSnapshotHeader_s);
        if (write (FhOut, &SNASnapshotHeader, sizeof (struct MMSnapshotHeader_s)) == -1)
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, PokedGamePath, strerror (errno));
          CleanupPatch (); return (FALSE); }
        if (FileLength - Offset != (word)49152)
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, "Wrong filesize of file %s", FullPath, strerror (errno));
          CleanupPatch (); return (FALSE); }
        if ((NRead = read (FhIn, _BufferDecrunched, (word)49152)) == -1)                                         /* Read old file */
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
          CleanupPatch (); return (FALSE); }
        OPEntry = PokeSetup;
        while (OPEntry != NULL)                                                                                /* Apply the pokes */
        {
          *(_BufferDecrunched + (word)OPEntry->Address - 16384) = OPEntry->Set;
          OPEntry = OPEntry->Next;
        }
        DestroyEntries (PokeSetup);
        PokeSetup = NULL;
        if (write (FhOut, _BufferDecrunched, (word)49152) == -1)                                           /* Write updated image */
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, PokedGamePath, strerror (errno));
          CleanupPatch (); return (FALSE); }
      }
      else if (FileType == FT_SP)
      {
        if (read (FhIn, &SPSnapshotHeader, sizeof (struct SPSnapshotHeader_s)) == -1)                      /* Copy over SP header */
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
          CleanupPatch (); return (FALSE); }
        Offset = sizeof (struct SPSnapshotHeader_s);
        if (write (FhOut, &SPSnapshotHeader, sizeof (struct SPSnapshotHeader_s)) == -1)
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, PokedGamePath, strerror (errno));
          CleanupPatch (); return (FALSE); }
        if ((NRead = read (FhIn, _BufferDecrunched + SPSnapshotHeader.Start - 16384, SPSnapshotHeader.Length)) == -1)
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));             /* Read old file */
          CleanupPatch (); return (FALSE); }
        OPEntry = PokeSetup;
        while (OPEntry != NULL)                                                                                /* Apply the pokes */
        {
          *(_BufferDecrunched + (word)OPEntry->Address - 16384) = OPEntry->Set;
          OPEntry = OPEntry->Next;
        }
        DestroyEntries (PokeSetup);
        PokeSetup = NULL;
        if (write (FhOut, _BufferDecrunched + SPSnapshotHeader.Start - 16384, SPSnapshotHeader.Length) == -1)
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, PokedGamePath, strerror (errno)); /* Write updated image */
          CleanupPatch (); return (FALSE); }
      }
      close (FhIn);
      close (FhOut);
      FhIn = FhOut = -1;
      CleanupPatch ();
      return (TRUE);
    }
  }
  return (FALSE);
}
