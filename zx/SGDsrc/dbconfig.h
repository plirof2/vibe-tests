/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBCONFIG.H                                                                                                      */
/* Description  : Configuration handler                                                                                           */
/* Version type : Module interface                                                                                                */
/* Last changed : 15-04-2001  10:15                                                                                               */
/* Update count : 44                                                                                                              */
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

#ifndef DBCONFIG_H_INC
#define DBCONFIG_H_INC

#include "dbdbase.h"                                                                                      /* Need value of D_NAME */

#ifdef EXTERN
#undef EXTERN
#endif
#ifdef __DBCONFIG_MAIN__
#define EXTERN
#else
#define EXTERN extern
#endif

#define CONFIGEXT             ".INI"

#define RELPATH               '$'                                           /* In OPT_RPATH config: denotes path relative to game */

#define ET_NONE               -1
#define ET_Z80                0
#define ET_JPP                1
#define ET_WAR                2
#define ET_X128               3
#define ET_WSPECEM            4
#define ET_ZX                 5
#define ET_SPANSPEC           6
#define ET_ZX32               7
#define ET_R80                8
#define ET_RUS                9
#define ET_YUD                10
#define ET_RSPEC              11
#define MAXETYPE              12

#define FT_UNKNOWN            -1
#define FT_TAP                0
#define FT_Z80                1
#define FT_SNA                2
#define FT_MDR                3
#define FT_VOC                4
#define FT_SLT                5
#define FT_SNP                6
#define FT_SEM                7
#define FT_LTP                8
#define FT_TZX                9
#define FT_SP                 10
#define FT_TRD                11
#define FT_DSK                12
#define FT_FDI                13
#define MAXFTYPE              14

#define IsSnapshot(_FT)      ((_FT) == FT_Z80 || (_FT) == FT_SNA || (_FT) == FT_SLT || (_FT) == FT_SNP || (_FT) == FT_SEM || (_FT) == FT_SP)
#define IsTAPERSnapshot(_FT) ((_FT) == FT_Z80 || (_FT) == FT_SNA)
#define IsTape(_FT)          ((_FT) == FT_TAP || (_FT) == FT_VOC || (_FT) == FT_LTP || (_FT) == FT_TZX)
#define IsDisk(_FT)          ((_FT) == FT_TRD || (_FT) == FT_DSK || (_FT) == FT_FDI)

/**********************************************************************************************************************************/
/* Define the global variables                                                                                                    */
/**********************************************************************************************************************************/

struct FileLocMapping_s
{
  char                         Path[MAXDIR];
};

struct Language_s
{
  char                         Language[D_LANGUAGE + 1];
};

struct DBConfig_s
{
  bool                         RemapBrightBlack;
  bool                         EmulateFlash;
  bool                         SlowFlash;
  bool                         ConfirmQuit;
  bool                         ConfirmDelete;
  bool                         ConfirmOverwrite;
  bool                         AlwaysBackupDatabase;
  bool                         BackupDatabase;
  bool                         ValidateAfterLoad;
  bool                         AlwaysCheat;
  bool                         PickBestScreen;
  bool                         AutoLoadTapes;
  bool                         ForceSnapJoystick;
  byte                         StartInfoButton;
  byte                         EmulType;
  char                         EmulPath[MAXETYPE][101];
  char                         ExtraOpt[MAXETYPE][101];
  char                         LinkExtension[MAXFTYPE];
  char                         EditPath[MAXDIR];
  char                         ZXEditPath[MAXDIR];
  char                         GIFViewerPath[MAXDIR];
  char                         JPGViewerPath[MAXDIR];
  char                         GIFViewerOptions[MAXDIR];
  char                         JPGViewerOptions[MAXDIR];
  bool                         AlwaysUseZXEdit;
  byte                         Printer;
  byte                         LinesPerPage;
  char                         FieldsString[81];
  word                         BlankerTimeOut;
  bool                         BlankerOn;
  bool                         AssumeStableFloppies;
  bool                         PreLoadResources;
  bool                         DirectVideoControl;
  bool                         OnlyZ80Tapes;
  bool                         ValidateAtEmulChange;
  bool                         AlwaysSelectGame;
  bool                         AllFilesInOneDir;
  char                         SnapNeedsTAP[D_NAME];
  char                         SnapNeedsVOC[D_NAME];
  char                         GIFPath[MAXDIR];
  char                         Compressor[MAXDIR];
  char                         CompressOptions[101];
  bool                         UsePhonebookSorting;
  bool                         UseCustomPalette;
  dword                        CustomColor00;
  dword                        CustomColor01;
  dword                        CustomColor02;
  dword                        CustomColor03;
  dword                        CustomColor04;
  dword                        CustomColor05;
  dword                        CustomColor06;
  dword                        CustomColor07;
  dword                        CustomColor08;
  dword                        CustomColor09;
  dword                        CustomColor10;
  dword                        CustomColor11;
  dword                        CustomColor12;
  dword                        CustomColor13;
  dword                        CustomColor14;
  dword                        CustomColor15;
  byte                         NumLocs;
  struct FileLocMapping_s     *FileLocMapping;
  byte                         NumAddedLanguages;
  struct Language_s           *AddedLanguage;
  char                         MouseSpeedString[80];
};

EXTERN struct DBConfig_s       DBConfig;
EXTERN char                    ConfigFile[MAXDIR];

EXTERN struct WindowInfo       AboutWindow;

/**********************************************************************************************************************************/
/* Define the interface functions                                                                                                 */
/**********************************************************************************************************************************/

void ReadConfig     (void);
void WriteConfig    (void);
byte TestForNewMask (char *Path);
void ChangeConfig   (void);
void AboutBox       (bool MakeWindow);
void SysInfoBox     (void);

#endif
