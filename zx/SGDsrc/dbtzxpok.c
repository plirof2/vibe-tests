/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBTZXPOK.C                                                                                                      */
/* Description  : TZX Custom Info - Pokes block handling                                                                          */
/* Version type : Program module                                                                                                  */
/* Last changed : 05-10-1998  6:30                                                                                                */
/* Update count : 1                                                                                                               */
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

#define __DBTZXPOK_MAIN__

#include "wbench32/wbench32.h"
#include "dbconfig.h"
#include "dbmain.h"
#include "dbpoke.h"
#include "dbtzxpok.h"

#pragma pack                ()

/**********************************************************************************************************************************/
/* Define the static variables                                                                                                    */
/**********************************************************************************************************************************/

static char        _GeneralDescription[257];
static byte       *_StartAddress;

/**********************************************************************************************************************************/
/* >>>>> Used by the POKEs block                                                                                                  */
/**********************************************************************************************************************************/

#define MAXPOKNAME    30
#define FLAG_IGNBANK  0x08
#define FLAG_ASK      0x10
#define FLAG_NOORG    0x20
#define FLAG_ALL      0xF8

static struct PokeEntry_s                                                                                       /* One poke entry */
{
  struct PokeEntry_s *Next;
  struct PokeEntry_s *Previous;
  byte                Bank;
  word                Address;
  byte                Set;
  byte                Org;
  byte                Flags;
};

static struct GamePokes_s                                                                                 /* One poke description */
{
  struct GamePokes_s *Next;
  struct GamePokes_s *Previous;
  char                Name[MAXPOKNAME + 1];
  struct PokeEntry_s *Pokes;
} *GamePokes = NULL;

static comboent       SelectedPoke       = 0;
static comboent       SelectedEntry      = 0;

static struct ComboInfo PokesWindow;
static struct ComboInfo EntriesWindow;

/**********************************************************************************************************************************/
/* Define the static functions                                                                                                    */
/**********************************************************************************************************************************/

static void _DestroyPokes          (void);
static void _DestroyEntries        (struct PokeEntry_s *PokeEntries);
static bool _SetupPokesWindow      (void);
static bool _SetupEntriesWindow    (comboent SelectedLine);
static bool _ReadGamePokes         (dword OldSize);
static bool _WriteGamePokes        (dword *NewSize);

static int  _HandleAddPoke         (struct WindowInfo *Window, struct ButtonInfo *Button);
static int  _HandleDelPoke         (struct WindowInfo *Window, struct ButtonInfo *Button);
static bool _HandleUpdatePoke      (void);
static int  _HandleAddEntry        (struct WindowInfo *Window, struct ButtonInfo *Button);
static int  _HandleDelEntry        (struct WindowInfo *Window, struct ButtonInfo *Button);
static bool _HandleUpdateEntry     (void);

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE INTERNAL POKE LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

static void _DestroyEntries (struct PokeEntry_s *PokeEntries)

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

static void _DestroyPokes (void)

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
    _DestroyEntries (OPokes->Pokes);
    OPokes = OPokes->Next;
    free (OPokes->Previous);
  }
  _DestroyEntries (OPokes->Pokes);
  free (OPokes);
  GamePokes = NULL;
}

static bool _SetupPokesWindow (void)

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

static bool _SetupEntriesWindow (comboent SelectedLine)

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
    if (!(OEntry->Flags & FLAG_IGNBANK))
      sprintf (ABank, "%u", OEntry->Bank);
    else
      strcpy (ABank, "-");
    if (!(OEntry->Flags & FLAG_ASK))
      sprintf (ASet, "%3u", OEntry->Set);
    else
      strcpy (ASet, "ASK");
    if (!(OEntry->Flags & FLAG_NOORG))
      sprintf (NEntry.Text, "%s  %5u,%s (%3u)", ABank, OEntry->Address, ASet, OEntry->Org);
    else
      sprintf (NEntry.Text, "%s  %5u,%s", ABank, OEntry->Address, ASet);
    NEntry.Identifier = (dword)OEntry;
    if (!InsertComboEntry (&EntriesWindow, &NEntry, NULL, -1))
      AllOk = FALSE;
    OEntry = OEntry->Next;
  }
  PrintComboEntries (&EntriesWindow);
  return (AllOk);
}

static bool _ReadGamePokes (dword OldSize)

/**********************************************************************************************************************************/
/* Pre   : `OldSize' holds the current size of the block.                                                                         */
/* Post  : The table, which is in `_StartAddress' has been setup into the pokes lists.                                            */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *NPokes;
  struct GamePokes_s *OPokes = NULL;
  struct PokeEntry_s *NEntry;
  struct PokeEntry_s *OEntry = NULL;
  byte      register  CntTrainers;
  byte      register  CntPokes;
  byte               *BIndex;

  _GeneralDescription[0] = '\0';
  if (OldSize == 0)
    return (TRUE);
  strncpy (_GeneralDescription, _StartAddress + 1, *_StartAddress);                             /* Read General Description field */
  _GeneralDescription[*_StartAddress] = '\0';
  OPokes = GamePokes;
  BIndex = _StartAddress + (*_StartAddress) + 1;                                            /* Point to the start of the trainers */
  CntTrainers = *(BIndex ++);                                                              /* Get number of trainers in the block */
  for ( ; CntTrainers > 0 ; CntTrainers --)                                                                  /* Read all trainers */
  {
    if ((NPokes = (struct GamePokes_s *)malloc (sizeof (struct GamePokes_s))) == NULL)     return (FALSE);  
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
    if (*BIndex > MAXPOKNAME)                                                            /* Description more than 30 characters ? */
    {
      strncpy (NPokes->Name, BIndex + 1, MAXPOKNAME);                                                     /* Strip the rest of it */
      NPokes->Name[MAXPOKNAME] = '\0';
    }
    else
    {
      strncpy (NPokes->Name, BIndex + 1, *BIndex);
      NPokes->Name[*BIndex] = '\0';
    }
    BIndex += (*BIndex) + 1;                                                                       /* Point past the trainer name */
    CntPokes = *(BIndex ++);                                                               /* Get number of pokes in this trainer */
    OEntry = NULL;
    for ( ; CntPokes > 0 ; CntPokes --)                                                          /* Read all pokes in the trainer */
    {
      if ((NEntry = (struct PokeEntry_s *)malloc (sizeof (struct PokeEntry_s))) == NULL)    return (FALSE);
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
      NEntry->Flags = (*BIndex) & FLAG_ALL;                                                 /* Split the flags off the bank field */
      NEntry->Bank = (*BIndex) & ~FLAG_ALL;
      NEntry->Address = *((word *)(BIndex + 1));
      NEntry->Set = *(BIndex + 3);
      NEntry->Org = *(BIndex + 4);
      BIndex += 5;                                                                 /* Prepare to do the next poke in this trainer */
      OEntry = NEntry;
    }
    OPokes = NPokes;
  }
  return (TRUE);
}

static bool _WriteGamePokes (dword *NewSize)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The pokes table has been reconstructed into _StartAddress, from the lists in memory.                                   */
/*         The new size of the block is returned in `NewSize'.                                                                    */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *NPokes;
  struct PokeEntry_s *NEntry;
  byte               *BIndex;
  byte               *BNumTrainers;
  byte               *BNumPokes;

  NPokes = GamePokes;
  *_StartAddress = strlen (_GeneralDescription);
  strcpy (_StartAddress + 1, _GeneralDescription);
  BIndex = _StartAddress + (*_StartAddress) + 1;
  BNumTrainers = BIndex ++;                                                 /* Keep location where the number of trainers is kept */
  *BNumTrainers = 0;
  while (NPokes != NULL)
  {
    while (NPokes != NULL && NPokes->Pokes == NULL)                                           /* Only a name, no pokes attached ? */
      NPokes = NPokes->Next;                                                                                           /* Skip it */
    if (NPokes != NULL)                                                                                 /* Still anything to do ? */
    {
      (*BNumTrainers) ++;
      *BIndex = strlen (NPokes->Name);
      strcpy (BIndex + 1, NPokes->Name);
      BIndex += (*BIndex) + 1;
      BNumPokes = BIndex ++;                                       /* Keep location where number of pokes in this trainer is kept */
      *BNumPokes = 0;
      NEntry = NPokes->Pokes;
      while (NEntry != NULL)
      {
        (*BNumPokes) ++;
        *BIndex = (NEntry->Flags | NEntry->Bank);
        *((word *)(BIndex + 1)) = NEntry->Address;
        *(BIndex + 3) = NEntry->Set;
        *(BIndex + 4) = NEntry->Org;
        BIndex += 5;
        NEntry = NEntry->Next;
      }
      NPokes = NPokes->Next;
    }
  }
  *NewSize = ((dword)BIndex - (dword)_StartAddress);
  return (TRUE);
}

static int _HandleAddPoke (struct WindowInfo *Window, struct ButtonInfo *Button)

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
    if (InputRequester (-1, -1, NULL, NULL, NEntry.Text, MAXPOKNAME, TRUE, FALSE, "Add Trainer Description",
                        "{CGive the description of the new trainer}C"))
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
      _SetupEntriesWindow (SelectedPoke);
      SelectEntry (&PokesWindow, SelectedPoke);
      SelectEntry (&EntriesWindow, SelectedEntry);
      _HandleAddEntry (Window, Button);                                                                /* Auto-insert first entry */
      return (WINH_EDITEND);                                                             /* Signal: (un)ghost appropriate buttons */
    }
  }
  return (WINH_NONE);
}

static int _HandleDelPoke (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Delete' button has been pressed (or released).                                                                    */
/* Post  : The current poke has been removed.                                                                                     */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *OPokes;
  struct PokeEntry_s *OEntry;
  struct ComboEntry   NEntry;

  Window = Window; /* keep compiler happy */
  if (!(Button->Flags & BF_PRESSED) && PokesWindow.NumEntries >= 0)
  {
    GetComboEntry (&PokesWindow, &NEntry, SelectedPoke);
    if (DBConfig.ConfirmDelete)                                                               /* Confirmation needed: handle it ? */
      if (QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Delete",
                             "Are you sure you want to delete the trainer\n\"%s\"?", NEntry.Text))
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
    _SetupEntriesWindow (SelectedPoke);
    SelectEntry (&PokesWindow, SelectedPoke);
    SelectEntry (&EntriesWindow, SelectedEntry);
    return (WINH_EDITEND);                                                               /* Signal: (un)ghost appropriate buttons */
  }
  return (WINH_NONE);
}

static bool _HandleUpdatePoke (void)

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
  if (InputRequester (-1, -1, NULL, NULL, CEntry, MAXPOKNAME, TRUE, FALSE, "Update Trainer Description",
                      "{CGive the new description of the trainer}C"))
    if (strcmp (CEntry, OPokes->Name))                                                                      /* Actually changed ? */
    {
      PutComboEntry (&PokesWindow, &NEntry, SelectedPoke, TRUE);
      strcpy (OPokes->Name, CEntry);
      SelectEntry (&PokesWindow, SelectedPoke);
      return (TRUE);
    }
  return (FALSE);
}

static int _HandleAddEntry (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Insert' button has been pressed (or released).                                                                    */
/* Post  : A new poke entry has been inserted.                                                                                    */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct WindowInfo       CWindow  = {190, 220, 160, 160, 160, 160, 160, 160, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                      WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "Add Poke"};
  struct ButtonInfo       BBank    = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 40, 1, DBLACK, DWHITE, NULL,
                                      1, NULL, NULL};
  struct ButtonInfo       BNoBank  = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 85, 40, 6, DBLACK, DWHITE, NULL,
                                      9, "\007", NULL};
  struct ButtonInfo       BAddress = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 60, 5, DBLACK, DWHITE, NULL,
                                      2, NULL, NULL};
  struct ButtonInfo       BSet     = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 80, 3, DBLACK, DWHITE, NULL,
                                      3, NULL, NULL};
  struct ButtonInfo       BAskSet  = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 97, 80, 6, DBLACK, DWHITE, NULL,
                                      8, "\007", NULL};
  struct ButtonInfo       BOrg     = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 100, 3, DBLACK, DWHITE, NULL,
                                      4, NULL, NULL};
  struct ButtonInfo       BNoOrg   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 97, 100, 6, DBLACK, DWHITE, NULL,
                                      5, "\007", NULL};
  struct ButtonInfo       BCancel  = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 10, 135, 8, DBLACK, DWHITE, NULL,
                                      6, "@Cancel", NULL};
  struct ButtonInfo       BOk      = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_GHOSTED, 96, 135, 8, DBLACK, DWHITE, NULL,
                                      7, "@Ok", NULL};
  struct ButtonInfo      *ButtonP;
  struct GamePokes_s     *OPokes;
  struct PokeEntry_s     *OEntry;
  struct PokeEntry_s     *NEntry;
  struct ComboEntry       NCEntry;
  char                    ABank[2];
  char                    AAddress[6];
  char                    ASet[4];
  char                    AOrg[4];
  bool                    Finished;
  bool                    Cancelled = FALSE;
  bool                    Accepted = FALSE;
  bool                    ChangeOKButton;
  byte                    Flags = FLAG_IGNBANK | FLAG_NOORG;

  Window = Window; /* keep compiler happy */
  BBank.Contents = ABank;
  BAddress.Contents = AAddress;
  BSet.Contents = ASet;
  BOrg.Contents = AOrg;
  if (!(Button->Flags & BF_PRESSED))
  {
    AAddress[0] = ASet[0] = '\0';
    if (Flags & FLAG_NOORG)
    {
      strcpy (AOrg, "???");
      BOrg.Flags |= BF_GHOSTED;
    }
    else
      AOrg[0] = '\0';
    if (Flags & FLAG_IGNBANK)
    {
      strcpy (ABank, "-");
      BBank.Flags |= BF_GHOSTED;
    }
    else
      ABank[0] = '\0';
    AddButton (&CWindow, &BAddress, FALSE);
    AddButton (&CWindow, &BSet, FALSE);
    AddButton (&CWindow, &BAskSet, FALSE);
    AddButton (&CWindow, &BOrg, FALSE);
    AddButton (&CWindow, &BNoOrg, FALSE);
    AddButton (&CWindow, &BCancel, FALSE);
    AddButton (&CWindow, &BOk, TRUE);
    AddButton (&CWindow, &BBank, FALSE);
    AddButton (&CWindow, &BNoBank, FALSE);
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
                                case 6 : Finished = TRUE; Cancelled = TRUE; break;                                    /* (Cancel) */
                                case 7 : Finished = TRUE; Cancelled = FALSE; break;                                       /* (Ok) */
                                case 5 : if (Flags & FLAG_NOORG)                                                     /* ORG cycle */
                                         {
                                           Flags &= ~FLAG_NOORG;
                                           strcpy (AOrg, "0");
                                           BOrg.Flags &= ~BF_GHOSTED;
                                           UpdateButtonText (&CWindow, &BOrg, BF_FITTXT);
                                         }
                                         else
                                         {
                                           Flags |= FLAG_NOORG;
                                           strcpy (AOrg, "???");
                                           BOrg.Flags |= BF_GHOSTED;
                                           UpdateButtonText (&CWindow, &BOrg, BF_FITTXT);
                                         }
                                         break;
                                case 8 : if (Flags & FLAG_ASK)                                                       /* SET cycle */
                                         {
                                           Flags &= ~FLAG_ASK;
                                           strcpy (ASet, "0");
                                           BSet.Flags &= ~BF_GHOSTED;
                                           UpdateButtonText (&CWindow, &BSet, BF_FITTXT);
                                         }
                                         else
                                         {
                                           Flags |= FLAG_ASK;
                                           strcpy (ASet, "ASK");
                                           BSet.Flags |= BF_GHOSTED;
                                           UpdateButtonText (&CWindow, &BSet, BF_FITTXT);
                                         }
                                         ChangeOKButton = TRUE;
                                         break;
                                case 9 : if (Flags & FLAG_IGNBANK)                                                  /* BANK cycle */
                                         {
                                           Flags &= ~FLAG_IGNBANK;
                                           strcpy (ABank, "0");
                                           BBank.Flags &= ~BF_GHOSTED;
                                           UpdateButtonText (&CWindow, &BBank, BF_FITTXT);
                                         }
                                         else
                                         {
                                           Flags |= FLAG_IGNBANK;
                                           strcpy (ABank, "-");
                                           BBank.Flags |= BF_GHOSTED;
                                           UpdateButtonText (&CWindow, &BBank, BF_FITTXT);
                                         }
                                         break;
                              }
                              break;
          case WINH_EDITEND : ChangeOKButton = TRUE;
                              break;
        }
        if (ChangeOKButton)
        {
          if (AAddress[0] != '\0' && ASet[0] != '\0')
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
        if (((Flags & FLAG_IGNBANK) || atoi (ABank) < 8) &&
            atol (AAddress) >= 16384 && atol (AAddress) < 65536 &&
            ((Flags & FLAG_ASK) || atol (ASet) < 256) &&
            ((Flags & FLAG_NOORG) || atol (AOrg) < 256))
          Accepted = TRUE;
      if (!Accepted)
        ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - One or more values are out of range");
    }
    DestroyWindow (&CWindow);
    GlobalKey = 0x0000;
    if (!Cancelled)
    {
      if ((NEntry = (struct PokeEntry_s *)malloc (sizeof (struct PokeEntry_s))) == NULL)
        FatalError (1, _ErrOutOfMemory, sizeof (struct PokeEntry_s));
      NEntry->Next = NULL;
      if (Flags & FLAG_IGNBANK)
        NEntry->Bank = 0;
      else
        NEntry->Bank = atoi (ABank);
      NEntry->Address = atoi (AAddress);
      if (Flags & FLAG_ASK)
        NEntry->Set = 0;
      else
        NEntry->Set = atoi (ASet);
      if (Flags & FLAG_NOORG)
        NEntry->Org = 0;
      else
        NEntry->Org = atoi (AOrg);
      NEntry->Flags = Flags;
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
      if (!(NEntry->Flags & FLAG_IGNBANK))
        sprintf (ABank, "%u", NEntry->Bank);
      else
        strcpy (ABank, "-");
      if (!(NEntry->Flags & FLAG_ASK))
        sprintf (ASet, "%3u", NEntry->Set);
      else
        strcpy (ASet, "ASK");
      if (!(NEntry->Flags & FLAG_NOORG))
        sprintf (NCEntry.Text, "%s  %5u,%s (%3u)", ABank, NEntry->Address, ASet, NEntry->Org);
      else
        sprintf (NCEntry.Text, "%s  %5u,%s", ABank, NEntry->Address, ASet);
      if (!InsertComboEntry (&EntriesWindow, &NCEntry, NULL, -1))
        FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
      SelectedEntry = EntriesWindow.NumEntries;
      SelectEntry (&EntriesWindow, SelectedEntry);
      return (WINH_EDITEND);                                                             /* Signal: (un)ghost appropriate buttons */
    }
  }
  return (WINH_NONE);
}

static int _HandleDelEntry (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Remove' button has been pressed (or released).                                                                    */
/* Post  : The current poke entry has been removed.                                                                               */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct GamePokes_s *OPokes;
  struct PokeEntry_s *OEntry;
  struct ComboEntry   NEntry;

  Window = Window; /* keep compiler happy */
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

static bool _HandleUpdateEntry (void)

/**********************************************************************************************************************************/
/* Pre   : A poke entry has been double-clicked.                                                                                  */
/* Post  : The current poke entry has been updated.                                                                               */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct WindowInfo       CWindow  = {190, 220, 160, 160, 160, 160, 160, 160, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                      WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "Update Poke"};
  struct ButtonInfo       BBank    = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 40, 1, DBLACK, DWHITE, NULL,
                                      1, NULL, NULL};
  struct ButtonInfo       BNoBank  = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 85, 40, 6, DBLACK, DWHITE, NULL,
                                      9, "\007", NULL};
  struct ButtonInfo       BAddress = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 60, 5, DBLACK, DWHITE, NULL,
                                      2, NULL, NULL};
  struct ButtonInfo       BSet     = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 80, 3, DBLACK, DWHITE, NULL,
                                      3, NULL, NULL};
  struct ButtonInfo       BAskSet  = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 97, 80, 6, DBLACK, DWHITE, NULL,
                                      8, "\007", NULL};
  struct ButtonInfo       BOrg     = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 100, 3, DBLACK, DWHITE, NULL,
                                      4, NULL, NULL};
  struct ButtonInfo       BNoOrg   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 97, 100, 6, DBLACK, DWHITE, NULL,
                                      5, "\007", NULL};
  struct ButtonInfo       BCancel  = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 10, 135, 8, DBLACK, DWHITE, NULL,
                                      6, "@Cancel", NULL};
  struct ButtonInfo       BOk      = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 96, 135, 8, DBLACK, DWHITE, NULL,
                                      7, "@Ok", NULL};
  struct ButtonInfo      *ButtonP;
  struct PokeEntry_s     *OEntry;
  char                   *CEntry;
  struct ComboEntry       NEntry;
  char                    OldCEntry[30];
  char                    ABank[2];
  char                    AAddress[6];
  char                    ASet[4];
  char                    AOrg[4];
  bool                    Finished;
  bool                    Cancelled;
  bool                    Accepted = FALSE;
  bool                    ChangeOKButton;

  BBank.Contents = ABank;
  BAddress.Contents = AAddress;
  BSet.Contents = ASet;
  BOrg.Contents = AOrg;
  GetComboEntry (&EntriesWindow, &NEntry, SelectedEntry);
  OEntry = (struct PokeEntry_s *)NEntry.Identifier;
  CEntry = NEntry.Text;
  strcpy (OldCEntry, CEntry);
  AAddress[0] = ASet[0] = '\0';
  if (OEntry->Flags & FLAG_NOORG)
  {
    strcpy (AOrg, "???");
    BOrg.Flags |= BF_GHOSTED;
  }
  else
    sprintf (AOrg, "%d", OEntry->Org);
  if (OEntry->Flags & FLAG_IGNBANK)
  {
    strcpy (ABank, "-");
    BBank.Flags |= BF_GHOSTED;
  }
  else
    sprintf (ABank, "%d", OEntry->Bank);
  if (OEntry->Flags & FLAG_ASK)
  {
    strcpy (ASet, "ASK");
    BSet.Flags |= BF_GHOSTED;
  }
  else
    sprintf (ASet, "%d", OEntry->Set);
  sprintf (AAddress, "%u", OEntry->Address);
  AddButton (&CWindow, &BOk, TRUE);
  AddButton (&CWindow, &BBank, FALSE);
  AddButton (&CWindow, &BNoBank, FALSE);
  AddButton (&CWindow, &BAddress, FALSE);
  AddButton (&CWindow, &BSet, FALSE);
  AddButton (&CWindow, &BAskSet, FALSE);
  AddButton (&CWindow, &BOrg, FALSE);
  AddButton (&CWindow, &BNoOrg, FALSE);
  AddButton (&CWindow, &BCancel, FALSE);
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
  Wprintf (&CWindow, 1, TEXT_CENTER, "Update poke parameters");
  Wprintf (&CWindow,  4, 2, "{+1Bank}N");
  Wprintf (&CWindow,  6, 2, "{+1Address}N");
  Wprintf (&CWindow,  8, 2, "{+1Value}N");
  Wprintf (&CWindow, 10, 2, "{+1Original}N");
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
                              case 6 : Finished = TRUE; break;                                                        /* (Cancel) */
                              case 7 : Finished = TRUE; Cancelled = FALSE; break;                                         /* (Ok) */
                              case 5 : if (OEntry->Flags & FLAG_NOORG)                                               /* ORG cycle */
                                       {
                                         OEntry->Flags &= ~FLAG_NOORG;
                                         strcpy (AOrg, "0");
                                         BOrg.Flags &= ~BF_GHOSTED;
                                         UpdateButtonText (&CWindow, &BOrg, BF_FITTXT);
                                       }
                                       else
                                       {
                                         OEntry->Flags |= FLAG_NOORG;
                                         strcpy (AOrg, "???");
                                         BOrg.Flags |= BF_GHOSTED;
                                         UpdateButtonText (&CWindow, &BOrg, BF_FITTXT);
                                       }
                                       break;
                              case 8 : if (OEntry->Flags & FLAG_ASK)                                                 /* SET cycle */
                                       {
                                         OEntry->Flags &= ~FLAG_ASK;
                                         strcpy (ASet, "0");
                                         BSet.Flags &= ~BF_GHOSTED;
                                         UpdateButtonText (&CWindow, &BSet, BF_FITTXT);
                                       }
                                       else
                                       {
                                         OEntry->Flags |= FLAG_ASK;
                                         strcpy (ASet, "ASK");
                                         BSet.Flags |= BF_GHOSTED;
                                         UpdateButtonText (&CWindow, &BSet, BF_FITTXT);
                                       }
                                       ChangeOKButton = TRUE;
                                       break;
                              case 9 : if (OEntry->Flags & FLAG_IGNBANK)                                            /* BANK cycle */
                                       {
                                         OEntry->Flags &= ~FLAG_IGNBANK;
                                         strcpy (ABank, "0");
                                         BBank.Flags &= ~BF_GHOSTED;
                                         UpdateButtonText (&CWindow, &BBank, BF_FITTXT);
                                       }
                                       else
                                       {
                                         OEntry->Flags |= FLAG_IGNBANK;
                                         strcpy (ABank, "-");
                                         BBank.Flags |= BF_GHOSTED;
                                         UpdateButtonText (&CWindow, &BBank, BF_FITTXT);
                                       }
                                       break;
                            }
                            break;
        case WINH_EDITEND : ChangeOKButton = TRUE;
                            break;
      }
      if (ChangeOKButton)
      {
        if (AAddress[0] != '\0' && ASet[0] != '\0')
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
      if (((OEntry->Flags & FLAG_IGNBANK) || atoi (ABank) < 8) &&
          atol (AAddress) >= 16384 && atol (AAddress) < 65536 &&
          ((OEntry->Flags & FLAG_ASK) || atol (ASet) < 256) &&
          ((OEntry->Flags & FLAG_NOORG) || atol (AOrg) < 256))
        Accepted = TRUE;
    if (!Accepted)
      ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - One or more values are out of range");
  }
  DestroyWindow (&CWindow);
  GlobalKey = 0x0000;
  if (!Cancelled)
  {
    if (OEntry->Flags & FLAG_IGNBANK)
      OEntry->Bank = 0;
    else
      OEntry->Bank = atoi (ABank);
    OEntry->Address = atoi (AAddress);
    if (OEntry->Flags & FLAG_ASK)
      OEntry->Set = 0;
    else
      OEntry->Set = atoi (ASet);
    if (OEntry->Flags & FLAG_NOORG)
      OEntry->Org = 0;
    else
      OEntry->Org = atoi (AOrg);
    if (!(OEntry->Flags & FLAG_IGNBANK))
      sprintf (ABank, "%u", (OEntry->Bank & ~FLAG_ALL));
    else
      strcpy (ABank, "-");
    if (!(OEntry->Flags & FLAG_ASK))
      sprintf (ASet, "%3u", OEntry->Set);
    else
      strcpy (ASet, "ASK");
    if (!(OEntry->Flags & FLAG_NOORG))
      sprintf (NEntry.Text, "%s  %5u,%s (%3u)", ABank, OEntry->Address, ASet, OEntry->Org);
    else
      sprintf (NEntry.Text, "%s  %5u,%s", ABank, OEntry->Address, ASet);
    PutComboEntry (&EntriesWindow, &NEntry, SelectedEntry, TRUE);
    SelectEntry (&EntriesWindow, SelectedEntry);
    return ((bool)(strcmp (OldCEntry, CEntry) == 0) ? FALSE : TRUE);
  }
  return (FALSE);
}

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE EXPORTED POKE LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

bool EditTZXGamePokes (byte *Where, dword OldSize, dword *NewSize)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The pokes file for the currently selected games has been edited and written back.                                      */
/* Import: _ReadGamePokes, _WriteGamePokes, _SetupPokesWindow, _SetupEntriesWindow, _DestroyPokes.                                */
/**********************************************************************************************************************************/

{
  struct WindowInfo      CWindow = {120, 85, 382, 318, 382, 318, 382, 318, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                    WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL,
                                    "Edit Custom Info - POKEs block"};
  struct InputBoxInfo    BGenDes = {100, 30, 30, 7, DBLACK, DWHITE, NULL, 7, _GeneralDescription, BF_NOSURROUND | BF_KEYSELECT,
                                    255, 10};
  struct ButtonInfo      BAddP   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 18, 251, 8, DBLACK, DWHITE, NULL,
                                    1, "@Add", _HandleAddPoke};
  struct ButtonInfo      BDelP   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 78, 251, 8, DBLACK, DWHITE, NULL,
                                    2, "@Delete", _HandleDelPoke};
  struct ButtonInfo      BAddE   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 246, 251, 8, DBLACK, DWHITE, NULL,
                                    3, "@Insert", _HandleAddEntry};
  struct ButtonInfo      BDelE   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 306, 251, 8, DBLACK, DWHITE, NULL,
                                    4, "Re@move", _HandleDelEntry};
  struct ButtonInfo      BCancel = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 70, 290, 8, DBLACK, DWHITE, NULL,
                                    5, "@Cancel", NULL};
  struct ButtonInfo      BOk     = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 260, 290, 8, DBLACK, DWHITE, NULL,
                                    6, "@Save", NULL};
  struct ButtonInfo     *ButtonP;
  comboent               SelectedLine;
  bool                   Finished     = FALSE;
  bool                   Cancelled    = TRUE;
  bool                   PokesChanged = FALSE;
  bool                   PokesExisted = TRUE;

  _StartAddress = Where;
  if (!_ReadGamePokes (OldSize))
  { ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Out of memory while setting up"); return (FALSE); }
  AddButton (&CWindow, &BOk, TRUE);
  AddInputBox (&CWindow, &BGenDes, FALSE);
  AddButton (&CWindow, &BAddP, FALSE);
  AddButton (&CWindow, &BDelP, FALSE);
  AddButton (&CWindow, &BAddE, FALSE);
  AddButton (&CWindow, &BDelE, FALSE);
  AddButton (&CWindow, &BCancel, FALSE);
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
  WDrawShadowBox (&CWindow, 1, 0, 11, 62, "General Description");
  WDrawShadowBox (&CWindow, 12, 0, 27, 36, "Trainers");
  WDrawShadowBox (&CWindow, 12, 38, 27, 62, "Pokes");
  MakeComboWindow (&PokesWindow, &CWindow, COMT_SELECT, COMF_NOBUFFER | COMF_VSLIDER | COMF_KEYSELECT | COMF_SYNCSLIDER,
                   CWindow.StartX + 16, CWindow.StartY + 140, 15, 64, MAXPOKNAME + 5, MAXPOKNAME, 10, NULL, FALSE,
                   0, NULL, DBLACK, DWHITE, 0, 0);
  MakeComboWindow (&EntriesWindow, &CWindow, COMT_SELECT, COMF_NOBUFFER | COMF_VSLIDER | COMF_SYNCSLIDER,
                   CWindow.StartX + 244, CWindow.StartY + 140, 15, 64, 19, 18, 10, NULL, FALSE,
                   0, NULL, DBLACK, DWHITE, 0, 0);
  SelectedPoke = 0;
  SelectedEntry = 0;
  _SetupPokesWindow ();
  _SetupEntriesWindow (SelectedPoke);
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
                              if (!PokesExisted)                                            /* Allow full delete if not inserting */
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
                            _SetupEntriesWindow (SelectedPoke);
                            SelectEntry (&EntriesWindow, SelectedEntry);
                            break;
      case WINH_LINESEL   : if (_HandleUpdatePoke ())
                              PokesChanged = TRUE;
                            break;
    }
    if (HandleComboWindow (&EntriesWindow, &SelectedEntry) == WINH_LINESEL)
      if (_HandleUpdateEntry ())
        PokesChanged = TRUE;
    MouseStatus ();
    if (GlobalKey == K_ESC)
      Finished = TRUE;
  }
  if (!Cancelled && PokesChanged)
    _WriteGamePokes (NewSize);
  DestroyComboWindow (&PokesWindow, 0);
  DestroyComboWindow (&EntriesWindow, 0);
  DestroyWindow (&CWindow);
  GlobalKey = 0x0000;
  _DestroyPokes ();
  return (!Cancelled && PokesChanged);
}
