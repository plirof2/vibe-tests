/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBDBASE.C                                                                                                       */
/* Description  : Spectrum Database database handler                                                                              */
/* Version type : Program module                                                                                                  */
/* Last changed : 28-05-1999  21:05                                                                                               */
/* Update count : 66                                                                                                              */
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

#define __DBDBASE_MAIN__

#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <bios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "wbench32/wbench32.h"
#include "dbdbase.h"
#include "dbconfig.h"
#include "dbselect.h"
#include "dbmenu.h"
#include "dbmain.h"

#pragma pack                ()

#define C_CR                0x0D
#define C_LF                0x0A
#define C_FF                '\f'

/**********************************************************************************************************************************/
/* Define the static variables                                                                                                    */
/**********************************************************************************************************************************/

static struct PReqInfo  _PReq;
static char             _ShrinkenString[MAXBTEXT];
static short            _SeperatorLength = 1;                                         /* # Field seperator spaces in database */

static comboent         _SelectedObject  = -1;
static comboent         _SelectedEntry   = -1;
static struct ComboInfo _ObjectsWindow;
static struct ComboInfo _EntriesWindow;

static struct _PrintObjects_s
{
  char *_ObjectName;
  byte  _ObjectSize;
  char  _ObjectIdent;
} _PrintObjects[] = {
   { "Name",       D_NAME,       'A' },
   { "Year",       D_YEAR,       'B' },
   { "Publisher",  D_PUBLISHER,  'C' },
   { "Memory",     3,            'D' },
   { "Players",    D_PLAYERS,    'E' },
   { "Together",   1,            'F' },
   { "Sticks",     D_STICKS + 1, 'G' },
   { "PC-Name",    D_PCNAME,     'H' },
   { "Type",       D_TYPE,       'I' },
   { "FileSize",   D_FILESIZE,   'J' },
   { "OrigScreen", 1,            'K' },
   { "Origin",     1,            'L' },
   { "DiskId",     3,            'M' },
   { "Override",   4,            'N' },
   { "AY-Sound",   1,            'O' },
   { "MultiLoad",  1,            'P' },
   { "Language",   D_LANGUAGE,   'Q' },
   { "Score",      D_SCORE,      'R' },
   { "Author",     D_AUTHOR,     'S' } };
static byte _NumPrintObjects = sizeof (_PrintObjects) / sizeof (struct _PrintObjects_s);

/**********************************************************************************************************************************/
/* Define the static routines                                                                                                     */
/**********************************************************************************************************************************/

static bool _PrintLine          (char *Line);
static void _DestroyPReq        (void);
static bool _SetupObjectsWindow (void);
static bool _SetupEntriesWindow (void);
static void _EditPrintObjects   (void);

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE INTERNAL LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

static bool _SetupObjectsWindow (void)

/**********************************************************************************************************************************/
/* Pre   : The ObjectsWindow must have been created.                                                                              */
/* Post  : All possible printable database fields have been inserted into the ObjectsWindow.                                      */
/*         The return value is FALSE if an out-of-memory error occured (which is not reported here).                              */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct ComboEntry NEntry;
  bool              AllOk = TRUE;
  byte              Cnt;

  NEntry.Selected = 0x00;
  ReInitComboWindow (&_ObjectsWindow, 0);
  for (Cnt = 0 ; Cnt < _NumPrintObjects && AllOk ; Cnt ++)
  {
    strcpy (NEntry.Text, _PrintObjects[Cnt]._ObjectName);
    NEntry.Identifier = (dword)Cnt;
    if (!InsertComboEntry (&_ObjectsWindow, &NEntry, NULL, -1))
      AllOk = FALSE;
  }
  return (AllOk);
}

static bool _SetupEntriesWindow (void)

/**********************************************************************************************************************************/
/* Pre   : The EntriesWindow must have been created. _SetupObjectWindow must have been called.                                    */
/* Post  : All database fields as set in the DBConfig.FieldsString are inserted into the EntriesWindow.                           */
/*         The return value is FALSE if an out-of-memory error occured (which is not reported here).                              */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct ComboEntry NEntry;
  bool              AllOk = TRUE;
  bool              Found;
  byte              Cnt;
  byte              Cnt2;
  char             *FString;

  ReInitComboWindow (&_EntriesWindow, 0);
  FString = DBConfig.FieldsString;
  while (AllOk && *FString)
  {
    if (*FString >= 'A' && *FString < ('A' + _NumPrintObjects))                                     /* Correct field identifier ? */
      for (Cnt = 0, Found = FALSE ; Cnt < _NumPrintObjects && !Found ; Cnt ++)                                   /* Walk the list */
        if (_PrintObjects[Cnt]._ObjectIdent == *FString)                                                     /* This is the one ? */
        {
          Found = TRUE;
          GetComboEntry (&_ObjectsWindow, &NEntry, Cnt);
          if (!InsertComboEntry (&_EntriesWindow, &NEntry, NULL, -1))                                         /* Append the field */
            AllOk = FALSE;
        }
    FString ++;                                                                                /* (This also skips 'bad' options) */
  }
  FString = DBConfig.FieldsString;
  while (AllOk && *FString)
  {
    if (*FString >= 'A' && *FString < ('A' + _NumPrintObjects))                                     /* Correct field identifier ? */
      for (Cnt = 0, Found = FALSE ; Cnt < _NumPrintObjects && !Found ; Cnt ++)                                   /* Walk the list */
        if (_PrintObjects[Cnt]._ObjectIdent == *FString)                                                     /* This is the one ? */
        {
          for (Cnt2 = 0 ; Cnt2 <= _ObjectsWindow.NumEntries && !Found ; Cnt2 ++)                     /* Find corresponding object */
          {
            GetComboEntry (&_ObjectsWindow, &NEntry, Cnt2);
            if (NEntry.Identifier == (dword)Cnt)
            {
              DeleteComboEntry (&_ObjectsWindow, Cnt2);                                             /* Make the entry unavailable */
              Found = TRUE;
            }
          }
          Found = TRUE;
        }
    FString ++;                                                                                /* (This also skips 'bad' options) */
  }
  return (AllOk);
}

static void _EditPrintObjects (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The pokes file for the currently selected games has been edited and written back.                                      */
/* Import: _SetupObjectsWindow, _SetupEntriesWindow.                                                                              */
/**********************************************************************************************************************************/

{
  char                   CTLSize[4] = "  0";
  struct WindowInfo      CWindow = {138, 155, 346, 228, 346, 228, 346, 228, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                    WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "Setup Print Fields"};
  struct ButtonInfo      BAppend = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 135, 40, 10, DBLACK, DWHITE, NULL,
                                    1, "@Append ->", NULL};
  struct ButtonInfo      BInsert = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 135, 60, 10, DBLACK, DWHITE, NULL,
                                    2, "@Insert ->", NULL};
  struct ButtonInfo      BRemove = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 135, 80, 10, DBLACK, DWHITE, NULL,
                                    3, "<- @Remove", NULL};
  struct ButtonInfo      BAll    = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 135, 120, 10, DBLACK, DWHITE, NULL,
                                    4, "A@ll ->", NULL};
  struct ButtonInfo      BClear  = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 135, 140, 10, DBLACK, DWHITE, NULL,
                                    5, "<- Cl@ear", NULL};
  struct ButtonInfo      TLSize  = {BT_TEXT, BC_NUMBER, BF_FWIDTH, 175, 170, 3, DBLACK, DWHITE, NULL, 0, NULL, NULL};
  struct ButtonInfo      BCancel = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 58, 200, 8, DBLACK, DWHITE, NULL,
                                    6, "@Cancel", NULL};
  struct ButtonInfo      BOk     = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 236, 200, 8, DBLACK, DWHITE, NULL,
                                    7, "@Ok", NULL};
  struct ButtonInfo     *ButtonP;
  struct ComboEntry      NEntry;
  struct ComboEntry      CEntry;
  bool                   Finished          = FALSE;
  bool                   Cancelled         = TRUE;
  bool                   ReCalcTLSize      = TRUE;
  bool                   ReconsiderButtons = TRUE;
  bool                   Changed           = FALSE;
  bool                   Done;
  byte                   Cnt;
  byte                   Tmp;

  TLSize.Contents = CTLSize;
  AddButton (&CWindow, &BCancel, FALSE);
  AddButton (&CWindow, &BOk, TRUE);
  AddButton (&CWindow, &BAppend, FALSE);
  AddButton (&CWindow, &BInsert, FALSE);
  AddButton (&CWindow, &BRemove, FALSE);
  AddButton (&CWindow, &BAll, FALSE);
  AddButton (&CWindow, &BClear, FALSE);
  AddButton (&CWindow, &TLSize, FALSE);
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
  WDrawShadowBox (&CWindow, 1, 0, 18, 20, "Printable fields");
  WDrawShadowBox (&CWindow, 1, 34, 18, 54, "Chosen fields");
  Wprintf (&CWindow, 17, 23, "{+1Width}N");
  MakeComboWindow (&_ObjectsWindow, &CWindow, COMT_SELECT, COMF_NOBUFFER | COMF_VSLIDER | COMF_KEYSELECT | COMF_SYNCSLIDER,
                   CWindow.StartX + 16, CWindow.StartY + 30, 15, 64, 15, 14, 14, NULL, FALSE,
                   0, NULL, DBLACK, DWHITE, 0, 0);
  MakeComboWindow (&_EntriesWindow, &CWindow, COMT_SELECT, COMF_NOBUFFER | COMF_VSLIDER | COMF_SYNCSLIDER,
                   CWindow.StartX + 220, CWindow.StartY + 30, 15, 64, 15, 14, 14, NULL, FALSE,
                   0, NULL, DBLACK, DWHITE, 0, 0);
  _SetupObjectsWindow ();
  _SetupEntriesWindow ();
  _SelectedObject = _ObjectsWindow.NumEntries >= 0 ? 0 : -1;
  _SelectedEntry = _EntriesWindow.NumEntries >= 0 ? 0 : -1;
  PrintComboEntries (&_ObjectsWindow);
  PrintComboEntries (&_EntriesWindow);
  while (!Finished)
  {
    if (ReCalcTLSize)
    {
      ReCalcTLSize = FALSE;
      Tmp = 0;
      if (_EntriesWindow.NumEntries >= 0)
      {
        for (Cnt = 0 ; Cnt <= _EntriesWindow.NumEntries ; Cnt ++)                                           /* Added object sizes */
        {
          GetComboEntry (&_EntriesWindow, &NEntry, Cnt);
          Tmp += _PrintObjects[NEntry.Identifier]._ObjectSize;
        }
        Tmp += _EntriesWindow.NumEntries;                                                         /* Add (num objects - 1) spaces */
      }
      sprintf (CTLSize, "%3d", Tmp);
      UpdateButtonText (&CWindow, &TLSize, 0);
    }
    if (ReconsiderButtons)
    {
      ReconsiderButtons = FALSE;
      if (_ObjectsWindow.NumEntries < 0)
      {
        BAppend.Flags |= BF_GHOSTED;
        BInsert.Flags |= BF_GHOSTED;
        BAll.Flags |= BF_GHOSTED;
      }
      else
      {
        BAppend.Flags &= ~BF_GHOSTED;
        BInsert.Flags &= ~BF_GHOSTED;
        BAll.Flags &= ~BF_GHOSTED;
      }
      if (_EntriesWindow.NumEntries < 0)
      {
        BRemove.Flags |= BF_GHOSTED;
        BClear.Flags |= BF_GHOSTED;
        BOk.Flags |= BF_GHOSTED;
      }
      else
      {
        BRemove.Flags &= ~BF_GHOSTED;
        BClear.Flags &= ~BF_GHOSTED;
        BOk.Flags &= ~BF_GHOSTED;
      }
      UpdateButtonText (&CWindow, &BAppend, 0);
      UpdateButtonText (&CWindow, &BInsert, 0);
      UpdateButtonText (&CWindow, &BRemove, 0);
      UpdateButtonText (&CWindow, &BAll, 0);
      UpdateButtonText (&CWindow, &BClear, 0);
      UpdateButtonText (&CWindow, &BOk, 0);
      SelectEntry (&_ObjectsWindow, _SelectedObject);
      SelectEntry (&_EntriesWindow, _SelectedEntry);
      Changed = TRUE;
    }
    switch (HandleWindow (&CWindow, &ButtonP))
    {
      case WINH_CLOSE     : Finished = TRUE; break;
      case WINH_RNOHAND   : switch (ButtonP->ReactNum)
                            {
                              case 1 : GetComboEntry (&_ObjectsWindow, &NEntry, _SelectedObject);                     /* (Append) */
                                       InsertComboEntry (&_EntriesWindow, &NEntry, NULL, -1);
                                       DeleteComboEntry (&_ObjectsWindow, _SelectedObject);
                                       _SelectedEntry = _EntriesWindow.NumEntries;
                                       PrintComboEntries (&_EntriesWindow);
                                       PrintComboEntries (&_ObjectsWindow);
                                       ReCalcTLSize = TRUE;
                                       ReconsiderButtons = TRUE;
                                       break;
                              case 2 : GetComboEntry (&_ObjectsWindow, &NEntry, _SelectedObject);                     /* (Insert) */
                                       InsertComboEntry (&_EntriesWindow, &NEntry, NULL, _SelectedEntry);
                                       DeleteComboEntry (&_ObjectsWindow, _SelectedObject);
                                       if (_SelectedEntry == -1)
                                         _SelectedEntry = 0;
                                       PrintComboEntries (&_EntriesWindow);
                                       PrintComboEntries (&_ObjectsWindow);
                                       ReCalcTLSize = TRUE;
                                       ReconsiderButtons = TRUE;
                                       break;
                              case 3 : GetComboEntry (&_EntriesWindow, &NEntry, _SelectedEntry);                      /* (Remove) */
                                       for (Cnt = 0, Done = FALSE ; Cnt <= _ObjectsWindow.NumEntries && !Done ; Cnt ++)
                                       {
                                         GetComboEntry (&_ObjectsWindow, &CEntry, Cnt);
                                         if (CEntry.Identifier > NEntry.Identifier)
                                         {
                                           Done = TRUE;
                                           InsertComboEntry (&_ObjectsWindow, &NEntry, NULL, (comboent)Cnt);
                                         }
                                       }
                                       if (!Done)
                                         InsertComboEntry (&_ObjectsWindow, &NEntry, NULL, -1);
                                       DeleteComboEntry (&_EntriesWindow, _SelectedEntry);
                                       if (_SelectedEntry > _EntriesWindow.NumEntries)
                                         _SelectedEntry = _EntriesWindow.NumEntries;
                                       PrintComboEntries (&_EntriesWindow);
                                       PrintComboEntries (&_ObjectsWindow);
                                       ReCalcTLSize = TRUE;
                                       ReconsiderButtons = TRUE;
                                       break;
                              case 4 : while (_ObjectsWindow.NumEntries >= 0)                                            /* (All) */
                                       {
                                         GetComboEntry (&_ObjectsWindow, &NEntry, 0);
                                         InsertComboEntry (&_EntriesWindow, &NEntry, NULL, -1);
                                         DeleteComboEntry (&_ObjectsWindow, 0);
                                       }
                                       _SelectedObject = -1;
                                       _SelectedEntry = _EntriesWindow.NumEntries;
                                       PrintComboEntries (&_EntriesWindow);
                                       PrintComboEntries (&_ObjectsWindow);
                                       ReCalcTLSize = TRUE;
                                       ReconsiderButtons = TRUE;
                                       break;
                              case 5 : ReInitComboWindow (&_EntriesWindow, 0);                                         /* (Clear) */
                                       _SetupObjectsWindow ();
                                       _SelectedObject = 0;
                                       _SelectedEntry = -1;
                                       PrintComboEntries (&_EntriesWindow);
                                       PrintComboEntries (&_ObjectsWindow);
                                       ReCalcTLSize = TRUE;
                                       ReconsiderButtons = TRUE;
                                       break;
                              case 6 : Finished = TRUE; break;                                                        /* (Cancel) */
                              case 7 : Finished = TRUE; Cancelled = FALSE; break;                                         /* (Ok) */
                            }
                            break;
    }
    if (HandleComboWindow (&_ObjectsWindow, &_SelectedObject) == WINH_LINESEL)
    {
      GetComboEntry (&_ObjectsWindow, &NEntry, _SelectedObject);                                              /* Append the entry */
      InsertComboEntry (&_EntriesWindow, &NEntry, NULL, -1);
      DeleteComboEntry (&_ObjectsWindow, _SelectedObject);
      _SelectedEntry = _EntriesWindow.NumEntries;
      PrintComboEntries (&_EntriesWindow);
      PrintComboEntries (&_ObjectsWindow);
      ReCalcTLSize = TRUE;
      ReconsiderButtons = TRUE;
    }
    if (HandleComboWindow (&_EntriesWindow, &_SelectedEntry) == WINH_LINESEL)
    {
      GetComboEntry (&_EntriesWindow, &NEntry, _SelectedEntry);                                               /* Remove the entry */
      for (Cnt = 0, Done = FALSE ; Cnt <= _ObjectsWindow.NumEntries && !Done ; Cnt ++)
      {
        GetComboEntry (&_ObjectsWindow, &CEntry, Cnt);
        if (CEntry.Identifier > NEntry.Identifier)
        {
          Done = TRUE;
          InsertComboEntry (&_ObjectsWindow, &NEntry, NULL, (comboent)Cnt);
        }
      }
      if (!Done)
        InsertComboEntry (&_ObjectsWindow, &NEntry, NULL, -1);
      DeleteComboEntry (&_EntriesWindow, _SelectedEntry);
      if (_SelectedEntry > _EntriesWindow.NumEntries)
        _SelectedEntry = _EntriesWindow.NumEntries;
      PrintComboEntries (&_EntriesWindow);
      PrintComboEntries (&_ObjectsWindow);
      ReCalcTLSize = TRUE;
      ReconsiderButtons = TRUE;
    }
    MouseStatus ();
    if (GlobalKey == K_ESC)
      Finished = TRUE;
  }
  if (!Cancelled && Changed)                                                                                      /* Any change ? */
  {
    for (Cnt = 0 ; Cnt <= _EntriesWindow.NumEntries ; Cnt ++)
    {
      GetComboEntry (&_EntriesWindow, &NEntry, Cnt);
      DBConfig.FieldsString[Cnt] = _PrintObjects[NEntry.Identifier]._ObjectIdent;
    }
    DBConfig.FieldsString[Cnt] = '\0';
    WriteConfig ();
  }
  DestroyComboWindow (&_ObjectsWindow, 0);
  DestroyComboWindow (&_EntriesWindow, 0);
  DestroyWindow (&CWindow);
}

static bool _PrintLine (char *Line)

/**********************************************************************************************************************************/
/* Pre   : `Line' points to the line to be sent to the printer.                                                                   */
/* Post  : The line has been sent, possibly after dealing with printer errors. The return value is FALSE if the user canceled.    */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct ButtonInfo *Dummy;
  char               ErrorCode[128] = "\0";
  char              *CharP;
  word               PrinterCode;
  bool               Accepted;

  CharP = Line;
  while (*CharP)
  {
    Accepted = FALSE;
    while (!Accepted)
    {
      if (PrinterCode = ((_bios_printer (_PRINTER_WRITE, DBConfig.Printer - 1, (word)*CharP) & 0x00B8) ^ 0x0010))  /* Exclude ACK */
      {
        Accepted = FALSE;
        if (PrinterCode & 0x007F)                                                                                   /* Not BUSY ? */
        {
          sprintf (ErrorCode, "{BPRINTER ERROR:}B");
          if (PrinterCode & 0x0001) strcat (ErrorCode, "\n- Printer timed out");
          if (PrinterCode & 0x0002) strcat (ErrorCode, "\n- Unknown error 1");
          if (PrinterCode & 0x0004) strcat (ErrorCode, "\n- Unknown error 2");
          if (PrinterCode & 0x0008) strcat (ErrorCode, "\n- I/O error or printer offline");
          if (PrinterCode & 0x0010) strcat (ErrorCode, "\n- Printer can not be selected");                          /* (Inverted) */
          if (PrinterCode & 0x0020) strcat (ErrorCode, "\n- Out of paper");
          if (PrinterCode & 0x0080) strcat (ErrorCode, "\n- Printer busy");
          strcat (ErrorCode, "\n\nRetry the operation ?");
          if (QuestionRequester (-1, -1, FALSE, FALSE, "@Retry", "@Cancel", NULL, NULL, NULL, "Printer Error", ErrorCode))
            return (FALSE);
        }
      }
      else
        Accepted = TRUE;
      if (HandleWindow (&_PReq.PReqWindow, &Dummy) == WINH_PNOHAND)                            /* The Cancel button was pressed ? */
        return (FALSE);                                                                                               /* Step out */
      MouseStatus ();
    }
    CharP ++;
  }
  return (TRUE);
}

static void _DestroyPReq (void)

/**********************************************************************************************************************************/
/* Pre   : The percentage requester is initialized.                                                                               */
/* Post  : The percentage requester has been removed.                                                                             */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  if (!NoPercentageBar)
    DestroyPRequester (&_PReq);
}

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE EXPORTED LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

bool ReadDatabase (bool Merge, bool RetainConvertFlags, bool UpdateButton)

/**********************************************************************************************************************************/
/* Pre   : `Merge' is FALSE if the database is currently empty, TRUE if the database should be added in memory.                   */
/*         `UpdateButton' is TRUE if the 'database changed' button should be updated on screen as well.                           */
/* Post  : The spectrum games database has been read into extended memory. If the database was found corrupt, FALSE is returned,  */
/*         otherwise TRUE.                                                                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
#define DBRDCHUNK 32                                                                           /* Read 32 entries (4Kb) at a time */
  FILE                  *Fp;
  struct DBaseEntry_s    Entry[DBRDCHUNK];
  int                    ENum;
  int   register         RCnt;
  int   register         Cnt;
  word                   Seperator;
  dword                  FileLength;
  dword register         Index     = 0;
  char                   SticksBuffer[D_STICKS];
  char                   Buffer[D_PCNAME + 1];
  bool                   AllDone   = FALSE;
  bool                   FirstLine = TRUE;
  comboent               ChunkStart;

  CancelAutoConvert = FALSE;
  if (DatabaseName[1] == ':' && IsFloppy (DatabaseName[0]))
  {
    ReadFloppyId (DatabaseName[0], TRUE);
    DatabaseFloppyId = CurrentFloppyId;
  }
  if ((Fp = fopen (DatabaseName, "rt")) == NULL)
    return (TRUE);
  if (!RetainConvertFlags)
  {
    _SeperatorLength = 1;
    Convert107 = Convert109 = Convert117 = Convert121 = Convert129 = Convert130 = Convert136 = Convert202 = ConvertImprove = FALSE;
  }
  else if (Convert107 || Convert109)
    _SeperatorLength = 2;                                                                             /* Old format used 2 spaces */
  DBConfig.BackupDatabase = DBConfig.AlwaysBackupDatabase;
  if (!NoPercentageBar)
    MakePRequester (&_PReq, -1, -1, FALSE, NULL, "Reading Database",
                    "%s database into extended memory ...", Merge ? "Adding" : "Reading");
  fseek (Fp, 0L, SEEK_END);
  FileLength = ftell (Fp);
  fseek (Fp, 0L, SEEK_SET);
  Buffer[D_PCNAME] = '\0';
  ChunkStart = DBase.NumEntries + 1;                                                                 /* Determine insertion point */
  while (!AllDone)
  {
    ENum = -1;
    if (DBase.NumEntries == -1)
      XMSAlloc (&DBase.XMSHandle, DBLEN * DBRDCHUNK);
    else
      XMSRealloc (DBase.XMSHandle, DBLEN * (DBase.NumEntries + DBRDCHUNK), TRUE);
    memset (&Entry[0], 0, (word)(DBLEN * DBRDCHUNK));
    while (++ ENum < DBRDCHUNK && Index < FileLength)
    {
      if (++ DBase.NumEntries > MAXENTRIES)
        if (QuestionRequester (-1, -1, FALSE, FALSE, "@Ok", "@Quit", NULL, NULL, NULL, "Confirm Truncate",
            "Cannot handle more than %d entries.\nThe database will be truncated. Continue ?", MAXENTRIES))
          FatalError (11, "Aborted");
        else
        {
          _DestroyPReq ();
          fclose (Fp);
          EntryChanged = Merge;
          SignalDBChanged (Merge);
          return (TRUE);
        }
      if (fread (Entry[ENum].Name, 1, D_NAME, Fp) == 0)                 { _DestroyPReq (); fclose (Fp); return (FALSE); }
      RCnt = D_NAME;
      while (-- RCnt >= 0 && Entry[ENum].Name[RCnt] == ' ')
        ;
      Entry[ENum].Name[++ RCnt] = '\0';
      if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)             { _DestroyPReq (); fclose (Fp); return (FALSE); }
      if (fread (Entry[ENum].Year, 1, D_YEAR, Fp) == 0)                 { _DestroyPReq (); fclose (Fp); return (FALSE); }
      RCnt = D_YEAR;
      while (-- RCnt >= 0 && Entry[ENum].Year[RCnt] == ' ')
        ;
      Entry[ENum].Year[++ RCnt] = '\0';
      if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)             { _DestroyPReq (); fclose (Fp); return (FALSE); }
      if (fread (Entry[ENum].Publisher, 1, D_PUBLISHER, Fp) == 0)       { _DestroyPReq (); fclose (Fp); return (FALSE); }
      RCnt = D_PUBLISHER;
      while (-- RCnt >= 0 && Entry[ENum].Publisher[RCnt] == ' ')
        ;
      Entry[ENum].Publisher[++ RCnt] = '\0';
      if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)             { _DestroyPReq (); fclose (Fp); return (FALSE); }
      if (fread (Buffer, 1, D_MEMORY, Fp) == 0)                         { _DestroyPReq (); fclose (Fp); return (FALSE); }
      RCnt = D_MEMORY;
      while (-- RCnt >= 0 && Buffer[RCnt] == ' ')
        ;
      Buffer[++ RCnt] = '\0';
      if      (!strcmp (Buffer, "128"))  Entry[ENum].Memory = 128;
      else if (!strcmp (Buffer, "129"))  Entry[ENum].Memory = 129;
      else if (!strcmp (Buffer, "US0"))  Entry[ENum].Memory = 129;
      else if (!strcmp (Buffer, " +2"))  Entry[ENum].Memory = 130;
      else if (!strcmp (Buffer, " +3"))  Entry[ENum].Memory = 131;
      else if (!strcmp (Buffer, "Pen"))  Entry[ENum].Memory = 228;
      else if (!strcmp (Buffer, "4/1"))  Entry[ENum].Memory = 176;
      else if (!strcmp (Buffer, " 16"))  Entry[ENum].Memory = 16;
      else                               Entry[ENum].Memory = 48;
      if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)             { _DestroyPReq (); fclose (Fp); return (FALSE); }
      if (fread (Entry[ENum].Players, 1, D_PLAYERS, Fp) == 0)           { _DestroyPReq (); fclose (Fp); return (FALSE); }
      if (Entry[ENum].Players[0] == ' ')
        Entry[ENum].Players[0] = '1';                                                                      /* Default to 1 player */
      if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)             { _DestroyPReq (); fclose (Fp); return (FALSE); }
      if (fread (&Entry[ENum].Together, 1, 1, Fp) == 0)                 { _DestroyPReq (); fclose (Fp); return (FALSE); }
      if (Entry[ENum].Players[0] == '1')
        Entry[ENum].Together = FALSE;
      else
        Entry[ENum].Together = (Entry[ENum].Together == 'Y' ? TRUE : FALSE);
      if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)             { _DestroyPReq (); fclose (Fp); return (FALSE); }
      if (fread (SticksBuffer, 1, D_STICKS, Fp) == 0)                   { _DestroyPReq (); fclose (Fp); return (FALSE); }
      for (RCnt = 0 ; RCnt < D_STICKS ; RCnt ++)
        switch (SticksBuffer[RCnt])
        {
          case 'K' : Entry[ENum].Sticks[0] = TRUE; break;
          case '1' : Entry[ENum].Sticks[1] = TRUE; break;
          case '2' : Entry[ENum].Sticks[2] = TRUE; break;
          case 'C' : Entry[ENum].Sticks[3] = TRUE; break;
          case 'R' : Entry[ENum].Sticks[4] = TRUE; break;
        }
      if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)             { _DestroyPReq (); fclose (Fp); return (FALSE); }
      if (fread (Buffer, 1, D_PCNAME, Fp) == 0)                         { _DestroyPReq (); fclose (Fp); return (FALSE); }
      for (Cnt = 0 ; Cnt < D_PCNAME && Buffer[Cnt] != ' ' ; Cnt ++)
        Entry[ENum].PCName[Cnt] = Buffer[Cnt];
      RCnt = Cnt;
      for ( ; Cnt < D_PCNAME && Buffer[Cnt] != '.' ; Cnt ++)
        ;
      for ( ; Cnt < D_PCNAME && Buffer[Cnt] != ' ' ; Cnt ++, RCnt ++)
        Entry[ENum].PCName[RCnt] = Buffer[Cnt];
      Entry[ENum].PCName[RCnt] = '\0';
      if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)             { _DestroyPReq (); fclose (Fp); return (FALSE); }
      if (Convert107)
      {
        RCnt = -1;
        do
        {
          if (fread (&Entry[ENum].Type[++ RCnt], 1, 1, Fp) == 0)        { _DestroyPReq (); fclose (Fp); return (FALSE); }
        } while (RCnt < D_TYPE && Entry[ENum].Type[RCnt] != '\n');
        Entry[ENum].Type[RCnt] = '\0';
        Entry[ENum].DirIndex[0] = '\0';
        Entry[ENum].FileSize[0] = '\0';
        Entry[ENum].OrigScreen = FALSE;
        Entry[ENum].Origin = ' ';
        Entry[ENum].DiskId = 0;
        Entry[ENum].OverrideEmulator = ET_NONE;
        Entry[ENum].AYSound = FALSE;
        Entry[ENum].MultiLoad = 0;
        strcpy (Entry[ENum].Language, "Eng");
        Entry[ENum].Score = 0;
        Entry[ENum].Author[0] = '\0';
      }
      else
      {
        if (fread (Entry[ENum].Type, 1, D_TYPE, Fp) == 0)               { _DestroyPReq (); fclose (Fp); return (FALSE); }
        RCnt = D_TYPE;
        while (-- RCnt >= 0 && Entry[ENum].Type[RCnt] == ' ')
          ;
        Entry[ENum].Type[++ RCnt] = '\0';
        if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)           { _DestroyPReq (); fclose (Fp); return (FALSE); }
        if (Convert109)
        {
          RCnt = -1;
          do
          {
            if (fread (&Entry[ENum].DirIndex[++ RCnt], 1, 1, Fp) == 0)  { _DestroyPReq (); fclose (Fp); return (FALSE); }
          } while (RCnt < D_DIRINDEX && Entry[ENum].DirIndex[RCnt] != '\n');
          Seperator = Entry[ENum].DirIndex[RCnt];
          Entry[ENum].DirIndex[RCnt] = '\0';
          Entry[ENum].FileSize[0] = '\0';
          Entry[ENum].OrigScreen = FALSE;
          Entry[ENum].Origin = ' ';
          Entry[ENum].DiskId = 0;
          Entry[ENum].OverrideEmulator = ET_NONE;
          Entry[ENum].AYSound = FALSE;
          Entry[ENum].MultiLoad = 0;
          strcpy (Entry[ENum].Language, "Eng");
          Entry[ENum].Score = 0;
          Entry[ENum].Author[0] = '\0';
        }
        else
        {
          if (fread (Entry[ENum].DirIndex, 1, D_DIRINDEX, Fp) == 0)     { _DestroyPReq (); fclose (Fp); return (FALSE); }
          RCnt = D_DIRINDEX;
          while (-- RCnt >= 0 && Entry[ENum].DirIndex[RCnt] == ' ')
            ;
          Entry[ENum].DirIndex[++ RCnt] = '\0';
          if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)         { _DestroyPReq (); fclose (Fp); return (FALSE); }
          if (fread (Entry[ENum].FileSize, 1, D_FILESIZE, Fp) == 0)     { _DestroyPReq (); fclose (Fp); return (FALSE); }
          if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)         { _DestroyPReq (); fclose (Fp); return (FALSE); }
          if (fread (&Entry[ENum].OrigScreen, 1, 1, Fp) == 0)           { _DestroyPReq (); fclose (Fp); return (FALSE); }
          Entry[ENum].OrigScreen = (Entry[ENum].OrigScreen == 'Y' ? TRUE : FALSE);
          if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)         { _DestroyPReq (); fclose (Fp); return (FALSE); }
          if (fread (&Entry[ENum].Origin, 1, 1, Fp) == 0)               { _DestroyPReq (); fclose (Fp); return (FALSE); }
          if (Entry[ENum].Origin == 'Y')                                                /* Convert improveable to 'Bugged' origin */
          {
            Entry[ENum].Origin = 'B';
            ConvertImprove = TRUE;
          }
          if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)         { _DestroyPReq (); fclose (Fp); return (FALSE); }
          if (FirstLine && (char)Seperator == '\n' &&
              !Convert117 && !Convert121)
          {
            if (QuestionRequester (-1, -1, FALSE, FALSE, "@Ok", "@Cancel", NULL, NULL, NULL, "Confirm Auto-Conversion",
                "Your database %s\n"
                "seems to have format version 1.17 or below.\n"
                "It will be converted to version 1.36.", DatabaseName))
            { _DestroyPReq (); fclose (Fp); DBase.NumEntries --; CancelAutoConvert = TRUE; return (TRUE); }
            else
              Convert117 = TRUE;
          }
          if (Convert117)
          {
            Entry[ENum].DiskId = 0;
            Entry[ENum].OverrideEmulator = ET_NONE;
            Entry[ENum].AYSound = FALSE;
            Entry[ENum].MultiLoad = 0;
            strcpy (Entry[ENum].Language, "Eng");
            Entry[ENum].Score = 0;
            Entry[ENum].Author[0] = '\0';
          }
          else
          {
            if (fread (Buffer, 1, D_DISKID, Fp) == 0)                   { _DestroyPReq (); fclose (Fp); return (FALSE); }
            Buffer[D_DISKID] = '\0';
            Entry[ENum].DiskId = atoi (Buffer);
            if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)       { _DestroyPReq (); fclose (Fp); return (FALSE); }
            if (FirstLine && (char)Seperator == '\n' &&
                !Convert117 && !Convert121)
            {
              if (QuestionRequester (-1, -1, FALSE, FALSE, "@Ok", "@Cancel", NULL, NULL, NULL, "Confirm Auto-Conversion",
                  "Your database %s\n"
                  "seems to have format version 1.21 or below.\n"
                  "It will be converted to version 1.36.", DatabaseName))
              { _DestroyPReq (); fclose (Fp); DBase.NumEntries --; CancelAutoConvert = TRUE; return (TRUE); }
              else
                Convert121 = TRUE;
            }
            if (Convert121)
            {
              Entry[ENum].OverrideEmulator = ET_NONE;
              Entry[ENum].AYSound = FALSE;
              Entry[ENum].MultiLoad = 0;
              strcpy (Entry[ENum].Language, "Eng");
              Entry[ENum].Score = 0;
              Entry[ENum].Author[0] = '\0';
            }
            else
            {
              if (fread (Buffer, 1, D_OVERRD, Fp) == 0)                 { _DestroyPReq (); fclose (Fp); return (FALSE); }
              Buffer[D_OVERRD] = '\0';
              Entry[ENum].OverrideEmulator = atoi (Buffer);
              if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)     { _DestroyPReq (); fclose (Fp); return (FALSE); }
              if (FirstLine && (char)Seperator == '\n' &&
                  !Convert117 && !Convert121 && !Convert129 && !Convert130 && !Convert136 && !Convert202)
              {
                if (QuestionRequester (-1, -1, FALSE, FALSE, "@Ok", "@Cancel", NULL, NULL, NULL, "Confirm Auto-Conversion",
                    "Your database %s\n"
                    "seems to have format version 1.29 or below.\n"
                    "It will be converted to version 1.36.", DatabaseName))
                { _DestroyPReq (); fclose (Fp); DBase.NumEntries --; CancelAutoConvert = TRUE; return (TRUE); }
                else
                  Convert129 = TRUE;
              }
              if (Convert129)
              {
                Entry[ENum].AYSound = FALSE;
                Entry[ENum].MultiLoad = 0;
                strcpy (Entry[ENum].Language, "Eng");
                Entry[ENum].Score = 0;
                Entry[ENum].Author[0] = '\0';
              }
              else
              {
                if (fread (&Entry[ENum].AYSound, 1, 1, Fp) == 0)        { _DestroyPReq (); fclose (Fp); return (FALSE); }
                Entry[ENum].AYSound = (Entry[ENum].AYSound == 'Y' ? TRUE : FALSE);
                if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)   { _DestroyPReq (); fclose (Fp); return (FALSE); }
                if (fread (&Entry[ENum].MultiLoad, 1, 1, Fp) == 0)      { _DestroyPReq (); fclose (Fp); return (FALSE); }
                if (Entry[ENum].MultiLoad == 'Y')
                  Entry[ENum].MultiLoad = 1;
                else if (Entry[ENum].MultiLoad == '4')
                  Entry[ENum].MultiLoad = 4;
                else
                  Entry[ENum].MultiLoad = 0;
                if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)   { _DestroyPReq (); fclose (Fp); return (FALSE); }
                if (FirstLine && (char)Seperator == '\n' &&
                    !Convert117 && !Convert121 && !Convert129 && !Convert130 && !Convert136 && !Convert202)
                {
                  if (QuestionRequester (-1, -1, FALSE, FALSE, "@Ok", "@Cancel", NULL, NULL, NULL, "Confirm Auto-Conversion",
                      "Your database %s\n"
                      "seems to have format version 1.30 or below.\n"
                      "It will be converted to version 1.36.", DatabaseName))
                  { _DestroyPReq (); fclose (Fp); DBase.NumEntries --; CancelAutoConvert = TRUE; return (TRUE); }
                  else
                    Convert130 = TRUE;
                }
                if (Convert130)
                {
                  strcpy (Entry[ENum].Language, "Eng");
                  Entry[ENum].Score = 0;
                  Entry[ENum].Author[0] = '\0';
                }
                else
                {
                  if (fread (&Entry[ENum].Language, 1, D_LANGUAGE, Fp) == 0)  { _DestroyPReq (); fclose (Fp); return (FALSE); }
                  Entry[ENum].Language[D_LANGUAGE] = '\0';
                  if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)       { _DestroyPReq (); fclose (Fp); return (FALSE); }
                  if (FirstLine && (char)Seperator == '\n' &&
                      !Convert117 && !Convert121 && !Convert129 && !Convert130 && !Convert136 && !Convert202)
                  {
                    if (QuestionRequester (-1, -1, FALSE, FALSE, "@Ok", "@Cancel", NULL, NULL, NULL, "Confirm Auto-Conversion",
                        "Your database %s\n"
                        "seems to have format version 1.35 or below.\n"
                        "It will be converted to version 1.36.", DatabaseName))
                    { _DestroyPReq (); fclose (Fp); DBase.NumEntries --; CancelAutoConvert = TRUE; return (TRUE); }
                    else
                      Convert136 = TRUE;
                  }
                  if (Convert136)
                  {
                    Entry[ENum].Score = 0;
                    Entry[ENum].Author[0] = '\0';
                  }
                  else
                  {
                    if (fread (Buffer, 1, D_SCORE, Fp) == 0)                    { _DestroyPReq (); fclose (Fp); return (FALSE); }
                    Buffer[D_SCORE] = '\0';
                    Entry[ENum].Score = atoi (Buffer);
                    if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)       { _DestroyPReq (); fclose (Fp); return (FALSE); }
                    if (FirstLine && (char)Seperator == '\n' &&
                        !Convert117 && !Convert121 && !Convert129 && !Convert130 && !Convert136 && !Convert202)
                    {
                      if (QuestionRequester (-1, -1, FALSE, FALSE, "@Ok", "@Cancel", NULL, NULL, NULL, "Confirm Auto-Conversion",
                          "Your database %s\n"
                          "seems to have format version 2.01 or below.\n"
                          "It will be converted to version 2.02.", DatabaseName))
                      { _DestroyPReq (); fclose (Fp); DBase.NumEntries --; CancelAutoConvert = TRUE; return (TRUE); }
                      else
                        Convert202 = TRUE;
                    }
                    if (Convert202)
                      Entry[ENum].Author[0] = '\0';
                    else
                    {
                      if (fread (&Entry[ENum].Author, 1, D_AUTHOR, Fp) == 0)     { _DestroyPReq (); fclose (Fp); return (FALSE); }
                      RCnt = D_AUTHOR;
                      while (-- RCnt >= 0 && Entry[ENum].Author[RCnt] == ' ')
                        ;
                      Entry[ENum].Author[++ RCnt] = '\0';
                      if (fread (&Seperator, 1, _SeperatorLength, Fp) == 0)      { _DestroyPReq (); fclose (Fp); return (FALSE); }
                    }
                  }
                }
              }
            }
          }
        }
        if ((char)Seperator != '\n')                                 { _DestroyPReq (); fclose (Fp); return (FALSE); }
      }
      Index += D_NAME + D_YEAR + D_PUBLISHER + D_MEMORY + D_PLAYERS + D_STICKS + D_PCNAME + _SeperatorLength * 8 + 3;
      if (!Convert107)
        if (!Convert109)
          if (!Convert117)
            if (!Convert121)
              if (!Convert129)
                if (!Convert130)
                  if (!Convert136)
                    if (!Convert202)
                      Index += D_DIRINDEX + D_FILESIZE + D_TYPE + D_DISKID + D_OVERRD + D_LANGUAGE + D_SCORE + D_AUTHOR + _SeperatorLength + 14;
                    else
                      Index += D_DIRINDEX + D_FILESIZE + D_TYPE + D_DISKID + D_OVERRD + D_LANGUAGE + D_SCORE + _SeperatorLength + 13;
                  else
                    Index += D_DIRINDEX + D_FILESIZE + D_TYPE + D_DISKID + D_OVERRD + D_LANGUAGE + _SeperatorLength + 12;
                else
                  Index += D_DIRINDEX + D_FILESIZE + D_TYPE + D_DISKID + D_OVERRD + _SeperatorLength + 11;
              else
                Index += D_DIRINDEX + D_FILESIZE + D_TYPE + D_DISKID + D_OVERRD + _SeperatorLength + 7;
            else
              Index += D_DIRINDEX + D_FILESIZE + D_TYPE + D_DISKID + _SeperatorLength + 6;
          else
            Index += D_DIRINDEX + D_FILESIZE + D_TYPE + _SeperatorLength + 5;
        else
          Index += D_TYPE + RCnt + _SeperatorLength;
      else
        Index += RCnt;
      FirstLine = FALSE;
    }
    EntryChanged = TRUE;
    PutDBEntry (&Entry[0], DBLEN * ENum, ChunkStart);
    ChunkStart += ENum;
    if (!NoPercentageBar)
    {
      UpdatePRequester (&_PReq, (double)Index * 100 / FileLength, NULL);
      MouseStatus ();
    }
    AllDone = (bool)(Index >= FileLength ? TRUE : FALSE);
  }
  if (fclose (Fp) != 0)
    FatalError (10, "Could not close database: %s", strerror (errno));
  _DestroyPReq ();
  EntryChanged = FALSE;
  if (Convert107 || Convert109 || Convert117 || Convert121 || Convert129 || Convert130 || Convert136 || Convert202 ||
      ConvertImprove)
    DatabaseChanged = TRUE;
  else if (UpdateButton)
    SignalDBChanged (Merge);
  else
    DatabaseChanged = FALSE;
  return (TRUE);
}

void WriteDatabase (bool FatalSave, bool All)

/**********************************************************************************************************************************/
/* Pre   : `FatalSave' is TRUE in a FatalError handling. `All' is TRUE if the entire database should be saved, FALSE for only the */
/*         selected entries.                                                                                                      */
/* Post  : The spectrum games database has been written back to disk. Any errors stop the program.                                */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  FILE                  *Fp;
  struct ComboEntry      NEntry;
  struct DBaseEntry_s    Entry;
  int    register        Cnt;
  int    register        PCNCnt;
  char                   EntryLine[300];
  char                   Syntax[120];
  char                   SticksBuffer[D_STICKS + 1];
  char                   Buffer[D_PCNAME + 1];
  char                   SMemory[D_MEMORY + 1];
  bool                   Dummy = TRUE;
  int                    WriteCounter = 0;
  int                    TotalToWrite;
  int                    LastEntry;

  if (DatabaseName[1] == ':' && IsFloppy (DatabaseName[0]))
    if (!RequestFloppy (DatabaseName[0], DatabaseFloppyId))
      return;
  if (!NonexistingDatabase && !FatalSave && DBConfig.BackupDatabase)
  {
    DBConfig.BackupDatabase = FALSE;
    if (!CopyFile (DatabaseName, DatabaseBackupName, &DBConfig.BackupDatabase, &Dummy, "Making a database backup..."))
    {
      unlink (DatabaseBackupName);
      return;
    }
  }
  if (!FatalSave && !NoPercentageBar)
    MakePRequester (&_PReq, -1, -1, TRUE, NULL, "Writing Database", "Writing database ...");
  if ((Fp = fopen (FatalSave ? DBSAVE : DatabaseTemp, "wt")) == NULL)
    FatalError (10, "Cannot create new database file: %s", strerror (errno));
  if (FatalSave || Convert107 || Convert109 || Convert117 || Convert121 || Convert129 || Convert130 || Convert136 || Convert202)
    LastEntry = DBase.NumEntries;
  else
    LastEntry = SWindow->NumEntries;
  TotalToWrite = All ? LastEntry + 1 : NumMatches;
  for (Cnt = 0 ; Cnt <= LastEntry ; Cnt ++)
  {
    GetComboEntry (SWindow, &NEntry, Cnt);
    if (All || (NEntry.Selected & SELECTBIT))
    {
      GetDBEntry (&Entry, DBLEN, Cnt);
      if (Entry.Players[0] == '0' || Entry.Players[0] == '1')
        Entry.Together = ' ';
      else
        Entry.Together = (Entry.Together ? 'Y' : 'N');
      SticksBuffer[0] = '\0';
      if (Entry.Sticks[0])  strcat (SticksBuffer, "K");
      if (Entry.Sticks[1])  strcat (SticksBuffer, "1");
      if (Entry.Sticks[2])  strcat (SticksBuffer, "2");
      if (Entry.Sticks[3])  strcat (SticksBuffer, "C");
      if (Entry.Sticks[4])  strcat (SticksBuffer, "R");
      for (PCNCnt = 0 ; PCNCnt < strlen (Entry.PCName) && Entry.PCName[PCNCnt] != '.' ; PCNCnt ++)
        ;
      Entry.PCName[PCNCnt] = '\0';
      Entry.OrigScreen = (Entry.OrigScreen ? 'Y' : ' ');
      Entry.AYSound = (Entry.AYSound ? 'Y' : ' ');
      switch (Entry.MultiLoad)
      {
        case 1 : Entry.MultiLoad = 'Y'; break;
        case 4 : Entry.MultiLoad = '4'; break;
        default: Entry.MultiLoad = ' '; break;
      }
      switch (Entry.Memory)
      {
        case 128 : strcpy (SMemory, "128"); break;
        case 129 : strcpy (SMemory, "US0"); break;
        case 176 : strcpy (SMemory, "4/1"); break;
        case 130 : strcpy (SMemory, " +2"); break;
        case 131 : strcpy (SMemory, " +3"); break;
        case 228 : strcpy (SMemory, "Pen"); break;
        case  16 : strcpy (SMemory, " 16"); break;
        default  : strcpy (SMemory, " 48"); break;
      }
      if (Entry.Origin == 0)   Entry.Origin = ' ';                                                              /* BUG in v2.02b4 */
      sprintf (Buffer, "%-8s.%-3s", Entry.PCName, Entry.PCName + PCNCnt + 1);
      sprintf (Syntax, "%%-%ds %%-%ds %%-%ds %%-%ds %%-%ds %%c %%-%ds %%-%ds %%-%ds %%-%ds %%-%ds %%c %%c %%%dd %%%dd %%c %%c %%-%ds %%%dd %%-%ds\n",
               D_NAME, D_YEAR, D_PUBLISHER, D_MEMORY, D_PLAYERS, D_STICKS, D_PCNAME, D_TYPE, D_DIRINDEX, D_FILESIZE, D_DISKID,
               D_OVERRD, D_LANGUAGE, D_SCORE, D_AUTHOR);
      sprintf (EntryLine, Syntax, Entry.Name, Entry.Year, Entry.Publisher, SMemory, Entry.Players, Entry.Together,
               SticksBuffer, Buffer, Entry.Type, Entry.DirIndex, Entry.FileSize, Entry.OrigScreen, Entry.Origin, Entry.DiskId,
               Entry.OverrideEmulator, Entry.AYSound, Entry.MultiLoad, Entry.Language, Entry.Score, Entry.Author);
      if (fwrite (EntryLine, 1, strlen (EntryLine), Fp) == 0)
        FatalError (10, "Error writing to database: %s", strerror (errno));
      if (!FatalSave && !NoPercentageBar)
        if (!UpdatePRequester (&_PReq, (double)(++ WriteCounter) * 100 / TotalToWrite, NULL))                      /* Cancelled ? */
        { fclose (Fp); unlink (DatabaseTemp); _DestroyPReq (); return; }
    }
    MouseStatus ();
  }
  if (fclose (Fp) == -1)
    FatalError (10, "Could not close database: %s", strerror (errno));
  if (!FatalSave)
  {
    if (unlink (DatabaseName) == -1 && errno != ENOENT)
      FatalError (10, "Could not remove old database: %s", strerror (errno));
    if (rename (DatabaseTemp, DatabaseName) == -1)
      FatalError (10, "Could not install new database: %s", strerror (errno));
    SignalDBChanged (FALSE);
    _DestroyPReq ();
  }
  NonexistingDatabase = FALSE;                                                                   /* Signal: database exists (now) */
  NewDatabase = FALSE;
  UnghostMenuBackup ();
}

void ReleaseDatabase (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The spectrum games database has been released from XMS.                                                                */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  if (DBase.NumEntries >= 0)                                                                    /* Cleanup any remaining database */
    XMSFree (DBase.XMSHandle);
  DBase.NumEntries = -1;
  EntryChanged = FALSE;
  SignalDBChanged (FALSE);
}

bool ReloadDatabase (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The spectrum games database has been reloaded from disk.                                                               */
/* Import: ReleaseDatabase, ReadDatabase.                                                                                         */
/**********************************************************************************************************************************/

{
  bool Result;

  if (DatabaseName[1] == ':' && IsFloppy (DatabaseName[0]))
    if (!RequestFloppy (DatabaseName[0], DatabaseFloppyId))
      return (TRUE);
  ReleaseDatabase ();                                                                           /* Cleanup any remaining database */
  Result = ReadDatabase (FALSE, FALSE, TRUE);
  NonexistingDatabase = (DBase.NumEntries == -1 ? TRUE : FALSE);                                  /* New (nonexisting) database ? */
  if (NonexistingDatabase)
    GhostMenuBackup ();
  else
    UnghostMenuBackup ();
  if (Convert107 || Convert109 || Convert117 || Convert121 || Convert130 || Convert136)
    SignalDBChanged (TRUE);
  return (Result);
}

void PrintDatabase (bool All)

/**********************************************************************************************************************************/
/* Pre   : `All' is TRUE if the entire database should be printed, FALSE for only the selected entries.                           */
/* Post  : The spectrum games database has been printed on the printer.                                                           */
/* Import: _PrintLine, _EditPrintObjects.                                                                                         */
/**********************************************************************************************************************************/

{
  int    register        PCNCnt;
  int                    LineLength;                                                                  /* Maximum length of a line */
  int                    Fh;                                                                      /* Used when printing to a file */
  char                   Syntax[160];
  char                   SticksBuffer[D_STICKS + 1];
  char                   Buffer[D_PCNAME + 1];
  byte   register        LCnt = 0;                                                                          /* Lines in each page */
  byte   register        PCnt = 0;                                                                                /* Current page */
  word   register        ECnt = 0;                                                                               /* Entry counter */
  struct DBaseEntry_s    Entry;
  struct ComboEntry      NEntry;
  struct stat            FileStatus;
  char                   OneLine[256];
  char                   FileTime[80];
  char                   FileName[MAXDIR];
  char                   Drive[_MAX_DRIVE];
  char                   Ext[_MAX_EXT];
  char                   UsedGameName[D_NAME + 1];
  char                   DiskIdBuffer[5];
  char                   EmulBuffer[5];
  char                   SMemory[D_MEMORY + 1];
  int                    NameI;
  char                  *NameP;
  bool                   Again = TRUE;
  char                  *FString;
  bool                   AddSpace;
  
  if (DBConfig.Printer > 3)
    FatalError (11, "Printer LPT%d does not exist!", DBConfig.Printer);
  while (Again)
  {
    if (DBConfig.Printer == 0)                                                                            /* Printing to a file ? */
    {
      _splitpath (DatabaseName, Drive, OneLine, FileTime, Ext);
      _makepath (FileName, Drive, OneLine, FileTime, DBPRFEXT);
      switch (QuestionRequester (-1, -1, TRUE, FALSE, "@Start", "@Cancel", "S@etup", NULL, NULL, "Confirm Print Database",
                             "{BPRINT NOTE:}B\n\nAbout to print %sthe database to the printfile\n%s\nContinue?",
                             All ? "" : "the selected part of ", FileName))
      {
        case 0 : Again = FALSE; break;
        case 1 : return;
        case 2 : _EditPrintObjects (); break;
      }
      if (!Again)
        if ((Fh = open (FileName, O_BINARY | O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1)
        {
          ErrorRequester (-1, -1, NULL, NULL, NULL, "Cannot create printfile: %s", strerror (errno));
          return;
        }
    }
    else
      switch (QuestionRequester (-1, -1, TRUE, FALSE, "@Start", "@Cancel", "S@etup", NULL, NULL, NULL, "Confirm Print Database",
                             "{BPRINT NOTE:}B\n\nAbout to print %sthe database.\nPlease make sure that the printer is online.",
                             All ? "" : "the selected part of "))
      {
        case 0 : Again = FALSE; break;
        case 1 : return;
        case 2 : _EditPrintObjects (); break;
      }
  }
  if (stat (DatabaseName, &FileStatus))                                                    /* Get modification status of the file */
    FatalError (12, "Cannot get the status of the database");
  sprintf (FileTime, "%s - %s", DatabaseName, ctime (&FileStatus.st_atime));                  /* Store the last modification time */
  FileTime[strlen (FileTime) - 1] = '\0';                                                      /* (Remove the NewLine at the end) */
  MakePRequester (&_PReq, -1, -1, TRUE, NULL, "Printing Database", "Printing the database ...");
  while (ECnt <= SWindow->NumEntries)
  {
    GetComboEntry (SWindow, &NEntry, ECnt);
    if (All || (NEntry.Selected & SELECTBIT))
    {
      if (LCnt == 0)
      {
        AddSpace = FALSE;
        FString = DBConfig.FieldsString;
        OneLine[0] = '\0';
        while (*FString)
        {
          if (AddSpace)
            strcat (OneLine, " ");
          switch (*FString)
          {
            case 'A' : sprintf (OneLine + strlen (OneLine), "%-36s", "Name"); break;
            case 'B' : sprintf (OneLine + strlen (OneLine), "%-4s",  "Year"); break;
            case 'C' : sprintf (OneLine + strlen (OneLine), "%-36s", "Publisher"); break;
            case 'D' : sprintf (OneLine + strlen (OneLine), "%-3s",  "Mem"); break;
            case 'E' : sprintf (OneLine + strlen (OneLine), "%-1s",  "P"); break;
            case 'F' : sprintf (OneLine + strlen (OneLine), "%-1s",  "T"); break;
            case 'G' : sprintf (OneLine + strlen (OneLine), "%-6s",  "Sticks"); break;
            case 'H' : sprintf (OneLine + strlen (OneLine), "%-12s", "PC-Name"); break;
            case 'I' : sprintf (OneLine + strlen (OneLine), "%-7s",  "Type"); break;
            case 'J' : sprintf (OneLine + strlen (OneLine), "%-7s",  "Size"); break;
            case 'K' : sprintf (OneLine + strlen (OneLine), "%-1s",  "S"); break;
            case 'L' : sprintf (OneLine + strlen (OneLine), "%-1s",  "I"); break;
            case 'M' : sprintf (OneLine + strlen (OneLine), "%-3s",  "Dsk"); break;
            case 'N' : sprintf (OneLine + strlen (OneLine), "%-4s",  "Emul"); break;
            case 'O' : sprintf (OneLine + strlen (OneLine), "%-1s",  "A"); break;
            case 'P' : sprintf (OneLine + strlen (OneLine), "%-1s",  "M"); break;
            case 'Q' : sprintf (OneLine + strlen (OneLine), "%-3s",  "Lan"); break;
            case 'R' : sprintf (OneLine + strlen (OneLine), "%-3s",  "Sco"); break;
            case 'S' : sprintf (OneLine + strlen (OneLine), "%-100s","Author(s)"); break;
          }
          FString ++;
          AddSpace = TRUE;
        }
        sprintf (OneLine + strlen (OneLine), "%c%c%c", C_CR, C_LF, C_LF);
        LineLength = strlen (OneLine);
        if (DBConfig.Printer == 0)
        {
          if (write (Fh, OneLine, LineLength) == -1)
          { ErrorRequester (-1, -1, NULL, NULL, NULL, "Error writing to printfile: %s", strerror (errno));
            close (Fh); DestroyPRequester (&_PReq); return; }
        }
        else
          if (!_PrintLine (OneLine))
          { DestroyPRequester (&_PReq); return; }
        LCnt = 3;
      }
      if (++ LCnt == DBConfig.LinesPerPage)
      {
        sprintf (Syntax, "%c%cPage %%-3d %%%ds%c%c", C_CR, C_LF, LineLength - 12, C_CR, C_FF);    /* Fileinfo at the right margin */
        sprintf (OneLine, Syntax, ++ PCnt, FileTime);
        if (DBConfig.Printer == 0)
        {
          if (write (Fh, OneLine, strlen (OneLine)) == -1)
          { ErrorRequester (-1, -1, NULL, NULL, NULL, "Error writing to printfile: %s", strerror (errno));
            close (Fh); DestroyPRequester (&_PReq); return; }
        }
        else
          if (!_PrintLine (OneLine))
          { DestroyPRequester (&_PReq); return; }
        LCnt = 0;
      }
      else
      {
        GetDBEntry (&Entry, DBLEN, ECnt);
        NameP = Entry.Name - 1;
        NameI = 0;
        while (*(++ NameP))                                                                   /* Filter out '{' and '}' sequences */
        {
          if (*NameP == '{')
            switch (*(++ NameP))
            {
              case '+' :
              case '-' : NameP ++; break;
              case 'B' :
              case 'T' :
              case 'U' :
              case 'I' : 
              case 'C' : break;
              case '{' :
              case '}' : UsedGameName[NameI ++] = *NameP;
                         break;
              default  : UsedGameName[NameI ++] = '{';
                         UsedGameName[NameI ++] = *NameP;
            }
          else if (*NameP == '}')
            switch (*(++ NameP))
            {
              case 'B' :
              case 'T' :
              case 'U' :
              case 'I' :
              case 'C' :
              case 'N' : break;
              case '{' :
              case '}' : UsedGameName[NameI ++] = *NameP;
                         break;
              default  : UsedGameName[NameI ++] = '}';
                         UsedGameName[NameI ++] = *NameP;
            }
          else
            UsedGameName[NameI ++] = *NameP;
        }
        UsedGameName[NameI] = '\0';
        if (Entry.Players[0] == '1')
          Entry.Together = ' ';
        else
          Entry.Together = (Entry.Together ? 'Y' : 'N');
        SticksBuffer[0] = '\0';
        if (Entry.Sticks[0])  strcat (SticksBuffer, "K");
        if (Entry.Sticks[1])  strcat (SticksBuffer, "1");
        if (Entry.Sticks[2])  strcat (SticksBuffer, "2");
        if (Entry.Sticks[3])  strcat (SticksBuffer, "C");
        if (Entry.Sticks[4])  strcat (SticksBuffer, "R");
        for (PCNCnt = 0 ; PCNCnt < strlen (Entry.PCName) && Entry.PCName[PCNCnt] != '.' ; PCNCnt ++)
          ;
        Entry.PCName[PCNCnt] = '\0';
        Entry.OrigScreen = (Entry.OrigScreen ? 'Y' : ' ');
        Entry.AYSound = (Entry.AYSound ? 'Y' : ' ');
        switch (Entry.MultiLoad)
        {
          case 1 : Entry.MultiLoad = 'Y'; break;
          case 4 : Entry.MultiLoad = '4'; break;
          default: Entry.MultiLoad = ' '; break;
        }
        sprintf (Buffer, "%-8s.%-3s", Entry.PCName, Entry.PCName + PCNCnt + 1);
        if (Entry.DiskId > 0)
          sprintf (DiskIdBuffer, "%-3d", Entry.DiskId);
        else
          DiskIdBuffer[0] = '\0';
        if (Entry.OverrideEmulator == ET_NONE)
          EmulBuffer[0] = '\0';
        else
          GetEmulatorName (Entry.OverrideEmulator, EmulBuffer);
        switch (Entry.Memory)
        {
          case 128 : strcpy (SMemory, "128"); break;
          case 129 : strcpy (SMemory, "US0"); break;
          case 176 : strcpy (SMemory, "4/1"); break;
          case 130 : strcpy (SMemory, " +2"); break;
          case 131 : strcpy (SMemory, " +3"); break;
          case 228 : strcpy (SMemory, "Pen"); break;
          case  16 : strcpy (SMemory, " 16"); break;
          default  : strcpy (SMemory, " 48"); break;
        }
        AddSpace = FALSE;
        FString = DBConfig.FieldsString;
        OneLine[0] = '\0';
        while (*FString)
        {
          if (AddSpace)
            strcat (OneLine, " ");
          switch (*FString)
          {
            case 'A' : sprintf (OneLine + strlen (OneLine), "%-36s", UsedGameName); break;
            case 'B' : sprintf (OneLine + strlen (OneLine), "%-4s",  Entry.Year); break;
            case 'C' : sprintf (OneLine + strlen (OneLine), "%-36s", Entry.Publisher); break;
            case 'D' : sprintf (OneLine + strlen (OneLine), "%-3s",  SMemory); break;
            case 'E' : sprintf (OneLine + strlen (OneLine), "%-1s",  Entry.Players); break;
            case 'F' : sprintf (OneLine + strlen (OneLine), "%c",    Entry.Together); break;
            case 'G' : sprintf (OneLine + strlen (OneLine), "%-6s",  SticksBuffer); break;
            case 'H' : sprintf (OneLine + strlen (OneLine), "%-12s", Buffer); break;
            case 'I' : sprintf (OneLine + strlen (OneLine), "%-7s",  Entry.Type); break;
            case 'J' : sprintf (OneLine + strlen (OneLine), "%7s",   Entry.FileSize); break;
            case 'K' : sprintf (OneLine + strlen (OneLine), "%c",    Entry.OrigScreen); break;
            case 'L' : sprintf (OneLine + strlen (OneLine), "%c",    Entry.Origin); break;
            case 'M' : sprintf (OneLine + strlen (OneLine), "%-3s",  DiskIdBuffer); break;
            case 'N' : sprintf (OneLine + strlen (OneLine), "%-4s",  EmulBuffer); break;
            case 'O' : sprintf (OneLine + strlen (OneLine), "%c",    Entry.AYSound); break;
            case 'P' : sprintf (OneLine + strlen (OneLine), "%c",    Entry.MultiLoad); break;
            case 'Q' : sprintf (OneLine + strlen (OneLine), "%-3s",  Entry.Language); break;
            case 'R' : sprintf (OneLine + strlen (OneLine), "%3d",   Entry.Score); break;
            case 'S' : sprintf (OneLine + strlen (OneLine), "%-100s",Entry.Author); break;
          }
          FString ++;
          AddSpace = TRUE;
        }
        sprintf (OneLine + strlen (OneLine), "%c%c", C_CR, C_LF);
        if (DBConfig.Printer == 0)
        {
          if (write (Fh, OneLine, strlen (OneLine)) == -1)
          { ErrorRequester (-1, -1, NULL, NULL, NULL, "Error writing to printfile: %s", strerror (errno));
            close (Fh); DestroyPRequester (&_PReq); return; }
        }
        else
          if (!_PrintLine (OneLine))
          { DestroyPRequester (&_PReq); return; }
        if (!UpdatePRequester (&_PReq, (double)ECnt * 100 / (SWindow->NumEntries + 1), NULL))                      /* Cancelled ? */
        { DestroyPRequester (&_PReq); return; }
        ECnt ++;
        MouseStatus ();
      }
    }
  }
  if (LCnt > 0)
  {
    sprintf (OneLine, "%c", 0x0A);
    while (++ LCnt < DBConfig.LinesPerPage)
      if (DBConfig.Printer == 0)
      {
        if (write (Fh, OneLine, strlen (OneLine)) == -1)
        { ErrorRequester (-1, -1, NULL, NULL, NULL, "Error writing to printfile: %s", strerror (errno));
          close (Fh); DestroyPRequester (&_PReq); return; }
      }
      else
        if (!_PrintLine (OneLine))
        { DestroyPRequester (&_PReq); return; }
    sprintf (Syntax, "%c%cPage %%-3d %%%ds%c%c", C_CR, C_LF, LineLength - 12, C_CR, C_FF);
    sprintf (OneLine, Syntax, ++ PCnt, FileTime);
    if (DBConfig.Printer == 0)
    {
      if (write (Fh, OneLine, strlen (OneLine)) == -1)
      { ErrorRequester (-1, -1, NULL, NULL, NULL, "Error writing to printfile: %s", strerror (errno));
        close (Fh); DestroyPRequester (&_PReq); return; }
    }
    else
      if (!_PrintLine (OneLine))
      { DestroyPRequester (&_PReq); return; }
  }
  if (DBConfig.Printer == 0)
    if (close (Fh) == -1)
      ErrorRequester (-1, -1, NULL, NULL, NULL, "Error closing printfile: %s", strerror (errno));
  DestroyPRequester (&_PReq);
}

void SaveDatabase (bool All)

/**********************************************************************************************************************************/
/* Pre   : `All' is TRUE if the entire database should be saved, FALSE if only the selected entries.                              */
/* Post  : The requested (part of the) database has been saved. If it was just created (new), a name is requested first.          */
/* Import: WriteDatabase.                                                                                                         */
/**********************************************************************************************************************************/

{
  if (NewDatabase)
    ForceNewDatabaseName (All);
  else
    WriteDatabase (FALSE, All);
}

void PutDBEntry (struct DBaseEntry_s *Entry, dword Length, comboent EntryNumber)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the entry that should be written to the table at location `EntryNumber'.                             */
/* Post  : The entry has been written if it had been changed.                                                                     */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  if (EntryChanged)                                                               /* Previously selected entry has been changed ? */
  {
    CopyToXMS (Length, DBase.XMSHandle, Entry, DBLEN * EntryNumber);                                             /* Write it back */
    EntryChanged = FALSE;                                                                    /* Signal: entry has not been edited */
  }
}

void GetDBEntry (struct DBaseEntry_s *Entry, dword Length, comboent EntryNumber)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the location where entry number `EntryNumber' should be retrieved.                                   */
/* Post  : The entry has been read.                                                                                               */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  CopyFromXMS (Length, DBase.XMSHandle, DBLEN * EntryNumber, Entry);
}

void ClearDBEntry (struct DBaseEntry_s *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the entry to be cleared.                                                                             */
/* Post  : The given entry has been cleared/defaulted.                                                                            */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  memset (Entry, 0, (word)DBLEN);
  strcpy (Entry->Players, "1");                                                                          /* Insert default fields */
  strcpy (Entry->Language, "Eng");
  Entry->Memory = 48;
  Entry->DiskId = 0;                                                                                           /* Assume harddisk */
  Entry->OverrideEmulator = ET_NONE;
  Entry->Origin = ' ';
}

void AddDBEntry (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : A (cleared/defaulted) entry has been added to the XMS database set.                                                    */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct DBaseEntry_s ClearedEntry;

  if (++ DBase.NumEntries == 0)
    XMSAlloc (&DBase.XMSHandle, DBLEN);
  else
    XMSRealloc (DBase.XMSHandle, DBLEN * (DBase.NumEntries + 1), TRUE);
  ClearDBEntry (&ClearedEntry);
  CopyToXMS (DBLEN, DBase.XMSHandle, &ClearedEntry, DBLEN * DBase.NumEntries);
  EntryChanged = TRUE;
  SignalDBChanged (TRUE);
}

bool DelDBEntry (comboent EntryNumber)

/**********************************************************************************************************************************/
/* Pre   : `EntryNumber' holds the entry to be removed.                                                                           */
/* Post  : The entry has been removed. If it did not exist or the database is now empty, TRUE is returned.                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  if (DBase.NumEntries == -1 || EntryNumber > DBase.NumEntries)
    return (TRUE);
  if (-- DBase.NumEntries == -1)
  {
    XMSFree (DBase.XMSHandle);
    return (TRUE);
  }
  if (DBase.NumEntries - EntryNumber + 1 > 0)
    CopyInXMS (DBLEN * (DBase.NumEntries - EntryNumber + 1), DBase.XMSHandle, DBLEN * (EntryNumber + 1), DBase.XMSHandle,
               DBLEN * EntryNumber);
  XMSRealloc (DBase.XMSHandle, DBLEN * (DBase.NumEntries + 1), FALSE);
  SignalDBChanged (TRUE);
  return (FALSE);
}

char *ShrinkString (char *OldString)

/**********************************************************************************************************************************/
/* Pre   : `OldString' points to the string to be shrinken.                                                                       */
/* Post  : The (static) string `_ShrinkenString' holds a copy of the `OldString' after removing all characters not in the ranges  */
/*         [A-Z], [a-z], [0-9]. A pointer to the `_ShrinkenString' has been returned.                                             */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  short register  Cnt = 0;
  char  register *StringWalk;

  StringWalk = OldString - 1;
  while (*(++ StringWalk))
  {
    if (*StringWalk == '{')
      switch (*(++ StringWalk))
      {
        case '+' :
        case '-' :
        case 'L' : StringWalk ++; break;
        case 'B' :
        case 'T' :
        case 'U' :
        case 'I' : 
        case 'C' :
        case '{' :
        case '}' : break;
        default  : if (isalnum (*StringWalk))
                     _ShrinkenString[Cnt ++] = tolower (*StringWalk);
      }
    else if (*StringWalk == '}')
      switch (*(++ StringWalk))
      {
        case 'B' :
        case 'T' :
        case 'U' :
        case 'I' :
        case 'C' :
        case 'L' :
        case 'N' :
        case '{' :
        case '}' : break;
        default  : if (!DBConfig.UsePhonebookSorting || isalnum (*StringWalk))
                     _ShrinkenString[Cnt ++] = tolower (*StringWalk);
      }
    else if (!DBConfig.UsePhonebookSorting || isalnum (*StringWalk))
      _ShrinkenString[Cnt ++] = tolower (*StringWalk);
  }
  _ShrinkenString[Cnt] = '\0';
  return (_ShrinkenString);
}

comboent SortEntry (struct ComboInfo *Window, struct DBaseEntry_s *Entry, comboent OldNumber)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the combo window where entry `Entry' should be put sorted. `OldNumber' is the old location.         */
/* Post  : The entry has been moved inside the entry table to the right location. The XMS database has been adjusted as well.     */
/* Import: ShrinkString.                                                                                                          */
/**********************************************************************************************************************************/

{
  struct ComboEntry   SortEntry;
  struct ComboEntry   NEntry;
  struct DBaseEntry_s DEntry;
  comboent register   Cnt;
  bool                Found = FALSE;
  char                ShrName[80];
  char                ShrName2[80];
  int                 Match;

  GetComboEntry (Window, &SortEntry, OldNumber);
  strcpy (ShrName, ShrinkString (Entry->Name));
  strcpy (ShrName2, ShrinkString (Entry->Publisher));
  for (Cnt = 0 ; !Found && Cnt <= Window->NumEntries ; Cnt ++)
    if (Cnt != OldNumber)
    {
      GetComboEntry (Window, &NEntry, Cnt);
      Match = stricmp (ShrinkString (NEntry.Text), ShrName);
      if (!Match)                                                                                           /* Titles are equal ? */
      {
        GetDBEntry (&DEntry, DBLEN, Cnt);
        Match = stricmp (ShrinkString (DEntry.Publisher), ShrName2);                                     /* Sort key 2: Publisher */
        if (!Match)                                                                        /* Publisher names are equal as well ? */
        {
          Match = strcmp (DEntry.Year, Entry->Year);                                                          /* Sort key 3: Year */
          if (!Match)                                                                                /* Years are equal as well ? */
          {
            if (DEntry.Memory == Entry->Memory)                                                      /* Sort key 4: Memory type ? */
            {                                                                                       /* Same memory type as well ? */
              Match = stricmp (DEntry.PCName, Entry->PCName);                                             /* Sort key 5: Filename */
              Found = (Match >= 0 ? TRUE : FALSE);
            }
            else
              Found = (DEntry.Memory > Entry->Memory ? TRUE : FALSE);
          }
          else
            Found = (Match > 0 ? TRUE : FALSE);
        }
        else
          Found = (Match > 0 ? TRUE : FALSE);
      }
      else
        Found = (Match >= 0 ? TRUE : FALSE);
    }
  if (Found)
    Cnt --;
  if (OldNumber < Cnt)                                                                                    /* Entry is moving up ? */
  {
    Cnt --;
    CopyInXMS ((dword)Window->XMSEntrySize * (Cnt - OldNumber),
               Window->TableXMSHandle, (dword)Window->XMSEntrySize * (OldNumber + 1),
               Window->TableXMSHandle, (dword)Window->XMSEntrySize * OldNumber);
    CopyInXMS (DBLEN * (Cnt - OldNumber), DBase.XMSHandle, DBLEN * (OldNumber + 1), DBase.XMSHandle, DBLEN * OldNumber);
  }
  else if (OldNumber > Cnt)                                                                             /* Entry is moving down ? */
  {
    CopyInXMS ((dword)Window->XMSEntrySize * (OldNumber - Cnt),
               Window->TableXMSHandle, (dword)Window->XMSEntrySize * Cnt,
               Window->TableXMSHandle, (dword)Window->XMSEntrySize * (Cnt + 1));
    CopyInXMS (DBLEN * (OldNumber - Cnt), DBase.XMSHandle, DBLEN * Cnt, DBase.XMSHandle, DBLEN * (Cnt + 1));
  }
  memcpy (SortEntry.Text, Entry->Name, strlen (Entry->Name) + 1);
  PutComboEntry (Window, &SortEntry, Cnt, FALSE);
  CopyToXMS (DBLEN, DBase.XMSHandle, Entry, DBLEN * Cnt);
  return (Cnt);
}
