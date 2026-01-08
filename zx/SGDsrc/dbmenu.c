/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBMENU.C                                                                                                        */
/* Description  : Menu stuff                                                                                                      */
/* Version type : Program module                                                                                                  */
/* Last changed : 19-10-1998  6:45                                                                                                */
/* Update count : 55                                                                                                              */
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

#define __DBMENU_MAIN__

#include <dos.h>
#include <fcntl.h>
#include <direct.h>
#include "wbench32/wbench32.h"
#include "dbconfig.h"
#include "dbscreen.h"
#include "dbdbase.h"
#include "dbselect.h"
#include "dbmain.h"
#include "dbcompr.h"
#include "dbfile.h"
#include "dbmenu.h"

#pragma pack          ()

/**********************************************************************************************************************************/
/* Define the things we need even if the menu strip is not linked                                                                 */
/**********************************************************************************************************************************/

static char            _Path[MAXDIR + 1];
static char            _File[13];
static char            _Mask[13]                = "*.DAT";
static bool            _OldSnapButton;
static bool            _OldSaved = FALSE;

/**********************************************************************************************************************************/
/* Define all static routines - the handlers for the menu strip entries                                                           */
/**********************************************************************************************************************************/

static void _SetupDatabasePath   (void);
static bool _InsertGameDirectory (char *DirectoryName, bool DoSubDirs, struct WindowInfo *MessageWindow,
                                  comboent *TotalFound, comboent *TotalAdded, bool UseFileName, bool *AddAll);
static int  _HandleMENew         (struct MenuEntry *Entry);
static int  _HandleMEOpen        (struct MenuEntry *Entry);
static int  _HandleMESaveAll     (struct MenuEntry *Entry);
static int  _HandleMESaveSel     (struct MenuEntry *Entry);
static int  _HandleMEPrintAll    (struct MenuEntry *Entry);
static int  _HandleMEPrintSel    (struct MenuEntry *Entry);
static int  _HandleMEComprAll    (struct MenuEntry *Entry);
static int  _HandleMEComprSel    (struct MenuEntry *Entry);
static int  _HandleMEImport      (struct MenuEntry *Entry);
static int  _HandleMEResort      (struct MenuEntry *Entry);
static int  _HandleMEValidAll    (struct MenuEntry *Entry);
static int  _HandleMEValidSel    (struct MenuEntry *Entry);
static int  _HandleMEDelSel      (struct MenuEntry *Entry);
static int  _HandleMEInvSel      (struct MenuEntry *Entry);
static int  _HandleMEDupEntry    (struct MenuEntry *Entry);
static int  _HandleMEFindNew     (struct MenuEntry *Entry);
static int  _HandleMESrchGames   (struct MenuEntry *Entry);
static int  _HandleMEShowScr     (struct MenuEntry *Entry);
static int  _HandleMESoundVol    (struct MenuEntry *Entry);

/**********************************************************************************************************************************/
/* Define the menu strip entries                                                                                                  */
/**********************************************************************************************************************************/

struct MenuEntry MENew       = {'n', 0, MEF_KEYSELECT, 0, 0, _HandleMENew,      "New"};
struct MenuEntry MEOpen      = {'j', 0, MEF_KEYSELECT, 0, 0, _HandleMEOpen,     "Open ..."};
struct MenuEntry MESaveAs    = {0,   0, 0, 0, 0, NULL,                          "Save as ..."};
struct MenuEntry MESaveAll   = {'w', 0, MEF_KEYSELECT, 0, 0, _HandleMESaveAll,    "All"};
struct MenuEntry MESaveSel   = {0,   0, 0, 0, 0, _HandleMESaveSel,                "Selected"};
struct MenuEntry MEPrint     = {0,   0, 0, 0, 0, NULL,                          "Print"};
struct MenuEntry MEPrintAll  = {'t', 0, MEF_KEYSELECT, 0, 0, _HandleMEPrintAll,   "All"};
struct MenuEntry MEPrintSel  = {0,   0, 0, 0, 0, _HandleMEPrintSel,               "Selected"};
struct MenuEntry MECompress  = {0,   0, 0, 0, 0, NULL,                          "Compress games"};
struct MenuEntry MEComprAll  = {0,   0, 0, 0, 0, _HandleMEComprAll,               "All"};
struct MenuEntry MEComprSel  = {0,   0, 0, 0, 0, _HandleMEComprSel,               "Selected"};
struct MenuEntry MEImport    = {'m', 0, MEF_KEYSELECT, 0, 0, _HandleMEImport,   "Import database"};
struct MenuEntry MEResort    = {0,   0, 0, 0, 0, _HandleMEResort,               "Resort database"};
struct MenuEntry MEValidate  = {0,   0, 0, 0, 0, NULL,                          "Validate database"};
struct MenuEntry MEValidAll  = {0,   0, 0, 0, 0, _HandleMEValidAll,               "All"};
struct MenuEntry MEValidSel  = {0,   0, 0, 0, 0, _HandleMEValidSel,               "Selected"};
struct MenuEntry MEFindNew   = {0,   0, 0, 0, 0, _HandleMEFindNew,              "Find new games"};
struct MenuEntry MESrchGames = {0,   0, 0, 0, 0, _HandleMESrchGames,            "Search for games"};
struct MenuEntry MEShowScr   = {'z', 0, MEF_KEYSELECT, 0, 0, _HandleMEShowScr,  "View screenshot"};
struct MenuEntry MESeperate  = {0,   0, MEF_SEPERATOR, 0, 0, NULL, ""};
struct MenuEntry MEBackup    = {0,   0, MEF_BOOL | MEF_BOOLCLEAR,
                                             &DBConfig.BackupDatabase, 0, NULL, "Backup database"};
struct MenuEntry MEAutoVal   = {0,   0, MEF_BOOL | MEF_BOOLCLEAR,
                                          &DBConfig.ValidateAfterLoad, 0, NULL, "Auto-validate"};
struct MenuEntry MEQuit      = {'q', 2, MEF_KEYSELECT, 0, 0, NULL,              "Quit"};

struct MenuEntry MEDelSel    = {0,   0, 0, 0, 0, _HandleMEDelSel,               "Delete selected"};
struct MenuEntry MEInvSel    = {'v', 0, MEF_KEYSELECT, 0, 0, _HandleMEInvSel,   "Invert selected"};
struct MenuEntry MEDupEntry  = {'u', 0, MEF_KEYSELECT, 0, 0, _HandleMEDupEntry, "Duplicate entry"};
struct MenuEntry MEMakeGIF   = {'g', 1, MEF_KEYSELECT, 0, 0, NULL,              "Create GIF-file"};
struct MenuEntry MEMakeSCR   = {'k', 3, MEF_KEYSELECT, 0, 0, NULL,              "Create SCR-file"};
struct MenuEntry MEFixTape   = {0,   4, 0, 0, 0, NULL,                          "Fix TAP-file"};
struct MenuEntry MEViewBasic = {0,   5, 0, 0, 0, NULL,                          "View BASIC-part"};
struct MenuEntry MEViewScrn  = {0,   6, 0, 0, 0, NULL,                          "View SCREEN-part"};
struct MenuEntry MEViewDump  = {0,   7, 0, 0, 0, NULL,                          "View memory dump"};
struct MenuEntry METZXOpts   = {0,   0, 0, 0, 0, NULL,                          "TZX options"};
struct MenuEntry MEInstHWTp  = {0,  12, 0, 0, 0, NULL,                            "Insert Hardware Type"};
struct MenuEntry MEEditHWTp  = {0,  13, 0, 0, 0, NULL,                            "Edit Hardware Type"};
struct MenuEntry MEInstArIn  = {0,  14, 0, 0, 0, NULL,                            "Insert Archive Info"};
struct MenuEntry MEEditArIn  = {0,  15, 0, 0, 0, NULL,                            "Edit Archive Info"};
struct MenuEntry MEInstTZXP  = {0,  16, 0, 0, 0, NULL,                            "Insert Pokes"};
struct MenuEntry MEEditTZXP  = {0,  17, 0, 0, 0, NULL,                            "Edit Pokes"};
struct MenuEntry MERunTAPER  = {0,   9, 0, 0, 0, NULL,                          "Call TAPER"};

struct MenuEntry MEQuiet     = {0,   0, MEF_BOOL | MEF_BOOLCLEAR,
                                                &SystemDefaults.Quiet, 0, NULL, "Disable sounds"};
struct MenuEntry MESoundVol  = {0, 0, 0, 0, 0, _HandleMESoundVol,               "Sound volume"};
struct MenuEntry MEEmulFlsh  = {0,   0, MEF_BOOL | MEF_BOOLCLEAR,
                                               &DBConfig.EmulateFlash, 0, NULL, "Emulate flash"};
struct MenuEntry MESlowFlsh  = {0,   0, MEF_BOOL | MEF_BOOLCLEAR,
                                                  &DBConfig.SlowFlash, 0, NULL, "Slow flash"};
struct MenuEntry MEAlwCheat  = {0,   0, MEF_BOOL | MEF_BOOLCLEAR,
                                                &DBConfig.AlwaysCheat, 0, NULL, "Always cheat"};
struct MenuEntry MEBestScrn  = {0,   0, MEF_BOOL | MEF_BOOLCLEAR,
                                             &DBConfig.PickBestScreen, 0, NULL, "Best screen"};
struct MenuEntry MEAlwSelect = {0,   0, MEF_BOOL | MEF_BOOLCLEAR,
                                           &DBConfig.AlwaysSelectGame, 0, NULL, "Always select"};

/**********************************************************************************************************************************/
/* Define all other (static) global variables                                                                                     */
/**********************************************************************************************************************************/

static char      _LastScrPath[MAXDIR + 1];
static char      _LastScrFile[13];

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE NON-EXPORTED LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

static int _HandleMESoundVol (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The soundcard soft mixer's registers have been adjusted by the user.                                                   */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  short NMaster;
  short NGain;
  short NDACLevel;
  short NPCSpeak;
  short NLineIn;
  short NTreble;
  short NBass;
  struct WindowInfo CWindow     = {170, 160, 240, 195, 240, 195, 240, 195, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                   WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "Adjust Sound Volume"};
  struct ButtonInfo POk         = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 96, 170, 8, DBLACK, DWHITE, NULL, 0, "@Ok", NULL};
  struct SliderButton SMaster   = {10,   9, 128, DBLACK, DWHITE, 1, BF_KEYSELECT, 0, NULL};
  struct SliderButton SGain     = {10,  29, 128, DBLACK, DWHITE, 2, BF_KEYSELECT, 0, NULL};
  struct SliderButton STreble   = {10,  49, 128, DBLACK, DWHITE, 6, BF_KEYSELECT, 0, NULL};
  struct SliderButton SBass     = {10,  69, 128, DBLACK, DWHITE, 7, BF_KEYSELECT, 0, NULL};
  struct SliderButton SDACLevel = {10,  99, 128, DBLACK, DWHITE, 3, BF_KEYSELECT, 0, NULL};
  struct SliderButton SLineIn   = {10, 119, 128, DBLACK, DWHITE, 5, BF_KEYSELECT, 0, NULL};
  struct SliderButton SPCSpeak  = {10, 139, 128, DBLACK, DWHITE, 4, BF_KEYSELECT, 0, NULL};
  struct ButtonInfo   *ButtonP;
  struct SliderButton *SliderP;
  bool                Finished = FALSE;

  Entry = Entry; /* keep compiler happy */
  SMaster.NumSteps   = SoundBlasterInfo.MixerMax.MasterVolume;
  SGain.NumSteps     = SoundBlasterInfo.MixerMax.OutputGain;
  SDACLevel.NumSteps = SoundBlasterInfo.MixerMax.DACLevel;
  SPCSpeak.NumSteps  = SoundBlasterInfo.MixerMax.PCSpeakerLevel;
  SLineIn.NumSteps   = SoundBlasterInfo.MixerMax.LineInLevel;
  STreble.NumSteps   = SoundBlasterInfo.MixerMax.Treble;
  SBass.NumSteps     = SoundBlasterInfo.MixerMax.Bass;
  NMaster   = SoundBlasterInfo.Mixer.MasterVolume;
  NGain     = SoundBlasterInfo.Mixer.OutputGain;
  NDACLevel = SoundBlasterInfo.Mixer.DACLevel;
  NPCSpeak  = SoundBlasterInfo.Mixer.PCSpeakerLevel;
  NLineIn   = SoundBlasterInfo.Mixer.LineInLevel;
  NTreble   = SoundBlasterInfo.Mixer.Treble;
  NBass     = SoundBlasterInfo.Mixer.Bass;
  SMaster.Contents = &NMaster;
  SGain.Contents = &NGain;
  SDACLevel.Contents = &NDACLevel;
  SPCSpeak.Contents = &NPCSpeak;
  SLineIn.Contents = &NLineIn;
  STreble.Contents = &NTreble;
  SBass.Contents = &NBass;
  AddButton (&CWindow, &POk, FALSE);
  AddSliderButton (&CWindow, &SMaster, FALSE);
  if (SoundBlasterInfo.MixerType == SBMTYPE_SB16)
    AddSliderButton (&CWindow, &SGain, FALSE);
  if (SoundBlasterInfo.MixerType == SBMTYPE_SB16)
    AddSliderButton (&CWindow, &STreble, FALSE);
  if (SoundBlasterInfo.MixerType == SBMTYPE_SB16)
    AddSliderButton (&CWindow, &SBass, FALSE);
  AddSliderButton (&CWindow, &SDACLevel, FALSE);
  AddSliderButton (&CWindow, &SLineIn, FALSE);
  if (SoundBlasterInfo.MixerType == SBMTYPE_SB16)
    AddSliderButton (&CWindow, &SPCSpeak, FALSE);
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
  Wprintf (&CWindow,  1, 25, "{+1Master volume}N");
  Wprintf (&CWindow,  3, 25, "{+1Loudness}N");
  Wprintf (&CWindow,  5, 25, "{+1Treble}N");
  Wprintf (&CWindow,  7, 25, "{+1Bass}N");
  Wprintf (&CWindow, 10, 25, "{+1Play level}N");
  Wprintf (&CWindow, 12, 25, "{+1Line-In level}N");
  Wprintf (&CWindow, 14, 25, "{+1PC Speaker}N");
  while (!Finished)
  {
    switch (HandleWindow (&CWindow, &ButtonP))
    {
      case WINH_RNOHAND    : switch (ButtonP->ReactNum)
                             {
                               case 0 : Finished = TRUE; break;                                                           /* (Ok) */
                             }
                             break;
      case WINH_CLOSE      : Finished = TRUE; break;
      case WINH_SLIDERMOVE : SliderP = (struct SliderButton *)(ButtonP->Font);                              /* One of the sliders */
                             switch (SliderP->ReactNum)
                             {
                               case 1 : SB_WriteMixer (SBM_MASTERVOL, NMaster); break;                         /* (Master volume) */
                               case 2 : SB_WriteMixer (SBM_OUTPUTGAIN, NGain); break;                            /* (Output gain) */
                               case 3 : SB_WriteMixer (SBM_DACLEVEL, NDACLevel); break;                            /* (DAC level) */
                               case 4 : SB_WriteMixer (SBM_PCSPEAKER, NPCSpeak); break;                           /* (PC Speaker) */
                               case 5 : SB_WriteMixer (SBM_LINEINLEVEL, NLineIn); break;                       /* (Line-In level) */
                               case 6 : SB_WriteMixer (SBM_TREBLELEVEL, NTreble); break;                              /* (Treble) */
                               case 7 : SB_WriteMixer (SBM_BASSLEVEL, NBass); break;                                    /* (Bass) */
                             }
                             break;
    }
    if (GlobalKey == K_ESC)
      Finished = TRUE;
    MouseStatus ();
  }
  DestroyWindow (&CWindow);
  return (WINH_NONE);
}

static int _HandleMENew (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : The current database has been released from memory. The databasename has been set to a 'NONAME' value.                 */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Entry = Entry;
  StoreEntry ();
  if (DatabaseChanged)
    switch (QuestionRequester (-1, -1, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL, NULL, "Database has been changed! Write back?"))
    {
      case 0 : SaveDatabase (TRUE);                                                                                      /* (Yes) */
               break;
      case 1 : break;                                                                                                     /* (No) */
      case 2 : return (WINH_NONE);                                                                                    /* (Cancel) */
    }
  SetDatabaseName (DBNONE, TRUE);
  ReleaseDatabase ();
  GhostEmptyDbase ();
  SelectedGame = -1;
  NumMatches = 0;
  NonexistingDatabase = TRUE;
  NewDatabase = TRUE;
  DatabaseFloppyId = 0;
  ReInitComboWindow (SWindow, 0);
  PrintComboEntries (SWindow);
  ReportTotal ();
  GhostMenuBackup ();
  return (WINH_NONE);
}

static int _HandleMEOpen (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : The current database has been saved if changed and a new user-selected database has been read.                         */
/* Import: _SetupDatabasePath.                                                                                                    */
/**********************************************************************************************************************************/

{
  StoreEntry ();
  _SetupDatabasePath ();
  if (FileRequester (180, 50, 48, 29, 20, _Path, _File, _Mask, "Select New Database", TRUE, FALSE))
  {
    if (DatabaseChanged)
      switch (QuestionRequester (-1, -1, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL, NULL, "Database has been changed! Write back?"))
      {
        case 0 : SaveDatabase (TRUE);                                                                                    /* (Yes) */
                 break;
        case 1 : break;                                                                                                   /* (No) */
        case 2 : if (MaxFloppyIdChanged)                                                                              /* (Cancel) */
                   WriteConfig ();
                 return (WINH_NONE);
      }
    MakeFullPath (_Path, _Path, _File);
    SetDatabaseName (_Path, TRUE);
    ReloadDatabase ();                                                                                    /* Re-read the database */
    if (CancelAutoConvert)
      return (_HandleMENew (Entry));
    SetupNewData (FALSE);
    if (DBase.NumEntries >= 0)
      SelectGame (0);                                                                                   /* Auto-select first game */
  }
  if (MaxFloppyIdChanged)
    WriteConfig ();
  return (WINH_NONE);
}

static int _HandleMESaveAll (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : The entire database has been saved to disk, but under a user-defined (new) name.                                       */
/* Import: _SetupDatabasePath.                                                                                                    */
/**********************************************************************************************************************************/

{
  Entry = Entry;
  StoreEntry ();                                                                                             /* Store any changes */
  if (!DatabaseChanged)                                              /* Ask for comfirmation if the database has not been changed */
  {
    if (QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Unneeded Save", "Database has not "
                           "been changed! Save anyway?"))
      return (WINH_NONE);
  }
  else if (DatabaseTruncated)
  {
    if (QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Save of Truncated Database",
        "{BWARNING}B: Remember that the database has been truncated\n"
        "Are you sure you want to throw away these entries?"))
      return (WINH_NONE);
  }
  ForceNewDatabaseName (TRUE);
  return (WINH_NONE);
}

static int _HandleMESaveSel (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : The selected entries of the database have been saved to disk, but under a user-defined (new) name.                     */
/* Import: _SetupDatabasePath.                                                                                                    */
/**********************************************************************************************************************************/

{
  char OldDatabaseName[MAXDIR];
  bool Done = FALSE;

  Entry = Entry;
  StoreEntry ();                                                                                             /* Store any changes */
  strcpy (OldDatabaseName, DatabaseName);
  _SetupDatabasePath ();
  while (!Done)
    if (FileRequester (180, 50, 48, 29, 20, _Path, _File, _Mask, "Choose New Database Name", FALSE, FALSE))
    {
      MakeFullPath (_Path, _Path, _File);
      if (!TestFileOverwrite (_Path))
      {
        SetDatabaseName (_Path, FALSE);
        DBConfig.BackupDatabase = FALSE;                                                         /* No need for a database backup */
        WriteDatabase (FALSE, FALSE);                                                      /* Write selected part of the database */
        SetDatabaseName (OldDatabaseName, FALSE);                                                 /* Keep the old name as current */
        Done = TRUE;
      }
    }
    else
      Done = TRUE;
  return (WINH_NONE);
}

static int _HandleMEPrintAll (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : The entire database has been printed.                                                                                  */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Entry = Entry;
  PrintDatabase (TRUE);
  return (WINH_NONE);
}

static int _HandleMEPrintSel (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : The selected entries of the database have been printed.                                                                */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Entry = Entry;
  PrintDatabase (FALSE);
  return (WINH_NONE);
}

static int _HandleMEComprAll (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : All games in the database have been compressed.                                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Entry = Entry;
  CompressGames (TRUE);
  return (WINH_NONE);
}

static int _HandleMEComprSel (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : The selected games have been compressed.                                                                               */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Entry = Entry;
  CompressGames (FALSE);
  return (WINH_NONE);
}

static int _HandleMEImport (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : A (new) database has been appended in memory - the result has not been resorted.                                       */
/* Import: _SetupDatabasePath.                                                                                                    */
/**********************************************************************************************************************************/

{
  char     OldDatabaseName[MAXDIR];
  comboent OldNum;

  Entry = Entry;
  StoreEntry ();                                                                                             /* Store any changes */
  OldNum = DBase.NumEntries + 1;
  strcpy (OldDatabaseName, DatabaseName);
  _SetupDatabasePath ();
  if (FileRequester (180, 50, 48, 29, 20, _Path, _File, _Mask, "Select Database to Import", TRUE, FALSE))
  {
    MakeFullPath (_Path, _Path, _File);
    SetDatabaseName (_Path, FALSE);
    ReadDatabase (TRUE, FALSE, TRUE);                                                                       /* Merge the database */
    SetDatabaseName (OldDatabaseName, FALSE);
    if (!CancelAutoConvert)
    {
      SetupNewData (FALSE);
      if (DBase.NumEntries >= 0 && OldNum <= DBase.NumEntries)
      {
        SelectEntry (SWindow, OldNum);
        SelectGame (OldNum);                                                                            /* Auto-select first game */
      }
      SignalDBChanged (TRUE);
    }
  }
  return (WINH_NONE);
}

static int _HandleMEDelSel (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : All selected entries of the database have been removed. The user must confirm all.                                     */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct MouseShape *OldShape;
  struct ComboEntry  NEntry;
  bool               DeleteAll = FALSE;
  bool               DeleteThis = FALSE;
  bool               AtLeastOne = FALSE;
  comboent register  Cnt;

  Entry = Entry;
  OldShape = CurrentMouseShape;
  SetMouseShape (DefaultWaitingMouseShape);
  for (Cnt = 0 ; Cnt <= DBase.NumEntries ; )
  {
    GetComboEntry (SWindow, &NEntry, Cnt);
    if (!(NEntry.Selected & SELECTBIT))                                                               /* The game doesn't match ? */
      Cnt ++;
    else
    {
      if (!DeleteAll)
        switch (QuestionRequester (-1, -1, TRUE, TRUE, NULL, NULL, NULL, NULL, NULL, "Confirm Delete",
                "Are you sure you want to delete the entry\n\"%s\"?", NEntry.Text))
        {
          case 0 : DeleteThis = TRUE;                                                                                    /* (Yes) */
                   break;
          case 1 : DeleteThis = FALSE;                                                                                    /* (No) */
                   break;
          case 2 : DeleteThis = FALSE;                                                                                /* (Cancel) */
                   Cnt = DBase.NumEntries;
                   break;
          case 3 : DeleteThis = TRUE;                                                                                    /* (All) */
                   DeleteAll = TRUE;
        }
        else
          DeleteThis = TRUE;
      if (!_OldSaved)                                                  /* Keep the mouse where it is, it's much too anoying if it */
      {                                                           /* moved to the default button for every confirmation requester */
        _OldSnapButton = Mouse.SnapToDefaultButton;
        Mouse.SnapToDefaultButton = FALSE;
        _OldSaved = TRUE;
      }
      if (DeleteThis)
      {
        NumMatches --;
        AtLeastOne = TRUE;
        DelDBEntry (Cnt);                                                                             /* Remove the game from XMS */
        DeleteComboEntry (SWindow, Cnt);                                                              /* And from the entry table */
      }
      else
        Cnt ++;
    }
  }
  if (AtLeastOne)
  {
    if (SelectedGame > SWindow->NumEntries)
      SelectedGame = SWindow->NumEntries;
    SelectEntry (SWindow, SelectedGame);
    ReportTotal ();
    if (SWindow->NumEntries < 0)                                                                       /* Database is now empty ? */
    {
      SelectedGame = -1;
      GhostEmptyDbase ();
    }
    else
      SelectGame (SelectedGame);                                                                      /* Select next game in line */
    SignalDBChanged (TRUE);
  }
  SetMouseShape (OldShape);
  if (_OldSaved)
    Mouse.SnapToDefaultButton = _OldSnapButton;
  return (WINH_NONE);
}

static int _HandleMEInvSel (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : All selected entries have been deselected and vice versa.                                                              */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct ComboEntry NEntry;
  comboent register Cnt;

  Entry = Entry;
  for (Cnt = 0 ; Cnt <= SWindow->NumEntries ; Cnt ++)
  {
    GetComboEntry (SWindow, &NEntry, Cnt);
    NEntry.Selected ^= SELECTBIT;
    PutComboEntry (SWindow, &NEntry, Cnt, FALSE);
  }
  PrintComboEntries (SWindow);
  NumMatches = SWindow->NumEntries + 1 - NumMatches;
  ReportTotal ();
  return (WINH_NONE);
}

static int _HandleMEResort (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : The entire database has been resorted in memory.                                                                       */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct DBaseEntry_s    DEntry;
  comboent     register  Cnt;
  comboent     register  Cnt2;
  struct PReqInfo        PReq;
  bool                   Cancelled = FALSE;

  Entry = Entry;
  if (DBase.NumEntries < 1)                                                                          /* Less than 2 (!) entries ? */
    return (WINH_NONE);                                                                         /* Then there's nothing to resort */
  MakePRequester (&PReq, -1, -1, TRUE, NULL, "Resorting Database", "Resorting the database ...");
  for (Cnt = 0 ; !Cancelled && Cnt <= DBase.NumEntries ; Cnt ++)
  {
    Cnt2 = Cnt + 1;
    while (Cnt2 > Cnt)
    {
      GetDBEntry (&DEntry, DBLEN, Cnt);
      Cnt2 = SortEntry (SWindow, &DEntry, Cnt);                                                            /* Sort alphanumerical */
    }
    Cancelled = !UpdatePRequester (&PReq, (double)Cnt * 100 / DBase.NumEntries, NULL);
    MouseStatus ();
  }
  DestroyPRequester (&PReq);
  if (DBase.NumEntries >= 0)
  {
    SelectEntry (SWindow, 0);
    SelectGame (0);
  }
  SignalDBChanged (TRUE);
  return (WINH_NONE);
}

static int _HandleMEValidAll (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : The entire database has been validated.                                                                                */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Entry = Entry;
  ValidateEntries (TRUE);
  return (WINH_NONE);
}

static int _HandleMEValidSel (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : The selected entries of the database have been validated.                                                              */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Entry = Entry;
  ValidateEntries (FALSE);
  return (WINH_NONE);
}

static int _HandleMEDupEntry (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the activated menu entry.                                                                            */
/* Post  : The currently selected entry has been duplicated and selected.                                                         */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct DBaseEntry_s DupEntry;
  struct ComboEntry   NEntry;

  Entry = Entry;
  StoreEntry ();                                                                            /* Re-store previously selected entry */
  memcpy (&DupEntry, &CurrentEntry, (word)DBLEN);                                                               /* Save full info */
  GetComboEntry (SWindow, &NewEntry, SelectedGame);
  AddDBEntry ();                                                                                           /* Add an entry in XMS */
  if (!InsertComboEntry (SWindow, &NewEntry, NULL, -1))                                             /* And one in the entry table */
    FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
  GetDBEntry (&CurrentEntry, DBLEN, SWindow->NumEntries);                                                  /* Fetch the new entry */
  memcpy (&CurrentEntry, &DupEntry, (word)DBLEN);                                                             /* Update XMS entry */
  SelectedGame = SortEntry (SWindow, &CurrentEntry, SWindow->NumEntries);                                  /* Sort alphanumerical */
  GetComboEntry (SWindow, &NEntry, SelectedGame);
  if (NEntry.Selected & SELECTBIT)                                                                         /* Game was selected ? */
    NumMatches ++;
  ReselectAnEntry (SWindow, SelectedGame, &CurrentEntry, FALSE);                                  /* Rematch the duplicated entry */
  SelectEntry (SWindow, SelectedGame);
  SelectGame (SelectedGame);                                                                          /* Make it the current game */
  ReportTotal ();
  EntryChanged = TRUE;
  SignalDBChanged (TRUE);
  return (WINH_NONE);
}

static bool _InsertGameDirectory (char *DirectoryName, bool DoSubDirs, struct WindowInfo *MessageWindow,
                                  comboent *TotalFound, comboent *TotalAdded, bool UseFileName, bool *AddAll)

/**********************************************************************************************************************************/
/* Pre   : `Directoryname' points to the directory to be handled, `DoSubDirs' is TRUE if all subdirectories should be considered  */
/*         as well, `MessageWindow' is non-NULL if the currently searched directory should be reported (in MessageWindow),        */
/*         `TotalFound' reports the number of found new files, `TotalAdded' reports the number of actually added new files,       */
/*         `UseFileName' is TRUE if the raw filename should be used as game name when inserting an entry.                         */
/*         If `AddAll' is TRUE, no questions are asked; all found new files are directly inserted.                                */
/* Post  : All files in the selected have been inserted into the database if new and the user accepted.                           */
/*         If the user chose `Cancel', the return value is TRUE, otherwise FALSE.                                                 */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
#ifdef __WATCOMC__
#define __FoundFileName  DirEntP->d_name
#else
#define __FoundFileName  FileBlock.name
#endif

  struct ButtonInfo *ButtonP;
  bool           More;
  bool           Cancel = FALSE;
  bool           AddThis = FALSE;
  bool           FirstOne = TRUE;
  int            FileType;
  int            TLen;
  int            VLen;
#ifdef __WATCOMC__
  DIR           *DirP;
  struct dirent *DirEntP;
#else
  struct find_t  FileBlock;
  char           TestPath[MAXDIR];
#endif
  comboent       CntEnt;
  comboent       Cnt;
  char           ReportPath[MAXDIR];

  if (DoSubDirs && MessageWindow != NULL)
    Wprintf (MessageWindow, 1, 1, "Searching %s", DirectoryName);
  TLen = strlen (DBConfig.SnapNeedsTAP);
  VLen = strlen (DBConfig.SnapNeedsVOC);
#ifdef __WATCOMC__
  DirP = opendir (DirectoryName);
  More = ((DirEntP = readdir (DirP)) == NULL ? FALSE : TRUE);
#else
  MakeFullPath (TestPath, DirectoryName, "*.*");                                        /* Prepare for all files in the directory */
  More = !_dos_findfirst (TestPath, _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_ARCH, &FileBlock);                /* Consider ANY file */
#endif
  while (!Cancel && More)
  {
#ifdef __WATCOMC__
    while (More && (DirEntP->d_attr & _A_SUBDIR))
      More = ((DirEntP = readdir (DirP)) == NULL ? FALSE : TRUE);
#else
    while (More && (FileBlock.attrib & _A_SUBDIR))                                                            /* Skip directories */
      More = !_dos_findnext (&FileBlock);
#endif
    if (More)                                                                                                   /* Found a file ? */
    {
      if (MessageWindow != NULL)
      {
        if (HandleWindow (MessageWindow, &ButtonP) == WINH_CLOSE)
          Cancel = TRUE;
        else if (GlobalKey == K_ESC)
          Cancel = TRUE;
        MouseStatus ();
      }
      if (!Cancel)
      {
        if ((FileType = GetFileType (__FoundFileName, FALSE)) != FT_UNKNOWN)                          /* Test for a Spectrum file */
        {
          for (CntEnt = 0 ; CntEnt <= DBase.NumEntries && More ; CntEnt ++)                              /* Test against database */
          {
            GetDBEntry (&CurrentEntry, DBLEN, CntEnt);
            More = strcmp (__FoundFileName, CurrentEntry.PCName);                                               /* Match filename */
            if (FileType == FT_TAP && !strncmp (__FoundFileName, CurrentEntry.PCName, strlen (__FoundFileName) - 3))
            {                                                                                          /* Already auto-included ? */
              if (TLen > 0 && !strcmp (CurrentEntry.Name + strlen (CurrentEntry.Name) - TLen, DBConfig.SnapNeedsTAP))
                More = FALSE;
            }
            else if (FileType == FT_VOC && !strncmp (__FoundFileName, CurrentEntry.PCName, strlen (__FoundFileName) - 3))
            {
              if (VLen > 0 && !strcmp (CurrentEntry.Name + strlen (CurrentEntry.Name) - VLen, DBConfig.SnapNeedsVOC))
                More = FALSE;
            }
          }
          if (More)                                                                             /* File was not in the database ? */
          {
            (*TotalFound) ++;
            MakeFullPath (ReportPath, DirectoryName, __FoundFileName);
            if (!*AddAll)
              switch (QuestionRequester (-1, -1, TRUE, TRUE, NULL, NULL, NULL, NULL, NULL, "Confirm Add",
                      "Found unaccounted Spectrum game file\n%s\nAdd it to the database ?", ReportPath))
              {
                case 0 : AddThis = TRUE;                                                                                 /* (Yes) */
                         break;
                case 1 : AddThis = FALSE;                                                                                 /* (No) */
                         break;
                case 2 : AddThis = FALSE;                                                                             /* (Cancel) */
                         Cancel = TRUE;
                         break;
                case 3 : AddThis = TRUE;                                                                                 /* (All) */
                         *AddAll = TRUE;
              }
              else
                AddThis = TRUE;
            if (!Cancel)
            {
              if (!_OldSaved)                                       /* Keep the mouse where it is, it's much too anoying if it is */
              {                                                   /* moved to the default button for every confirmation requester */
                _OldSnapButton = Mouse.SnapToDefaultButton;
                Mouse.SnapToDefaultButton = FALSE;
                _OldSaved = TRUE;
              }
              if (AddThis)
              {
                (*TotalAdded) ++;
                if (FirstOne)                                                                     /* First one in this directoy ? */
                {
                  FirstOne = FALSE;
                  TestForNewMask (DirectoryName);                                       /* Then test if this is a 'new' directory */
                }
                AddDBEntry ();                                                                             /* Add an entry in XMS */
                if (UseFileName)                                                                /* Use PC-Filename as game name ? */
                {
                  Cnt = -1;
                  while (__FoundFileName[++ Cnt] != '.')                                                       /* Insert raw name */
                    NewEntry.Text[Cnt] = __FoundFileName[Cnt];
                  NewEntry.Text[Cnt] = '\0';
                }
                else
                  NewEntry.Text[0] = '\0';                                                              /* Nope, enter empty name */
                if (!InsertComboEntry (SWindow, &NewEntry, NULL, -1))                               /* And one in the entry table */
                  FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
                SelectedGame = SWindow->NumEntries;
                GetDBEntry (&CurrentEntry, DBLEN, SelectedGame);                                           /* Fetch the new entry */
                strcpy (CurrentEntry.PCName, __FoundFileName);
                strcpy (CurrentEntry.Name, NewEntry.Text);
                ValidateAnEntry (-1, TRUE, FALSE);                                     /* Get file type/location/size information */
                ReselectAnEntry (SWindow, SelectedGame, &CurrentEntry, FALSE);                           /* Match the empty entry */
                StoreEntry ();                                                                       /* And store the result back */
              }
            }
          }
        }
#ifdef __WATCOMC__
        More = ((DirEntP = readdir (DirP)) == NULL ? FALSE : TRUE);
#else
        More = !_dos_findnext (&FileBlock);                                                                     /* Find next file */
#endif
      }
    }
  }
#ifdef __WATCOMC__
  closedir (DirP);
#endif
  if (!Cancel && DoSubDirs)                                                                    /* Insert subdirectories as well ? */
  {
#ifdef __WATCOMC__
    DirP = opendir (DirectoryName);
    DirEntP = readdir (DirP);
    while (!Cancel && DirEntP != NULL)
    {
      while (DirEntP != NULL && (!(DirEntP->d_attr & _A_SUBDIR) || (DirEntP->d_name[0] == '.')))                    /* Skip files */
        DirEntP = readdir (DirP);
      if (DirEntP != NULL)                                                                              /* Found a subdirectory ? */
      {
        MakeFullPath (ReportPath, DirectoryName, DirEntP->d_name);                                                     /* Recurse */
        Cancel = _InsertGameDirectory (ReportPath, DoSubDirs, MessageWindow, TotalFound, TotalAdded, UseFileName, AddAll);
        DirEntP = readdir (DirP);                                                                       /* Find next subdirectory */
      }
    }
    closedir (DirP);
#else
    MakeFullPath (TestPath, DirectoryName, "*.*");
    More = !_dos_findfirst (TestPath, _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_ARCH | _A_SUBDIR, &FileBlock);   /* Consider subdirs */
    while (!Cancel && More)
    {
      while (More && (!(FileBlock.attrib & _A_SUBDIR) || (FileBlock.name[0] == '.')))                               /* Skip files */
        More = !_dos_findnext (&FileBlock);
      if (More)                                                                                         /* Found a subdirectory ? */
      {
        MakeFullPath (ReportPath, DirectoryName, FileBlock.name);                                                      /* Recurse */
        Cancel = _InsertGameDirectory (ReportPath, DoSubDirs, MessageWindow, TotalFound, TotalAdded, UseFileName, AddAll);
        More = !_dos_findnext (&FileBlock);                                                             /* Find next subdirectory */
      }
    }
#endif
  }
  return (Cancel);
#undef __FoundFileName
}

static int _HandleMEFindNew (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : All GameDir directories have been searched for unaccounted spectrum files.                                             */
/* Import: _InsertGameDirectory.                                                                                                  */
/**********************************************************************************************************************************/

{
  struct MouseShape  *OldShape;
  bool                UseFileName = TRUE;
  struct ComboEntry   PathEntry;
  struct ComboInfo    OWindow;
  struct WindowInfo   CWindow = {138, 110, 364, 230, 364, 230, 364, 230, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                 WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "Select Search Directories"};
  struct ButtonInfo   BCancel = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 80, 200, 8, DBLACK, DWHITE, NULL,
                                 1, "@Cancel", NULL};
  struct ButtonInfo   BClear  = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 155, 200, 8, DBLACK, DWHITE, NULL,
                                 2, "C@lear", NULL};
  struct ButtonInfo   BOk     = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 230, 200, 8, DBLACK, DWHITE, NULL,
                                 3, "@Search", NULL};
  struct ButtonInfo   BFName  = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 100, 160, 1, DBLACK, DWHITE, NULL, 0,
                                 NULL, NULL};
  struct ButtonInfo  *ButtonP;
  struct ComboEntry   NEntry;
  comboent            SelectedLine;
  bool                Finished     = FALSE;
  bool                Cancel       = TRUE;
  bool                AddAll       = FALSE;
  struct PReqInfo     PReq;
  byte                CntPath;
  short               DirNumber;
  comboent            TotalAdded = 0;
  comboent            TotalFound = 0;
  comboent            OldNumEntries;
  comboent            OldSelectedGame;
  double              Percentage;
  comboent            NumSelected;

  Entry = Entry;
  BFName.Contents = &UseFileName;
  StoreEntry ();
  AddButton (&CWindow, &BCancel, FALSE);
  AddButton (&CWindow, &BClear, FALSE);
  AddButton (&CWindow, &BOk, TRUE);
  AddButton (&CWindow, &BFName, FALSE);
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
  Wprintf (&CWindow, 1, TEXT_CENTER, "Select the directories to be searched");
  Wprintf (&CWindow, 16, 19, "{+1Use File name as Game name}N");
  MakeComboWindow (&OWindow, &CWindow, COMT_SELECT, COMF_NOBUFFER | COMF_VSLIDER | COMF_HSLIDER | COMF_KEYSELECT | COMF_SYNCSLIDER,
                   CWindow.StartX + 10, CWindow.StartY + 30, 15, 64, 50, 55, 10, NULL, FALSE, 0, NULL, DBLACK, DWHITE, 0x02, 0);
  PathEntry.Identifier = 0;
  PathEntry.Selected = 0x02;                                                                                /* Select all entries */
  for (CntPath = 0 ; CntPath < DBConfig.NumLocs ; CntPath ++)                                       /* Insert all GameDir entries */
  {
    strcpy (PathEntry.Text, DBConfig.FileLocMapping[CntPath].Path);
    InsertComboEntry (&OWindow, &PathEntry, NULL, -1);
  }
  PrintComboEntries (&OWindow);
  while (!Finished)
  {
    switch (HandleWindow (&CWindow, &ButtonP))
    {
      case WINH_CLOSE     : Finished = TRUE; break;
      case WINH_RNOHAND   : switch (ButtonP->ReactNum)
                            {
                              case 1 : Finished = TRUE; break;                                                        /* (Cancel) */
                              case 2 : for (CntPath = 0 ; CntPath <= OWindow.NumEntries ; CntPath ++)                  /* (Clear) */
                                       {
                                         GetComboEntry (&OWindow, &NEntry, CntPath);
                                         NEntry.Selected = 0x00;                                          /* Deselect all entries */
                                         PutComboEntry (&OWindow, &NEntry, CntPath, FALSE);
                                       }
                                       PrintComboEntries (&OWindow);
                                       break;
                              case 3 : Finished = TRUE; Cancel = FALSE; break;                                            /* (Ok) */
                            }
                            break;
    }
    HandleComboWindow (&OWindow, &SelectedLine);
    MouseStatus ();
    if (GlobalKey == K_ESC)
      Finished = TRUE;
  }
  if (Cancel)
  {
    DestroyComboWindow (&OWindow, 0);
    DestroyWindow (&CWindow);
    return (WINH_NONE);
  }
  for (CntPath = 0, NumSelected = 0 ; CntPath <= OWindow.NumEntries ; CntPath ++)
  {                                                                                  /* Determine number of directories to search */
    GetComboEntry (&OWindow, &NEntry, CntPath);
    if (NEntry.Selected)
      NumSelected ++;
  }
  if (NumSelected == 0)
  {
    DestroyComboWindow (&OWindow, 0);
    DestroyWindow (&CWindow);
    return (WINH_NONE);
  }
  Cancel = FALSE;
  OldNumEntries = DBase.NumEntries + 1;                                                     /* (Right after the current database) */
  OldSelectedGame = SelectedGame;
  DirNumber = 0;
  MakePRequester (&PReq, -1, -1, TRUE, NULL, "Search for New Games", "Searching for new games");
  for (CntPath = 0 ; !Cancel && CntPath <= OWindow.NumEntries ; CntPath ++)                /* Handle all selected GameDir entries */
  {
    GetComboEntry (&OWindow, &NEntry, CntPath);
    if (NEntry.Selected)
    {
      OldShape = CurrentMouseShape;
      SetMouseShape (DefaultWaitingMouseShape);
      Percentage = (double)DirNumber * 100 / NumSelected;
      DirNumber ++;
      if (!UpdatePRequester (&PReq, Percentage, "Searching %s ...", DBConfig.FileLocMapping[CntPath].Path))
        Cancel = TRUE;
      else
      {
        MouseStatus ();                                                                      /* Insert all files in the directory */
        Cancel = _InsertGameDirectory (DBConfig.FileLocMapping[CntPath].Path, FALSE, NULL, &TotalFound, &TotalAdded, UseFileName,
                                       &AddAll);
      }
    }
  }
  SetMouseShape (OldShape);
  if (_OldSaved)
    Mouse.SnapToDefaultButton = _OldSnapButton;
  DestroyPRequester (&PReq);
  DestroyComboWindow (&OWindow, 0);
  DestroyWindow (&CWindow);
  PrintComboEntries (SWindow);
  if (TotalAdded > 0)
  {
    SignalDBChanged (TRUE);
    SelectedGame = OldNumEntries;
    UnghostDbase ();                                                                       /* Activate the buttons and menu strip */
    ReportTotal ();
  }
  else
    SelectedGame = OldSelectedGame;
  if (TotalFound > 0)                                                                                   /* Did we find any game ? */
    ErrorRequester (-1, -1, NULL, WBPics[2], "Overview", "Total found: %d entr%s\nTotal added: %d entr%s",
                    TotalFound, TotalFound == 1 ? "y" : "ies", TotalAdded, TotalAdded == 1 ? "y" : "ies");
  if (SWindow->NumEntries >= 0)
  {
    SelectEntry (SWindow, SelectedGame);                                                 /* Ensure selected game is still correct */
    SelectGame (SelectedGame);
  }
  return (WINH_NONE);
}

static int _HandleMESrchGames (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : After the user selected a root directory, all games in this tree have been possibly inserted into the database.        */
/* Import: _InsertGameDirectory.                                                                                                  */
/**********************************************************************************************************************************/

{
  struct MouseShape *OldShape;
  struct WindowInfo CWindow            = {170, 215, 300, 30, 300, 30, 300, 30, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                          WINF_HUSE | WINF_HCENTER | WINF_TCLEAR | WINF_ACTIVATE | WINF_XMSBUFFER, NULL,
                                          "Searching Game Tree"};
  char              Path[MAXDIR + 1];
  char              File[13]           = "";
  char              Mask[13]           = "*.*";
  bool              Cancel             = FALSE;
  bool              AddAll             = FALSE;
  comboent          TotalAdded         = 0;
  comboent          TotalFound         = 0;
  comboent          OldNumEntries;
  comboent          OldSelectedGame;

  Entry = Entry;
  StoreEntry ();
  strcpy (Path, CurPath);
  if (FileRequester (180, 50, 48, 29, 20, Path, File, Mask, "Select starting directory", FALSE, TRUE))
  {
    if (IsFloppy (Path[0]))                                                                                  /* Search a floppy ? */
    {                                                                                     /* Have they gone nuts or something ??? */
      ErrorRequester (-1, -1, NULL, NULL, "Bad Media Choise", "It is not a good idea to search a floppy!");
      return (WINH_NONE);
    }
    OldShape = CurrentMouseShape;
    SetMouseShape (DefaultWaitingMouseShape);
    OldNumEntries = DBase.NumEntries + 1;                                                   /* (Right after the current database) */
    OldSelectedGame = SelectedGame;
    DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
    Cancel = _InsertGameDirectory (Path, TRUE, &CWindow, &TotalFound, &TotalAdded, TRUE, &AddAll);
    SetMouseShape (OldShape);
    if (_OldSaved)
      Mouse.SnapToDefaultButton = _OldSnapButton;
    DestroyWindow (&CWindow);
    PrintComboEntries (SWindow);
    if (TotalAdded > 0)
    {
      SignalDBChanged (TRUE);
      SelectedGame = OldNumEntries;
      UnghostDbase ();                                                                     /* Activate the buttons and menu strip */
      ReportTotal ();
    }
    else
      SelectedGame = OldSelectedGame;
    if (TotalFound > 0)                                                                                 /* Did we find any game ? */
      ErrorRequester (-1, -1, NULL, WBPics[2], "Overview", "Total found: %d entr%s\nTotal added: %d entr%s",
                      TotalFound, TotalFound == 1 ? "y" : "ies", TotalAdded, TotalAdded == 1 ? "y" : "ies");
    if (SWindow->NumEntries >= 0)
    {
      SelectEntry (SWindow, SelectedGame);                                               /* Ensure selected game is still correct */
      SelectGame (SelectedGame);
    }
  }
  return (WINH_NONE);
}

static int _HandleMEShowScr (struct MenuEntry *Entry)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : All GameDir directories have been searched for unaccounted spectrum files.                                             */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Entry = Entry;
  strcpy (_Path, _LastScrPath);
  strcpy (_File, _LastScrFile);
  strcpy (_Mask, "*.SCR");
  if (FileRequester (180, 50, 48, 29, 20, _Path, _File, _Mask, "Select screenshot to view", TRUE, FALSE))
  {
    strcpy (_LastScrPath, _Path);
    strcpy (_LastScrFile, _File);
    MakeFullPath (_Path, _Path, _File);
    DisplayZ80Screendump (_Path, _File);
    return (100);                                                                        /* Signal: redraw any active game screen */
  }
  return (WINH_NONE);
}

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE EXPORTED LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

void GhostMenuEmptyDbase (void)

/**********************************************************************************************************************************/
/* Pre   : The database is empty.                                                                                                 */
/* Post  : All appropriate menu entries have been ghosted.                                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MESaveAs.Flags |= MEF_GHOSTED;                                                            /* (This disables the entire submenu) */
  MEPrint.Flags |= MEF_GHOSTED;
  MEResort.Flags |= MEF_GHOSTED;
  MEValidate.Flags |= MEF_GHOSTED;
  MECompress.Flags |= MEF_GHOSTED;
  MEInvSel.Flags |= MEF_GHOSTED;
  MEDupEntry.Flags |= MEF_GHOSTED;
  MEMakeGIF.Flags |= MEF_GHOSTED;
  MEMakeSCR.Flags |= MEF_GHOSTED;
  MEFixTape.Flags |= MEF_GHOSTED;
  METZXOpts.Flags |= MEF_GHOSTED;
  MERunTAPER.Flags |= MEF_GHOSTED;
}

void UnghostMenu (void)

/**********************************************************************************************************************************/
/* Pre   : The database is no longer empty.                                                                                       */
/* Post  : All appropriate menu entries have been unghosted.                                                                      */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MESaveAs.Flags &= ~MEF_GHOSTED;
  MEPrint.Flags &= ~MEF_GHOSTED;
  MEResort.Flags &= ~MEF_GHOSTED;
  MEValidate.Flags &= ~MEF_GHOSTED;
  if (DBConfig.Compressor[0] != '\0')
    MECompress.Flags &= ~MEF_GHOSTED;
  MEInvSel.Flags &= ~MEF_GHOSTED;
  MEDupEntry.Flags &= ~MEF_GHOSTED;
}

void GhostMenuNoMatches (void)

/**********************************************************************************************************************************/
/* Pre   : No entries have been selected.                                                                                         */
/* Post  : All appropriate menu entries have been ghosted.                                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MESaveSel.Flags |= MEF_GHOSTED;
  MEPrintSel.Flags |= MEF_GHOSTED;
  MEComprSel.Flags |= MEF_GHOSTED;
  MEValidSel.Flags |= MEF_GHOSTED;
  MEDelSel.Flags |= MEF_GHOSTED;
}

void UnghostMenuNoMatches (void)

/**********************************************************************************************************************************/
/* Pre   : No entries have been selected.                                                                                         */
/* Post  : All appropriate menu entries have been ghosted.                                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MESaveSel.Flags &= ~MEF_GHOSTED;
  MEPrintSel.Flags &= ~MEF_GHOSTED;
  MEComprSel.Flags &= ~MEF_GHOSTED;
  MEValidSel.Flags &= ~MEF_GHOSTED;
  MEDelSel.Flags &= ~MEF_GHOSTED;
}

void GhostMenuGIF (void)

/**********************************************************************************************************************************/
/* Pre   : No spectrum screen is being displayed now.                                                                             */
/* Post  : The 'Make GIF' menu entry has been ghosted.                                                                            */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEMakeGIF.Flags |= MEF_GHOSTED;
}

void GhostMenuSCR (void)

/**********************************************************************************************************************************/
/* Pre   : No spectrum screen is being displayed now.                                                                             */
/* Post  : The 'Make SCR' menu entry has been ghosted.                                                                            */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEMakeSCR.Flags |= MEF_GHOSTED;
}

void GhostMenuBackup (void)

/**********************************************************************************************************************************/
/* Pre   : SGD was started with a new (nonexisting) databasename.                                                                 */
/* Post  : The 'Backup' menu entry has been ghosted.                                                                              */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEBackup.Flags |= MEF_GHOSTED;
}

void GhostMenuFindNew (void)

/**********************************************************************************************************************************/
/* Pre   : No GamePath entries exist yet.                                                                                         */
/* Post  : The 'Find New' menu entry has been ghosted.                                                                            */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEFindNew.Flags |= MEF_GHOSTED;
}

void UnghostMenuGIF (void)

/**********************************************************************************************************************************/
/* Pre   : A valid spectrum screen is being displayed.                                                                            */
/* Post  : The 'Make GIF' menu entry has been unghosted.                                                                          */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEMakeGIF.Flags &= ~MEF_GHOSTED;
}

void UnghostMenuSCR (void)

/**********************************************************************************************************************************/
/* Pre   : A valid spectrum screen is being displayed.                                                                            */
/* Post  : The 'Make SCR' menu entry has been unghosted.                                                                          */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEMakeSCR.Flags &= ~MEF_GHOSTED;
}

void UnghostMenuBackup (void)

/**********************************************************************************************************************************/
/* Pre   : The current databasename exists.                                                                                       */
/* Post  : The 'Backup' menu entry has been unghosted.                                                                            */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEBackup.Flags &= ~MEF_GHOSTED;
}

void UnghostMenuFindNew (void)

/**********************************************************************************************************************************/
/* Pre   : At least one GamePath entry exists.                                                                                    */
/* Post  : The 'Find New' menu entry has been unghosted.                                                                          */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEFindNew.Flags &= ~MEF_GHOSTED;
}

void GhostMenuCompress (void)

/**********************************************************************************************************************************/
/* Pre   : The config file must have been parsed.                                                                                 */
/* Post  : The 'Compress' menu entries have been ghosted if no compressor has been defined.                                       */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  if (DBConfig.Compressor[0] == '\0')
  {
    MECompress.Flags |= MEF_GHOSTED;
    MEComprAll.Flags |= MEF_GHOSTED;
    MEComprSel.Flags |= MEF_GHOSTED;
  }
}

void GhostMenuFixTape (void)

/**********************************************************************************************************************************/
/* Pre   : No game is selected, or the currently selected game is not in a .TAP file.                                             */
/* Post  : The 'Fix Tape' menu entry has been ghosted.                                                                            */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEFixTape.Flags |= MEF_GHOSTED;
}

void GhostMenuCallTAPER (void)

/**********************************************************************************************************************************/
/* Pre   : No game is selected, or the currently selected game is not in a tape file.                                             */
/* Post  : The 'Call TAPER' menu entry has been ghosted.                                                                          */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MERunTAPER.Flags |= MEF_GHOSTED;
}

void UnghostMenuFixTape (void)

/**********************************************************************************************************************************/
/* Pre   : The currently selected game is in a .TAP file.                                                                         */
/* Post  : The 'Fix Tape' menu entry has been unghosted.                                                                          */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEFixTape.Flags &= ~MEF_GHOSTED;
}

void UnghostMenuCallTAPER (void)

/**********************************************************************************************************************************/
/* Pre   : The currently selected game is in a tape file.                                                                         */
/* Post  : The 'Call TAPER' menu entry has been unghosted.                                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MERunTAPER.Flags &= ~MEF_GHOSTED;
}

void GhostMenuViewBasic (void)

/**********************************************************************************************************************************/
/* Pre   : No game is selected, or the currently selected game is in a .TAP or .VOC file.                                         */
/* Post  : The 'View Basic' menu entry has been ghosted.                                                                          */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEViewBasic.Flags |= MEF_GHOSTED;
}

void UnghostMenuViewBasic (void)

/**********************************************************************************************************************************/
/* Pre   : The currently selected game is in a snapshot file.                                                                     */
/* Post  : The 'View Basic' menu entry has been unghosted.                                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEViewBasic.Flags &= ~MEF_GHOSTED;
}

void GhostMenuEditHardwareType (void)

/**********************************************************************************************************************************/
/* Pre   : No game is selected, or the currently selected entry is not a TZX Hardware Type block.                                 */
/* Post  : The 'Edit Hardware Type' menu entry has been ghosted.                                                                  */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEEditHWTp.Flags |= MEF_GHOSTED;
}

void UnghostMenuEditHardwareType (void)

/**********************************************************************************************************************************/
/* Pre   : The currently selected block is a TZX Hardware Type block.                                                             */
/* Post  : The 'Edit Hardware Type' menu entry has been unghosted.                                                                */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEEditHWTp.Flags &= ~MEF_GHOSTED;
}

void GhostMenuEditArchiveInfo (void)

/**********************************************************************************************************************************/
/* Pre   : No game is selected, or the currently selected entry is not a TZX Archive Info block.                                  */
/* Post  : The 'Edit Archive Info' menu entry has been ghosted.                                                                   */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEEditArIn.Flags |= MEF_GHOSTED;
}

void GhostMenuEditTZXPokes (void)

/**********************************************************************************************************************************/
/* Pre   : No game is selected, or the currently selected entry is not a TZX Custom Info - Pokes block.                           */
/* Post  : The 'Edit Pokes' menu entry has been ghosted.                                                                          */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEEditTZXP.Flags |= MEF_GHOSTED;
}

void UnghostMenuEditArchiveInfo (void)

/**********************************************************************************************************************************/
/* Pre   : The currently selected block is a TZX Archive Info block.                                                              */
/* Post  : The 'Edit Archive Info' menu entry has been unghosted.                                                                 */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEEditArIn.Flags &= ~MEF_GHOSTED;
}

void UnghostMenuEditTZXPokes (void)

/**********************************************************************************************************************************/
/* Pre   : The currently selected block is a TZX Custom Info - Pokes block.                                                       */
/* Post  : The 'Edit Pokes' menu entry has been unghosted.                                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEEditTZXP.Flags &= ~MEF_GHOSTED;
}

void GhostMenuTZXOptions (void)

/**********************************************************************************************************************************/
/* Pre   : No game is selected, or the currently selected game is not in a .TZX file.                                             */
/* Post  : The entire submenu 'TZX options' has been ghosted.                                                                     */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  METZXOpts.Flags |= MEF_GHOSTED;
}

void UnghostMenuTZXOptions (void)

/**********************************************************************************************************************************/
/* Pre   : The currently selected game is in a .TZX file.                                                                         */
/* Post  : The entire submenu 'TZX options' has been unghosted.                                                                   */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  METZXOpts.Flags &= ~MEF_GHOSTED;
}

void GhostMenuViewScreen (void)

/**********************************************************************************************************************************/
/* Pre   : No game is selected, or the currently selected game is in a .TAP or .VOC file.                                         */
/* Post  : The 'View Screen' menu entry has been ghosted.                                                                         */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEViewScrn.Flags |= MEF_GHOSTED;
}

void UnghostMenuViewScreen (void)

/**********************************************************************************************************************************/
/* Pre   : The currently selected game is in a snapshot file.                                                                     */
/* Post  : The 'View Screen' menu entry has been unghosted.                                                                       */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEViewScrn.Flags &= ~MEF_GHOSTED;
}

void InstallMenu (struct WindowInfo *Window)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the window to which the menu strip should be attached.                                              */
/* Post  : The menu strip has been attached to this window.                                                                       */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct MenuInfo *MenuP;

  Window->MenuFont = _UsedMenuFont;
  MenuP = AddMenu (Window, "Database", 0);
  AddMenuEntry (MenuP, &MENew);
  AddMenuEntry (MenuP, &MEOpen);
  AddMenuEntry (MenuP, &MESaveAs);
  ConvertToSubMenu (MenuP, &MESaveAs);
  AddMenuEntry (MESaveAs.SubMenu, &MESaveAll);
  AddMenuEntry (MESaveAs.SubMenu, &MESaveSel);
  AddMenuEntry (MenuP, &MEPrint);
  ConvertToSubMenu (MenuP, &MEPrint);
  AddMenuEntry (MEPrint.SubMenu, &MEPrintAll);
  AddMenuEntry (MEPrint.SubMenu, &MEPrintSel);
  AddMenuEntry (MenuP, &MEImport);
  AddMenuEntry (MenuP, &MEResort);
  AddMenuEntry (MenuP, &MEValidate);
  ConvertToSubMenu (MenuP, &MEValidate);
  AddMenuEntry (MEValidate.SubMenu, &MEValidAll);
  AddMenuEntry (MEValidate.SubMenu, &MEValidSel);
  AddMenuEntry (MenuP, &MECompress);
  ConvertToSubMenu (MenuP, &MECompress);
  AddMenuEntry (MECompress.SubMenu, &MEComprAll);
  AddMenuEntry (MECompress.SubMenu, &MEComprSel);
  AddMenuEntry (MenuP, &MEFindNew);
  AddMenuEntry (MenuP, &MESrchGames);
  AddMenuEntry (MenuP, &MEShowScr);
  AddMenuEntry (MenuP, &MESeperate);
  AddMenuEntry (MenuP, &MEBackup);
  AddMenuEntry (MenuP, &MEAutoVal);
  AddMenuEntry (MenuP, &MESeperate);
  AddMenuEntry (MenuP, &MEQuit);

  MenuP = AddMenu (Window, "Edit", 0);
  AddMenuEntry (MenuP, &MEDelSel);
  AddMenuEntry (MenuP, &MEInvSel);
  AddMenuEntry (MenuP, &MEDupEntry);
  AddMenuEntry (MenuP, &MEMakeGIF);
  AddMenuEntry (MenuP, &MEMakeSCR);
  AddMenuEntry (MenuP, &MEFixTape);
  AddMenuEntry (MenuP, &MEViewBasic);
  AddMenuEntry (MenuP, &MEViewScrn);
  AddMenuEntry (MenuP, &MEViewDump);
  AddMenuEntry (MenuP, &METZXOpts);
  ConvertToSubMenu (MenuP, &METZXOpts);
  AddMenuEntry (METZXOpts.SubMenu, &MEInstHWTp);
  AddMenuEntry (METZXOpts.SubMenu, &MEEditHWTp);
  AddMenuEntry (METZXOpts.SubMenu, &MEInstArIn);
  AddMenuEntry (METZXOpts.SubMenu, &MEEditArIn);
  AddMenuEntry (METZXOpts.SubMenu, &MEInstTZXP);
  AddMenuEntry (METZXOpts.SubMenu, &MEEditTZXP);
  AddMenuEntry (MenuP, &MESeperate);
  AddMenuEntry (MenuP, &MERunTAPER);

  MenuP = AddMenu (Window, "System", 0);
  AddMenuEntry (MenuP, &MEQuiet);
  if (SoundBlasterInfo.MixerType == SBMTYPE_NONE)
    MESoundVol.Flags |= MEF_GHOSTED;
  AddMenuEntry (MenuP, &MESoundVol);
  AddMenuEntry (MenuP, &MESeperate);
  AddMenuEntry (MenuP, &MEEmulFlsh);
  AddMenuEntry (MenuP, &MESlowFlsh);
  AddMenuEntry (MenuP, &MEAlwCheat);
  AddMenuEntry (MenuP, &MEBestScrn);
  AddMenuEntry (MenuP, &MEAlwSelect);

  strcpy (_LastScrPath, CurPath);
  _LastScrFile[0] = '\0';
}

static void _SetupDatabasePath (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The name of the current database has been moved to the Path and Name variables.                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  short register Cnt;

  strcpy (_Path, DatabaseName);
  Cnt = strlen (_Path);
  while (-- Cnt > 0 && _Path[Cnt] != '\\')                                                                       /* Find the path */
    ;
  if (Cnt == 0)                                                                                    /* No path - only a filename ? */
  {
    strcpy (_File, DatabaseName);
    strcpy (_Path, CurPath);
  }
  else
  {
    strcpy (_File, _Path + Cnt + 1);
    _Path[Cnt] = '\0';
  }
  strcpy (_Mask, "*.DAT");
}

void ForceNewDatabaseName (bool All)

/**********************************************************************************************************************************/
/* Pre   : `All' is TRUE if the entire database should be saved, FALSE if only the selected entries.                              */
/* Post  : The user has been forced a 'Save As...' requester and the database has been saved with this name.                      */
/* Import: _SetupDatabasePath.                                                                                                    */
/**********************************************************************************************************************************/

{
  bool Done = FALSE;

  _SetupDatabasePath ();
  while (!Done)
    if (FileRequester (180, 50, 48, 29, 20, _Path, _File, _Mask, "Choose New Database Name", FALSE, FALSE))
    {
      MakeFullPath (_Path, _Path, _File);
      if (!TestFileOverwrite (_Path))
      {
        SetDatabaseName (_Path, TRUE);
        DBConfig.BackupDatabase = FALSE;                                                         /* No need for a database backup */
        WriteDatabase (FALSE, All);                                                                             /* Write database */
        Done = TRUE;
      }
    }
    else
      Done = TRUE;
}
