/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBDBASE.H                                                                                                       */
/* Description  : Spectrum Database database handler                                                                              */
/* Version type : Module interface                                                                                                */
/* Last changed : 26-05-1999  19:25                                                                                               */
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

#ifndef DBDBASE_H_INC
#define DBDBASE_H_INC

#ifdef EXTERN
#undef EXTERN
#endif
#ifdef __DBDBASE_MAIN__
#define EXTERN
#else
#define EXTERN extern
#endif

#define DBNAME      "GAMES.DAT"
#define DBTEMP      ".TMP"
#define DBBACKUP    ".BAK"
#define DBSAVE      "GAMES.SAV"
#define DBNONE      "NONAME.DAT"                                                          /* Database name after 'New' menu entry */
#define DBPRFEXT    ".OUT"                                                                       /* File extension for printfiles */

#define D_NAME      36
#define D_YEAR      4
#define D_PUBLISHER 36
#define D_AUTHOR    100
#define D_MEMORY    3
#define D_PLAYERS   1
#define D_STICKS    5
#define D_PCNAME    12
#define D_TYPE      7
#define D_DIRINDEX  3
#define D_FILESIZE  7
#define D_DISKID    4
#define D_OVERRD    2
#define D_LANGUAGE  3
#define D_SCORE     3

/**********************************************************************************************************************************/
/* Define the global variables                                                                                                    */
/**********************************************************************************************************************************/

struct DBaseEntry_s
{
  char              Name[D_NAME + 1];
  char              Year[D_YEAR + 1];
  char              Publisher[D_PUBLISHER + 1];
  char              Author[D_AUTHOR + 1];
  byte              Memory;
  char              Players[D_PLAYERS + 1];
  char              Together;
  char              Sticks[D_STICKS];
  char              PCName[D_PCNAME + 1];
  char              Type[D_TYPE + 1];
  char              DirIndex[D_DIRINDEX + 1];
  char              FileSize[D_FILESIZE + 1];
  char              OrigScreen;
  char              Origin;
  word              DiskId;
  char              OverrideEmulator;
  char              AYSound;
  char              MultiLoad;
  char              Language[D_LANGUAGE + 1];
  word              Score;
  char              Padding[240 - D_NAME - D_YEAR - D_PUBLISHER - D_PLAYERS - D_STICKS - D_PCNAME - D_TYPE - D_DIRINDEX - D_FILESIZE - D_LANGUAGE - D_AUTHOR - 21];
};

#define DBELEN      ((dword)sizeof (struct DBaseEntry_s))
#define DBLEN       ((dword)240)

struct DBase_s
{
  comboent          NumEntries;
  short             XMSHandle;
};

EXTERN struct DBase_s DBase;
EXTERN char           DatabaseName[MAXDIR];                                                      /* (Defaulted to GAMES.DAT) */
EXTERN char           DatabaseTemp[MAXDIR];
EXTERN char           DatabaseBackupName[MAXDIR];
EXTERN bool           EntryChanged;
EXTERN bool           DatabaseChanged;
EXTERN bool           NonexistingDatabase;
EXTERN bool           NewDatabase;
EXTERN bool           Convert107;
EXTERN bool           Convert109;
EXTERN bool           Convert117;
EXTERN bool           Convert121;
EXTERN bool           Convert129;
EXTERN bool           Convert130;
EXTERN bool           Convert136;
EXTERN bool           Convert202;
EXTERN bool           ConvertImprove;
EXTERN bool           CancelAutoConvert;
EXTERN bool           NoPercentageBar;
EXTERN word           DatabaseFloppyId;

/**********************************************************************************************************************************/
/* Define the function prototypes                                                                                                 */
/**********************************************************************************************************************************/

bool      ReadDatabase      (bool Merge, bool RetainConvertFlags, bool UpdateButton);
void      WriteDatabase     (bool FatalSave, bool All);
void      ReleaseDatabase   (void);
bool      ReloadDatabase    (void);
void      PrintDatabase     (bool All);
void      SaveDatabase      (bool All);
void      GetDBEntry        (struct DBaseEntry_s *Entry, dword Length, comboent EntryNumber);
void      PutDBEntry        (struct DBaseEntry_s *Entry, dword Length, comboent EntryNumber);
void      ClearDBEntry      (struct DBaseEntry_s *Entry);
void      AddDBEntry        (void);
bool      DelDBEntry        (comboent EntryNumber);
char     *ShrinkString      (char *OldString);
comboent  SortEntry         (struct ComboInfo *Window, struct DBaseEntry_s *Entry, comboent OldNumber);

#endif
