/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBFILE.H                                                                                                        */
/* Description  : File handler                                                                                                    */
/* Version type : Module interface                                                                                                */
/* Last changed : 06-10-1998  6:20                                                                                                */
/* Update count : 24                                                                                                              */
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

#ifndef DBFILE_H_INC
#define DBFILE_H_INC

#include "dbconfig.h"
#include "dbdbase.h"

#ifdef EXTERN
#undef EXTERN
#endif
#ifdef __DBFILE_MAIN__
#define EXTERN
#else
#define EXTERN extern
#endif

#define  DI_UNKNOWN    "255"                                                               /* Unresolved file location (DirIndex) */

struct Z80145SnapshotHeader_s                                              /* Snapshot header for Z80 snap v1.45 (Little Endian!) */
{
  byte                      A_n, F_n;                                      /* AF                                  */
  word                      BC_n;                                          /* BC                                  */
  word                      HL_n;                                          /* HL                                  */
  word                      PC;                                            /* PC                                  */
  word                      SP;                                            /* SP                                  */
  byte                      I;                                             /* I                                   */
  byte                      R;                                             /* R (bit 0-6)                         */
  byte                      Stat1;                                         /* Bit   0: Bit 7 of R                 */
                                                                           /* Bit 1-3: Border Color               */
                                                                           /* Bit   4: Basic SamRam switched in ? */
                                                                           /* Bit   5: Compressed datablocks ?    */
  word                      DE_n;                                          /* DE                                  */
  word                      BC_a;                                          /* BC'                                 */
  word                      DE_a;                                          /* DE'                                 */
  word                      HL_a;                                          /* HL'                                 */
  byte                      A_a, F_a;                                      /* AF'                                 */
  word                      IY;                                            /* IY                                  */
  word                      IX;                                            /* IX                                  */
  byte                      IFF1;                                          /* in bit 0                            */
  byte                      IFF2;
  byte                      Stat2;                                         /* Bit 0-1: IM 0/1/2                   */
                                                                           /* Bit   2: Issue 2 emulation ?        */
                                                                           /* Bit   3: Double int frequency ?     */
                                                                           /* Bit 4-5: 01 = High Video Sync       */
                                                                           /*          11 = Low                   */
                                                                           /*          else Normal Video Sync     */
                                                                           /* Bit 6-7: 00 = Cursor Joystick,      */
                                                                           /*          01 = Kempston,             */
                                                                           /*          10 = Interface 2 #1,       */
                                                                           /*          11 = Interface 2 #2        */
};

struct Z80201SnapshotHeader_s                                              /* Additional snapshot header for 2.01 */
{
  word                      AdditionalLength;                              /* Length of additional header         */
  word                      NewPC;                                         /* PC for v2.01 and up                 */
  byte                      HWMode;                                        /* Hardware mode                       */
  word                      xxxxx1;
  byte                      Emulation;                                     /* Bit 0 : R register emulation on ?   */
                                                                           /* Bit 1 : LDIR emulation on ?         */
  dword                     xxxxx2, xxxxx3, xxxxx4, xxxxx5;         
  byte                      xxxxx6;
};

struct Z80300SnapshotHeader_s                                              /* Additional snapshot header for 3.00 */
{
  word                      xxxxx7, xxxxx8, xxxxx9;
  byte                      RAMLo;                                         /* 0xFF if 0x0000-0x1FFF is RAM        */
  byte                      RAMHi;                                         /* oxFF if 0x2000-0x3FFF is RAM        */
  word                      UKey0, UKey1, UKey2, UKey3, UKey4;             /* ASCII words of user defined keys    */
  word                      MKey0, MKey1, MKey2, MKey3, MKey4;             /* Mappings of these keys              */
  byte                      MGTType;                                       /* M.G.T. type                         */
  word                      xxxxx10;
};

struct MMSnapshotHeader_s                                                  /* Snapshot format for MM snapshot (Little-Endian!) */
{
  byte                      I;                                             /* I                                   */
  word                      HL_a;                                          /* HL'                                 */
  word                      DE_a;                                          /* DE'                                 */
  word                      BC_a;                                          /* BC'                                 */
  word                      AF_a;                                          /* AF'                                 */
  word                      HL_n;                                          /* HL                                  */
  word                      DE_n;                                          /* DE                                  */
  word                      BC_n;                                          /* BC                                  */
  word                      IY;                                            /* IY                                  */
  word                      IX;                                            /* IX                                  */
  byte                      IFF1;                                          /* Bit 2: IFF1                         */
  byte                      R;                                             /* R                                   */
  word                      AF_n;                                          /* AF                                  */
  word                      SP;                                            /* SP                                  */
  byte                      IM;                                            /* IM 0/1/2                            */
  byte                      xxxxx1;                                        /* Unused (padding) byte               */
};

struct SPSnapshotHeader_s                                                  /* Snapshot format for SP snapshot (Little-Endian!) */
{
  word                      ID;                                            /* 'SP'                                */
  word                      Length;                                        /* Length of snap (should be 49152)    */
  word                      Start;                                         /* Start of snap (should be 16384)     */
  word                      BC_n;                                          /* BC                                  */
  word                      DE_n;                                          /* DE                                  */
  word                      HL_n;                                          /* HL                                  */
  word                      AF_n;                                          /* AF                                  */
  word                      IX;                                            /* IX                                  */
  word                      IY;                                            /* IY                                  */
  word                      BC_a;                                          /* BC'                                 */
  word                      DE_a;                                          /* DE'                                 */
  word                      HL_a;                                          /* HL'                                 */
  word                      AF_a;                                          /* AF'                                 */
  byte                      R;                                             /* R                                   */
  byte                      I;                                             /* I                                   */
  word                      SP;                                            /* SP                                  */
  word                      PC;                                            /* PC                                  */
  word                      xxxx1;                                         /* Reserved                            */
  byte                      Border;                                        /* Border color                        */
  byte                      xxxx2;                                         /* Reserved                            */
  word                      Status;                                        /* Status word                         */
                                                                           /* Bit 0   : IFF1: 0=DI, 1=EI          */
                                                                           /* Bit 1   : Int mode: 0=IM1, 1=IM2    */
                                                                           /* Bit 2   : IFF2                      */
                                                                           /* Bit 3   : reserved                  */
                                                                           /* Bit 4   : Int pending ?             */
                                                                           /* Bit 5   : FLASH: 0=INK, 1=PAPER     */
                                                                           /* Bit 6-7 : Reserved                  */
                                                                           /* Bit 8-15: Reserved                  */
};

struct MicrodriveRecord_s                                                  /* One sector in a microdrive file     */
{
  byte                      HdFlag;                                        /* Value 1, to indicate header block   */
  byte                      HdNumb;                                        /* Sector number (values 254 to 1)     */
  word                      xxxxx1;                                        /* Unused bytes                        */
  char                      HdName[10];                                    /* Microdrive cartridge name (padded)  */
  byte                      HdChk;                                         /* Checksum of previous fields         */
  byte                      RecFlg;                                        /* Bit    0: Always 0 -indicate record */
                                                                           /* Bit    1: Set for the EOF block     */
                                                                           /* Bit    2: Reset for a PRINT file    */
                                                                           /* Bits 3-7: Unused bits (0)           */
  byte                      RecNum;                                        /* Datablock sequence number (first=0) */
  word                      RecLen;                                        /* Datablock length (<=512, LSB first) */
  char                      RecNam[10];                                    /* Filename (padded)                   */
  byte                      DesChk;                                        /* Record descriptor checksum          */
                                                                           /* Split datablock for headers:        */
  byte                        DType;                                       /* Type of datablock                   */
  word                        DLength;                                     /* Length of datablock                 */
  word                        DLoad;                                       /* Load address of datablock           */
  word                        DExtra;                                      /* Extra parameter of datablock        */
  word                        DAuto;                                       /* Autostart line if BASIC             */
  byte                        DBlock[503];                                 /* Rest of datablock                   */
  byte                      DChk;                                          /* Datablock checksum                  */
};

struct TapeRecord_s                                                        /* One header of a tape file           */
{
  byte                      Flag;                                          /* Flag byte                           */
                                                                           /* Split datablock for headers:        */
  byte                        HType;                                       /* Type of datablock                   */
  char                        HName[10];                                   /* Name of datablock                   */
  word                        HLength;                                     /* Length of following datablock       */
  word                        HStart;                                      /* Startaddress of following datablock */
  word                        HExtra;                                      /* Extra parameter of datablock        */
};

struct SLTExtension_s                                                      /* One SLT extension block header */
{
  word                      DataType;                                      /* 0 for end of table */
  word                      Level;                                         /* Historical name */
  dword                     Length;                                        /* Length of this extension */
};

struct SLTMap_s                                                            /* File offsets to all interesting SLT extensions */
{
  dword                     TableStart;                                    /* First byte after the SLT signature string */
  dword                     TableEnd;                                      /* First byte after the SLT extension table */
  struct SLTExtension_s     HeaderInfo;                                    /* The SLT header */
  dword                     OffsetInfo;                                    /* Data start, relative to TableEnd */
  struct SLTExtension_s     HeaderScrShot;
  dword                     OffsetScrShot;
  struct SLTExtension_s     HeaderPokes;
  dword                     OffsetPokes;
  word                      NumEntries;                                    /* Number of entries in the table */
  bool                      InfoIncluded;
  bool                      ScrShotIncluded;
  bool                      PokesIncluded;
};

/* The (proposed) DataType values in an SLT extension block */
#define                     SLT_END     0x0000                             /* End of table/file */
#define                     SLT_LDATA   0x0001                             /* Level data */
#define                     SLT_INFO    0x0002                             /* Instructions (INFO) */
#define                     SLT_SCRSHOT 0x0003                             /* Loading screen (SCRSHOT) */
#define                     SLT_PICTURE 0x0004                             /* Scanned picture - needs external viewer */
#define                     SLT_POKES   0x0005                             /* Cheats and spoilers :) */

#define                     F_PAPER     0x0001
#define                     F_INK       0x0002
#define                     F_BRIGHT    0x0004
#define                     F_INVERSE   0x0008
#define                     F_FLASH     0x0010
#define                     F_OVER      0x0020
#define                     F_PAD_L     0x1000
#define                     F_PAD_R     0x2000
#define                     F_SKIP1     0x4000
#define                     F_SKIP2     0x8000

struct RemapCharacterSet_s
{
  char                     *Token;
  word                      Flags;
};

/**********************************************************************************************************************************/
/* Define the global variables                                                                                                    */
/**********************************************************************************************************************************/

EXTERN struct Z80145SnapshotHeader_s  Z80145SnapshotHeader;
EXTERN struct Z80201SnapshotHeader_s  Z80201SnapshotHeader;
EXTERN struct Z80300SnapshotHeader_s  Z80300SnapshotHeader;
EXTERN struct MMSnapshotHeader_s      SNASnapshotHeader;
EXTERN struct SPSnapshotHeader_s      SPSnapshotHeader;
EXTERN struct RemapCharacterSet_s    *RemapCharacterSet;
EXTERN struct SLTMap_s                SLTMap;
EXTERN int                            Z80Version;
EXTERN bool                           Z80VersionValid;
EXTERN word                           TZXVersion;
EXTERN bool                           TZXVersionValid;
EXTERN bool                           FileNotFound;

/**********************************************************************************************************************************/
/* Define the interface functions                                                                                                 */
/**********************************************************************************************************************************/

bool SpawnApplication      (char *Name, char *Path, char *Options, char *File);
void RemapSpectrumUDGs     (bool Assume48K, bool Reset, byte *AlternateMap);
bool TestFileLocation      (struct DBaseEntry_s *Entry);
bool ResolveFileLocation   (struct DBaseEntry_s *Entry, char *ResolvedPath, bool *PathChanged);
bool GetZ80Version         (char *FullPath);
bool GetTZXVersion         (char *FullPath);
bool GetSLTExtensionOffset (char *FullPath);
bool InsertTAPIndex        (struct ComboInfo *Window, char *FullPath);
bool InsertMDRIndex        (struct ComboInfo *Window, char *FullPath);
bool InsertTZXIndex        (struct ComboInfo *Window, char *FullPath);
void ViewInfoFromSLT       (short StartX, short StartY, short MaxEntrySize, short MaxEntryView, short TextWidth,
                            short TextHeight, char *FullPath, char *Header);
bool FixTape               (void);
void ViewBASICFromSnap     (bool ViewBASIC, char *FullPath);
void ViewBASICFromTAPBlock (bool ViewBASIC, struct ComboInfo *Window, comboent EntryNumber, char *FullPath);
void ViewBASICFromMDRBlock (bool ViewBASIC, struct ComboInfo *Window, comboent EntryNumber, char *FullPath);
void ViewBASICFromTZXBlock (bool ViewBASIC, struct ComboInfo *Window, comboent EntryNumber, char *FullPath);
void ViewTZXCIInstructions (struct ComboInfo *Window, comboent EntryNumber, char *FullPath);
void ViewTZXCIPicture      (struct ComboInfo *Window, comboent EntryNumber, char *FullPath);
void EditHardwareTypeBlock (bool Create, struct ComboInfo *Window, comboent EntryNumber, char *FullPath);
void EditArchiveInfoBlock  (bool Create, struct ComboInfo *Window, comboent EntryNumber, char *FullPath);
void EditTZXPokesBlock     (bool Create, struct ComboInfo *Window, comboent EntryNumber, char *FullPath);
bool GetConfNewFileName    (char *Operation, char *OldName, char *ExtraOptions, char *ExtReason);

#endif
