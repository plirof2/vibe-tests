/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBCOMPR.C                                                                                                       */
/* Description  : Game compression handler for internet archives                                                                  */
/* Version type : Program module                                                                                                  */
/* Last changed : 28-10-1998  6:15                                                                                                */
/* Update count : 6                                                                                                               */
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

#define __DBCOMPR_MAIN__

#include <fcntl.h>
#include <io.h>
#include <dos.h>
#include <direct.h>
#include "wbench32/wbench32.h"
#include "dbdbase.h"
#include "dbconfig.h"
#include "dbmenu.h"
#include "dbmain.h"
#include "dbcompr.h"

#pragma pack                ()

/**********************************************************************************************************************************/
/* Define the static variables                                                                                                    */
/**********************************************************************************************************************************/

static char  _CompressDir[MAXDIR];
static char  _CustomDir[MAXDIR]    = "\0";
static bool  _UseCustomDir         = FALSE;
static FILE *_Fp;

/**********************************************************************************************************************************/
/* Define the static routines                                                                                                     */
/**********************************************************************************************************************************/

static bool _CompressRequest (bool All);
static void _CompressAnEntry (comboent EntryNumber);

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE INTERNAL LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

static bool _CompressRequest (bool All)

/**********************************************************************************************************************************/
/* Pre   : `All' is TRUE if all games should be compressed, FALSE if only the selected games.                                     */
/* Post  : The user had to confirm that he wanted to compress. He had the possibility to select a custom output directory.        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  bool                   BoolNorDir = TRUE;
  bool                   BoolOthDir = FALSE;
  struct WindowInfo OWindow  = {100, 296, 440, 152, 440, 152, 440, 152, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_EXCLUSIVE | WINF_WADJUST | WINF_XMSBUFFER, NULL,
                                "Confirm Compress Games"};
  struct WindowInfo TWindow  = {156, 296, 374, 112, 374, 112, 374, 112, 1, WINB_NONE, DBLACK, DWHITE, NULL,
                                WINF_WADJUST | WINF_TWRAP | WINF_TWSMART | WINF_NOBUFFER, NULL, NULL};
  struct ButtonInfo Button1  = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 3, 122, 8, DBLACK, DWHITE, NULL, 0, "@Cancel", NULL};
  struct ButtonInfo Button2  = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 300, 122, 8, DBLACK, DWHITE, NULL, 1, "@Ok", NULL};
  struct ButtonInfo BtNorDir = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 3, 60, 1, DBLACK, DWHITE, NULL, 2, NULL, NULL};
  struct ButtonInfo BtOthDir = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 3, 80, 1, DBLACK, DWHITE, NULL, 3, NULL, NULL};
  struct ButtonInfo BtInput  = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITDIR | BF_GHOSTED, 86, 80, 46, DBLACK, DWHITE,
                                NULL, 0, NULL, NULL};
  struct ButtonInfo *ButtonP;
  struct MouseShape *OldShape;
  bool               Accept        = FALSE;
  bool               Cancelled     = FALSE;
  bool               ButtonPressed = FALSE;
  char              *TestDir;

  OWindow.APen = TWindow.APen = Button1.APen = Button2.APen = BtNorDir.APen = BtOthDir.APen = SystemColors.ReqAPen;
  OWindow.BPen = TWindow.BPen = Button1.BPen = Button2.BPen = BtNorDir.BPen = BtOthDir.BPen = SystemColors.ReqBPen;
  BtNorDir.Contents = &BoolNorDir;
  BtOthDir.Contents = &BoolOthDir;
  OldShape = CurrentMouseShape;
  SetMouseShape (DefaultNormalMouseShape);
  if (Blanker.Active)
    BlankerWakeUp ();
  AddButton (&TWindow, &Button1, TRUE);
  AddButton (&TWindow, &Button2, FALSE);
  AddButton (&TWindow, &BtNorDir, FALSE);
  AddButton (&TWindow, &BtOthDir, FALSE);
  AddButton (&TWindow, &BtInput, FALSE);
  BtInput.MaxInputLength = MAXDIR - 14;                                                             /* Set maximum message length */
  BtInput.Contents = _CustomDir;
  DrawWindow (&OWindow, NULL, WING_CLOSE, TRUE);
  DrawWindow (&TWindow, &OWindow, 0, TRUE);
  WDrawPicture (&OWindow, 20, 14, WBPics[1]);
  Wprintf (&TWindow, 1, 0, "About to compress all %sgames", All ? "" : "selected ");                /* Print the question message */
  Wprintf (&TWindow, 3, 0, "{+4{BMake sure that this is what you want, as the process cannot be interrupted!}N");
  Wprintf (&TWindow, 6, 3, "{+1Use game subdirectory %s to store the compressed games}N", COMPRESSDIR);
  Wprintf (&TWindow, 8, 3, "{+1Custom dir}N");
  SoundErrorBeeper (BEEP_SHORT);                                                                          /* Sound the error bell */
  while (!Accept)
  {
    ButtonPressed = FALSE;
    while (!ButtonPressed)                                                               /* Wait for the user to close the window */
    {
      if (HandleWindow (&OWindow, &ButtonP) == WINH_CLOSE || GlobalKey == K_ESC)
      {
        ButtonPressed = TRUE;
        Cancelled = TRUE;
      }
      switch (HandleWindow (&TWindow, &ButtonP))
      {
        case WINH_RNOHAND  : ButtonPressed = TRUE; break;
        case WINH_BUTTBOOL : switch (ButtonP->ReactNum)
                             {
                               case 2 : BoolOthDir = FALSE;
                                        DrawButton (&TWindow, &BtOthDir);
                                        BtInput.Flags |= BF_GHOSTED;
                                        UpdateButtonText (&TWindow, &BtInput, BF_FITDIR);
                                        break;
                               case 3 : BoolNorDir = FALSE;
                                        DrawButton (&TWindow, &BtNorDir);
                                        BtInput.Flags &= ~BF_GHOSTED;
                                        UpdateButtonText (&TWindow, &BtInput, BF_FITDIR);
                                        break;
                             }
      }
      MouseStatus ();
    }
    if (!Cancelled && BoolOthDir == TRUE)                                                                   /* Custom directory ? */
      if (_CustomDir[0] == '\0')                                                          /* Choose 'customdir' without dirname ? */
      {
        ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: No custom directory name defined");
        Accept = FALSE;
      }
      else
      {
        TestDir = TestName (_CustomDir, FALSE, &ButtonPressed);
        if (ButtonPressed)
        {
          ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", TestDir);
          Accept = FALSE;
        }
        else
        {
          strcpy (_CustomDir, TestDir);
          Accept = TRUE;
        }
      }
    else
      Accept = TRUE;
  }
  _UseCustomDir = BoolOthDir;
  DestroyWindow (&OWindow);
  SetMouseShape (OldShape);
  if (Cancelled)
    return (FALSE);
  else
    return ((bool)(ButtonP->ReactNum));
}

static void _CompressAnEntry (comboent EntryNumber)

/**********************************************************************************************************************************/
/* Pre   : `EntryNumber' holds the number of the entry to be compressed.                                                          */
/* Post  : The requested entry has been compressed.                                                                               */
/* Import: StoreEntry.                                                                                                            */
/**********************************************************************************************************************************/

{
  char  PathBuffer[MAXDIR];
  char  CompressFullPath[MAXDIR];
  char  Param[40];
  bool  CompressDirExists = FALSE;
  unsigned  Dummy;

  SelectEntry (SWindow, EntryNumber);
  SelectGame (EntryNumber);
  if (!GamePathFound)                                                                                   /* Ignore non-found games */
    return;
  if (_UseCustomDir)                                                                          /* Compress into custom directory ? */
    strcpy (_CompressDir, _CustomDir);
  else
    MakeFullPath (_CompressDir, GamePath, COMPRESSDIR);
  HWEAbort = TRUE;                                              /* Return immediately if the next functions cause a hardware trap */
  HWEOccured = FALSE;
  _dos_setdrive (ToNumber (_CompressDir[0]), &Dummy);                                                         /* Go to this drive */
  getcwd (PathBuffer, MAXDIR);
  CompressDirExists = !(chdir (_CompressDir + 2));                                                /* Can I go to that directory ? */
  if (HWEOccured)                                                                         /* Not found if a hardware trap occured */
    CompressDirExists = FALSE;
  HWEAbort = FALSE;
  if (!CompressDirExists)                                                                   /* Does the directory exist already ? */
    if (!QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Create Directory",
        "The Compress directory %s\ndoes not exist yet.\nDo you want to create it now ?", _CompressDir))
      if (mkdir (_CompressDir))                                                                    /* Try to create the directory */
        ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Could not create directory:\n%s", strerror (errno));
      else
        CompressDirExists = TRUE;
  chdir (PathBuffer);
  _dos_setdrive (CurDrv, &Dummy);
  chdir (CurPath);                                                                                 /* Restore drive and directory */
  if (!CompressDirExists)                                                                      /* Compress path creation failed ? */
    return;
  MakeFullPath (CompressFullPath, _CompressDir, CurrentEntry.PCName);
  CompressFullPath[strlen (CompressFullPath) - 4] = '\0';                                                      /* Strip extension */

  fprintf (_Fp, "%c:\n", GamePath[0]);
  fprintf (_Fp, "cd %s\n", GamePath + 2);
  fprintf (_Fp, "%s %s %s %s ", DBConfig.Compressor, DBConfig.CompressOptions, CompressFullPath, CurrentEntry.PCName);
  if ((FileType == FT_Z80 || FileType == FT_SNA) && DBConfig.SnapNeedsTAP[0] != '\0')               /* Do we have to add a tape ? */
  {
    if (!strcmp (CurrentEntry.Name + strlen (CurrentEntry.Name) - strlen (DBConfig.SnapNeedsTAP), DBConfig.SnapNeedsTAP))
    {                                                                                        /* This snapshot needs a tape file ? */
      strcpy (Param, CurrentEntry.PCName);
      strcpy (Param + strlen (Param) - 3, "TAP");                                                    /* Change filename extension */
      fprintf (_Fp, "%s ", Param);
    }
  }
  if ((FileType == FT_Z80 || FileType == FT_SNA) && DBConfig.SnapNeedsVOC[0] != '\0')                /* Do we have to add a VOC ? */
  {
    if (!strcmp (CurrentEntry.Name + strlen (CurrentEntry.Name) - strlen (DBConfig.SnapNeedsVOC), DBConfig.SnapNeedsVOC))
    {                                                                                        /* This snapshot needs a tape file ? */
      strcpy (Param, CurrentEntry.PCName);
      strcpy (Param + strlen (Param) - 3, "VOC");                                                    /* Change filename extension */
      fprintf (_Fp, "%s ", Param);
    }
  }
  if (FileType == FT_Z80)
  {
    strcpy (Param, CurrentEntry.PCName);                                                       /* Add possible level-loader files */
    strcpy (Param + strlen (Param) - 4, "*.DAT");
    fprintf (_Fp, "%s ", Param);
  }
  if (InfoFound)                                                                                /* Game info file for this game ? */
  {
    strcpy (Param, CurrentEntry.PCName);                                                                 /* Add it to the archive */
    if (InfoIsZXEdit)
      strcpy (Param + strlen (Param) - 3, ZXEDEXT);
    else
      strcpy (Param + strlen (Param) - 3, INFOEXT);
    if (DBConfig.AllFilesInOneDir)
      fprintf (_Fp, "%s ", Param);
    else
      fprintf (_Fp, "%s\\%s ", INFODIR, Param);
  }
  if (ScrShotFound)                                                                            /* Screenshot file for this game ? */
  {
    strcpy (Param, CurrentEntry.PCName);                                                                 /* Add it to the archive */
    strcpy (Param + strlen (Param) - 3, SCRSHOTEXT);
    if (DBConfig.AllFilesInOneDir)
      fprintf (_Fp, "%s ", Param);
    else
      fprintf (_Fp, "%s\\%s ", SCRSHOTDIR, Param);
  }
  if (PokesFound)                                                                                   /* Pokes file for this game ? */
  {
    strcpy (Param, CurrentEntry.PCName);                                                                 /* Add it to the archive */
    strcpy (Param + strlen (Param) - 3, POKESEXT);
    if (DBConfig.AllFilesInOneDir)
      fprintf (_Fp, "%s ", Param);
    else
      fprintf (_Fp, "%s\\%s ", POKESDIR, Param);
  }
  if (InlayFound)                                                                                   /* Inlay file for this game ? */
  {
    strcpy (Param, CurrentEntry.PCName);                                                                 /* Add it to the archive */
    strcpy (Param + strlen (Param) - 3, InlayPath + strlen (InlayPath) - 3);                               /* (Either JPG or GIF) */
    if (DBConfig.AllFilesInOneDir)
      fprintf (_Fp, "%s ", Param);
    else
      fprintf (_Fp, "%s\\%s ", INLAYDIR, Param);
  }
  fprintf (_Fp, "\n");
}

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE EXPORTED LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

void CompressGames (bool All)

/**********************************************************************************************************************************/
/* Pre   : `All' is TRUE if all entries must be compressed, FALSE if only the selected entries must be done.                      */
/* Post  : The chosen entries have been compressed into the `COMPRESSDIR' subdirectory.                                           */
/* Import: CompressAnEntry, SelectGame.                                                                                           */
/**********************************************************************************************************************************/

{
  comboent register ECnt;
  comboent          OldSelectedGame;
  char              ComprBatchPath[MAXDIR + 1];
  struct ComboEntry NEntry;

  StoreEntry ();
  if (!_CompressRequest (All))
    return;
  SetMouseShape (DefaultWaitingMouseShape);
  MakeFullPath (ComprBatchPath, TmpDirectory, COMPRBATCH);
  if ((_Fp = fopen (ComprBatchPath, "w")) == NULL)
    FatalError (11, "Cannot write compress batch %s", ComprBatchPath);
  fprintf (_Fp, "@echo off\n");
  OldSelectedGame = SelectedGame;
  for (ECnt = 0 ; ECnt <= SWindow->NumEntries ; ECnt ++)                                                  /* Compress the entries */
  {
    GetComboEntry (SWindow, &NEntry, ECnt);
    if (All || (NEntry.Selected & SELECTBIT))
      _CompressAnEntry (ECnt);
  }
  fprintf (_Fp, "%c:\n", ToName (CurDrv));                                                    /* At the end, return to home drive */
  fprintf (_Fp, "cd %s\n", CurPath);
  fclose (_Fp);
  SwapOut (TRUE, ComprBatchPath, "", NULL, NULL, FALSE, FALSE, FALSE);                                               /* Compress! */
  unlink (ComprBatchPath);
  SelectEntry (SWindow, OldSelectedGame);                                                /* Ensure selected game is still correct */
  SelectGame (OldSelectedGame);
  SetMouseShape (DefaultNormalMouseShape);
  SoundErrorBeeper (BEEP_LONG);                                                                           /* Sound the ready bell */
}
