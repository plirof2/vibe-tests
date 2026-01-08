/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBMAIN.H                                                                                                        */
/* Description  : Main program                                                                                                    */
/* Version type : Main program interface                                                                                          */
/* Last changed : 05-10-1998  7:00                                                                                                */
/* Update count : 30                                                                                                              */
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

#ifndef DBMAIN_H_INC
#define DBMAIN_H_INC

#ifdef EXTERN
#undef EXTERN
#endif
#ifdef __DBMAIN_MAIN__
#define EXTERN
#else
#define EXTERN extern
#endif

#define REGISTER

#define INFODIR       "INFO"
#define INFOEXT       "TXT"
#define ZXEDEXT       "ZED"
#define SCRSHOTDIR    "SCRSHOT"
#define SCRSHOTEXT    "SCR"
#define POKESDIR      "POKES"
#define POKESEXT      "POK"
#define INLAYDIR      "INLAYS"
#define INLAYEXT1     "JPG"
#define INLAYEXT2     "GIF"
#define BASICEXT      "BAS"

#define COMPRESSDIR   "ZIPPED"

#define COMPRBATCH    "COMPRSGD.BAT"

#define PLAYBIT        0x01                                                         /* Bits for SelectWindow entry.Selected field */
#define SELECTBIT      0x02

#define EN_TYPEBITS    0x1C                                                                                           /* Bits 2-4 */
                                                                                                       /* 000 means none of these */
#define TZXHWTBLOCK    0x04                                                                    /* (TZX block - Hardware Type) 001 */
#define TZXAIBLOCK     0x08                                                                     /* (TZX block - Archive Info) 010 */
#define TZXCUSTSCR     0x0C                                                             /* (TZX block - Custom Info - Screen) 011 */
#define TZXCUSTTXT     0x10                                                          /* (TZX block - Custom Info - Text Info) 100 */
#define TZXCUSTPIC     0x14                                                            /* (TZX block - Custom Info - Picture) 101 */
#define TZXCUSTPOK     0x18                                                              /* (TZX block - Custom Info - Pokes) 110 */
#define BASICBLOCK     0x1C                                                                        /* Used in TAP/TZX indexes 111 */

/**********************************************************************************************************************************/
/* Define the global variables                                                                                                    */
/**********************************************************************************************************************************/

EXTERN struct ComboInfo    *SWindow;
EXTERN struct ComboInfo    *TWindow;
EXTERN struct DBaseEntry_s  CurrentEntry;
EXTERN struct ComboEntry    CurrentSEntry;
EXTERN struct FontInfo     *SpectrumFont;
EXTERN char                 GamePath[MAXDIR];
EXTERN char                 TotalText[40];
EXTERN char                 Criterion[257];
EXTERN struct ComboEntry    NewEntry;
EXTERN comboent             SelectedGame;
EXTERN int                  FileType;
EXTERN bool                 GamePathFound;
EXTERN char                 FullPath[MAXDIR];
EXTERN char                 InfoDir[MAXDIR];                                                            /* Directory of INFO file */
EXTERN char                 InfoPath[MAXDIR];                                                     /* Full path including filename */
EXTERN bool                 InfoFound;                                                             /* TRUE if `InfoPath' is valid */
EXTERN bool                 InfoIsZXEdit;                                    /* TRUE if `InfoPath' points to a ZX-Editor document */
EXTERN bool                 InfoDirExists;
EXTERN char                 ScrShotDir[MAXDIR];
EXTERN char                 ScrShotPath[MAXDIR];
EXTERN bool                 ScrShotFound;
EXTERN bool                 ScrShotDirExists;
EXTERN char                 PokesDir[MAXDIR];
EXTERN char                 PokesPath[MAXDIR];
EXTERN bool                 PokesFound;
EXTERN bool                 PokesDirExists;
EXTERN char                 InlayDir[MAXDIR];
EXTERN char                 InlayPath[MAXDIR];
EXTERN bool                 InlayFound;
EXTERN bool                 InlayDirExists;
EXTERN bool                 TAPListing;
EXTERN bool                 MDRListing;
EXTERN bool                 DatabaseTruncated;
 
/**********************************************************************************************************************************/
/* Define the function prototypes                                                                                                 */
/**********************************************************************************************************************************/

short GetDropDownEmulator  (struct WindowInfo *HostWindow, short StartX, short StartY, short OldValue, bool AllowNone);
void  SurroundScreen       (struct WindowInfo *Window, short StartX, short StartY);
void  GhostEmptyDbase      (void);
void  UnghostDbase         (void);
void  LowMemHandler        (int MemoryType, char *ErrorMessage);
bool  TestFileOverwrite    (char *Path);
void  ValidateAnEntry      (comboent EntryNumber, bool AdjustTable, bool AdjustScreen);
bool  ValidateEntries      (bool All);
void  SetupNewData         (bool FirstRun);
void  SetDatabaseName      (char *FileName, bool UpdateButton);
void  TestEmptyDbase       (void);
void  SignalDBChanged      (bool Changed);
int   GetFileType          (char *FileName, bool TestZ80);
void  ReportTotal          (void);
void  ClearMainLower       (void);
void  TestPlayGhosted      (void);
void  TestInfoGhosted      (void);
void  TestScrShotGhosted   (void);
void  TestInlayGhosted     (void);
void  TestPokeGhosted      (void);
bool  TestAGame            (comboent Num, bool TestZ80);
void  TestGames            (void);
void  StoreEntry           (void);
void  SelectGame           (comboent SelectedLine);
bool  TestScrShotDirExists (void);
void  GetEmulatorName      (short EmulatorType, char *Name);

#endif
