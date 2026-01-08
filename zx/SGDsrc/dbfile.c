/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBFILE.C                                                                                                        */
/* Description  : File handler                                                                                                    */
/* Version type : Program module                                                                                                  */
/* Last changed : 26-05-1999  20:55                                                                                               */
/* Update count : 56                                                                                                              */
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

#define __DBFILE_MAIN__

#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "wbench32/wbench32.h"
#include "dbdbase.h"
#include "dbmain.h"
#include "dbfile.h"
#include "dbpoke.h"
#include "dbtzxpok.h"
#include "dbmenu.h"
#include "dbscreen.h"

#pragma pack                ()

#include "specudg.inc"                                                                         /* Include Spectrum Font UDG remap */

#define HDRLEN               19                                                      /* A header is 17 bytes + Flag byte + Parity */

/**********************************************************************************************************************************/
/* Define the global variables                                                                                                    */
/**********************************************************************************************************************************/

struct RemapCharacterSet_s TokenMap[256] = {
   {"\x3F", 0x00 }, {"\x3F", 0x00  }, {"\x3F", 0x00   }, {"\x3F", 0x00 },  {"\x3F", 0x00 }, {"\x3F", 0x00 },
   {"\t",   0x00 }, {"\x3F", 0x00  }, {"\x3F", 0x00   }, {"\x3F", 0x00 },  {"\x3F", 0x00 }, {"\x3F", 0x00 },
   {"\x3F", 0x00 }, {"\r",   0x00  }, {"\0",   0x00   }, {"\x3F", 0x00 },  {"\0", F_INK  }, {"\0", F_PAPER},
   {"\0", F_FLASH}, {"\0", F_BRIGHT}, {"\0", F_INVERSE}, {"\x3F", F_OVER}, {"\0", F_SKIP2}, {"\0", F_SKIP2},
   {"\x3F", 0x00 }, {"\x3F", 0x00  }, {"\x3F", 0x00   }, {"\x3F", 0x00 },  {"\x3F", 0x00 }, {"\x3F", 0x00 },
   {"\x3F", 0x00 }, {"\x3F", 0x00  },

   {"\x20", 0x00}, {"\x21", 0x00}, {"\x22", 0x00}, {"\x23", 0x00}, {"\x24", 0x00}, {"\x25", 0x00}, {"\x26", 0x00}, {"\x27", 0x00},
   {"\x28", 0x00}, {"\x29", 0x00}, {"\x2A", 0x00}, {"\x2B", 0x00}, {"\x2C", 0x00}, {"\x2D", 0x00}, {"\x2E", 0x00}, {"\x2F", 0x00},
   {"\x30", 0x00}, {"\x31", 0x00}, {"\x32", 0x00}, {"\x33", 0x00}, {"\x34", 0x00}, {"\x35", 0x00}, {"\x36", 0x00}, {"\x37", 0x00},
   {"\x38", 0x00}, {"\x39", 0x00}, {"\x3A", 0x00}, {"\x3B", 0x00}, {"\x3C", 0x00}, {"\x3D", 0x00}, {"\x3E", 0x00}, {"\x3F", 0x00},
   {"\x40", 0x00}, {"\x41", 0x00}, {"\x42", 0x00}, {"\x43", 0x00}, {"\x44", 0x00}, {"\x45", 0x00}, {"\x46", 0x00}, {"\x47", 0x00},
   {"\x48", 0x00}, {"\x49", 0x00}, {"\x4A", 0x00}, {"\x4B", 0x00}, {"\x4C", 0x00}, {"\x4D", 0x00}, {"\x4E", 0x00}, {"\x4F", 0x00},
   {"\x50", 0x00}, {"\x51", 0x00}, {"\x52", 0x00}, {"\x53", 0x00}, {"\x54", 0x00}, {"\x55", 0x00}, {"\x56", 0x00}, {"\x57", 0x00},
   {"\x58", 0x00}, {"\x59", 0x00}, {"\x5A", 0x00}, {"\x5B", 0x00}, {"\x5C", 0x00}, {"\x5D", 0x00}, {"\x5E", 0x00}, {"\x5F", 0x00},
   {"\x60", 0x00}, {"\x61", 0x00}, {"\x62", 0x00}, {"\x63", 0x00}, {"\x64", 0x00}, {"\x65", 0x00}, {"\x66", 0x00}, {"\x67", 0x00},
   {"\x68", 0x00}, {"\x69", 0x00}, {"\x6A", 0x00}, {"\x6B", 0x00}, {"\x6C", 0x00}, {"\x6D", 0x00}, {"\x6E", 0x00}, {"\x6F", 0x00},
   {"\x70", 0x00}, {"\x71", 0x00}, {"\x72", 0x00}, {"\x73", 0x00}, {"\x74", 0x00}, {"\x75", 0x00}, {"\x76", 0x00}, {"\x77", 0x00},
   {"\x78", 0x00}, {"\x79", 0x00}, {"\x7A", 0x00}, {"\x7B", 0x00}, {"\x7C", 0x00}, {"\x7D", 0x00}, {"\x7E", 0x00}, {"\x7F", 0x00},

   {"\x80", 0x00}, {"\x81", 0x00}, {"\x82", 0x00}, {"\x83", 0x00}, {"\x84", 0x00}, {"\x85", 0x00}, {"\x86", 0x00}, {"\x87", 0x00},
   {"\x88", 0x00}, {"\x89", 0x00}, {"\x8A", 0x00}, {"\x8B", 0x00}, {"\x8C", 0x00}, {"\x8D", 0x00}, {"\x8E", 0x00}, {"\x8F", 0x00},

   {"\x90", 0x00}, {"\x91", 0x00}, {"\x92", 0x00}, {"\x93", 0x00}, {"\x94", 0x00}, {"\x95", 0x00}, {"\x96", 0x00}, {"\x97", 0x00},
   {"\x98", 0x00}, {"\x99", 0x00}, {"\x9A", 0x00}, {"\x9B", 0x00}, {"\x9C", 0x00}, {"\x9D", 0x00}, {"\x9E", 0x00}, {"\x9F", 0x00},
   {"\xA0", 0x00}, {"\xA1", 0x00}, {"\xA2", 0x00},

   {"SPECTRUM",  F_PAD_R | F_PAD_L}, {"PLAY",    F_PAD_R | F_PAD_L},               /* For Spectrum 128. 48 are remapped (shorter) */

   {"RND",       0x00             }, {"INKEY$",  0x00             }, {"PI",     0x00             }, {"FN",       F_PAD_R          },
   {"POINT",     F_PAD_R          }, {"SCREEN$", F_PAD_R          }, {"ATTR",   F_PAD_R          }, {"AT",       F_PAD_R          },
   {"TAB",       F_PAD_R          }, {"VAL$",    F_PAD_R          }, {"CODE",   F_PAD_R          }, {"VAL",      F_PAD_R          },
   {"LEN",       F_PAD_R          }, {"SIN",     F_PAD_R          }, {"COS",    F_PAD_R          }, {"TAN",      F_PAD_R          },
   {"ASN",       F_PAD_R          }, {"ACS",     F_PAD_R          }, {"ATN",    F_PAD_R          }, {"LN",       F_PAD_R          },
   {"EXP",       F_PAD_R          }, {"INT",     F_PAD_R          }, {"SQR",    F_PAD_R          }, {"SGN",      F_PAD_R          },
   {"ABS",       F_PAD_R          }, {"PEEK",    F_PAD_R          }, {"IN",     F_PAD_R          }, {"USR",      F_PAD_R          },
   {"STR$",      F_PAD_R          }, {"CHR$",    F_PAD_R          }, {"NOT",    F_PAD_R          }, {"BIN",      F_PAD_R          },
   {"OR",        F_PAD_R | F_PAD_L}, {"AND",     F_PAD_R | F_PAD_L}, {"<=",     0x00             }, {">=",       0x00             },
   {"<>",        0x00             }, {"LINE",    F_PAD_R          }, {"THEN",   F_PAD_R | F_PAD_L}, {"TO",       F_PAD_R | F_PAD_L},
   {"STEP",      F_PAD_R | F_PAD_L}, {"DEF FN",  F_PAD_R | F_PAD_L}, {"CAT",    F_PAD_R | F_PAD_L}, {"FORMAT",   F_PAD_R | F_PAD_L},
   {"MOVE",      F_PAD_R | F_PAD_L}, {"ERASE",   F_PAD_R | F_PAD_L}, {"OPEN #", F_PAD_R | F_PAD_L}, {"CLOSE #",  F_PAD_R | F_PAD_L},
   {"MERGE",     F_PAD_R | F_PAD_L}, {"VERIFY",  F_PAD_R | F_PAD_L}, {"BEEP",   F_PAD_R | F_PAD_L}, {"CIRCLE",   F_PAD_R | F_PAD_L},
   {"INK",       F_PAD_R | F_PAD_L}, {"PAPER",   F_PAD_R | F_PAD_L}, {"FLASH",  F_PAD_R | F_PAD_L}, {"BRIGHT",   F_PAD_R | F_PAD_L},
   {"INVERSE",   F_PAD_R | F_PAD_L}, {"OVER",    F_PAD_R | F_PAD_L}, {"OUT",    F_PAD_R | F_PAD_L}, {"LPRINT",   F_PAD_R | F_PAD_L},
   {"LLIST",     F_PAD_R | F_PAD_L}, {"STOP",    F_PAD_R | F_PAD_L}, {"READ",   F_PAD_R | F_PAD_L}, {"DATA",     F_PAD_R | F_PAD_L},
   {"RESTORE",   F_PAD_R | F_PAD_L}, {"NEW",     F_PAD_R | F_PAD_L}, {"BORDER", F_PAD_R | F_PAD_L}, {"CONTINUE", F_PAD_R | F_PAD_L},
   {"DIM",       F_PAD_R | F_PAD_L}, {"REM",     F_PAD_R | F_PAD_L}, {"FOR",    F_PAD_R | F_PAD_L}, {"GO TO",    F_PAD_R | F_PAD_L},
   {"GO SUB",    F_PAD_R | F_PAD_L}, {"INPUT",   F_PAD_R | F_PAD_L}, {"LOAD",   F_PAD_R | F_PAD_L}, {"LIST",     F_PAD_R | F_PAD_L},
   {"LET",       F_PAD_R | F_PAD_L}, {"PAUSE",   F_PAD_R | F_PAD_L}, {"NEXT",   F_PAD_R | F_PAD_L}, {"POKE",     F_PAD_R | F_PAD_L},
   {"PRINT",     F_PAD_R | F_PAD_L}, {"PLOT",    F_PAD_R | F_PAD_L}, {"RUN",    F_PAD_R | F_PAD_L}, {"SAVE",     F_PAD_R | F_PAD_L},
   {"RANDOMIZE", F_PAD_R | F_PAD_L}, {"IF",      F_PAD_R | F_PAD_L}, {"CLS",    F_PAD_R | F_PAD_L}, {"DRAW",     F_PAD_R | F_PAD_L},
   {"CLEAR",     F_PAD_R | F_PAD_L}, {"RETURN",  F_PAD_R | F_PAD_L}, {"COPY",   F_PAD_R | F_PAD_L}};

static short _SpectrumColourTable[16] = { 0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15 };

/**********************************************************************************************************************************/
/* Define the static variables                                                                                                    */
/**********************************************************************************************************************************/

static bool   _PadLeft         = FALSE;
static char  *_InvalidSLTFile  = "Invalid SLT file %s";
static dword  _HWTableLength;
static short  _HWTableXMSHandle;
static bool   _HardwareTableRead = FALSE;


/**********************************************************************************************************************************/
/* Define the static routines                                                                                                     */
/**********************************************************************************************************************************/

static void  _LoadHardwareTable   (void);
static short _RemapSpectrumString (byte *SpectrumString, short Length, char *RemappedString, short MaxLength, bool DropColors,
                                   bool InitialPad, short *RawLength, bool ToCombo, bool DropTABs);
static void  _ConvertAttributes   (char *OldLine);
static bool  _RewriteTZXFile      (char *FullPath, dword FileIndex, word OldLength, word NewLength, bool Make112);
static void  _ViewBASICListing    (short StartX, short StartY, short TextWidth, short TextHeight, char *Header,
                                   byte *Listing, byte *UDGArea, word MaxBlockSize);
static void  _ViewMemoryDump      (byte *Start, word BlockLength, word FirstAddress);

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE INTERNAL GRAPHICS INTERFACE LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

static void _LoadHardwareTable (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The file 'HARDWARE.INC' has been loaded into its own XMS space.                                                        */
/*         Mo return is made if any error occured.                                                                                */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  int   Fh;
  dword FileLength;
  char  FullName[MAXDIR + 1];

  if (!_HardwareTableRead)
  {
    _makepath (FullName, ProgramDrive, ProgramPath, "hardware", "inc");
    if ((Fh = open (FullName, O_RDONLY)) < 0)
      FatalError (7, "Could not load HARDWARE.INC: %s", strerror (errno));
    FileLength = filelength (Fh);
    if (read (Fh, _BufferDecrunched, (word)FileLength) != FileLength)
      FatalError (6, "Error loading HARDWARE.INC");
    _HWTableLength = (FileLength & 0xFFFFFFFC) + (FileLength % 4 ? 4 : 0);
    XMSAlloc (&_HWTableXMSHandle, _HWTableLength);
    CopyToXMS (_HWTableLength, _HWTableXMSHandle, _BufferDecrunched, 0);
    close (Fh);
    _HardwareTableRead = TRUE;
  }
}

static short _RemapSpectrumString (byte *SpectrumString, short Length, char *RemappedString, short MaxLength, bool DropColors,
                                   bool InitialPad, short *RawLength, bool ToCombo, bool DropTABs)

/**********************************************************************************************************************************/
/* Pre   : `SpectrumString' holds the string to be remapped, the length is `Length', The result should go to `RemappedString',    */
/*         which can contain a maximum of `MaxLength' characters.                                                                 */
/*         If `DropColors' is TRUE, no color attributes are to be inserted.                                                       */
/*         If `InitialPad' is TRUE, the first character inserted is a (padding) space (needed for listings).                      */
/*         The allocated length of `RemappedString' should be at least 9 bytes larger then `MaxLength'!                           */
/* Post  : The string has been remapped to the full Spectrum equivalent. All tokens are expanded. The return value is the number  */
/*         of bytes in the `RemappedString'.                                                                                      */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  short register  CharCnt;
  short register  StrLength  = 0;
  short register  SkipLength = 0;
  short register  RealLength = 0;                                                                  /* (Length without attributes) */
  bool  register  Done = FALSE;
  static char     Buffer[20] = { '\0' };

  RemappedString[0] = '\0';
  _PadLeft = InitialPad;
  for (CharCnt = 0 ; CharCnt < Length ; CharCnt ++)
  {
    Done = FALSE;
    Buffer[0] = '\0';
    if (RemapCharacterSet[SpectrumString[CharCnt]].Flags & F_PAD_L)                               /* This char has left padding ? */
      _PadLeft = TRUE;
    if (_PadLeft)
    {
      strcat (RemappedString + StrLength ++, " ");
      RealLength ++;
    }
    if (RemapCharacterSet[SpectrumString[CharCnt]].Flags & F_PAD_R)                              /* This char has right padding ? */
      _PadLeft = TRUE;                                                                                  /* Prepare for next token */
    else
      _PadLeft = FALSE;
    if (ToCombo)
      switch (RemapCharacterSet[SpectrumString[CharCnt]].Flags)
      {
        case F_PAPER   : if (!DropColors)
                           sprintf (Buffer, "{-%d", SpectrumString[CharCnt + 1] & 0x07);
                         SkipLength = 1;
                         Done = TRUE;
                         break;
        case F_INK     : if (!DropColors)
                           sprintf (Buffer, "{+%d", SpectrumString[CharCnt + 1] & 0x07);
                         SkipLength = 1;
                         Done = TRUE;
                         break;
        case F_BRIGHT  : if (!DropColors)
                         {
                           if (SpectrumString[CharCnt + 1] == 0)
                             strcpy (Buffer, "{+/{-/");
                           else
                             strcpy (Buffer, "{+*{-*");
                         }
                         SkipLength = 1;
                         Done = TRUE;
                         break;
        case F_INVERSE : if (!DropColors)
                         {
                           if (SpectrumString[CharCnt + 1] == 0)
                             strcpy (Buffer, "}I");
                           else
                             strcpy (Buffer, "{I");
                         }
                         SkipLength = 1;
                         Done = TRUE;
                         break;
        case F_FLASH   : if (!DropColors)
                         {
                           if (SpectrumString[CharCnt + 1] == 0)
                             strcpy (Buffer, "}L");
                           else
                             strcpy (Buffer, "{L2");
                         }
                         SkipLength = 1;
                         Done = TRUE;
                         break;
        case F_SKIP2   : SkipLength = 2;
                         Buffer[0] = '\0';
                         Done = TRUE;
                         break;
        default        : if (!strcmp (RemapCharacterSet[SpectrumString[CharCnt]].Token, "{"))
                         {
                           strcpy (Buffer, "{{");                                                      /* (Insert extra switches) */
                           RealLength ++;
                           Done = TRUE;
                         }
                         else if (!strcmp (RemapCharacterSet[SpectrumString[CharCnt]].Token, "}"))
                         {
                           strcpy (Buffer, "}}");
                           RealLength ++;
                           Done = TRUE;
                         }
                         SkipLength = 0;
      }
    else                                                                                                   /* (Writing to a file) */
    {
      SkipLength = 0;
      if (SpectrumString[CharCnt] < 0x20)
      {
        Buffer[0] = '\0';
        switch (RemapCharacterSet[SpectrumString[CharCnt]].Flags)
        {
          case F_PAPER   : if (!DropColors)
                             sprintf (Buffer, "{PAPER %d}", SpectrumString[CharCnt + 1]);
                           SkipLength = 1;
                           break;
          case F_INK     : if (!DropColors)
                             sprintf (Buffer, "{INK %d}", SpectrumString[CharCnt + 1]);
                           SkipLength = 1;
                           break;
          case F_BRIGHT  : if (!DropColors)
                             sprintf (Buffer, "{BRIGHT %d}", SpectrumString[CharCnt + 1]);
                           SkipLength = 1;
                           break;
          case F_INVERSE : if (!DropColors)
                             sprintf (Buffer, "{INVERSE %d}", SpectrumString[CharCnt + 1]);
                           SkipLength = 1;
                           break;
          case F_FLASH   : if (!DropColors)
                             sprintf (Buffer, "{FLASH %d}", SpectrumString[CharCnt + 1]);
                           SkipLength = 1;
                           break;
          case F_OVER    : if (!DropColors)
                             sprintf (Buffer, "{OVER %d}", SpectrumString[CharCnt + 1]);
                           SkipLength = 1;
                           break;
          case F_SKIP2   : if (!DropColors)
                             sprintf (Buffer, "{%s %d,%d}", SpectrumString[CharCnt] == 0x16 ? "AT" : "TAB",
                                    SpectrumString[CharCnt + 1], SpectrumString[CharCnt + 2]);
                           SkipLength = 2;
                           break;
          default        : if (SpectrumString[CharCnt] == 0x06)
                             strcpy (Buffer, "\t");
                           else if (SpectrumString[CharCnt] == 0x0D)
                             strcpy (Buffer, "\n");
                           else if (SpectrumString[CharCnt] != 0x0E)
                             sprintf (Buffer, "{%02X}", SpectrumString[CharCnt]);
                           break;
        }
        Done = TRUE;
      }
      else if (SpectrumString[CharCnt] >= 0x80 && SpectrumString[CharCnt] < 0x88)
      {
        if (SpectrumString[CharCnt] == 0x80)
          strcpy (Buffer, "{-8}");
        else
          sprintf (Buffer, "{-%X}", SpectrumString[CharCnt] - 0x80);
        Done = TRUE;
      }
      else if (SpectrumString[CharCnt] >= 0x88 && SpectrumString[CharCnt] < 0x90)
      {
        if (SpectrumString[CharCnt] == 0x8F)
          strcpy (Buffer, "{+8}");
        else
          sprintf (Buffer, "{+%X}", 7 - (SpectrumString[CharCnt] - 0x88));
        Done = TRUE;
      }
      else if (SpectrumString[CharCnt] >= 0x90 && SpectrumString[CharCnt] < 0xA3)
      {
        sprintf (Buffer, "{%c}", SpectrumString[CharCnt] - 0x90 + 'A');
        Done = TRUE;
      }
      else if (SpectrumString[CharCnt] >= 0xA3 && SpectrumString[CharCnt] < 0xA4)
        if (!strcmp (RemapCharacterSet[0xA3].Token, "SPECTRUM"))                              /* Special exception - doing 128K ? */
        {
          strcpy (Buffer, RemapCharacterSet[SpectrumString[CharCnt]].Token);
          Done = TRUE;
        }
        else
        {
          sprintf (Buffer, "{%c}", SpectrumString[CharCnt] - 0x90 + 'A');
          Done = TRUE;
        }
      else if (SpectrumString[CharCnt] == 0x7F)                                             /* Special exception - copyright sign */
      {
        strcpy (Buffer, "{(C)}");
        Done = TRUE;
      }
    }
    if (StrLength >= MaxLength)
      return (MaxLength);
    if (Done)
    {
      strcat (RemappedString + StrLength, Buffer);
      StrLength += strlen (Buffer);
    }
    else if (DropTABs && SpectrumString[CharCnt] == 0x06)
    {
      strcat (RemappedString + (StrLength ++), " ");                                                 /* Remap TAB to single space */
      RealLength ++;
    }
    else
    {
      strcat (RemappedString + StrLength, RemapCharacterSet[SpectrumString[CharCnt]].Token);
      StrLength += strlen (RemapCharacterSet[SpectrumString[CharCnt]].Token);
      RealLength += strlen (RemapCharacterSet[SpectrumString[CharCnt]].Token);
    }
    if (SpectrumString[CharCnt] == 0x0E)
      CharCnt += 5;
    else
      CharCnt += SkipLength;
  }
  *RawLength = RealLength;
  return (StrLength);
}

static void _ConvertAttributes (char *OldLine)

/**********************************************************************************************************************************/
/* Pre   : `OldLine' points to the line to be converted.                                                                          */
/* Post  : All sequences <B>, <U>, </B> and </U> have been converted to the Wprintf format.                                       */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  char  LineBuffer[1024];
  char *Line;
  char *NewLine = LineBuffer;

  Line = OldLine;
  while (*Line)
    if (*Line == '<')
    {
      if (!strnicmp (Line + 1, "B>", 2))
      {
        Line += 3;
        *(NewLine ++) = '{';
        *(NewLine ++) = 'B';
      }
      else if (!strnicmp (Line + 1, "U>", 2))
      {
        Line += 3;
        *(NewLine ++) = '{';
        *(NewLine ++) = 'U';
      }
      else if (!strnicmp (Line + 1, "/B>", 3))
      {
        Line += 4;
        *(NewLine ++) = '}';
        *(NewLine ++) = 'B';
      }
      else if (!strnicmp (Line + 1, "/U>", 3))
      {
        Line += 4;
        *(NewLine ++) = '}';
        *(NewLine ++) = 'U';
      }
    }
    else
      *(NewLine ++) = *(Line ++);
  *NewLine = '\0';
  strcpy (OldLine, NewLine);                                                                      /* Copy the converted line back */
}

static bool _RewriteTZXFile (char *FullPath, dword FileIndex, word OldLength, word NewLength, bool Make112)

/**********************************************************************************************************************************/
/* Pre   : `FullPath' holds the full path name of the TZX file, `FileIndex' holds the starting position of the requested TZX      */
/*         block, `OldLength' holds the old and `NewLength' holds the new length of that block.                                   */
/*         `Make112' is TRUE if the rewritten TZX file must have version number 1.12.                                             */
/* Post  : The TZX file has been updated. The old block has been replaced with the new block.                                     */
/*         If `OldLength' is 0, the block is now created (and `FileIndex' selects where in the tape).                             */
/*         If `NewLength' is 0, the block is removed.                                                                             */
/*         If the version of the original TZX file is lower than 1.10, it has been updated as well.                               */
/*         The return value is TRUE if everything went well.                                                                      */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  char  TZXHeader[11] = "ZXTape!\x1A\x01\x0A";
  char  TmpGameName[MAXDIR];
  int   FhIn;
  int   FhOut;
  dword FileLength;
  bool  More = TRUE;
  bool  Multiple = FALSE;
  bool  OverwriteAll = TRUE;

  MakeFullPath (TmpGameName, TmpDirectory, mktemp (NULL));
  if ((FhIn = open (FullPath, O_BINARY | O_RDONLY)) == -1)
  { ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Cannot open file %s", FullPath); return (FALSE); }
  if ((FhOut = open (TmpGameName, O_BINARY | O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1)
  { ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Cannot create new file:\n %s", strerror (errno)); close (FhIn);
    return (FALSE); }
  FileLength = filelength (FhIn);
  More = TRUE;
  if (lseek (FhIn, 10L, SEEK_SET) == -1)                                                              /* Skip original TZX header */
  { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); More = FALSE; }
  if (Make112)                                                                                /* We need to have at least v1.12 ? */
    TZXVersion = 0x010C;                                                                           /* Enforce this version number */
  if (TZXVersion >= 0x010A)                                               /* Current file has a higher version number than 1.10 ? */
  { TZXHeader[8] = (byte)((TZXVersion & 0xFF00) >> 8); TZXHeader[9] = (byte)(TZXVersion & 0x00FF); }         /* Keep that version */
  if (More)
    if (write (FhOut, TZXHeader, 10) != 10)                                            /* Write new TZX header: file version 1.10 */
    { ErrorRequester (-1, -1, "@Close", NULL, NULL, "ERROR - Write error: %s", strerror (errno)); More = FALSE; }
  if (FileIndex > 10 && More)
    switch (CopyFilePart (FhIn, FhOut, FileIndex - 10))                                   /* Copy everything before the TZX block */
    {
      case -1 : ErrorRequester (-1, -1, "@Close", NULL, NULL, _ErrOutOfMemory, 2048); More = FALSE; break;
      case  1 : ErrorRequester (-1, -1, "@Close", NULL, NULL, "ERROR - File I/O error: %s", strerror (errno));
                More = FALSE; break;
      case  0 : break;
    }
  if (More)
    if (lseek (FhIn, FileIndex + OldLength, SEEK_SET) == -1)                                               /* Skip original block */
    { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); More = FALSE; }
  if (NewLength > 0 && More)                                                       /* (This effectively removes an emptied block) */
    if (write (FhOut, _BufferDecrunched, NewLength) == -1)                                                 /* Write the new table */
    { ErrorRequester (-1, -1, "@Close", NULL, NULL, "ERROR - Write error: %s", strerror (errno)); More = FALSE; }
  if (More)
    switch (CopyFilePart (FhIn, FhOut, FileLength - FileIndex - OldLength))                              /* Copy rest of the file */
    {
      case -1 : ErrorRequester (-1, -1, "@Close", NULL, NULL, _ErrOutOfMemory, 2048); More = FALSE; break;
      case  1 : ErrorRequester (-1, -1, "@Close", NULL, NULL, "ERROR - File I/O error: %s", strerror (errno));
                More = FALSE; break;
      case  0 : break;
    }
  close (FhIn);
  close (FhOut);
  if (!More)
  { unlink (TmpGameName); return (FALSE); }
  else if (!MoveFile (TmpGameName, FullPath, &Multiple, &OverwriteAll))
  { unlink (TmpGameName); return (FALSE); }
  return (TRUE);
}

static void _ViewBASICListing (short StartX, short StartY, short TextWidth, short TextHeight, char *Header,
                               byte *Listing, byte *UDGArea, word MaxBlockSize)

/**********************************************************************************************************************************/
/* Pre   : `StartX', `StartY' holds the top-left edge to put the window, `TextWidth' holds the number of rows, `TextHeight' holds */
/*         the number of columns, `MaxEntrySize' holds the number of bytes to be reserved for a text field per combo entry.       */
/* Post  : The BASIC Listing has been displayed in a view requester.                                                              */
/* Import: RemapSpectrumUDGs.                                                                                                     */
/**********************************************************************************************************************************/

{
#define MAXVWLINES   40
  struct PrintAttributes Attributes;
  struct ButtonInfo     *ButtonP;
  struct WindowInfo      ViewWindow;
  struct ComboInfo       TextWindow;
  bool                   DropColors   = FALSE;
  bool                   RemapUDGs    = TRUE;
  struct ButtonInfo      BDropColors  = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 7, 10, 0, DBLACK, DWHITE,
                                         NULL, 1, NULL, NULL};
  struct ButtonInfo      BRemapUDGs   = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 121, 10, 0, DBLACK, DWHITE,
                                         NULL, 2, NULL, NULL};
  struct ButtonInfo      BSave        = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 234, 10, 6, DBLACK, DWHITE,
                                         NULL, 3, "@Save", NULL};
  FILE                  *Fp;
  struct ComboEntry      NEntry;
  short                  InsertedChars;
  short                  MaxEntrySize;
  comboent               OldDFirstLine = -1;
  byte                  *NextLine;
  byte                  *Maximum;
  byte                  *ListIndex;
  word                   Number;
  word                   LineNumber;
  dword                  OldLineNumber;
  bool                   StillResizing = FALSE;
  comboent               Dummy;
  char                   Line[2049];
  char                   SavePath[MAXDIR];
  char                   SaveFilename[13];
  char                   SaveExt[5];
  static char            SaveMask[13]  = "*.*";
  bool                   More          = TRUE;
  bool                   Resizing      = FALSE;
  bool                   Rereading     = TRUE;
  byte                   CFlags = COMF_NOBUFFER | COMF_VSLIDER | COMF_SYNCSLIDER | COMF_KEYSELECT;
  short                  CurrentWidth;
  short                  CurrentHeight;

  sprintf (SaveMask, "*.%s", BASICEXT);
  BDropColors.Contents = &DropColors;
  BRemapUDGs.Contents = &RemapUDGs;
  SelectFont (NULL, FONT_NORMAL);                                            /* Listings are of course in the Spectrum's own font */
  memset (&ViewWindow, 0, sizeof (struct WindowInfo));                                           /* Initialize the interface part */
  CurrentWidth           = TextWidth;
  CurrentHeight          = TextHeight;
  MaxEntrySize           = TextWidth * 2;
  ViewWindow.StartX      = StartX;
  ViewWindow.StartY      = StartY;
  ViewWindow.Width       = ViewWindow.MinWidth = SpectrumFont->Width * CurrentWidth + 27;
  ViewWindow.MaxWidth    = SpectrumFont->Width * 64 + 27;                                           /* Max width is 64 characters */
  ViewWindow.Height      = ViewWindow.MinHeight = (SpectrumFont->Height + 2) * CurrentHeight + 42;
  ViewWindow.MaxHeight   = (SpectrumFont->Height + 2) * MAXVWLINES + 42;
  ViewWindow.BorderFlags = WINB_OUT | WINB_SURROUND;
  ViewWindow.BorderSize  = 1;
  ViewWindow.APen        = SystemColors.ReqAPen;
  ViewWindow.BPen        = SystemColors.ReqOPen;
  ViewWindow.Header      = Header == NULL ? "BASIC Listing" : Header;
  ViewWindow.Flags       = WINF_ACTIVATE | WINF_HUSE | WINF_HCENTER | WINF_XMSBUFFER;

  _SpectrumColourTable[8] = (DBConfig.RemapBrightBlack ? 0 : 8);
  AddButton (&ViewWindow, &BDropColors, FALSE);
  if (IsSnapshot (FileType))
    AddButton (&ViewWindow, &BRemapUDGs, FALSE);
  AddButton (&ViewWindow, &BSave, FALSE);
  DrawWindow (&ViewWindow, NULL, WING_CLOSE | WING_RESIZEMAX, TRUE);
  Wprintf (&ViewWindow, 1, 4, "{+1Drop Colors}N");
  if (IsSnapshot (FileType))
    Wprintf (&ViewWindow, 1, 23, "{+1Remap UDGs}N");

  while (More)                                                                                    /* Handle until close requested */
  {
    if (Resizing)
    {
      CurrentWidth = (short)((ViewWindow.Width - 27) / SpectrumFont->Width);
      CurrentHeight = (short)((ViewWindow.Height - 42) / (SpectrumFont->Height + 2));
      MaxEntrySize = CurrentWidth * 2;
      ViewWindow.Width  = SpectrumFont->Width * CurrentWidth + 27;
      ViewWindow.Height = (SpectrumFont->Height + 2) * CurrentHeight + 42;
      AddButton (&ViewWindow, &BDropColors, FALSE);
      if (IsSnapshot (FileType))
        AddButton (&ViewWindow, &BRemapUDGs, FALSE);
      AddButton (&ViewWindow, &BSave, FALSE);
      DrawWindow (&ViewWindow, NULL, WING_CLOSE | WING_RESIZEMAX, TRUE);
      Wprintf (&ViewWindow, 1, 4, "{+1Drop Colors}N");
      if (IsSnapshot (FileType))
        Wprintf (&ViewWindow, 1, 23, "{+1Remap UDGs}N");
      Resizing = FALSE;
      StillResizing = TRUE;
      Rereading = TRUE;
    }
    if (Rereading)
    {
      MakeComboWindow (&TextWindow, &ViewWindow, COMT_VIEW, CFlags, ViewWindow.A_StartX + 12, ViewWindow.StartY + 32, 0, 1024,
                       MaxEntrySize, CurrentWidth, CurrentHeight, SpectrumFont, FALSE, 0, NULL, SystemColors.ReqAPen,
                       SystemColors.ReqBPen, 0, 0);
      TextWindow.ComboWindow.ColorTable = _SpectrumColourTable;
      TextWindow.ComboWindow.TABLength = 16;                                                   /* Spectrum has a TAB length of 16 */
      TextWindow.BlinkRate = DBConfig.EmulateFlash ? (DBConfig.SlowFlash ? 960 : 320) : 0;            /* Set Spectrum blink speed */
      Rereading = FALSE;
      if (IsSnapshot (FileType))
        RemapSpectrumUDGs ((bool)!Snap128K, (bool)!RemapUDGs, UDGArea);
      NextLine = Listing;
      ListIndex = Listing;
      Maximum = Listing + MaxBlockSize;
      GetComboAttributes (&TextWindow, &Attributes);
      while (ListIndex < Maximum && *ListIndex < 64 && More)                                          /* Up to the variables area */
      {
        LineNumber = (*ListIndex * 256) + *(ListIndex + 1);                                         /* Line number is big endian! */
        sprintf (Line, "%4d", LineNumber);                                                              /* Insert the line number */
        ListIndex += 2;
        Number = *ListIndex + 256 * *(ListIndex + 1);                                                              /* Line length */
        ListIndex += 2;
        NextLine = ListIndex + Number;                                                             /* Adjust pointer to next line */
        _RemapSpectrumString (ListIndex, Number, Line + 4, 2038, DropColors, TRUE, &InsertedChars, TRUE, FALSE);    /* BASIC line */
        More = FitComboString (&TextWindow, &Attributes, Line, CurrentWidth, 0, LineNumber, TRUE);             /* Insert the line */
        ListIndex = NextLine;                                                                            /* Prepare for next line */
      }
      if (!More)                                                                                /* Listing truncated prematurally */
      {
        if (TextWindow.NumEntries == -1)                                                                     /* No entry at all ? */
          FatalError (1, _ErrOutOfMemory, 0);                                                                        /* Then quit */
        GetComboEntry (&TextWindow, &NEntry, TextWindow.NumEntries);
        strcpy (NEntry.Text, "{+1>>> TRUNCATED <<<}N");
        PutComboEntry (&TextWindow, &NEntry, TextWindow.NumEntries, FALSE);
        More = TRUE;
      }
      if (StillResizing)
      {
        for (Dummy = 0 ; StillResizing && Dummy <= TextWindow.NumEntries ; Dummy ++)
        {
          GetComboEntry (&TextWindow, &NEntry, Dummy);
          if (NEntry.Identifier == OldLineNumber)
          {
            TextWindow.DFirstLine = Dummy;
            TextWindow.VSlider->SliderStart = TextWindow.VSlider->StepRate * Dummy;
            StillResizing = FALSE;
          }
        }
        StillResizing = FALSE;
      }
      else if (OldDFirstLine >= 0)
      {
        TextWindow.DFirstLine = OldDFirstLine;
        TextWindow.VSlider->SliderStart = TextWindow.VSlider->StepRate * OldDFirstLine;
        OldDFirstLine = -1;
      }
      PrintComboEntries (&TextWindow);                                                                      /* Print the contents */
    }
    HandleComboWindow (&TextWindow, &Dummy);
    if (GlobalKey == K_ESC)
      More = FALSE;
    else
    {
      switch (HandleWindow (&ViewWindow, &ButtonP))
      {
        case WINH_CLOSE     : More = FALSE; break;
        case WINH_BUTTBOOL  : OldDFirstLine = TextWindow.DFirstLine;                               /* (Drop Colors or Remap UDGs) */
                              DestroyComboWindow (&TextWindow, 0);
                              Rereading = TRUE;
                              break;
        case WINH_WINRESIZE : Resizing = TRUE;
                              GetComboEntry (&TextWindow, &NEntry, TextWindow.DFirstLine);
                              OldLineNumber = NEntry.Identifier;                     /* Remember which line number was at the top */
                              DestroyComboWindow (&TextWindow, 0);                             /* Full destroy, as we must reread */
                              DestroyWindow (&ViewWindow);
                              break;
        case WINH_RNOHAND   : _splitpath (FullPath, SavePath, Line, SaveFilename, SaveExt);
                              strcat (SavePath, Line);
                              strcat (SaveFilename, ".");
                              strcat (SaveFilename, BASICEXT);
                              if (FileRequester (180, 50, 48, 29, 20, SavePath, SaveFilename, SaveMask, "Choose BASIC Save Name",
                                  FALSE, FALSE))
                              {
                                MakeFullPath (Line, SavePath, SaveFilename);
                                if (!TestFileOverwrite (Line))
                                  if ((Fp = fopen (Line, "w")) == NULL)
                                    ErrorRequester (-1, -1, "@Close", NULL, NULL, "Error creating file %s:\n    %s",
                                                    Line, StrError (errno));
                                  else
                                  {
                                    NextLine = Listing;
                                    ListIndex = Listing;
                                    Maximum = Listing + MaxBlockSize;
                                    while (ListIndex < Maximum && *ListIndex < 64 && More)
                                    {
                                      Number = (*ListIndex * 256) + *(ListIndex + 1);
                                      sprintf (Line, "%4d", Number);
                                      ListIndex += 2;
                                      Number = *ListIndex + 256 * *(ListIndex + 1);
                                      ListIndex += 2;
                                      NextLine = ListIndex + Number;
                                      _RemapSpectrumString (ListIndex, Number, Line + 4, 2038, DropColors, TRUE, &InsertedChars,
                                                            FALSE, FALSE);
                                      fputs (Line, Fp);
                                      ListIndex = NextLine;
                                    }
                                    fclose (Fp);
                                    SoundErrorBeeper (BEEP_SHORT);
                                  }
                              }
                              break;
      }
      if (More && GlobalKey == K_ESC)
        More = FALSE;
    }
    MouseStatus ();
  }
  DestroyComboWindow (&TextWindow, 0);
  DestroyWindow (&ViewWindow);
  if (IsSnapshot (FileType))
    RemapSpectrumUDGs (TRUE, TRUE, NULL);                                                                       /* Reset the UDGs */
}

static void _ViewMemoryDump (byte *Start, word BlockLength, word FirstAddress)

/**********************************************************************************************************************************/
/* Pre   : `Start' points to the start address, `BlockLength' holds the length of the block.                                      */
/*         `FirstAddress' holds the first address to be displayed (added to the starting address).                                */
/* Post  : The selected block has been shown as a dump in a seperate window, 16 bytes a line, both hex and ASCII.                 */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct MouseShape *OldShape;
  struct ButtonInfo *ButtonP;
  struct WindowInfo  ViewWindow  = {20, 41, 599, 398, 599, 398, 599, 398, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                    WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "Block Dump"};
  struct ComboInfo   TextWindow;
  struct ComboEntry  Entry;
  bool               More = TRUE;
  comboent           Dummy;
  word               Index;
  word               SubIndex;
  short              LineIndex;
  byte               Bt;

  DrawWindow (&ViewWindow, NULL, WING_CLOSE, TRUE);
  MakeComboWindow (&TextWindow, &ViewWindow, COMT_VIEW, COMF_NOBUFFER | COMF_VSLIDER | COMF_SYNCSLIDER | COMF_KEYSELECT,
                   ViewWindow.StartX - 4, ViewWindow.StartY - 2, 0, 1024, 91, 74, MAXVWLINES, SpectrumFont, FALSE, 0, NULL,
                   SystemColors.ReqAPen, SystemColors.ReqBPen, 0, 0);
  OldShape = CurrentMouseShape;
  SetMouseShape (DefaultWaitingMouseShape);
  Index = 0;
  Entry.Selected = 0;
  Entry.Identifier = 0;
  while (Index < BlockLength && More)
  {
    memset (Entry.Text, ' ', 74);
    sprintf (Entry.Text, "%5u - ", Index + FirstAddress);
    LineIndex = 8;
    SubIndex = 0;
    while (SubIndex < 16 && Index + SubIndex < BlockLength)                                                 /* Build the hex part */
    {
      sprintf (Entry.Text + LineIndex, "%02X ", *(Start + Index + SubIndex));
      LineIndex += 3;
      Entry.Text[LineIndex] = ' ';                                                                /* Don't want string endmarkers */
      if (++ SubIndex == 8)
        LineIndex ++;                                                                                  /* Add extra blank at half */
    }
    LineIndex = 58;
    SubIndex = 0;
    while (SubIndex < 16 && Index + SubIndex < BlockLength)                                               /* Build the ASCII part */
    {
      Bt = *(Start + Index + SubIndex);
      if (Bt >= 32 && Bt < 128)
      {
        if (Bt == '{' || Bt == '}')                                                                                 /* Plain text */
          Entry.Text[LineIndex ++] = Bt;
        Entry.Text[LineIndex ++] = Bt;
      }
      else
        Entry.Text[LineIndex ++] = '.';
      SubIndex ++;
    }
    Index += 16;
    Entry.Text[LineIndex] = '\0';                                                                            /* Add the endmarker */
    More = InsertComboEntry (&TextWindow, &Entry, NULL, -1);                                                   /* Insert the line */
  }
  if (!More)                                                                                    /* Listing truncated prematurally */
  {
    if (TextWindow.NumEntries == -1)                                                                         /* No entry at all ? */
      FatalError (1, _ErrOutOfMemory, 0);                                                                            /* Then quit */
    GetComboEntry (&TextWindow, &Entry, TextWindow.NumEntries);
    strcpy (Entry.Text, "{+1>>> TRUNCATED <<<}N");
    PutComboEntry (&TextWindow, &Entry, TextWindow.NumEntries, FALSE);
    More = TRUE;
  }
  PrintComboEntries (&TextWindow);                                                                          /* Print the contents */
  SetMouseShape (OldShape);
  while (More)
  {
    HandleComboWindow (&TextWindow, &Dummy);
    if (GlobalKey == K_ESC)
      More = FALSE;
    else
      if (HandleWindow (&ViewWindow, &ButtonP) == WINH_CLOSE)
        More = FALSE;
    MouseStatus ();
  }
  DestroyComboWindow (&TextWindow, 0);
  DestroyWindow (&ViewWindow);
}

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE EXPORTED LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

bool GetConfNewFileName (char *Operation, char *OldName, char *ExtraOptions, char *ExtReason)

/**********************************************************************************************************************************/
/* Pre   : `Operation' is a string for the file requester header, `OldName' is the current path name.                             */
/*         `ExtraOptions' is non-NULL if extra command line options may be required to `ExtReason'.                               */
/*         Both `OldName' and `ExtraOption' must be path fields in the TPConfig structure!                                        */
/* Post  : A new filename has been asked with a FileRequester, which is returned in `OldName'.                                    */
/*         If a new filename has been selected, TRUE is returned.                                                                 */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  char           Path[MAXDIR + 1];
  char           File[13];
  char           Mask[13] = "*.*";
  char           ExtBuffer[65];
  short register Cnt;
  bool           Selected;

  if (OldName[0] != '\0')                                                                              /* A path has been given ? */
  {
    Cnt = strlen (OldName);
    while (-- Cnt >= 0 && OldName[Cnt] != '\\' && OldName[Cnt] != ':')
      ;
    if (Cnt >= 0)                                                                                    /* Is there a leading path ? */
    {
      strncpy (Path, OldName, Cnt);
      Path[Cnt] = '\0';
    }
    else
      Path[0] = '\0';
    strcpy (File, OldName + Cnt + 1);
  }
  else
    Path[0] = File[0] = '\0';
  if ((Selected = FileRequester (180, 50, 48, 29, 20, Path, File, Mask, Operation, TRUE, FALSE)))
  {
    MakeFullPath (OldName, Path, File);
    if (ExtraOptions != NULL)                                              /* The program could need extra command line options ? */
    {
      if (InputRequester (-1, -1, NULL, NULL, ExtBuffer, 64, TRUE, FALSE, "Add Command-Line Options",
                          "If this program requires extra command line options to %s, please provide these now.", ExtReason))
        strcpy (ExtraOptions, ExtBuffer);
    }
  }
  if (Selected || MaxFloppyIdChanged)
    WriteConfig ();
  return (Selected);
}

bool SpawnApplication (char *Name, char *Path, char *Options, char *File)

/**********************************************************************************************************************************/
/* Pre   : `Path' holds the path to the application, `Options' hold any extra command line options, `File' holds the file to be   */
/*         processed by the spawned application.                                                                                  */
/* Post  : The application has been run from its own directory.                                                                   */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  char           Params[300] = {"\0"};
  char           PathBuffer[MAXDIR];
  char           StartPath[MAXDIR];
  char           StartName[13];
  unsigned       Dummy;
  bool           SpawnReturn;
  short register Cnt;

  if (Path[0] == '\0')                                                                                       /* Path not set up ? */
  {
    ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: Can not run the %s, since you have not defined its path in the "
                    "configuration!", Name);
    return (FALSE);
  }
  Cnt = strlen (Path);                                                                                 /* Split path and filename */
  while (-- Cnt >= 0 && Path[Cnt] != '\\' && Path[Cnt] != ':')
    ;
  if (Cnt >= 0)                                                                                   /* Is there a preceeding path ? */
  {
    if (Cnt > 0)
      strncpy (StartPath, Path, Cnt);
    StartPath[Cnt] = '\0';
    if (StartPath[1] == ':')                                                                            /* Preceeded by a drive ? */
    {
      _dos_setdrive (ToNumber (StartPath[0]), &Dummy);                                                      /* Change drive first */
      getcwd (PathBuffer, MAXDIR);
      if (StartPath[2] != '\0')                                                                                 /* Only a drive ? */
        chdir (StartPath + 2);                                                                     /* Change directory too if not */
      else if (Cnt == 2 && Path[2] == '\\')
        chdir ("\\");
    }
    else
    {
      getcwd (PathBuffer, MAXDIR);
      if (Cnt == 0 && Path[0] == '\\')
        chdir ("\\");
      else
        chdir (StartPath);                                                                          /* No drive, just change path */
    }
  }
  strcpy (StartName, Path + Cnt + 1);
  if (Options)                                                                         /* Do we have extra command line options ? */
  {
    strcat (Params, Options);
    strcat (Params, " ");
  }
  strcat (Params, File);                                                                                /* Add file to be handled */
  SpawnReturn = SwapOut (TRUE, StartName, Params, NULL, NULL, FALSE, FALSE, FALSE);                      /* Start the application */
  chdir (PathBuffer);
  _dos_setdrive (CurDrv, &Dummy);
  chdir (CurPath);                                                                                 /* Restore drive and directory */
  return (SpawnReturn);
}

void RemapSpectrumUDGs (bool Assume48K, bool Reset, byte *AlternateMap)

/**********************************************************************************************************************************/
/* Pre   : `Assume48K' is TRUE if the snapshot is 48K, `Reset' is TRUE if the font should be reset.                               */
/*         If `Reset' is FALSE, `AlternateMap' points to the new font data to be used.                                            */
/* Post  : The UDG characters have been adjusted. The TokenMap has been adjusted as well.                                         */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  word MapSize;

  SelectFont (SpectrumFont, FONT_NORMAL);                                                /* Ensure the Spectrum font is in memory */
  if (Assume48K)
  {
    MapSize = ('V' -'A') * 8;
    memcpy (SpectrumFont->NormalData + (SpectrumFont->Height * 144) + 8, Reset ? SpectrumUDGs : AlternateMap, MapSize);
    strcpy (TokenMap[163].Token, "\xA3");                                                            /* 'T' and 'U' exist as UDGs */
    TokenMap[163].Flags = 0x00;
    strcpy (TokenMap[164].Token, "\xA4");
    TokenMap[164].Flags = 0x00;
  }
  else
  {
    MapSize = ('T' -'A') * 8;
    memcpy (SpectrumFont->NormalData + (SpectrumFont->Height * 144) + 8, Reset ? SpectrumUDGs : AlternateMap, MapSize);
    strcpy (TokenMap[163].Token, "SPECTRUM");
    TokenMap[163].Flags = F_PAD_L | F_PAD_R;
    strcpy (TokenMap[164].Token, "PLAY");
    TokenMap[164].Flags = F_PAD_L | F_PAD_R;
  }
}

bool TestFileLocation (struct DBaseEntry_s *Entry)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the entry to be tested.                                                                              */
/* Post  : The return value is TRUE only if the game file is still at the (valid) previous location.                              */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  int  register LocCnt;
  char          TotalPath[MAXDIR];
  struct stat   StatInfo;

  if (Entry->DirIndex[0] != '\0')
  {
    LocCnt = atoi (Entry->DirIndex);                                                             /* Try the latest found location */
    if (LocCnt < DBConfig.NumLocs)                                                               /* Directory number is defined ? */
    {
      MakeFullPath (TotalPath, DBConfig.FileLocMapping[LocCnt].Path, Entry->PCName);
      if (IsFloppy (TotalPath[0]))
        return (TRUE);                                                                        /* Assume files on floppy are there */
      return (!stat (TotalPath, &StatInfo) && !(StatInfo.st_mode & S_IFDIR) ? TRUE : FALSE);
    }
  }
  return (FALSE);
}

bool ResolveFileLocation (struct DBaseEntry_s *Entry, char *ResolvedPath, bool *PathChanged)

/**********************************************************************************************************************************/
/* Pre   : `Entry' points to the current entry, `ResolvedPath' points to the outputbuffer, `PathChanged' is output too.           */
/* Post  : The path has been resolved that holds this file. If the path could not be resolved, FALSE is returned. Otherwise TRUE  */
/*         is returned and `ResolvedPath' holds a copy of the path. The Spectrum token set has been initialized.                  */
/*         If the DirIndex field of the entry pointed to a directory other then where the file was found, `PathChanged' is TRUE.  */
/*         Also the field FileSize has been tested and updated if a file with different size was found.                           */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  int  register LocCnt;
  char          OldLocation[D_DIRINDEX + 1];
  bool          Resolved = FALSE;
  char          TotalPath[MAXDIR];
  struct stat   StatInfo;
  long          OldSize;

  *PathChanged = FALSE;
  RemapCharacterSet = TokenMap;
  if (Entry->FileSize[0] == '\0')                                                                    /* Filesize has been given ? */
    OldSize = 0;
  else
    OldSize = atol (Entry->FileSize);                                                                        /* Keep a copy of it */
  strcpy (OldLocation, Entry->DirIndex);
  if (Entry->DirIndex[0] != '\0')
  {
    LocCnt = atoi (Entry->DirIndex);                                                             /* Try the latest found location */
    if (LocCnt < DBConfig.NumLocs)                                                               /* Directory number is defined ? */
    {
      MakeFullPath (TotalPath, DBConfig.FileLocMapping[LocCnt].Path, Entry->PCName);
      if (DBConfig.AssumeStableFloppies && IsFloppy (TotalPath[0]))
      {
        strcpy (ResolvedPath, DBConfig.FileLocMapping[LocCnt].Path);
        return (TRUE);
      }
      if (!RequestFloppy (TotalPath[0], Entry->DiskId))
        return (FALSE);
      Resolved = !stat (TotalPath, &StatInfo) && !(StatInfo.st_mode & S_IFDIR) ? TRUE : FALSE;
    }
  }
  if (!Resolved)
  {
    for (LocCnt = 0 ; LocCnt < DBConfig.NumLocs && !Resolved ; LocCnt ++)                       /* Not there; try all other paths */
    {
      MakeFullPath (TotalPath, DBConfig.FileLocMapping[LocCnt].Path, Entry->PCName);
      if (IsFloppy (TotalPath[0]))                                                         /* Don't resolve floppies in this case */
        Resolved = FALSE;
      else if (Resolved = !stat (TotalPath, &StatInfo) && !(StatInfo.st_mode & S_IFDIR) ? TRUE : FALSE)
      {                                                                                                /* Found in this directory */
        sprintf (Entry->DirIndex, "%d", LocCnt);
        *PathChanged = TRUE;
      }
    }
    LocCnt --;
  }
  if (!Resolved)
  {
    *PathChanged = (strcmp (OldLocation, DI_UNKNOWN) ? TRUE : FALSE);
    strcpy (Entry->DirIndex, DI_UNKNOWN);
    return (FALSE);
  }
  if (StatInfo.st_size != OldSize)                                                                      /* Filesize has changed ? */
  {
    if (StatInfo.st_size > 9999999)                                                              /* More than D_FILESIZE digits ? */
      strcpy (Entry->FileSize, "+");                                                      /* Cannot handle that (nonsense anyway) */
    else
      sprintf (Entry->FileSize, "%7ld", StatInfo.st_size);
    *PathChanged = TRUE;
  }
  strcpy (ResolvedPath, DBConfig.FileLocMapping[LocCnt].Path);
  if (!IsFloppy (ResolvedPath[0]) && Entry->DiskId > 0)                                    /* Not a floppy, but has a floppy id ? */
  {
    Entry->DiskId = 0;
    *PathChanged = TRUE;
  }
  return (TRUE);
}

bool GetZ80Version (char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `FullPath' holds the complete path to the file.                                                                        */
/* Post  : The version of the .Z80 file has been determined. If it is a valid file, TRUE is returned.                             */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  int    Fh;
  dword  FileLength;

  Z80VersionValid = FALSE;
  FileNotFound = TRUE;
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return (FALSE);
  FileNotFound = FALSE;
  if ((FileLength = filelength (Fh)) < sizeof (Z80145SnapshotHeader))
  { close (Fh); return (FALSE); }
  if (read (Fh, &Z80145SnapshotHeader, sizeof (Z80145SnapshotHeader)) == -1)
  { close (Fh); return (FALSE); }
  if (Z80145SnapshotHeader.Stat1 == 0xFF)
    Z80145SnapshotHeader.Stat1 = 0x01;
  if (Z80145SnapshotHeader.PC != 0)
  {
    Z80Version = 145;
    close (Fh);
    Z80VersionValid = TRUE;
    return (TRUE);
  }
  if (FileLength < (sizeof (Z80145SnapshotHeader) + sizeof (Z80201SnapshotHeader)))
  { close (Fh); return (FALSE); }
  if (read (Fh, &Z80201SnapshotHeader, sizeof (Z80201SnapshotHeader)) == -1)
  { close (Fh); return (FALSE); }
  Z80Version = 201;
  if (Z80201SnapshotHeader.AdditionalLength == 54)
  {
    if (FileLength < (sizeof (Z80145SnapshotHeader) + sizeof (Z80201SnapshotHeader) + sizeof (Z80300SnapshotHeader)))
    { close (Fh); return (FALSE); }
    if (read (Fh, &Z80300SnapshotHeader, sizeof (Z80300SnapshotHeader)) == -1)
    { close (Fh); return (FALSE); }
    Z80Version = 300;
  }
  Z80VersionValid = TRUE;
  close (Fh);
  return (TRUE);
}

bool GetTZXVersion (char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `FullPath' holds the complete path to the file.                                                                        */
/* Post  : The version of the .TZX file has been determined. If it is a valid file, TRUE is returned.                             */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  int    Fh;
  dword  FileLength;
  char   TZXBuffer[10];
  bool   AllOk = TRUE;

  TZXVersionValid = FALSE;
  FileNotFound = TRUE;
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return (FALSE);
  FileNotFound = FALSE;
  if ((FileLength = filelength (Fh)) < 10)
  { close (Fh); return (FALSE); }
  if (read (Fh, TZXBuffer, 10) == -1)
  { close (Fh); return (FALSE); }
  close (Fh);
  if (!strncmp (TZXBuffer, "ZXTape!\x1A", 8))                                          /* Does it start with the TZX identifier ? */
  {
    TZXVersionValid = TRUE;
    if (TZXBuffer[8] == 10 && TZXBuffer[9] == 1)                             /* An older MakeTZX reversed the number due to a bug */
    {
      TZXVersion = 0x010A;
      TZXBuffer[8] = (byte)1;                                                                           /* (Store correct number) */
      TZXBuffer[9] = (byte)10;                      
      if (!QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Invalid TZX Version",
                              "{BWARNING}B: The version number of this TZX file is invalid (reversed) and appears to be "
                              "created with an older release of MakeTZX. Emulators may not recognize (parts of) this file.\n"
                              "Would you like to have the version number corrected?"))
      {
        AllOk = TRUE;
        if ((Fh = open (FullPath, O_BINARY | O_RDWR)) == -1)
          AllOk = FALSE;
        else
        {
          if (write (Fh, TZXBuffer, 10) == -1)
            AllOk = FALSE;
          close (Fh);
        }
        if (AllOk)
          SoundErrorBeeper (BEEP_SHORT);
        else                                                                    /* (The file is still considered ZXVersionValid!) */
          ErrorRequester (-1, -1, "@Close", NULL, NULL, "ERROR: Cannot modify file:\n%s", StrError (errno));
      }
    }
    else
      TZXVersion = TZXBuffer[8] * 256 + TZXBuffer[9];
  }
  return (TZXVersionValid);
}

bool GetSLTExtensionOffset (char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `FullPath' holds the full path to the .SLT file.                                                                       */
/* Post  : The file is checked for consistency and a map of the SLT extensions has been created in `SLTMap'.                      */
/*         The return value is FALSE if any error occured.                                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct BlockInfo_s
  {
    word     Length;
    byte     Number;
  }                     BlockInfo;
  struct SLTExtension_s SLTExtensionHeader;
  char                  SLTSignature[3];
  int                   Fh;
  dword                 FileLength;
  dword                 ExtensionIndex;
  dword                 Offset        = sizeof (struct Z80145SnapshotHeader_s);
  word                  NRead;
  bool                  EndFound;

  FileNotFound = TRUE;
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return (FALSE);
  FileNotFound = FALSE;
  FileLength = filelength (Fh);
  //if (Z80Version == 145) Impossible!

  Offset += sizeof (struct Z80201SnapshotHeader_s);
  if (Z80Version == 300)
    Offset += sizeof (struct Z80300SnapshotHeader_s);
  if (lseek (Fh, Offset, SEEK_SET) == -1)
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath); return (FALSE); }
  if ((NRead = read (Fh, &BlockInfo, 3)) == -1)
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath); return (FALSE); }
  EndFound = (NRead == 3 ? FALSE : TRUE);
  while (!EndFound)                                                                                        /* More block follow ? */
  {
    if (BlockInfo.Number != 0x00 || BlockInfo.Length != 0x0000)                                              /* End of Z80-part ? */
    {                                                                                                            /* No, just skip */
      Offset += BlockInfo.Length + sizeof (struct BlockInfo_s);
      if (Offset > FileLength)
      { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _InvalidSLTFile, FullPath); return (FALSE); }
      if (lseek (Fh, Offset, SEEK_SET) == -1)
      { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath); return (FALSE); }
      if ((NRead = read (Fh, &BlockInfo, 3)) == -1)
      { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath); return (FALSE); }
    }
    else                                                                                                                  /* YES! */
    {
      EndFound = TRUE;
      if ((NRead = read (Fh, SLTSignature, 3)) == -1)
      { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath); return (FALSE); }
      if (NRead != 3)
      { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, "File %s has a missing endmarker", FullPath); return (FALSE); }
      if (strncmp (SLTSignature, "SLT", 3))
      { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, "File %s has a bad SLT signature (\"%s\")", FullPath,
        SLTSignature); return (FALSE); }
      Offset += 6;
    }
  }
  SLTMap.TableStart = Offset;
  SLTMap.NumEntries = 0;
  SLTMap.InfoIncluded = FALSE;
  SLTMap.ScrShotIncluded = FALSE;
  SLTMap.PokesIncluded = FALSE;
  ExtensionIndex = 0L;
  if ((NRead = read (Fh, &SLTExtensionHeader, sizeof (struct SLTExtension_s))) == -1)
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath); return (FALSE); }
  if (NRead != sizeof (struct SLTExtension_s))
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _InvalidSLTFile, FullPath); return (FALSE); }
  EndFound = (SLTExtensionHeader.DataType == SLT_END ? TRUE : FALSE);
  Offset += NRead;
  while (!EndFound)
  {
    SLTMap.NumEntries ++;
    switch (SLTExtensionHeader.DataType)
    {
      case SLT_INFO    : SLTMap.InfoIncluded = TRUE;
                         SLTMap.OffsetInfo = ExtensionIndex;
                         SLTMap.HeaderInfo.DataType = SLTExtensionHeader.DataType;
                         SLTMap.HeaderInfo.Level = SLTExtensionHeader.Level;
                         SLTMap.HeaderInfo.Length = SLTExtensionHeader.Length;
                         break;
      case SLT_SCRSHOT : SLTMap.ScrShotIncluded = TRUE;
                         SLTMap.OffsetScrShot = ExtensionIndex;
                         SLTMap.HeaderScrShot.DataType = SLTExtensionHeader.DataType;
                         SLTMap.HeaderScrShot.Level = SLTExtensionHeader.Level;
                         SLTMap.HeaderScrShot.Length = SLTExtensionHeader.Length;
                         break;
      case SLT_POKES   : SLTMap.PokesIncluded = TRUE;
                         SLTMap.OffsetPokes = ExtensionIndex;
                         SLTMap.HeaderPokes.DataType = SLTExtensionHeader.DataType;
                         SLTMap.HeaderPokes.Level = SLTExtensionHeader.Level;
                         SLTMap.HeaderPokes.Length = SLTExtensionHeader.Length;
                         break;
    }
    ExtensionIndex += SLTExtensionHeader.Length;
    if ((NRead = read (Fh, &SLTExtensionHeader, sizeof (struct SLTExtension_s))) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath); return (FALSE); }
    if (NRead != sizeof (struct SLTExtension_s))
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _InvalidSLTFile, FullPath); return (FALSE); }
    EndFound = (SLTExtensionHeader.DataType == SLT_END ? TRUE : FALSE);
    Offset += NRead;
  }
  SLTMap.TableEnd = Offset;
  close (Fh);
  return (TRUE);
}

bool InsertTAPIndex (struct ComboInfo *Window, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the combo window where the catalog of the .TAP file with full path `FullPath' should go.            */
/* Post  : A catalog of the file has been appended to the entry table of the combo window. A line is built up as follows:         */
/*         1. (Expanded) Spectrum file name;                                                                                      */
/*         2. Type of file: PROG, CODE, A-AR, N-AR, HEAD, ????;                                                                   */
/*         3. Depending on the type:                                                                                              */
/*            PROG (Program)                 : Auto-start line or ----- if none;                                                  */
/*            CODE (Code)                    : Startaddress of the code;                                                          */
/*            A-AR (Alphanumerical Array)    : -                                                                                  */
/*            N-AR (Numerical Array)         : -                                                                                  */
/*            HEAD (Headerless block)        : The Flag byte;                                                                     */
/*            ???? (Headerless block)        : The Type byte;                                                                     */
/*         4. Length of the block.                                                                                                */
/*         Headerless blocks with total length < 2 are reported as being noise (multiple blocks are reported once).               */
/*         CODE and headerless blocks are selectable, all others are not.                                                         */
/*         If the entry is selecteable, the `Identifier' field holds the file index to the code block in question.                */
/*         The return value is FALSE if a read error occured.                                                                     */
/* Import: _RemapSpectrumString.                                                                                                  */
/**********************************************************************************************************************************/

{
  struct MouseShape   *OldShape;
  struct TapeRecord_s *TRecord;
  struct ComboEntry    NewEntry;
  int                  Fh;
  dword                FileLength;
  dword                FileIndex;
  word                 NRead;
  word                 BlLength;                                                                  /* Length of one TAP file block */
  word       register  M;
  byte       register  LastFlag;
  char                 FType[49];
  bool                 Noise = FALSE;
  short                TokLen;

  NewEntry.Selected = 0;
  NewEntry.Identifier = 0;
  FileNotFound = TRUE;
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return (TRUE);
  if ((TRecord = (struct TapeRecord_s *)malloc (sizeof (struct TapeRecord_s))) == NULL)
  { close (Fh); FatalError (1, _ErrOutOfMemory, sizeof (struct TapeRecord_s)); }
  OldShape = CurrentMouseShape;
  SetMouseShape (DefaultWaitingMouseShape);
  FileNotFound = FALSE;
  FileLength = filelength (Fh);
  if (FileLength == 0)                                                                                       /* File of 0 bytes ? */
  {
    NewEntry.Selected = 0x01 | ENT_NOSELECT;
    strcpy (NewEntry.Text, "<Empty tape>");
    if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
      FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
    free (TRecord);
    close (Fh);
    SetMouseShape (OldShape);
    return (TRUE);
  }
  LastFlag = 0xFF;                                                                      /* Signal: last block was of unknown type */
  FileIndex = (dword)0;
  if ((NRead = read (Fh, &BlLength, 2)) == -1)                                                          /* Can I read two bytes ? */
  { free (TRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
    SetMouseShape (OldShape); return (FALSE); }                                                               /* Return the error */
  while (NRead == 2)                                                                     /* Then there is at least one more block */
  {
    if (FileType == FT_LTP)
      BlLength += 2;                                                     /* (.LTP files don't include flag/parity into the count) */
    if (BlLength < 2)                                                                        /* Flag + block + parity < 2 bytes ? */
    {
      if (!Noise)                                                                            /* Noise has not yet been reported ? */
      {
        Noise = TRUE;
        strcpy (NewEntry.Text, "            >>>>> NOISE <<<<<");
        NewEntry.Selected = ENT_NOSELECT;
        if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
          FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
      }
    }
    else
    {
      Noise = FALSE;                                                                               /* Reset 'noise reported' flag */
      if ((NRead = read (Fh, TRecord, HDRLEN - 2)) == -1)                                 /* Read the block as a header in memory */
      { free (TRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
        SetMouseShape (OldShape); return (FALSE); }
      if (TRecord->Flag != 0x00 || (TRecord->Flag == 0x00 && BlLength != HDRLEN))                           /* It is a data block */
      {
        if (LastFlag != 0x00)                                                            /* Last read block was also a data block */
        {
          sprintf (NewEntry.Text, "            HEAD  %5u  %5u", TRecord->Flag, BlLength - 2);      /* So it is a headerless block */
          NewEntry.Identifier = FileIndex;                                                          /* Point to the current block */
          NewEntry.Selected = 0x00;
          if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
            FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
        }
        else                                                                          /* It belongs to the previously read header */
          LastFlag = (TRecord->Flag == 0x00 ? 0xFF : TRecord->Flag);
      }
      else                                                                                                /* It is a header block */
      {
        if (LastFlag == 0x00)                                                           /* Previous block was also a header block */
        {
          strcpy (NewEntry.Text, ">>>>> Missing datablock");                                                  /* This is weird... */
          NewEntry.Selected = ENT_NOSELECT;
          if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
            FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
          NewEntry.Text[0] = '\0';
        }
        else
          LastFlag = 0x00;
        M = _RemapSpectrumString (TRecord->HName, 10, NewEntry.Text, 256, FALSE, FALSE, &TokLen, TRUE, TRUE);
        NewEntry.Selected = ENT_NOSELECT;                                                          /* Insert filename from header */
        NewEntry.Text[M] = '\0';
        if (TokLen > 10)                                                                                    /* Over 10 expanded ? */
        {
          while (TokLen > 10 && NewEntry.Text[M - 1] == ' ')                                 /* Try eliminating spaces at the end */
          {
            M --;
            TokLen --;
          }
          NewEntry.Text[M] = '\0';
          strcat (NewEntry.Text, "{++{--}L");
        }
        else if (TokLen < 10)                                                     /* Unexpandable tokens were removed (AT, TAB) ? */
        {
          strcat (NewEntry.Text, "{++{--}L");                                            /* Ensure there's an attribute endmarker */
          strcat (NewEntry.Text, "          ");                                                     /* Pad the result with spaces */
          NewEntry.Text[M + 18 - TokLen] = '\0';
          TokLen = 10;
        }
        else
          strcat (NewEntry.Text, "{++{--}L");
        if (TokLen > 10)
        {
          if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
            FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
          strcpy (NewEntry.Text, "          ");
          M = 10;
        }
        switch (TRecord->HType)                                                                         /* Block type from header */
        {
          case 0 : if (TRecord->HStart > 32767)
                     strcpy (FType, "  PROG  -----  ");
                   else
                     sprintf (FType, "  PROG  %5u  ", TRecord->HStart);
                   sprintf (FType + strlen (FType), "%5u", TRecord->HLength);
                   NewEntry.Identifier = FileIndex + BlLength + 2;                                     /* Point to the next block */
                   NewEntry.Selected = BASICBLOCK;                                                    /* Selection prints listing */
                   break;
          case 1 : sprintf (FType, "  N-AR         %5u", TRecord->HLength);
                   break;
          case 2 : sprintf (FType, "  A-AR         %5u", TRecord->HLength);
                   break;
          case 3 : sprintf (FType, "  CODE  %5u  %5u", TRecord->HStart, TRecord->HLength);
                   NewEntry.Identifier = FileIndex + BlLength + 2;                                     /* Point to the next block */
                   NewEntry.Selected = 0x00;
                   break;
          default: sprintf (FType, "  ????  %5u  %5u", TRecord->HType, BlLength - 2);      /* Usually in M1 blocks; just 17 bytes */
                   LastFlag = 0xFF;                                                   /* Signal: also print info on the datablock */
        }
        strcat (NewEntry.Text, FType);
        if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
          FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
      }
    }
    FileIndex += BlLength + 2;                                                                               /* Update file index */
    if (lseek (Fh, FileIndex, SEEK_SET) == -1)                                                            /* Go to the next block */
    { free (TRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno));
      SetMouseShape (OldShape); return (FALSE); }
    if ((NRead = read (Fh, &BlLength, 2)) == -1)                                                        /* Can I read two bytes ? */
    { free (TRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
      SetMouseShape (OldShape); return (FALSE); }
  }
  if (LastFlag == 0x00 && BlLength == HDRLEN)                                                 /* Last block was not a datablock ? */
  {
    strcpy (NewEntry.Text, ">>>>> Missing datablock");                                                        /* This is weird... */
    NewEntry.Selected = ENT_NOSELECT;
    if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
      FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
  }
  free (TRecord);
  close (Fh);
  SetMouseShape (OldShape);
  return (TRUE);
}

bool InsertMDRIndex (struct ComboInfo *Window, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the combo window where the catalog of the .MDR file with full path `FullPath' should go.            */
/* Post  : A catalog of the file has been appended to the entry table of the combo window. A line is built up as follows:         */
/*         1. (Expanded) Spectrum file name;                                                                                      */
/*         2. Type of file: PROG, CODE, A-AR, N-AR, ????;                                                                         */
/*         3. Depending on the type:                                                                                              */
/*            PROG (Program)                 : Auto-start line or ----- if none;                                                  */
/*            CODE (Code)                    : Startaddress of the code;                                                          */
/*            A-AR (Alphanumerical Array)    : -                                                                                  */
/*            N-AR (Numerical Array)         : -                                                                                  */
/*            ???? (Unknown block)           : The type byte;                                                                     */
/*         4. Length of the block (Unless type ????).                                                                             */
/*         Hidden files (first filename char = 0) have their Selected field 0x01.                                                 */
/*         The return value is FALSE if a read error occured.                                                                     */
/* Import: _RemapSpectrumString.                                                                                                  */
/**********************************************************************************************************************************/

{
  struct MouseShape         *OldShape;
  struct ComboEntry          NEntry1;
  struct ComboEntry          NEntry2;
  struct MicrodriveRecord_s *MRecord;
  struct ComboEntry          NewEntry;
  int                        Fh;
  dword                      FileLength;
  word                       FreeCount      = 0;
  bool                       FirstRecord    = TRUE;
  bool                       More;
  short            register  Cnt1;
  short            register  Cnt2;
  byte                       WriteProtected;
  char                       FType[247];
  short                      TokLen;

  NewEntry.Identifier = 0;
  FileNotFound = TRUE;
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return (TRUE);
  if ((MRecord = (struct MicrodriveRecord_s *)malloc (sizeof (struct MicrodriveRecord_s))) == NULL)
  { close (Fh); FatalError (1, _ErrOutOfMemory, sizeof (struct MicrodriveRecord_s)); }
  OldShape = CurrentMouseShape;
  SetMouseShape (DefaultWaitingMouseShape);
  FileNotFound = FALSE;
  FileLength = filelength (Fh);
  if (FileLength != (dword)137923)                                                                       /* Incorrect file size ? */
  {
    NewEntry.Selected = ENT_NOSELECT;
    strcpy (NewEntry.Text, "<Wrong file size>");
    if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
      FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
    free (MRecord);
    close (Fh);
    SetMouseShape (OldShape);
    return (TRUE);
  }
  if (lseek (Fh, (dword)137922, SEEK_SET) == -1)
  { free (MRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno));
    SetMouseShape (OldShape); return (FALSE); }
  if (read (Fh, &WriteProtected, 1) == -1)                                                             /* Read write-protect flag */
  { free (MRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
    SetMouseShape (OldShape); return (FALSE); }
  if (lseek (Fh, (dword)0, SEEK_SET) == -1)
  { free (MRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno));
    SetMouseShape (OldShape); return (FALSE); }
  for (Cnt1 = 0 ; Cnt1 < 254 ; Cnt1 ++)                                                                      /* Count 254 records */
  {
    NewEntry.Selected = ENT_NOSELECT;                                                                       /* Signal: not hidden */
    if (read (Fh, MRecord, sizeof (struct MicrodriveRecord_s)) == -1)                                         /* Read next record */
    { free (MRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
      SetMouseShape (OldShape); return (FALSE); }
    if (FirstRecord)                                                                                            /* First record ? */
    {
      FirstRecord = FALSE;
      Cnt2 = _RemapSpectrumString (MRecord->HdName, 10, NewEntry.Text, 256, FALSE, FALSE, &TokLen, TRUE, TRUE);
      NewEntry.Text[Cnt2] = '\0';
      strcat (NewEntry.Text, "{++{--}L");                                                /* Ensure there's an attribute endmarker */
      if (WriteProtected)
        strcat (NewEntry.Text, "   {+1(Write protected)}N");
      if (!InsertComboEntry (Window, &NewEntry, NULL, -1))                                    /* Insert cartridge name at the top */
        FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
      NewEntry.Text[0] = '\0';
      if (!InsertComboEntry (Window, &NewEntry, NULL, -1))                                           /* Followed by an empty line */
        FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
    }
    if (MRecord->RecLen != 0x0000)                                                                         /* Not empty nor bad ? */
    {
      if (MRecord->RecNum == 0x0000)                                                                                  /* Header ? */
      {
        if (MRecord->RecNam[0] == '\0')                                                                          /* Hidden file ? */
        {                                                                                                  /* Insert res filename */
          Cnt2 = _RemapSpectrumString (MRecord->RecNam + 1, 9, NewEntry.Text, 256, FALSE, FALSE, &TokLen, TRUE, TRUE);
          NewEntry.Text[Cnt2 ++] = ' ';                                                     /* Add a space (to get 10 characters) */
          NewEntry.Selected = 0x01;                                                                     /* Use SelectedAPen color */
        }
        else
          Cnt2 = _RemapSpectrumString (MRecord->RecNam, 10, NewEntry.Text, 256, FALSE, FALSE, &TokLen, TRUE, TRUE);
        NewEntry.Text[Cnt2] = '\0';
        if (TokLen > 10)                                                                                    /* Over 10 expanded ? */
        {
          while (NewEntry.Text[Cnt2 - 1] == ' ')                                             /* Try eliminating spaces at the end */
          {
            Cnt2 --;
            TokLen --;
          }
          NewEntry.Text[Cnt2] = '\0';
          strcat (NewEntry.Text, "{++{--}L");
        }
        else if (TokLen < 10)                                                     /* Unexpandable tokens were removed (AT, TAB) ? */
        {
          strcat (NewEntry.Text, "{++{--}L");                                            /* Ensure there's an attribute endmarker */
          strcat (NewEntry.Text, "          ");                                                     /* Pad the result with spaces */
          NewEntry.Text[Cnt2 + 18 - TokLen] = '\0';
          TokLen = 10;
        }
        else
          strcat (NewEntry.Text, "{++{--}L");
        if (TokLen > 10)
        {
          if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
            FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
          strcpy (NewEntry.Text, "          ");
          Cnt2 = 10;
        }
        switch (MRecord->DType)                                                                         /* Block type from header */
        {
          case 0 : if (MRecord->DAuto > 32767)
                     strcpy (FType, "  PROG  -----  ");
                   else
                     sprintf (FType, "  PROG  %5u  ", MRecord->DAuto);
                   sprintf (FType + strlen (FType), "%5u", MRecord->DLength);
                   NewEntry.Identifier = (dword)Cnt1 * sizeof (struct MicrodriveRecord_s);
                   NewEntry.Selected = BASICBLOCK;
                   break;
          case 1 : sprintf (FType, "  N-AR         %5u", MRecord->DLength);
                   break;
          case 2 : sprintf (FType, "  A-AR         %5u", MRecord->DLength);
                   break;
          case 3 : sprintf (FType, "  CODE  %5u  %5u", MRecord->DLoad, MRecord->DLength);
                   NewEntry.Identifier = (dword)Cnt1 * sizeof (struct MicrodriveRecord_s);
                   NewEntry.Selected = 0x00;
                   break;
          default: sprintf (FType, "  ????  %5u", MRecord->DType);                           /* Usually in M1 blocks; fake header */
                   NewEntry.Identifier = (dword)Cnt1 * sizeof (struct MicrodriveRecord_s);
                   NewEntry.Selected = 0x00;
        }
        strcat (NewEntry.Text, FType);
        if (!InsertComboEntry (Window, &NewEntry, NULL, -1))                                                   /* Insert filename */
          FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
      }
    }
    else if (!(MRecord->RecFlg & 0x02))                                                               /* Free block and not bad ? */
      FreeCount ++;                                                                                       /* Count all free space */
  }
  More = TRUE;
  for (Cnt1 = Window->NumEntries ; Cnt1 > 2 && More ; Cnt1 --)                                              /* Now sort the files */
  {
    More = FALSE;
    for (Cnt2 = 2 ; Cnt2 < Cnt1 ; Cnt2 ++)
    {
      GetComboEntry (Window, &NEntry1, Cnt2);
      GetComboEntry (Window, &NEntry2, Cnt2 + 1);
      if (strcmp (NEntry1.Text, NEntry2.Text) > 0)
      {
        PutComboEntry (Window, &NEntry1, Cnt2 + 1, FALSE);
        PutComboEntry (Window, &NEntry2, Cnt2, FALSE);
        More = TRUE;
      }
    }
  }
  NewEntry.Text[0] = '\0';
  if (!InsertComboEntry (Window, &NewEntry, NULL, -1))                                                    /* Insert an empty line */
    FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
  sprintf (NewEntry.Text, "%d", (int)(FreeCount / 2));                                     /* Insert free size in K, rounded down */
  if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
    FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
  free (MRecord);
  SetMouseShape (OldShape);
  return (TRUE);
}

bool InsertTZXIndex (struct ComboInfo *Window, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the combo window where the catalog of the .MDR file with full path `FullPath' should go.            */
/* Post  : A catalog of the file has been inserted in the entry table of the combo window. A line is built up as follows:         */
/*         1. (Expanded) Spectrum file name;                                                                                      */
/*         2. Type of file: PROG, CODE, A-AR, N-AR, HEAD, ????;                                                                   */
/*         3. Depending on the type:                                                                                              */
/*            PROG (Program)                 : Auto-start line or ----- if none;                                                  */
/*            CODE (Code)                    : Startaddress of the code;                                                          */
/*            A-AR (Alphanumerical Array)    : -                                                                                  */
/*            N-AR (Numerical Array)         : -                                                                                  */
/*            HEAD (Headerless block)        : The Flag byte;                                                                     */
/*            ???? (Headerless block)        : The Type byte;                                                                     */
/*            DATA (Following a header)      : -                                                                                  */
/*         4. Length of the block.                                                                                                */
/*         Headerless blocks with total length < 2 are reported as being noise (multiple blocks are reported once).               */
/*         PROG, CODE and headerless blocks are selectable, all others are not.                                                   */
/*         - Contrary to TAP/MDR listings, blocks are split into header/data and the data blocks can be viewed.                   */
/*         - Entries with an error (parity error) are printed in red.                                                             */
/*         - Entries with losing data (ID 0x10 with incomplete last byte) are printed in blue.                                    */
/* Import: _RemapSpectrumString.                                                                                                  */
/**********************************************************************************************************************************/

{
  struct MouseShape   *OldShape;
  struct TapeRecord_s *TRecord;
  struct ComboEntry    NewEntry;
  int                  Fh;
  dword                FileLength;
  dword                FileIndex;
  word                 NRead;
  word       register  M;
  char                 FType[49];
  short                TokLen;
  byte       register  LastFlag;
  byte                 BlockID;
  byte                 RByte;
  word                 RWord;
  short                RInt;
  dword                RLong;
  dword                Skip;
  long                 TotLen;
  byte                 LastBlockType = 0;

  NewEntry.Selected = 0;
  NewEntry.Identifier = 0;
  FileNotFound = TRUE;
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return (TRUE);
  if ((TRecord = (struct TapeRecord_s *)malloc (sizeof (struct TapeRecord_s))) == NULL)
  { close (Fh); FatalError (1, _ErrOutOfMemory, sizeof (struct TapeRecord_s)); }
  OldShape = CurrentMouseShape;
  SetMouseShape (DefaultWaitingMouseShape);
  FileNotFound = FALSE;
  FileLength = filelength (Fh);
  if (FileLength == 0)                                                                                       /* File of 0 bytes ? */
  {
    NewEntry.Selected = 0x01 | ENT_NOSELECT;
    strcpy (NewEntry.Text, "<Empty tape>");
    if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
      FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
    free (TRecord);
    close (Fh);
    SetMouseShape (OldShape);
    return (TRUE);
  }
  LastFlag = 0xFF;                                                                      /* Signal: last block was of unknown type */
  FileIndex = (dword)10;
  if (lseek (Fh, 10, SEEK_SET) == -1)                                                                   /* Go past the TZX header */
  { free (TRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
    SetMouseShape (OldShape); return (FALSE); }                                                               /* Return the error */
  if ((NRead = read (Fh, &BlockID, 1)) == -1)                                                              /* Read first block ID */
  { free (TRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
    SetMouseShape (OldShape); return (FALSE); }                                                               /* Return the error */
  while (NRead == 1)                                                                     /* Then there is at least one more block */
  {
    NewEntry.Selected = ENT_NOSELECT;
    if (BlockID == 0x10 || BlockID == 0x11)                                                             /* Identified data type ? */
    {
      switch (BlockID)
      {
        case 0x10: read (Fh, &RWord, 2); read (Fh, &RWord, 2); TotLen = RWord; break;                      /* (Normal Speed Data) */
        case 0x11: read (Fh, &RWord, 2); read (Fh, &RWord, 2); read (Fh, &RWord, 2);                       /* (Custom Speed Data) */
                   read (Fh, &RWord, 2); read (Fh, &RWord, 2); read (Fh, &RWord, 2);
                   read (Fh, &RByte, 1); read (Fh, &RWord, 2); read (Fh, &RLong, 3);
                   TotLen = (RLong & 0x00FFFFFF); break;
      }
      Skip = TotLen;
      if (TotLen >= HDRLEN)                                                                       /* Could it be a header block ? */
        RByte = HDRLEN - 1;
      else
        RByte = TotLen;
      Skip -= RByte;
      if (RByte == 0)
        TRecord->Flag = 0xFF;
      else if (RByte > 0)
        if ((NRead = read (Fh, TRecord, RByte)) == -1)                                    /* Read the block as a header in memory */
        { free (TRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
          SetMouseShape (OldShape); return (FALSE); }
      if (TRecord->Flag != 0x00 || (TRecord->Flag == 0x00 && TotLen != HDRLEN))                             /* It is a data block */
      {
        if (TotLen >= 2)                                                                            /* Includes flag and parity ? */
          TotLen -= 2;
        else
          TotLen = 0;
        if (LastFlag != 0x00)                                                            /* Last read block was also a data block */
        {
          sprintf (NewEntry.Text, "            HEAD  %5u  %5u", TRecord->Flag, TotLen);            /* So it is a headerless block */
          NewEntry.Identifier = FileIndex;                                                          /* Point to the current block */
        }
        else                                                                          /* It belongs to the previously read header */
        {
          sprintf (NewEntry.Text, "            DATA         %5u", TotLen);                         /* So it is a headerless block */
          NewEntry.Identifier = FileIndex;                                                          /* Point to the current block */
          LastFlag = (TRecord->Flag == 0x00 ? 0xFF : TRecord->Flag);
        }
        NewEntry.Selected = LastBlockType;
        LastBlockType = 0x00;
        if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
          FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
      }
      else                                                                                                /* It is a header block */
      {
        LastFlag = 0x00;                                                                           /* Insert filename from header */
        M = _RemapSpectrumString (TRecord->HName, 10, NewEntry.Text, 256, FALSE, FALSE, &TokLen, TRUE, TRUE);
        NewEntry.Text[M] = '\0';
        if (TokLen > 10)                                                                                    /* Over 10 expanded ? */
        {
          while (TokLen > 10 && NewEntry.Text[M - 1] == ' ')                                 /* Try eliminating spaces at the end */
          {
            M --;
            TokLen --;
          }
          NewEntry.Text[M] = '\0';
          strcat (NewEntry.Text, "{++{--}L");
        }
        else if (TokLen < 10)                                                     /* Unexpandable tokens were removed (AT, TAB) ? */
        {
          strcat (NewEntry.Text, "{++{--}L");                                            /* Ensure there's an attribute endmarker */
          strcat (NewEntry.Text, "          ");                                                     /* Pad the result with spaces */
          NewEntry.Text[M + 18 - TokLen] = '\0';
          TokLen = 10;
        }
        else
          strcat (NewEntry.Text, "{++{--}L");
        if (TokLen > 10)
        {
          if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
            FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
          strcpy (NewEntry.Text, "          ");
          M = 10;
        }
        switch (TRecord->HType)                                                                         /* Block type from header */
        {
          case 0 : if (TRecord->HStart > 32767)
                     strcpy (FType, "  PROG  -----  ");
                   else
                     sprintf (FType, "  PROG  %5u  ", TRecord->HStart);
                   sprintf (FType + strlen (FType), "%5u", TRecord->HLength);
                   LastBlockType = BASICBLOCK;                                                        /* Selection prints listing */
                   break;
          case 1 : sprintf (FType, "  N-AR         %5u", TRecord->HLength);
                   LastBlockType = 0x00;
                   break;
          case 2 : sprintf (FType, "  A-AR         %5u", TRecord->HLength);
                   LastBlockType = 0x00;
                   break;
          case 3 : sprintf (FType, "  CODE  %5u  %5u", TRecord->HStart, TRecord->HLength);
                   LastBlockType = 0x00;
                   break;
          default: sprintf (FType, "  ????  %5u  %5u", TRecord->HType, TotLen);            /* Usually in M1 blocks; just 17 bytes */
                   LastBlockType = 0x00;
                   LastFlag = 0xFF;                                                   /* Signal: also print info on the datablock */
        }
        strcat (NewEntry.Text, FType);
        if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
          FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
      }
    }
    else                                                                                         /* (Not an identified data type) */
    {
      LastFlag = 0xFF;                                                                  /* Signal: last block was of unknown type */
      LastBlockType = 0x00;
      memset (NewEntry.Text, ' ', 40);
      switch (BlockID)
      {
        case 0x12 : strcpy (NewEntry.Text, "{+1(Pure Tone)"); Skip = 4; TotLen = -1; break;                        /* (Pure Tune) */
        case 0x13 : strcpy (NewEntry.Text, "{+1(Row of Pulses)");                                              /* (Row Of Pulses) */
                    read (Fh, &RByte, 1); Skip = RByte * 2; TotLen = Skip; break;
        case 0x14 : read (Fh, &RWord, 2); read (Fh, &RWord, 2); read (Fh, &RByte, 1);                              /* (Pure Data) */
                    read (Fh, &RWord, 2); read (Fh, &RLong, 3); Skip = (RLong & 0x00FFFFFF);
                    sprintf (NewEntry.Text, "            PURE DATA    %5u", Skip);
                    NewEntry.Identifier = FileIndex;
                    NewEntry.Selected = 0x00;
                    TotLen = -1; break;
        case 0x15 : strcpy (NewEntry.Text, "{+1(Direct Recording)");                                        /* (Direct Recording) */
                    read (Fh, &RWord, 2); read (Fh, &RWord, 2); read (Fh, &RByte, 1);
                    read (Fh, &RLong, 3); Skip = (RLong & 0x00FFFFFF); TotLen = Skip + 8; break;
        case 0x20 : read (Fh, &RWord, 2);                                                                        /* (Pause Block) */
                    if (RWord == 0) strcpy (NewEntry.Text, "{+1(Pause: wait key)");
                    else            sprintf (NewEntry.Text, "{+1(Pause: %u ms.)", RWord);
                    Skip = 0; TotLen = -1; break;
        case 0x21 : read (Fh, &RByte, 1); read (Fh, _BufferDecrunched, RByte);                                   /* (Group Start) */
                    *(_BufferDecrunched + RByte) = '\0';
                    sprintf (NewEntry.Text, "{+1(GROUP \"%s\")", _BufferDecrunched); Skip = 0; TotLen = -1; break;
        case 0x22 : strcpy (NewEntry.Text, "{+1(GROUP END)"); Skip = 0; TotLen = -1; break;                        /* (Group End) */
        case 0x23 : read (Fh, &RInt, 2);                                                                       /* (Jump To Block) */
                    if (RInt >= 0)  sprintf (NewEntry.Text, "{+1(Jump %d blocks forward)", RInt);
                    else            sprintf (NewEntry.Text, "{+1(Jump %d blocks back)", -RInt);
                    Skip = 0; TotLen = -1; break;
        case 0x24 : read (Fh, &RInt, 2);                                                                          /* (Loop Start) */
                    sprintf (NewEntry.Text, "{+1(LOOP %d times)", RInt); Skip = 0; TotLen = -1; break;
        case 0x25 : strcpy (NewEntry.Text, "{+1(LOOP END)"); Skip = 0; TotLen = -1; break;                          /* (Loop End) */
        case 0x26 : read (Fh, &RInt, 2);                                                                       /* (Call Sequence) */
                    Skip = RInt * 2; TotLen = Skip;
                    sprintf (NewEntry.Text, "{+1(Call sequence)"); break;
        case 0x27 : strcpy (NewEntry.Text, "{+1(Sequence END)"); Skip = 0; TotLen = -1; break;          /* (Return From Sequence) */
        case 0x28 : read (Fh, &RInt, 2); read (Fh, &RByte, 1); Skip = RInt - 1; TotLen = RByte;                 /* (Select Block) */
                    strcpy (NewEntry.Text, "{+1(Select Block)"); break;
        case 0x2A : strcpy (NewEntry.Text, "{+1(Stop Tape if 48K)");                                /* (Stop Tape if in 48K Mode) */
                    read (Fh, &RLong, 4); Skip = RLong; TotLen = RLong + 4; break;
        case 0x30 : read (Fh, &RByte, 1); read (Fh, _BufferDecrunched, RByte);                              /* (Text Description) */
                    *(_BufferDecrunched + RByte) = '\0';
                    sprintf (NewEntry.Text, "{+1(Text \"%s\")", _BufferDecrunched); Skip = 0; TotLen = -1; break;
        case 0x31 : strcpy (NewEntry.Text, "{+1(Message Block)");                                              /* (Message Block) */
                    read (Fh, &RByte, 1); read (Fh, &RByte, 1); Skip = RByte; TotLen = Skip + 2; break;
        case 0x32 : strcpy (NewEntry.Text, "{+1(Archive Info)");                                                /* (Archive Info) */
                    read (Fh, &RWord, 2); Skip = RWord; TotLen = Skip + 2;
                    NewEntry.Identifier = FileIndex;
                    NewEntry.Selected = TZXAIBLOCK;
                    break;
        case 0x33 : strcpy (NewEntry.Text, "{+1(Hardware Type)");                                              /* (Hardware Type) */
                    read (Fh, &RByte, 1); Skip = RByte * 3; TotLen = Skip + 1;
                    NewEntry.Identifier = FileIndex;
                    NewEntry.Selected = TZXHWTBLOCK;
                    break;
        case 0x34 : strcpy (NewEntry.Text, "{+1(Emulation Info)"); Skip = 8; TotLen = -1; break;              /* (Emulation Info) */
        case 0x35 : read (Fh, _BufferDecrunched, 16); *(_BufferDecrunched + 16) = '\0';                    /* (Custom Info Block) */
                    read (Fh, &RLong, 4); Skip = RLong; TotLen = Skip + 20;
                    sprintf (NewEntry.Text, "{+1(CustIn-%s)", _BufferDecrunched);
                    if (!strnicmp (_BufferDecrunched, "Instructions    ", 16))                     /* Determine displayable types */
                    {
                      NewEntry.Identifier = FileIndex;
                      NewEntry.Selected = TZXCUSTTXT;
                    }
                    else if (!strnicmp (_BufferDecrunched, "Spectrum Screen ", 16))
                    {
                      NewEntry.Identifier = FileIndex;
                      NewEntry.Selected = TZXCUSTSCR;
                    }
                    else if (!strnicmp (_BufferDecrunched, "Picture         ", 16))
                    {
                      NewEntry.Identifier = FileIndex;
                      NewEntry.Selected = TZXCUSTPIC;
                    }
                    else if (!strnicmp (_BufferDecrunched, "POKEs           ", 16))
                    {
                      NewEntry.Identifier = FileIndex;
                      NewEntry.Selected = TZXCUSTPOK;
                    }
                    break;
        case 0x40 : read (Fh, &RByte, 1); read (Fh, &RLong, 3);                                               /* (Snapshot Block) */
                    sprintf (NewEntry.Text, "{+1(Snapshot Block [.%s])", RByte == 0 ? "Z80" : "SNA");
                    Skip = (RLong & 0x00FFFFFF); TotLen = Skip + 4; break;
        case 0x5A : strcpy (NewEntry.Text, "{+1(Joined TZX)"); Skip = 9; TotLen = -1; break;                      /* (Joined TZX) */
        default   : strcpy (NewEntry.Text, "{+1(UNKNOWN BLOCK)");
                    read (Fh, &RLong, 4); Skip = RLong; TotLen = RLong + 4; break;
      }
      if (TotLen >= 0)
      {
        NewEntry.Text[strlen (NewEntry.Text)] = ' ';
        sprintf (NewEntry.Text + 26, "%7ld", TotLen);
      }
      if (!InsertComboEntry (Window, &NewEntry, NULL, -1))
        FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
    }
    FileIndex = tell (Fh);
    if (Skip > 0)
    {
      FileIndex += Skip;
      if (lseek (Fh, FileIndex, SEEK_SET) == -1)                                                          /* Go to the next block */
      { free (TRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno));
        SetMouseShape (OldShape); return (FALSE); }
    }
    if ((NRead = read (Fh, &BlockID, 1)) == -1)                                                 /* Can I read the next Block ID ? */
    { free (TRecord); close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
      SetMouseShape (OldShape); return (FALSE); }
  }
  free (TRecord);
  close (Fh);
  SetMouseShape (OldShape);
  return (TRUE);
}

void ViewInfoFromSLT (short StartX, short StartY, short MaxEntrySize, short MaxEntryView, short TextWidth,
                      short TextHeight, char *FullPath, char *Header)

/**********************************************************************************************************************************/
/* Pre   : `StartX', `StartY' holds the top-left edge to put the filewindow, `TextWidth' holds the number of rows, `TextHeight'   */
/*         holds the number of columns, `FileName' holds the name of the file to be displayed, `MaxEntrySize' holds the number of */
/*         bytes to be reserved for a text field per combo entry `MaxEntryView' holds the max width to store per entry; this      */
/*         indicates at which point text lines are wrapped to the next entry.                                                     */
/* Post  : The info extension has been displayed in a view requester.                                                             */
/* Import: _ConvertAttributes.                                                                                                    */
/**********************************************************************************************************************************/

{
  struct ButtonInfo *ButtonP;
  struct WindowInfo  ViewWindow;
  struct ComboInfo   TextWindow;
  struct ComboEntry  NEntry;
  comboent           Dummy;
  short    register  Index        = 0;
  dword              Length       = 0;
  char               Bt;
  int                Fh;
  bool               More         = TRUE;
  bool               HSlider      = FALSE;
  char               Line[1024];
  byte               CFlags = COMF_NOBUFFER | COMF_VSLIDER | COMF_SYNCSLIDER | COMF_KEYSELECT;

  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
  { ErrorRequester (-1, -1, NULL, NULL, "Cannot open file %s", FullPath); return; }
  if (lseek (Fh, SLTMap.TableEnd + SLTMap.OffsetInfo, SEEK_SET) == -1)
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath); return; }
  SelectFont (NULL, FONT_NORMAL);
  memset (&ViewWindow, 0, sizeof (struct WindowInfo));                                           /* Initialize the interface part */
  ViewWindow.StartX      = StartX;
  ViewWindow.StartY      = StartY;
  ViewWindow.Width       = ViewWindow.MinWidth = ViewWindow.MaxWidth = CurrentFont->Width * TextWidth + 7;
  ViewWindow.Height    = ViewWindow.MinHeight = ViewWindow.MaxHeight = (CurrentFont->Height + 2) * TextHeight + (HSlider ? 10 : -2);
  ViewWindow.BorderFlags = WINB_OUT | WINB_SURROUND;
  ViewWindow.BorderSize  = 1;
  ViewWindow.APen        = SystemColors.ReqAPen;
  ViewWindow.BPen        = SystemColors.ReqOPen;
  ViewWindow.Header      = Header;
  ViewWindow.Flags       = WINF_ACTIVATE | WINF_HUSE | WINF_HCENTER | WINF_XMSBUFFER;
  DrawWindow (&ViewWindow, NULL, WING_CLOSE, TRUE);
  MakeComboWindow (&TextWindow, &ViewWindow, COMT_VIEW, CFlags, ViewWindow.A_StartX + 2, ViewWindow.StartY - 2, 0, 1024,
                   MaxEntrySize, TextWidth, TextHeight, CurrentFont, FALSE, 0, NULL, SystemColors.ReqAPen, SystemColors.ReqBPen,
                   0, 0);
  while (Length < SLTMap.HeaderInfo.Length && More)                                   /* Read the entire file into the entrytable */
  {
    if (read (Fh, &Bt, 1) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath); return; }
    if (Length < SLTMap.HeaderInfo.Length)
    {
      if (Bt != 0x0A && Bt != 0x0D && Index < 1023)
      {
        if (Bt == '{' || Bt == '}')
          Line[Index ++] = Bt;
        Line[Index ++] = Bt;
      }
      else if (Bt != 0x0A)                                                                             /* Eliminate LF characters */
      {
        Line[Index] = '\0';
        _ConvertAttributes (Line);
        More = FitComboString (&TextWindow, NULL, Line, MaxEntryView, 0, 0, FALSE);
        Index = 0;
      }
    }
  }
  if (More && Index != 0)                                                                         /* Resulting line without eol ? */
  {
    Line[Index] = '\0';
    _ConvertAttributes (Line);
    More = FitComboString (&TextWindow, NULL, Line, MaxEntryView, 0, 0, FALSE);
  }
  close (Fh);                                                                                                  /* Close the file */
  if (!More)                                                                                       /* File truncated prematurally */
  {
    if (TextWindow.NumEntries == -1)                                                                         /* No entry at all ? */
      FatalError (1, _ErrOutOfMemory, 0);                                                                            /* Then quit */
    GetComboEntry (&TextWindow, &NEntry, TextWindow.NumEntries);
    strcpy (NEntry.Text, "{+1>>> TRUNCATED <<<}N");
    PutComboEntry (&TextWindow, &NEntry, TextWindow.NumEntries, FALSE);
    More = TRUE;
  }
  PrintComboEntries (&TextWindow);                                                                          /* Print the contents */
  while (More)                                                                                    /* Handle until close requested */
  {
    HandleComboWindow (&TextWindow, &Dummy);
    if (GlobalKey == K_ESC)
      More = FALSE;
    else
    {
      More = (HandleWindow (&ViewWindow, &ButtonP) != WINH_CLOSE);
      if (More && GlobalKey == K_ESC)
        More = FALSE;
    }
    MouseStatus ();
  }
  DestroyComboWindow (&TextWindow, 0);
  DestroyWindow (&ViewWindow);
}

bool FixTape (void)

/**********************************************************************************************************************************/
/* Pre   : The currently selected game MUST be in a `.TAP' file.                                                                  */
/* Post  : Any noise blocks have been removed from the `.TAP' file.                                                               */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct WindowInfo    MessageWindow = {220, 200, 200, 50, 200, 50, 200, 50, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE,
                                        NULL, WINF_EXCLUSIVE | WINF_WADJUST | WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE |
                                        WINF_XMSBUFFER, NULL, "Removing Noise"};
  struct MouseShape   *OldShape;
  char                 TmpGameName[MAXDIR];
  int                  FhIn;
  int                  FhOut;
  dword                FileLength;
  word                 NRead;
  word                 BlLength;                                                                  /* Length of one TAP file block */
  bool                 RemovedNoise = FALSE;                                                          /* Assume there is no noise */
  bool                 Multiple = FALSE;
  bool                 OverwriteAll = TRUE;

  MakeFullPath (TmpGameName, TmpDirectory, mktemp (NULL));
  if ((FhIn = open (FullPath, O_BINARY | O_RDONLY)) == -1)
  { ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Cannot open file %s", FullPath); return (FALSE); }
  if ((FhOut = open (TmpGameName, O_BINARY | O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1)
  { ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Cannot create new file:\n %s", strerror (errno)); close (FhIn); return (FALSE); }
  OldShape = CurrentMouseShape;
  SetMouseShape (DefaultWaitingMouseShape);
  FileLength = filelength (FhIn);
  if (FileLength == 0)                                                                                       /* File of 0 bytes ? */
  {
    close (FhIn);
    close (FhOut);
    unlink (TmpGameName);
    SetMouseShape (OldShape);
    return (FALSE);
  }
  DrawWindow (&MessageWindow, NULL, 0, FALSE);
  Wprintf (&MessageWindow, 1, TEXT_CENTER, "Removing noise from .TAP file");
  Wprintf (&MessageWindow, 3, TEXT_CENTER, CurrentEntry.PCName);
  if ((NRead = read (FhIn, &BlLength, 2)) == -1)                                                        /* Can I read two bytes ? */
  { close (FhIn); close (FhOut); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
    unlink (TmpGameName); SetMouseShape (OldShape); DestroyWindow (&MessageWindow); return (FALSE); }
  while (NRead == 2)                                                                     /* Then there is at least one more block */
  {
    if (BlLength >= 2)                                                                      /* Flag + block + parity >= 2 bytes ? */
    {                                                                                                 /* (i.e. skip noise blocks) */
      if (write (FhOut, &BlLength, 2) == -1)                                                        /* Copy length to output file */
      { close (FhIn); close (FhOut); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileWriteError, FullPath, strerror (errno));
        unlink (TmpGameName); SetMouseShape (OldShape); DestroyWindow (&MessageWindow); return (FALSE); }
      switch (CopyFilePart (FhIn, FhOut, BlLength))
      {
        case -1 : ErrorRequester (-1, -1, "@Close", NULL, NULL, _ErrOutOfMemory, 2048);
                  close (FhIn);
                  close (FhOut);
                  unlink (TmpGameName);
                  SetMouseShape (OldShape);
                  DestroyWindow (&MessageWindow);
                  return (FALSE);
        case  1 : ErrorRequester (-1, -1, "@Close", NULL, NULL, "ERROR - File I/O error: %s", strerror (errno));
                  close (FhIn);
                  close (FhOut);
                  unlink (TmpGameName);
                  SetMouseShape (OldShape);
                  DestroyWindow (&MessageWindow);
                  return (FALSE);
        case  0 : break;
      }
    }
    else
      RemovedNoise = TRUE;
    if ((NRead = read (FhIn, &BlLength, 2)) == -1)                                                      /* Can I read two bytes ? */
    { close (FhIn); close (FhOut); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
      unlink (TmpGameName); SetMouseShape (OldShape); DestroyWindow (&MessageWindow); return (FALSE); }
  }
  close (FhIn);
  close (FhOut);
  SetMouseShape (OldShape);
  DestroyWindow (&MessageWindow);
  if (!RemovedNoise)                                                                                    /* Did we make a change ? */
  { unlink (TmpGameName); return (FALSE); }
  if (!MoveFile (TmpGameName, FullPath, &Multiple, &OverwriteAll))
  { unlink (TmpGameName); return (FALSE); }
  return (TRUE);
}

void ViewBASICFromSnap (bool ViewBASIC, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : The currently selected game MUST be in a snapshot file.                                                                */
/*         `ViewBASIC' is TRUE for a BASIC listing, FALSE for a memory image dump.                                                */
/* Post  : The BASIC part has been shown in a view requester.                                                                     */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  byte *ImageBuffer;
  word  ImageSize;
  word  BASICStart;
  word  VariablesStart;
  word  UDGArea;

  if (!GetMemoryImage (FullPath, AutoBank ((word)16384, ""), &ImageBuffer, &ImageSize))
    return;
  BASICStart = *(ImageBuffer + 23635 - 16384) + 256 * *(ImageBuffer + 23636 - 16384);                                /* (23635/6) */
  VariablesStart = *(ImageBuffer + 23627 - 16384) + 256 * *(ImageBuffer + 23628 - 16384);                            /* (23627/8) */
  UDGArea = *(ImageBuffer + 23675 - 16384) + 256 * *(ImageBuffer + 23676 - 16384);                                   /* (23675/6) */
  if ((FileType == FT_Z80 || FileType == FT_SLT) && Z80Version > 145)
  {
    if (!GetMemoryImage (FullPath, AutoBank ((word)32768, ""), &ImageBuffer, &ImageSize))              /* We want all banks there */
      return;
    if (!GetMemoryImage (FullPath, AutoBank ((word)49152, ""), &ImageBuffer, &ImageSize))
      return;
  }
  if (ViewBASIC)
    _ViewBASICListing (150, 100, 32, 24, NULL, ImageBuffer + BASICStart - 16384, ImageBuffer + UDGArea - 16384,
                       VariablesStart - BASICStart);
  else
    _ViewMemoryDump (ImageBuffer, (word)49152, 16384);
}

void ViewBASICFromTAPBlock (bool ViewBASIC, struct ComboInfo *Window, comboent EntryNumber, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the .TAP index window, `EntryNumber' holds the selected entry number, `FullPath' holds the full     */
/*         path to the .TAP file.                                                                                                 */
/*         `ViewBASIC' is TRUE for a BASIC listing, FALSE for a memory image dump.                                                */
/* Post  : The selected BASIC entry has been displayed in a BASIC Listing ViewWindow.                                             */
/* Import: PutSpectrumScreen, SpectrumScreenToGIF.                                                                                */
/**********************************************************************************************************************************/

{
  struct ComboEntry NEntry;
  int               Fh;
  word              BlockLength;
  dword             ImageStart;

  GetComboEntry (Window, &NEntry, EntryNumber);
  ImageStart = NEntry.Identifier + 3;
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
  { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); return; }
  if (lseek (Fh, NEntry.Identifier, SEEK_SET) == -1)                                             /* Wind tape to correct position */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
  if (read (Fh, &BlockLength, 2) == -1)                                                         /* Read length of following block */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  if (FileType == FT_TAP)                                                            /* (.LTP files have the correct size fields) */
    BlockLength -= 2;                                                                       /* Exclude flag and block parity byte */
  if (lseek (Fh, ImageStart, SEEK_SET) == -1)                                                    /* Wind tape to correct position */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
  if (read (Fh, _BufferDecrunched, BlockLength) == -1)                                       /* Read the entire block into memory */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  close (Fh);
  if (BlockLength < (word)49152)
    *(_BufferDecrunched + BlockLength) = 64;                                                       /* Ensure there's an endmarker */
  if (ViewBASIC)
    _ViewBASICListing (150, 100, 32, 24, NULL, _BufferDecrunched, NULL, BlockLength);
  else
    _ViewMemoryDump (_BufferDecrunched, BlockLength, 0);
}

void ViewBASICFromMDRBlock (bool ViewBASIC, struct ComboInfo *Window, comboent EntryNumber, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the .TAP index window, `EntryNumber' holds the selected entry number, `FullPath' holds the full     */
/*         path to the .TAP file.                                                                                                 */
/*         `ViewBASIC' is TRUE for a BASIC listing, FALSE for a memory image dump.                                                */
/* Post  : The selected BASIC entry has been displayed in a BASIC Listing ViewWindow.                                             */
/* Import: PutSpectrumScreen, SpectrumScreenToGIF.                                                                                */
/**********************************************************************************************************************************/

{
  struct MicrodriveRecord_s *MDriveRecord;
  struct ComboEntry          NEntry;
  int                        Fh;
  word                       BlockLength;
  char                       BlockName[11];
  int                        NumBlocks;
  bool                       MDRBlocks[128];
  int                        Cnt;

  GetComboEntry (Window, &NEntry, EntryNumber);
  if ((MDriveRecord = malloc (sizeof (struct MicrodriveRecord_s))) == NULL)
  { ErrorRequester (-1, -1, "@Stop", NULL, NULL, _ErrOutOfMemory, sizeof (struct MicrodriveRecord_s)); return; }
  for (Cnt = 0 ; Cnt < 128 ; Cnt ++)                                                               /* Clear the done-blocks table */
    MDRBlocks[Cnt] = FALSE;
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
  { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); return; }
  if (lseek (Fh, NEntry.Identifier, SEEK_SET) == -1)                                             /* Read the header (first) block */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
  if (read (Fh, MDriveRecord, sizeof (struct MicrodriveRecord_s)) == -1)                                        /* Read the block */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  memcpy (BlockName, MDriveRecord->RecNam, 10);                                                               /* Store block name */
  BlockName[10] = '\0';
  BlockLength = MDriveRecord->DLength;
  NumBlocks = (BlockLength / 512) + (BlockLength % 512 ? 1 : 0);                                              /* Number of blocks */
  if (lseek (Fh, (dword)0, SEEK_SET) == -1)                                                               /* Go back to the start */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
  for (Cnt = 0 ; Cnt < 254 ; Cnt ++)
  {
    if (read (Fh, MDriveRecord, sizeof (struct MicrodriveRecord_s)) == -1)                                     /* Read each block */
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
    if (MDriveRecord->RecNum < 128 && MDriveRecord->RecLen <= 512)                                         /* Acceptable record ? */
      if (!memcmp (MDriveRecord->RecNam, BlockName, 10))                                              /* A record of this block ? */
      {                                                                                  /* Copy datapart in the correct location */
        if (MDriveRecord->RecNum == 0)                                                                           /* First block ? */
          memcpy (_BufferDecrunched, (byte *)&MDriveRecord->DBlock, MDriveRecord->RecLen);
        else
          memcpy (_BufferDecrunched + (dword)503 + (dword)512 * (MDriveRecord->RecNum - 1), (byte *)&MDriveRecord->DType,
                    MDriveRecord->RecLen);
        MDRBlocks[MDriveRecord->RecNum] = TRUE;
      }
  }
  close (Fh);
  for (Cnt = 0 ; Cnt < NumBlocks && MDRBlocks[Cnt] ; Cnt ++)
    ;
  if (Cnt != NumBlocks)                                                                                  /* Did we miss a block ? */
  { ErrorRequester (-1, -1, "@Close", NULL, NULL, "Missing record number %d in file\n  %s", Cnt, FullPath); return; }
  if (BlockLength < (word)49152)
    *(_BufferDecrunched + BlockLength) = 64;                                                       /* Ensure there's an endmarker */
  if (ViewBASIC)
    _ViewBASICListing (150, 100, 32, 24, NULL, _BufferDecrunched, NULL, BlockLength);
  else
    _ViewMemoryDump (_BufferDecrunched, BlockLength, 0);
}

void ViewBASICFromTZXBlock (bool ViewBASIC, struct ComboInfo *Window, comboent EntryNumber, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the .TAP index window, `EntryNumber' holds the selected entry number, `FullPath' holds the full     */
/*         path to the .TZX file.                                                                                                 */
/*         `ViewBASIC' is TRUE for a BASIC listing, FALSE for a memory image dump.                                                */
/* Post  : The selected BASIC entry has been displayed in a BASIC Listing ViewWindow.                                             */
/* Import: PutSpectrumScreen, SpectrumScreenToGIF.                                                                                */
/**********************************************************************************************************************************/

{
  struct ComboEntry NEntry;
  int               Fh;
  byte              BlockID;
  byte              RByte;
  word              BlockLength;
  dword             FilePos;
  byte              TZXBuffer[18];

  GetComboEntry (Window, &NEntry, EntryNumber);
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
  { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); return; }
  FilePos = NEntry.Identifier;
  if (lseek (Fh, FilePos, SEEK_SET) == -1)                                                       /* Wind tape to correct position */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
  if (read (Fh, &BlockID, 1) == -1)                                                            /* Read Type ID of following block */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  switch (BlockID)
  {
    case 0x10 : if (read (Fh, TZXBuffer, 4) == -1)
                { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
                BlockLength = TZXBuffer[2] + 256 * TZXBuffer[3];
                if (BlockLength > 0)                                                                     /* Go past the flag byte */
                {
                  read (Fh, &RByte, 1);
                  BlockLength --;
                }
                break;
    case 0x11 : if (read (Fh, TZXBuffer, 18) == -1)
                { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
                BlockLength = TZXBuffer[15] + 256 * TZXBuffer[16];                                                  /* (Max 64K!) */
                if (BlockLength > 0)
                {
                  read (Fh, &RByte, 1);
                  BlockLength --;
                }
                break;
    case 0x14 : if (read (Fh, TZXBuffer, 10) == -1)
                { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
                BlockLength = TZXBuffer[7] + 256 * TZXBuffer[8];                                                   /* (Max 64K!) */
                break;
  }
  if (BlockLength > (word)49152)
    BlockLength = (word)49152;
  if (read (Fh, _BufferDecrunched, BlockLength) == -1)                                       /* Read the entire block into memory */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  close (Fh);
  if (BlockLength < (word)49152)
    *(_BufferDecrunched + BlockLength) = 64;                                                       /* Ensure there's an endmarker */
  if (ViewBASIC)
    _ViewBASICListing (150, 100, 32, 24, NULL, _BufferDecrunched, NULL, BlockLength);
  else
    _ViewMemoryDump (_BufferDecrunched, BlockLength, 0);
}

void ViewTZXCIInstructions (struct ComboInfo *Window, comboent EntryNumber, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the .TAP index window, `EntryNumber' holds the selected entry number, `FullPath' holds the full     */
/*         path to the .TZX file.                                                                                                 */
/* Post  : The selected Custom Info - Instructions block has been displayed in a FileViewWindow-style window.                     */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
#define MAXCILINES   45
  struct ButtonInfo  *ButtonP;
  struct WindowInfo   ViewWindow;
  struct ComboInfo    TextWindow;
  struct ComboEntry   NEntry;
  comboent            LineCnt;
  short     register  Index          = 0;
  char                Bt;
  char                PrevBt         = 0;
  bool                More           = TRUE;
  bool                Resizing       = FALSE;
  char                Line[1024];
  char                WindowHeader[80];
  dword               LineNumber     = 0;
  byte                CFlags         = COMF_NOBUFFER | COMF_VSLIDER | COMF_SYNCSLIDER | COMF_KEYSELECT;
  short               CurrentHeight;
  FILE               *Fp;
  dword               BlockLength;

  GetComboEntry (Window, &NEntry, EntryNumber);
  if ((Fp = fopen (FullPath, "rb")) == NULL)
  { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); return; }
  BlockLength = NEntry.Identifier;
  if (fseek (Fp, BlockLength + 17, SEEK_SET) == -1)                                              /* Wind tape to correct position */
  { fclose (Fp); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
  if (fread (&BlockLength, 1, 4, Fp) == -1)                                                     /* Read length of following block */
  { fclose (Fp); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  SelectFont (NULL, FONT_NORMAL);
  memset (&ViewWindow, 0, sizeof (struct WindowInfo));                                           /* Initialize the interface part */
  CurrentHeight          = MAXCILINES / 2;
  ViewWindow.Width       = ViewWindow.MinWidth = ViewWindow.MaxWidth = CurrentFont->Width * 80 + 7;
  ViewWindow.Height      = (CurrentFont->Height + 2) * CurrentHeight - 2;
  ViewWindow.MinHeight   = (CurrentFont->Height + 2) * 10 - 2;
  ViewWindow.MaxHeight   = (CurrentFont->Height + 2) * MAXCILINES - 2;
  ViewWindow.StartX      = (Screen.Width - ViewWindow.Width) / 2;
  ViewWindow.StartY      = (Screen.Height - ViewWindow.Height) / 2;
  ViewWindow.BorderFlags = WINB_OUT | WINB_SURROUND;
  ViewWindow.BorderSize  = 1;
  ViewWindow.APen        = SystemColors.ReqAPen;
  ViewWindow.BPen        = SystemColors.ReqOPen;
  sprintf (WindowHeader, "TZX Instructions Block [%s]", CurrentEntry.Name);
  ViewWindow.Header      = WindowHeader;
  ViewWindow.Flags       = WINF_ACTIVATE | WINF_HUSE | WINF_HCENTER;

  DrawWindow (&ViewWindow, NULL, WING_CLOSE | WING_RESIZEMAX, TRUE);
  MakeComboWindow (&TextWindow, &ViewWindow, COMT_VIEW, CFlags, ViewWindow.A_StartX + 2, ViewWindow.StartY - 2, 0, 1024,
                   100, 80, CurrentHeight, CurrentFont, FALSE, 0, NULL, SystemColors.ReqAPen, SystemColors.ReqBPen,
                   0, 0);
  while (BlockLength > 0 && !feof (Fp) && More)                                       /* Read the entire file into the entrytable */
  {
    Bt = fgetc (Fp);
    if (BlockLength > 0 && !feof (Fp) && More)
    {
      if (Bt != 0x0A && Bt != 0x0D && Index < 1023)
      {
        if (Bt == '{' || Bt == '}')
          Line[Index ++] = Bt;
        Line[Index ++] = Bt;
      }
      else if ((Bt == 0x0A && PrevBt != 0x0D) ||                                                                    /* CR or LF ? */
               (Bt == 0x0D && PrevBt != 0x0A) ||                                        /* Eliminate the combination of CR and LF */
               (PrevBt != 0x0A && PrevBt != 0x0D))
      {
        Line[Index] = '\0';
        More = FitComboString (&TextWindow, NULL, Line, 80, 0, LineNumber, FALSE);
        Index = 0;
        LineNumber ++;
      }
      if (Bt == 0x0A && PrevBt == 0x0D)                                                             /* Cleanup for PC empty lines */
        PrevBt = 0;
      else
        PrevBt = Bt;
    }
    BlockLength --;
  }
  if (More && Index != 0)                                                                         /* Resulting line without eol ? */
  {
    Line[Index] = '\0';
    More = FitComboString (&TextWindow, NULL, Line, 80, 0, LineNumber, FALSE);
  }
  fclose (Fp);                                                                                                  /* Close the file */
  if (!More)                                                                                       /* File truncated prematurally */
  {
    if (TextWindow.NumEntries == -1)                                                                         /* No entry at all ? */
      FatalError (1, _ErrOutOfMemory, 0);                                                                            /* Then quit */
    GetComboEntry (&TextWindow, &NEntry, TextWindow.NumEntries);
    strcpy (NEntry.Text, "{+1>>> TRUNCATED <<<}N");
    PutComboEntry (&TextWindow, &NEntry, TextWindow.NumEntries, FALSE);
    More = TRUE;
  }
  if (TextWindow.NumEntries + 1 < MAXCILINES)                                             /* Less lines than set maximum resize ? */
    if (TextWindow.NumEntries + 1 < CurrentHeight)                                             /* Even less than 1 page already ? */
      ViewWindow.MaxHeight = (CurrentFont->Height + 2) * CurrentHeight - 2;
    else                                                                                                     /* Adjust to maximum */
      ViewWindow.MaxHeight = (CurrentFont->Height + 2) * (TextWindow.NumEntries + 1) - 2;
  PrintComboEntries (&TextWindow);                                                                          /* Print the contents */
  while (More)                                                                                    /* Handle until close requested */
  {
    if (Resizing)
    {
      CurrentHeight = (short)((ViewWindow.Height + 2) / (CurrentFont->Height + 2));
      ViewWindow.Width  = CurrentFont->Width * 80 + 7;
      ViewWindow.Height = (CurrentFont->Height + 2) * CurrentHeight - 2;
      DrawWindow (&ViewWindow, NULL, WING_CLOSE | WING_RESIZEMAX, TRUE);
      MakeComboWindow (&TextWindow, &ViewWindow, COMT_VIEW, CFlags, ViewWindow.A_StartX + 2, ViewWindow.StartY - 2, 0, 1024,
                       100, 80, CurrentHeight, CurrentFont, FALSE, 0, NULL, SystemColors.ReqAPen,
                       SystemColors.ReqBPen, 0, 0);
      for (LineCnt = 0 ; Resizing && LineCnt <= TextWindow.NumEntries ; LineCnt ++)
      {
        GetComboEntry (&TextWindow, &NEntry, LineCnt);
        if (NEntry.Identifier == LineNumber)
        {
          TextWindow.DFirstLine = LineCnt;
          TextWindow.VSlider->SliderStart = TextWindow.VSlider->StepRate * LineCnt;
          Resizing = FALSE;
        }
      }
      PrintComboEntries (&TextWindow);
      Resizing = FALSE;
    }
    HandleComboWindow (&TextWindow, &LineCnt);
    if (GlobalKey == K_ESC)
      More = FALSE;
    else
    {
      switch (HandleWindow (&ViewWindow, &ButtonP))
      {
        case WINH_CLOSE     : More = FALSE; break;
        case WINH_WINRESIZE : Resizing = TRUE;
                              GetComboEntry (&TextWindow, &NEntry, TextWindow.DFirstLine);            /* Remember top line number */
                              LineNumber = NEntry.Identifier;
                              DestroyComboWindow (&TextWindow, COMF_RESIZING);
                              DestroyWindow (&ViewWindow);
                              CFlags |= COMF_RESIZING;
                              break;
      }
      if (More && GlobalKey == K_ESC)
        More = FALSE;
    }
    MouseStatus ();
  }
  DestroyComboWindow (&TextWindow, 0);
  DestroyWindow (&ViewWindow);
  GlobalKey = 0x0000;
}

void EditHardwareTypeBlock (bool Create, struct ComboInfo *Window, comboent EntryNumber, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the .TAP index window, `EntryNumber' holds the selected entry number, `FullPath' holds the full     */
/*         path to the .TZX file. If `Create' is TRUE, a new, empty block should be created and then edited.                      */
/* Post  : The selected TZX `Hardware Type' block has been edited.                                                                */
/*         The TZX index has been redrawn if the contents of the tape have been changed.                                          */
/* Import: _RewriteTZXFile.                                                                                                       */
/**********************************************************************************************************************************/

{
  char              HValue[20] = "Runs & could use";
  struct WindowInfo CWindow  = {36, 60, 564, 360, 564, 360, 564, 360, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "Edit TZX Hardware Type Block"};
  struct ButtonInfo PCancel  = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 254, 302, 8, DBLACK, DWHITE, NULL, 0, "@Cancel", NULL};
  struct ButtonInfo POk      = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 254, 322, 8, DBLACK, DWHITE, NULL, 1, "@Ok", NULL};
  struct ButtonInfo BAdd     = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 254, 55, 8, DBLACK, DWHITE, NULL, 2, "@Add", NULL};
  struct ButtonInfo BDelete  = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 254, 75, 8, DBLACK, DWHITE, NULL, 3, "@Delete", NULL};
  struct ButtonInfo THow     = {BT_TEXT, BC_TEXT, BF_FWIDTH | BF_FITTXT, 393, 30, 19, DBLACK, DWHITE, NULL, 4, NULL, NULL};
  struct ButtonInfo BHow     = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 514, 30, 1, DBLACK, DWHITE, NULL, 5, "\007", NULL};
  struct ComboInfo        TWindow;
  struct ComboInfo        IWindow;
  struct ComboEntry       TEntry;
  struct ButtonInfo      *ButtonP;
  dword                  *DTable;
  byte                   *BHWTypes;
  byte                   *HWIndex;
  byte                   *HWName;
  byte                    Bt1;
  byte                    Bt2;
  byte                    Bt3;
  byte                    LastValue = 0xFF;
  byte                    LastType = 0xFF;
  comboent                SelectedTLine;
  comboent                SelectedILine;
  byte          register  Cnt;
  byte          register  Cnt2;
  byte                    HNum        = 0;
  bool                    Done        = FALSE;
  bool                    Cancelled   = FALSE;
  bool                    Redraw      = TRUE;
  bool                    First       = TRUE;
  bool                    DoAdd       = FALSE;
  bool                    DoDelete    = FALSE;
  bool                    Changed     = FALSE;
  word                    NumHWTypes;
  dword                   NewField;
  int                     Fh;
  byte                    BlockID;
  word                    OldLength;
  word                    NewLength;
  dword                   FilePos;
  dword                   Swap;
  bool                    More = TRUE;

  THow.Contents = HValue;
  _LoadHardwareTable ();                                                                           /* Read HARDWARE.INC from disk */
  BHWTypes = _BufferCrunched;                                                               /* Use this as an intermediate buffer */
  HWIndex = _BufferDecrunched + (dword)49152 - _HWTableLength;                                   /* Use the end of the WorkBuffer */
  DTable = (dword *)_BufferDecrunched + 2;
  CopyFromXMS (_HWTableLength, _HWTableXMSHandle, 0, HWIndex);                                        /* Read hardware type table */
  if (Create)
  {
    NumHWTypes = 0;
    OldLength = 0;                                                                                /* Signal: no 'previous' length */
    FilePos = 10;                                                               /* Create new blocks at the beginning of the tape */
    BDelete.Flags |= BF_GHOSTED;                                                                         /* Nothing to delete yet */
  }
  else
  {
    GetComboEntry (Window, &TEntry, EntryNumber);
    if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); return; }
    FilePos = TEntry.Identifier;
    if (FilePos < 10)
      FilePos = 10;                                                                          /* Ensure we're after the TZX header */
    if (lseek (Fh, FilePos, SEEK_SET) == -1)                                                     /* Wind tape to correct position */
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
    if (read (Fh, &BlockID, 1) == -1)                                                          /* Read Type ID of following block */
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
    if (BlockID != 0x33)                                                                                 /* MUST be Hardware Type */
    { close (Fh); return; }
    if (read (Fh, &Bt1, 1) == -1)                                                                       /* Read number of entries */
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
    NumHWTypes = (word)Bt1;
    OldLength = NumHWTypes * 3;
    if (OldLength > 0)
    {
      if (read (Fh, BHWTypes, OldLength) == -1)                                                                 /* Read the table */
      { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
      for (Cnt = 0 ; Cnt < NumHWTypes ; Cnt ++)                                                            /* Copy hardware table */
      {
        Bt1 = *(BHWTypes ++);                                                                                  /* (Hardware Type) */
        Bt2 = *(BHWTypes ++);                                                                                    /* (Hardware ID) */
        Bt3 = *(BHWTypes ++);                                                                                          /* (Value) */
        *(DTable + Cnt) = ((dword)Bt3 << 16) | ((dword)Bt1 << 8) | Bt2;
      }
      for (Cnt = NumHWTypes - 1 ; Cnt > 0 && More ; Cnt --)                                                        /* Bubble-sort */
      {
        More = FALSE;
        for (Cnt2 = 0 ; Cnt2 < Cnt ; Cnt ++)
          if (*(DTable + Cnt2) > *(DTable + Cnt2 + 1))
          {
            Swap = *(DTable + Cnt2);
            *(DTable + Cnt2) = *(DTable + Cnt2 + 1);
            *(DTable + Cnt2 + 1) = Swap;
            More = TRUE;
          }
      }
    }
    else
      BDelete.Flags |= BF_GHOSTED;
    close (Fh);
    OldLength += 2;                                                                                      /* (Adjust for later on) */
  }
  AddButton (&CWindow, &PCancel, TRUE);
  AddButton (&CWindow, &POk, FALSE);
  AddButton (&CWindow, &BAdd, FALSE);
  AddButton (&CWindow, &BDelete, FALSE);
  AddButton (&CWindow, &THow, FALSE);
  AddButton (&CWindow, &BHow, FALSE);
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
  WDrawShadowBox (&CWindow, 1, 1, 34, 40, "Hardware Types");
  WDrawShadowBox (&CWindow, 1, 52, 34, 91, "Selected Types");
  Wprintf (&CWindow, 3, 57, "{+1Add as}N");
  MakeComboWindow (&TWindow, &CWindow, COMT_SELECT, COMF_NOBUFFER | COMF_VSLIDER | COMF_SYNCSLIDER, CWindow.StartX + 18,
                   CWindow.StartY + 33, 0, 0, 40, 34, 30, NULL, FALSE, 0, NULL, DBLACK, DWHITE, 0, 0);
  MakeComboWindow (&IWindow, &CWindow, COMT_SELECT, COMF_NOBUFFER | COMF_VSLIDER | COMF_SYNCSLIDER, CWindow.StartX + 324,
                   CWindow.StartY + 53, 0, 0, 40, 34, 28, NULL, FALSE, 0, NULL, DBLACK, DWHITE, 0, 0);
  TEntry.Selected = 0;
  for (Cnt = 0 ; Cnt < *HWIndex ; Cnt ++)                                                                        /* Do all groups */
  {
    sprintf (TEntry.Text, "{+1%s}N", HWIndex + 4 + (dword)*(HWIndex + 2) * Cnt);                             /* Insert group name */
    TEntry.Selected = ENT_NOSELECT;
    if (!InsertComboEntry (&TWindow, &TEntry, NULL, -1))
      FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
    Done = FALSE;
    for (Cnt2 = 0 ; !Done && Cnt2 < *(HWIndex + 1) ; Cnt2 ++)                                        /* Do all objects in a group */
    {
      HWName = HWIndex + 4 + ((dword)*(HWIndex + 2) * *HWIndex) +
                             ((dword)*(HWIndex + 2) * *(HWIndex + 1) * Cnt) +
                             ((dword)*(HWIndex + 2) * Cnt2);
      if (*HWName == '\0')                                                                                     /* Not filled in ? */
        Done = TRUE;
      else
      {
        sprintf (TEntry.Text, "  %s", HWName);
        TEntry.Selected = 0;
        TEntry.Identifier = ((word)Cnt << 8) | Cnt2;
        if (!InsertComboEntry (&TWindow, &TEntry, NULL, -1))
          FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
      }
    }
  }
  PrintComboEntries (&TWindow);
  while (Redraw)
  {
    if (First)
      First = FALSE;
    else
      ReInitComboWindow (&IWindow, 0);
    LastValue = LastType = 0xFF;
    if (NumHWTypes > 0)
      for (Cnt = 0 ; Cnt < NumHWTypes ; Cnt ++)                                                                   /* Do all types */
      {
        TEntry.Selected = 0;
        TEntry.Identifier = 0;
        Bt1 = (byte)((*(DTable + Cnt) & 0x0000FF00) >> 8);                                                     /* (Hardware Type) */
        Bt2 = (byte)(*(DTable + Cnt) & 0x000000FF);                                                              /* (Hardware ID) */
        Bt3 = (byte)((*(DTable + Cnt) & 0x00FF0000) >> 16);                                                            /* (Value) */
        if (LastValue == 0xFF || Bt3 != LastValue)
        {
          LastValue = Bt3;
          LastType = 0xFF;                                                                   /* Force hardware type to be printed */
          switch (Bt3)
          {
            case 0 : strcpy (TEntry.Text, "{BRuns & could use  }B"); break;
            case 1 : strcpy (TEntry.Text, "{BRuns & uses       }B"); break;
            case 2 : strcpy (TEntry.Text, "{BRuns & doesn\'t use}B"); break;
            case 3 : strcpy (TEntry.Text, "{BDoesn\'t run on    }B"); break;
            default: strcpy (TEntry.Text, "{B<UNDEFINED>       }B"); break;
          }
          TEntry.Selected = ENT_NOSELECT;
          if (!InsertComboEntry (&IWindow, &TEntry, NULL, -1))
            FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
          TEntry.Selected = 0;
        }
        if (Bt1 < *HWIndex)
        {
          if (LastType == 0xFF || Bt1 != LastType)
          {
            LastType = Bt1;
            sprintf (TEntry.Text, " {+1%s}N", HWIndex + 4 + (dword)*(HWIndex + 2) * Bt1);
            TEntry.Selected = ENT_NOSELECT;
            if (!InsertComboEntry (&IWindow, &TEntry, NULL, -1))
              FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
            TEntry.Selected = 0;
          }
          HWName = HWIndex + 4 + ((dword)*(HWIndex + 2) * *HWIndex) +
                                 ((dword)*(HWIndex + 2) * *(HWIndex + 1) * Bt1) +
                                 ((dword)*(HWIndex + 2) * Bt2);
          if (*HWName == '\0')                                                                                 /* Not filled in ? */
            strcpy (TEntry.Text, "  <Undefined>");
          else
            sprintf (TEntry.Text, "  %s", HWName);
          TEntry.Identifier = ((dword)Bt3 << 16) | ((dword)Bt1 << 8) | Bt2;
        }
        else
        {
          strcat (TEntry.Text, "<Undefined>)}N");
          TEntry.Selected = ENT_NOSELECT;
        }
        if (!InsertComboEntry (&IWindow, &TEntry, NULL, -1))
          FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
      }
    PrintComboEntries (&IWindow);
    Done = FALSE;
    while (!Done)
    {
      switch (HandleWindow (&CWindow, &ButtonP))
      {
        case WINH_CLOSE   : Done = TRUE; Redraw = FALSE; Cancelled = TRUE; break;
        case WINH_RNOHAND : switch (ButtonP->ReactNum)
                            {
                              case 0 : Done = TRUE; Redraw = FALSE; Cancelled = TRUE; break;                          /* (Cancel) */
                              case 1 : Done = TRUE; Redraw = FALSE; Cancelled = FALSE; break;                             /* (Ok) */
                              case 2 : Changed = TRUE; DoAdd = TRUE; break;                                              /* (Add) */
                              case 3 : Changed = TRUE; DoDelete = TRUE; break;                                        /* (Delete) */
                              case 5 : if (++ HNum == 4)                                                              /* (Add as) */
                                         HNum = 0;
                                       switch (HNum)
                                       {
                                         case 0 : strcpy (HValue, "Runs & could use  "); break;
                                         case 1 : strcpy (HValue, "Runs & uses       "); break;
                                         case 2 : strcpy (HValue, "Runs & doesn\'t use"); break;
                                         case 3 : strcpy (HValue, "Doesn\'t run on    "); break;
                                       }
                                       UpdateButtonText (&CWindow, &THow, BF_FITTXT);
                                       break;
                            }
                            break;
      }
      if (HandleComboWindow (&TWindow, &SelectedTLine) == WINH_LINESEL)
      { Changed = TRUE; DoAdd = TRUE; }
      if (HandleComboWindow (&IWindow, &SelectedILine) == WINH_LINESEL)
      { Changed = TRUE; DoDelete = TRUE; }
      if (DoAdd)
      {
        DoAdd = FALSE;
        GetComboEntry (&TWindow, &TEntry, SelectedTLine);                                              /* Fetch entry to be added */
        NewField = ((dword)HNum << 16) | TEntry.Identifier;                                                /* Build the new field */
        Done = FALSE;
        for (Cnt = 0 ; Cnt < NumHWTypes && !Done ; Cnt ++)                                                 /* Not there already ? */
          if ((*(DTable + Cnt) & 0x0000FFFF) == TEntry.Identifier)
            Done = TRUE;
        if (Done)
        {
          ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - This combination is already there!");
          Done = FALSE;
        }
        else
        {
          for (Cnt = 0 ; Cnt < NumHWTypes && !Done ; Cnt ++)
            if (NewField < *(DTable + Cnt))                                                            /* Must be inserted here ? */
            {
              memmove (DTable + Cnt + 1, DTable + Cnt, (NumHWTypes - Cnt) * 4);                                     /* (Shift up) */
              *(DTable + Cnt) = NewField;
              Done = TRUE;
            }
          if (!Done)                                                                                /* (Must be added at the end) */
            *(DTable + NumHWTypes) = NewField;
          if (++ NumHWTypes == 1)                                                                          /* First entry added ? */
          {
            BDelete.Flags &= ~BF_GHOSTED;                                                            /* Unghost the Delete button */
            UpdateButtonText (&CWindow, &BDelete, 0);
          }
          Done = TRUE; Redraw = TRUE;
        }
      }
      else if (DoDelete)
      {
        DoDelete = FALSE;
        GetComboEntry (&IWindow, &TEntry, SelectedILine);                                            /* Fetch entry to be deleted */
        Done = FALSE;
        for (Cnt = 0 ; Cnt < NumHWTypes && !Done ; Cnt ++)                                                    /* Find the entry ? */
          if (*(DTable + Cnt) == TEntry.Identifier)
          {
            memmove (DTable + Cnt, DTable + Cnt + 1, (NumHWTypes - Cnt - 1) * 4);                                 /* (Shift back) */
            Done = TRUE;
          }
        if (Done)                                                                                               /* (Sanity check) */
        {
          if (-- NumHWTypes == 0)                                                                         /* Last entry deleted ? */
          {
            BDelete.Flags |= BF_GHOSTED;                                                               /* Ghost the Delete button */
            UpdateButtonText (&CWindow, &BDelete, 0);
          }
          Done = TRUE; Redraw = TRUE;
        }
      }
      if (GlobalKey == K_ESC)  { Done = TRUE; Redraw = FALSE; Cancelled = TRUE; }
      MouseStatus ();
    }
  }
  DestroyComboWindow (&TWindow, 0);
  DestroyComboWindow (&IWindow, 0);
  DestroyWindow (&CWindow);
  if (Cancelled || (NumHWTypes == 0 && Create))
    return;
  if (Changed)
  {
    *(_BufferDecrunched) = 0x33;                                                                         /* Init TZX block header */
    *(_BufferDecrunched + 1) = NumHWTypes;
    HWName = _BufferDecrunched + 2;                                     /* (Table is shrinked 3:4, so there's no overlap problem) */
    for (Cnt = 0 ; Cnt < NumHWTypes ; Cnt ++)                                           /* Convert the result back to tape format */
    {
      NewField = *(DTable + Cnt);
      *(HWName ++) = (byte)((NewField & 0x0000FF00) >> 8);                                                     /* (Hardware Type) */
      *(HWName ++) = (byte)(NewField & 0x000000FF);                                                              /* (Hardware ID) */
      *(HWName ++) = (byte)((NewField & 0x00FF0000) >> 16);                                                            /* (Value) */
    }
    NewLength = (NumHWTypes > 0 ? (NumHWTypes * 3) + 2 : 0);                                        /* Determine new block length */
    if (_RewriteTZXFile (FullPath, FilePos, OldLength, NewLength, FALSE))                           /* Write out updated TZX file */
    {
      ReInitComboWindow (Window, 0);
      if (InsertTZXIndex (Window, FullPath))
      {
        if (Create)
          Window->LastSelected = 0;                                                           /* Appoint the newly inserted block */
        else
          Window->LastSelected = EntryNumber;                                              /* Keep pointing to the selected block */
        PrintComboEntries (Window);
        if (NewLength > 0)                                                              /* Do we have a Hardware Type block now ? */
          UnghostMenuEditHardwareType ();
        else
          GhostMenuEditHardwareType ();
        GhostMenuEditArchiveInfo ();
        GhostMenuEditTZXPokes ();
      }
      else
        GhostMenuTZXOptions ();
    }
    else
      GhostMenuTZXOptions ();
  }
  return;
}

void EditArchiveInfoBlock (bool Create, struct ComboInfo *Window, comboent EntryNumber, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the .TAP index window, `EntryNumber' holds the selected entry number, `FullPath' holds the full     */
/*         path to the .TZX file. If `Create' is TRUE, a new, empty block should be created and then edited.                      */
/* Post  : The selected TZX `Archive Info' block has been edited.                                                                 */
/*         The TZX index has been redrawn if the contents of the tape have been changed.                                          */
/* Import: _RewriteTZXFile.                                                                                                       */
/**********************************************************************************************************************************/

{
  struct WindowInfo   CWindow   = {34, 93, 571, 295, 571, 295, 571, 295, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                   WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "Edit Archive Info Block"};
  struct ButtonInfo   PCancel   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 204, 270, 8, DBLACK, DWHITE, NULL, 0, "@Cancel",
                                   NULL};
  struct ButtonInfo   POk       = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 314, 270, 8, DBLACK, DWHITE, NULL, 1, "@Ok", NULL};
  struct ButtonInfo   BTFullNam = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 10, 24, 40, DBLACK, DWHITE, NULL, 2,
                                   NULL, NULL};
  struct ButtonInfo   BTFullI   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 257, 24, 1, DBLACK, DWHITE, NULL, 101, "\003",
                                   NULL};
  struct ButtonInfo   BTFullE   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 269, 24, 1, DBLACK, DWHITE, NULL, 102, "\004",
                                   NULL};
  struct ButtonInfo   BTType    = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 292, 24, 40, DBLACK, DWHITE, NULL, 8,
                                   NULL, NULL};
  struct ButtonInfo   BTPublish = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 10, 64, 40, DBLACK, DWHITE, NULL, 3,
                                   NULL, NULL};
  struct ButtonInfo   BTPublI   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 257, 64, 1, DBLACK, DWHITE, NULL, 103, "\003",
                                   NULL};
  struct ButtonInfo   BTPublE   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 269, 64, 1, DBLACK, DWHITE, NULL, 104, "\004",
                                   NULL};
  struct ButtonInfo   BTYear    = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 292, 64, 40, DBLACK, DWHITE, NULL, 5,
                                   NULL, NULL};
  struct ButtonInfo   BTYearI   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 539, 64, 1, DBLACK, DWHITE, NULL, 105, "\003",
                                   NULL};
  struct ButtonInfo   BTYearE   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 551, 64, 1, DBLACK, DWHITE, NULL, 106, "\004",
                                   NULL};
  struct ButtonInfo   BTLang    = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 10, 104, 40, DBLACK, DWHITE, NULL, 6,
                                   NULL, NULL};
  struct ButtonInfo   BTLangI   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 257, 104, 1, DBLACK, DWHITE, NULL, 107, "\003",
                                   NULL};
  struct ButtonInfo   BTLangE   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 269, 104, 1, DBLACK, DWHITE, NULL, 108, "\004",
                                   NULL};
  struct ButtonInfo   BTScheme  = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 292, 104, 40, DBLACK, DWHITE, NULL, 10,
                                   NULL, NULL};
  struct ButtonInfo   BTPrice   = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 10, 144, 40, DBLACK, DWHITE, NULL, 9,
                                   NULL, NULL};
  struct ButtonInfo   BTOrigin  = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 292, 144, 40, DBLACK, DWHITE, NULL, 11,
                                   NULL, NULL};
  struct InputBoxInfo BTAuthors = {10, 184, 40, 6, DBLACK, DWHITE, NULL, 4, NULL, BF_NOSURROUND | BF_KEYSELECT, 255, 10};
  struct ButtonInfo   BTAuthI   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 257, 184, 1, DBLACK, DWHITE, NULL, 109, "\003",
                                   NULL};
  struct ButtonInfo   BTAuthE   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 269, 184, 1, DBLACK, DWHITE, NULL, 110, "\004",
                                   NULL};
  struct InputBoxInfo BTComment = {292, 184, 40, 6, DBLACK, DWHITE, NULL, 7, NULL, BF_NOSURROUND | BF_KEYSELECT, 255, 10};
  char                TextString[10][256] = {{"\0"}, {"\0"}, {"\0"}, {"\0"}, {"\0"}, {"\0"}, {"\0"}, {"\0"}, {"\0"}, {"\0"}};
  struct ButtonInfo  *ButtonP;
  struct ComboEntry   TEntry;
  char                Buffer[10];
  bool                Done      = FALSE;
  bool                Cancelled = FALSE;
  bool                Changed   = FALSE;
  bool                MainChanged = FALSE;
  byte               *BlockP;
  byte                NumFields;
  byte                FieldType;
  byte                FieldLength;
  byte      register  Cnt;
  byte      register  Cnt2;
  int                 Fh;
  byte                BlockID;
  word                OldLength;
  word                NewLength;
  char               *AuthIn;
  char               *AuthOut;
  word                AuthLength;
  bool                CommaDone;
  dword               FilePos;

  BTFullNam.Contents = TextString[0];
  BTPublish.Contents = TextString[1];
  BTAuthors.Contents = TextString[2];
  BTYear.Contents    = TextString[3];
  BTLang.Contents    = TextString[4];
  BTType.Contents    = TextString[5];
  BTPrice.Contents   = TextString[6];
  BTScheme.Contents  = TextString[7];
  BTOrigin.Contents  = TextString[8];
  BTComment.Contents = TextString[9];
  if (Create)
  {
    OldLength = 0;                                                                                /* Signal: no 'previous' length */
    FilePos = 10;                                                               /* Create new blocks at the beginning of the tape */
  }
  else
  {
    GetComboEntry (Window, &TEntry, EntryNumber);
    if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); return; }
    FilePos = TEntry.Identifier;
    if (FilePos < 10)
      FilePos = 10;                                                                          /* Ensure we're after the TZX header */
    if (lseek (Fh, FilePos, SEEK_SET) == -1)                                                     /* Wind tape to correct position */
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
    if (read (Fh, &BlockID, 1) == -1)                                                          /* Read Type ID of following block */
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
    if (BlockID != 0x32)                                                                                  /* MUST be Archive Info */
    { close (Fh); return; }
    if (read (Fh, &OldLength, 2) == -1)                                                              /* Read size of entire block */
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
    if (OldLength > 0)
    {
      if (read (Fh, _BufferDecrunched + 3, OldLength) == -1)                                                     /* Read the data */
      { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
      NumFields = *(_BufferDecrunched + 3);
      BlockP = _BufferDecrunched + 4;                                                                   /* Start at the beginning */
      while (NumFields > 0)
      {
        switch (*(BlockP ++))                                                             /* Determine what type of field this is */
        {
          case 0x00 : FieldType = 0x00; break;                                                                    /* (Full Title) */
          case 0x01 : FieldType = 0x01; break;                                                      /* (Software House/Publisher) */
          case 0x02 : FieldType = 0x02; break;                                                                     /* (Author(s)) */
          case 0x03 : FieldType = 0x03; break;                                                           /* (Year of Publication) */
          case 0x04 : FieldType = 0x04; break;                                                                      /* (Language) */
          case 0x05 : FieldType = 0x05; break;                                                             /* (Game/Utility Type) */
          case 0x06 : FieldType = 0x06; break;                                                                         /* (Price) */
          case 0x07 : FieldType = 0x07; break;                                                      /* (Protection Scheme/Loader) */
          case 0x08 : FieldType = 0x08; break;                                                                        /* (Origin) */
          default   : FieldType = 0x09; break;                                                            /* (Comment(s)/Unknown) */
        }
        FieldLength = *(BlockP ++);
        for (Cnt = 0 ; Cnt < FieldLength ; Cnt ++)
          TextString[FieldType][Cnt] = *(BlockP ++);
        TextString[FieldType][FieldLength] = '\0';                                             /* (Stick an endmarker at the end) */
        NumFields --;
      }
    }
    close (Fh);
    OldLength += 3;                                                                                      /* (Adjust for later on) */
  }
  AddButton (&CWindow, &PCancel, FALSE);
  AddButton (&CWindow, &POk, TRUE);
  AddButton (&CWindow, &BTFullNam, FALSE);
  AddButton (&CWindow, &BTFullI, FALSE);
  AddButton (&CWindow, &BTFullE, FALSE);
  AddButton (&CWindow, &BTType, FALSE);
  AddButton (&CWindow, &BTPublish, FALSE);
  AddButton (&CWindow, &BTPublI, FALSE);
  AddButton (&CWindow, &BTPublE, FALSE);
  AddButton (&CWindow, &BTYear, FALSE);
  AddButton (&CWindow, &BTYearI, FALSE);
  AddButton (&CWindow, &BTYearE, FALSE);
  AddButton (&CWindow, &BTLang, FALSE);
  AddButton (&CWindow, &BTLangI, FALSE);
  AddButton (&CWindow, &BTLangE, FALSE);
  AddButton (&CWindow, &BTScheme, FALSE);
  AddButton (&CWindow, &BTPrice, FALSE);
  AddButton (&CWindow, &BTOrigin, FALSE);
  AddInputBox (&CWindow, &BTAuthors, FALSE);
  AddButton (&CWindow, &BTAuthI, FALSE);
  AddButton (&CWindow, &BTAuthE, FALSE);
  AddInputBox (&CWindow, &BTComment, FALSE);
  BTFullNam.MaxInputLength = BTPublish.MaxInputLength = BTYear.MaxInputLength = BTLang.MaxInputLength = 
  BTType.MaxInputLength = BTPrice.MaxInputLength = BTScheme.MaxInputLength = BTOrigin.MaxInputLength = 255;
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
  Wprintf (&CWindow,  1,  1, "{+1Full Title}N");
  Wprintf (&CWindow,  1, 48, "{+1Game/Utility Type}N");
  Wprintf (&CWindow,  5,  1, "{+1Software House/Publisher}N");
  Wprintf (&CWindow,  5, 48, "{+1Year of Publication}N");
  Wprintf (&CWindow,  9,  1, "{+1Language}N");
  Wprintf (&CWindow,  9, 48, "{+1Protection Scheme/Loader}N");
  Wprintf (&CWindow, 13,  1, "{+1Price}N");
  Wprintf (&CWindow, 13, 48, "{+1Origin}N");
  Wprintf (&CWindow, 17,  1, "{+1Author(s)}N");
  Wprintf (&CWindow, 17, 48, "{+1Comment(s)}N");
  while (!Done)
  {
    switch (HandleWindow (&CWindow, &ButtonP))
    {
      case WINH_CLOSE   : Done = TRUE; Cancelled = TRUE; break;
      case WINH_RNOHAND : switch (ButtonP->ReactNum)
                          {
                            case 0  : Done = TRUE; Cancelled = TRUE; break;                                           /* (Cancel) */
                            case 1  : Done = TRUE; Cancelled = FALSE; break;                                              /* (Ok) */
                            case 101: strcpy (TextString[0], CurrentEntry.Name);                               /* Import FullName */
                                      UpdateButtonText (&CWindow, &BTFullNam, BF_FITTXT);
                                      Changed = TRUE;
                                      break;
                            case 102: if (strlen (TextString[0]) < D_NAME)                                     /* Export FullName */
                                        strcpy (CurrentEntry.Name, TextString[0]);
                                      else
                                      {
                                        strncpy (CurrentEntry.Name, TextString[0], D_NAME);
                                        CurrentEntry.Name[D_NAME] = '\0';
                                      }
                                      MainChanged = TRUE;
                                      break;
                            case 103: strcpy (TextString[1], CurrentEntry.Publisher);                         /* Import Publisher */
                                      UpdateButtonText (&CWindow, &BTPublish, BF_FITTXT);
                                      Changed = TRUE;
                                      break;
                            case 104: if (strlen (TextString[1]) < D_PUBLISHER)                               /* Export Publisher */
                                        strcpy (CurrentEntry.Publisher, TextString[1]);
                                      else
                                      {
                                        strncpy (CurrentEntry.Publisher, TextString[1], D_PUBLISHER);
                                        CurrentEntry.Publisher[D_PUBLISHER] = '\0';
                                      }
                                      MainChanged = TRUE;
                                      break;
                            case 105: strcpy (TextString[3], CurrentEntry.Year);                                   /* Import Year */
                                      UpdateButtonText (&CWindow, &BTYear, BF_FITTXT);
                                      Changed = TRUE;
                                      break;
                            case 106: Done = FALSE;                                                                /* Export Year */
                                      for (Cnt = 0 ; !Done && Cnt < strlen (TextString[3]) ; Cnt ++)
                                        if (isdigit (TextString[3][Cnt]) && isdigit (TextString[3][Cnt + 1]))       /* 2 digits ? */
                                          Done = TRUE;                                                     /* Mark start of block */
                                      if (Done)                                                    /* Found numerical component ? */
                                      {
                                        Cnt2 = 0;
                                        Cnt --;                                                      /* Adjust for-loop increment */
                                        while (isdigit (TextString[3][Cnt]) && Cnt2 < 4)        /* More than 4 digits is nonsense */
                                          Buffer[Cnt2 ++] = TextString[3][Cnt ++];
                                        Buffer[Cnt2] = '\0';                                                  /* Make it a string */
                                        strcpy (CurrentEntry.Year, Buffer);
                                        MainChanged = TRUE;
                                      }
                                      Done = FALSE;
                                      break;
                            case 107: strcpy (TextString[4], CurrentEntry.Language);                           /* Import Language */
                                      UpdateButtonText (&CWindow, &BTLang, BF_FITTXT);
                                      Changed = TRUE;
                                      break;
                            case 108: if (strlen (TextString[4]) < D_LANGUAGE)                                 /* Export Language */
                                        strcpy (CurrentEntry.Language, TextString[4]);
                                      else
                                      {
                                        strncpy (CurrentEntry.Language, TextString[4], D_LANGUAGE);
                                        CurrentEntry.Language[D_LANGUAGE] = '\0';
                                      }
                                      MainChanged = TRUE;
                                      break;
                            case 109: strcpy (TextString[2], CurrentEntry.Author);                              /* Import Authors */
                                      DrawInputBox (&CWindow, &BTAuthors);
                                      Changed = TRUE;
                                      break;
                            case 110: AuthLength = 0;                                                           /* Export Authors */
                                      AuthIn = TextString[2];
                                      AuthOut = CurrentEntry.Author;
                                      CommaDone = FALSE;
                                      while (++ AuthLength <= D_AUTHOR && *AuthIn)
                                      {
                                        if (*AuthIn == 0x10 || *AuthIn == 0x0D)                                  /* End-of-line ? */
                                        {
                                          AuthIn ++;
                                          if (!CommaDone)                                   /* (Convert "Name<LF>" into "Name, ") */
                                            if (AuthLength ++ <= D_AUTHOR)
                                              *(AuthOut ++) = ',';
                                          if (AuthLength <= D_AUTHOR)                      /* (Convert "Name,<LF>" into "Name, ") */
                                            *(AuthOut ++) = ' ';
                                        }
                                        else if (*AuthIn == ',' || *AuthIn == ';')
                                        {
                                          CommaDone = TRUE;
                                          *(AuthOut ++) = *(AuthIn ++);
                                        }
                                        else
                                        {
                                          if (*AuthIn != ' ')
                                            CommaDone = FALSE;
                                          *(AuthOut ++) = *(AuthIn ++);
                                        }
                                      }
                                      *AuthOut = '\0';
                                      MainChanged = TRUE;
                                      break;
                          }
                          break;
      case WINH_EDITEND : if (ButtonP->Flags & BF_FITINPUT)                                            /* Contents were changed ? */
                            Changed = TRUE;
    }
    if (GlobalKey == K_ESC)  { Done = TRUE; Cancelled = TRUE; }
    MouseStatus ();
  }
  DestroyWindow (&CWindow);
  if (MainChanged)                                                                                        /* Exported any texts ? */
  {
    EntryChanged = TRUE;
    SignalDBChanged (TRUE);
    SelectedGame = SortEntry (SWindow, &CurrentEntry, SelectedGame);
    SelectEntry (SWindow, SelectedGame);
    SelectGame (SelectedGame);
  }
  if (Cancelled)                                                                                                   /* Cancelled ? */
    return;
  if (Create)
  {
    Done = FALSE;
    for (FieldType = 0 ; FieldType < 10 ; FieldType ++)
      if (TextString[FieldType][0] != '\0')                                                                  /* Field filled in ? */
        Done = TRUE;                                                                                /* Signal: at least one entry */
    if (!Done)                                                                           /* Using insert and no field filled in ? */
      return;                                                                                        /* Then we changed our minds */
  }
  if (!Changed)                                                                                                    /* Unchanged ? */
    return;
  NumFields = 0;
  BlockP = _BufferDecrunched + 4;                                                         /* (After the number-of-fields counter) */
  for (FieldType = 0 ; FieldType < 10 ; FieldType ++)
    if (TextString[FieldType][0] != '\0')                                                                    /* Field filled in ? */
    {
      NumFields ++;
      FieldLength = strlen (TextString[FieldType]);
      switch (FieldType)
      {
        case 0x00 : *(BlockP ++) = 0x00; break;
        case 0x01 : *(BlockP ++) = 0x01; break;
        case 0x02 : *(BlockP ++) = 0x02; break;
        case 0x03 : *(BlockP ++) = 0x03; break;
        case 0x04 : *(BlockP ++) = 0x04; break;
        case 0x05 : *(BlockP ++) = 0x05; break;
        case 0x06 : *(BlockP ++) = 0x06; break;
        case 0x07 : *(BlockP ++) = 0x07; break;
        case 0x08 : *(BlockP ++) = 0x08; break;
        case 0x09 : *(BlockP ++) = 0xFF; break;
      }
      *(BlockP ++) = FieldLength;
      for (Cnt = 0 ; Cnt < FieldLength ; Cnt ++)
        *(BlockP ++) = TextString[FieldType][Cnt];
    }
  *(_BufferDecrunched) = 0x32;                                                                 /* Store TZX block header in front */
  *(_BufferDecrunched + 3) = NumFields;                                                        /* Store the number of used fields */
  NewLength = BlockP - _BufferDecrunched;
  *((word *)(_BufferDecrunched + 1)) = NewLength - 3;                                                    /* Store length of block */
  if (_RewriteTZXFile (FullPath, FilePos, OldLength, NewLength, TRUE))                              /* Write out updated TZX file */
  {
    ReInitComboWindow (Window, 0);
    if (InsertTZXIndex (Window, FullPath))
    {
      if (Create)
        Window->LastSelected = 0;                                                             /* Appoint the newly inserted block */
      else
        Window->LastSelected = EntryNumber;                                                /* Keep pointing to the selected block */
      PrintComboEntries (Window);
      if (NewLength > 0)                                                                /* Do we have a Hardware Type block now ? */
        UnghostMenuEditArchiveInfo ();
      else
        GhostMenuEditArchiveInfo ();
      GhostMenuEditHardwareType ();
      GhostMenuEditTZXPokes ();
    }
    else
      GhostMenuTZXOptions ();
  }
  else
    GhostMenuTZXOptions ();
}

void ViewTZXCIPicture (struct ComboInfo *Window, comboent EntryNumber, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `WhichWindow' is either TWLEFT or TWRIGHT, `Entry' points the the tape entry.                                          */
/* Post  : The user has viewed a Picture Block.                                                                                   */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct ComboEntry  NEntry;
  bool               More      = TRUE;
  bool               Continue;
  byte               PicType;
  byte               DescLength;
  dword              LengthToGo;
  int                FhIn;
  int                FhOut;
  char              *UsedViewer;
  char              *UsedViewerOptions;
  char               TmpFileName[MAXDIR];
  char               Description[257];

  GetComboEntry (Window, &NEntry, EntryNumber);
  if ((FhIn = open (FullPath, O_RDONLY | O_BINARY)) == -1)
  { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); return; }
  LengthToGo = NEntry.Identifier;
  if (lseek (FhIn, LengthToGo + 17, SEEK_SET) == -1)                                             /* Wind tape to correct position */
  { close (FhIn); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
  if (read (FhIn, &LengthToGo, 4) == -1)                                                       /* Read length of following block */
  { close (FhIn); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  if (read (FhIn, &PicType, 1) == -1)                                                                        /* Read picture type */
  { close (FhIn); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  if (PicType != 0x00 && PicType != 0x01)
  { close (FhIn); ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Unknown picture type (0x%02X)", PicType); return; }
  if (read (FhIn, &DescLength, 1) == -1)                                                    /* Read length of General Description */
  { close (FhIn); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  if (DescLength == 0)
    strcpy (Description, "Inlay Card");
  else
    if (read (FhIn, Description, DescLength) == -1)                                                   /* Read General Description */
    { fclose (FhIn); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  LengthToGo = LengthToGo - DescLength - 2;
  if (QuestionRequester (-1, -1, FALSE, FALSE, "@View", "@Cancel", NULL, NULL, WBPics[2], "General Picture Description",
                         Description))
  { close (FhIn); return; }
  UsedViewer = (PicType == 0x00 ? DBConfig.GIFViewerPath : DBConfig.JPGViewerPath);
  UsedViewerOptions = (PicType == 0x00 ? DBConfig.GIFViewerOptions : DBConfig.JPGViewerOptions);
  Continue = TRUE;
  if (UsedViewer[0] == '\0')                                                                             /* No viewer defined ? */
  {
    Continue = FALSE;
    if (!QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Picture Viewer Wizard",
                            "No %s viewer program path has not been set in your configuration.\n"
                            "Do you want to select it now?", PicType == 0x00 ? "GIF" : "JPG"))
      Continue = GetConfNewFileName (PicType == 0x00 ? "Select GIF Viewer Path" : "Select JPG Viewer Path", UsedViewer,
                                     UsedViewerOptions, "view a file");
    if (!Continue)
      return;
  }
  MakeFullPath (TmpFileName, TmpDirectory, mktemp (NULL));
  strcpy (TmpFileName + strlen (TmpFileName) - 3, PicType == 0x00 ? "GIF" : "JPG");
  if ((FhOut = open (TmpFileName, O_BINARY | O_RDWR | O_CREAT, 0777)) == -1)                         /* Create the temporary file */
  {
    close (FhIn);
    ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Cannot create %s\n      - %s", TmpFileName, StrError (errno));
    return;
  }
  switch (CopyFilePart (FhIn, FhOut, LengthToGo))                                                 /* Copy the entire picture over */
  {
    case -1 : ErrorRequester (-1, -1, "@Close", NULL, NULL, _ErrOutOfMemory, 2048); More = FALSE; break;
    case  1 : ErrorRequester (-1, -1, "@Close", NULL, NULL, "ERROR - File I/O error: %s", strerror (errno));
              More = FALSE; break;
    case  0 : break;
  }
  close (FhIn);
  close (FhOut);
  if (!More)                                                                                              /* An error occured ? */
    ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Error writing %s\n      - %s", TmpFileName, StrError (errno));
  else
    SpawnApplication ("picture viewer", UsedViewer, UsedViewerOptions, TmpFileName);                          /* Start the viewer */
  unlink (TmpFileName);                                                                                    /* Cleanup and leave */
}

void EditTZXPokesBlock (bool Create, struct ComboInfo *Window, comboent EntryNumber, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the .TAP index window, `EntryNumber' holds the selected entry number, `FullPath' holds the full     */
/*         path to the .TZX file. If `Create' is TRUE, a new, empty block should be created and then edited.                      */
/* Post  : The selected TZX `Custom Info - Pokes' block has been edited.                                                          */
/*         The TZX index has been redrawn if the contents of the tape have been changed.                                          */
/* Import: _RewriteTZXFile.                                                                                                       */
/**********************************************************************************************************************************/

{
  struct ComboEntry TEntry;
  int               Fh;
  dword             OldLength;
  dword             NewLength;
  dword             FilePos;

  *(_BufferDecrunched) = 0x35;
  strncpy (_BufferDecrunched + 1, "POKEs           ", 16);
  if (Create)
  {
    *(_BufferDecrunched + 21) = 0;                                                             /* (Length of General Description) */
    *(_BufferDecrunched + 22) = 0;                                                                        /* (Number of Trainers) */
    OldLength = 2;
    FilePos = 10;                                                               /* Create new blocks at the beginning of the tape */
  }
  else
  {
    GetComboEntry (Window, &TEntry, EntryNumber);
    if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); return; }
    FilePos = TEntry.Identifier;
    if (lseek (Fh, FilePos + 17, SEEK_SET) == -1)                                                /* Wind tape to correct position */
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
    if (read (Fh, &OldLength, 4) == -1)                                                         /* Read Length of following block */
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
    if (OldLength > 49100)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, "Block too large to handle"); return; }
    if (read (Fh, _BufferDecrunched + 21, OldLength) == -1)                                                         /* Read block */
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
    close (Fh);
  }
  if (EditTZXGamePokes (_BufferDecrunched + 21, OldLength, &NewLength))
  {
    *((dword *)(_BufferDecrunched + 17)) = NewLength;
    if (_RewriteTZXFile (FullPath, FilePos, OldLength + 21, NewLength + 21, FALSE))                 /* Write out updated TZX file */
    {
      ReInitComboWindow (Window, 0);
      if (InsertTZXIndex (Window, FullPath))
      {
        if (Create)
          Window->LastSelected = 0;                                                           /* Appoint the newly inserted block */
        else
          Window->LastSelected = EntryNumber;                                              /* Keep pointing to the selected block */
        PrintComboEntries (Window);
        if (NewLength > 0)                                                                      /* Do we have a Pokes block now ? */
          UnghostMenuEditTZXPokes ();
        else
          GhostMenuEditTZXPokes ();
        GhostMenuEditArchiveInfo ();
        GhostMenuEditHardwareType ();
      }
      else
        GhostMenuTZXOptions ();
    }
    else
      GhostMenuTZXOptions ();
  }
}
