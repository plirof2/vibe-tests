/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBSCREEN.C                                                                                                      */
/* Description  : Spectrum Screen dump                                                                                            */
/* Version type : Program module                                                                                                  */
/* Last changed : 13-09-1998  9:35                                                                                                */
/* Update count : 46                                                                                                              */
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

#define __DBSCREEN_MAIN__

#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <dos.h>
#include "wbench32/wbench32.h"
#include "dbfile.h"
#include "dbconfig.h"
#include "dbmain.h"
#include "dbpoke.h"
#include "dbscreen.h"

#pragma pack          ()

static char      *_InvalidZ80File     = "Invalid Z80 file %s";

static byte      *_ScreenBuffer;
static word       _BlockOffset;
static short      _TBY                = 4;

static byte       _ConvertBuffer[4][12288];                                                /* This buffer is used for 2 purposes: */
                                                                                /* 1. As planedata buffer of [4][6336] (33 * 192) */
                                                                              /* 2. As readbuffer: one large array of 49152 bytes */

static byte       _SpectrumPalette[16] = {DBLACK, DBLUE, DRED, DMAGENTA, DGREEN, DCYAN, DYELLOW, DWHITE,
                                          LBLACK, LBLUE, LRED, LMAGENTA, LGREEN, LCYAN, LYELLOW, LWHITE};

static char      *_TBWHeaderTAP  = "Tape Block Image";
static char      *_TBWHeaderMDR  = "MDR Block Image";
static char      *_TBWHeaderSnap = "Snapshot Image";
static struct WindowInfo TBWindow  = {185, 150, 264, 232, 264, 232, 264, 232, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                      WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, NULL};

static char       _ResidentFile[MAXDIR + 1] = {'\0'};
static byte       _ResidentBank = 255;
static word       _ResidentSize;

static bool _FindScreenInTZXFile   (int Fh);
static bool _GetMemoryImageFromZ80 (char *FullPath, byte Bank);
static bool _GetMemoryImageFromSNA (char *FullPath);
static void _WriteScrShotFile      (byte *Buffer);
static void _TBTimedFlash          (struct WindowInfo *Window);

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE INTERNAL SCREEN LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

static bool _FindScreenInTZXFile (int Fh)

/**********************************************************************************************************************************/
/* Pre   : `Fh' points to the open TZX file.                                                                                      */
/* Post  : The first occurence of a block of 6912 data bytes in a block type 0x10, 0x11 or 0x14 is found.                         */
/*         In that case, TRUE is returned and the file position set to the start of that block.                                   */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  dword                    FileIndex;
  word                     NRead;
  byte                     BlockID;
  byte                     RByte;
  word                     RWord;
  short                    RInt;
  dword                    RLong;
  dword                    Skip;

  FileIndex = (dword)10;
  if (lseek (Fh, 10, SEEK_SET) == -1)                                                                   /* Go past the TZX header */
  { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (FALSE); }
  if ((NRead = read (Fh, &BlockID, 1)) == -1)                                                              /* Read first block ID */
  { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (FALSE); }
  while (NRead == 1)                                                                     /* Then there is at least one more block */
  {
    if (BlockID == 0x10 || BlockID == 0x11 || BlockID == 0x14)                                          /* Identified data type ? */
    {
      switch (BlockID)
      {
        case 0x10 : read (Fh, &RWord, 2); read (Fh, &RWord, 2); Skip = RWord; break;                       /* (Normal Speed Data) */
        case 0x11 : read (Fh, &RWord, 2); read (Fh, &RWord, 2); read (Fh, &RWord, 2);                      /* (Custom Speed Data) */
                    read (Fh, &RWord, 2); read (Fh, &RWord, 2); read (Fh, &RWord, 2);
                    read (Fh, &RByte, 1); read (Fh, &RWord, 2); read (Fh, &RLong, 3);
                    Skip = (RLong & 0x00FFFFFF); break;
        case 0x14 : read (Fh, &RWord, 2); read (Fh, &RWord, 2); read (Fh, &RByte, 1);                              /* (Pure Data) */
                    read (Fh, &RWord, 2); read (Fh, &RLong, 3); Skip = (RLong & 0x00FFFFFF); break;
      }
      if ((BlockID == 0x14 && Skip == 6912) || (BlockID != 0x14 && Skip == 6914))                /* Correct length for a screen ? */
      {
        if (BlockID != 0x14)
          read (Fh, &RByte, 1);                                                            /* Skip the flag byte unless Pure Data */
        return (TRUE);
      }
    }
    else                                                                                         /* (Not an identified data type) */
    {
      switch (BlockID)
      {
        case 0x12 : Skip = 4; break;                                                                               /* (Pure Tune) */
        case 0x13 : read (Fh, &RByte, 1); Skip = RByte * 2; break;                                             /* (Row of Pulses) */
        case 0x15 : read (Fh, &RWord, 2); read (Fh, &RWord, 2); read (Fh, &RByte, 1);                       /* (Direct Recording) */
                    read (Fh, &RLong, 3); Skip = (RLong & 0x00FFFFFF); break;
        case 0x20 : read (Fh, &RWord, 2); Skip = 0; break;                                                       /* (Pause Block) */
        case 0x21 : read (Fh, &RByte, 1); Skip = RByte; break;                                                   /* (Group Start) */
        case 0x22 : Skip = 0; break;                                                                               /* (Group End) */
        case 0x23 : read (Fh, &RInt, 2); Skip = 0; break;                                                      /* (Jump To Block) */
        case 0x30 : read (Fh, &RByte, 1); Skip = RByte; break;                                              /* (Text Description) */
        case 0x31 : read (Fh, &RByte, 1); read (Fh, &RByte, 1); Skip = RByte; break;                           /* (Message Block) */
        case 0x32 : read (Fh, &RWord, 2); Skip = RWord; break;                                                  /* (Archive Info) */
        case 0x33 : read (Fh, &RByte, 1); Skip = RByte * 3; break;                                             /* (Hardware Type) */
        case 0x34 : Skip = 8; break;                                                                          /* (Emulation Info) */
        case 0x35 : read (Fh, _BufferDecrunched, 16); *(_BufferDecrunched + 16) = '\0';                    /* (Custom Info Block) */
                    read (Fh, &RLong, 4); Skip = RLong; break;
        case 0x40 : read (Fh, &RByte, 1); read (Fh, &RWord, 2); read (Fh, &RLong, 4); read (Fh, &RLong, 3);   /* (Snapshot Block) */
                    Skip = (RLong & 0x00FFFFFF); break;
        case 0x5A : Skip = 9; break;                                                                              /* (Joined TZX) */
        default   : read (Fh, &RLong, 4); Skip = RLong; break;
      }
    }
    FileIndex = tell (Fh);
    if (Skip > 0)
    {
      FileIndex += Skip;
      if (lseek (Fh, FileIndex, SEEK_SET) == -1)                                                          /* Go to the next block */
      { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return (FALSE); }
    }
    if ((NRead = read (Fh, &BlockID, 1)) == -1)                                                 /* Can I read the next Block ID ? */
    { ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (FALSE); }
  }
  return (FALSE);
}

static bool _GetMemoryImageFromZ80 (char *FullPath, byte Bank)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the window where the piture should go, `StartX', `StartY' hold the top-left edge and `FullPath'     */
/*         holds the full path to the .Z80 file. The version of the Z80 file must have been determined.                           */
/* Post  : The screen is decrunched from the Z80 file and put on the VGA display.                                                 */
/*         The return value is FALSE if any error occured.                                                                        */
/* Import: DecrunchZ80Block.                                                                                                      */
/**********************************************************************************************************************************/

{
  struct BlockInfo_s
  {
    word     Length;
    byte     Number;
  }          BlockInfo;
  int        Fh;
  dword      FileLength;
  dword      Offset         = sizeof (struct Z80145SnapshotHeader_s);
  word       NRead;
  byte       IBank;
  bool       Snap128K;                      /* TRUE for a 128K snapshot */
  word       BankStart48[]  = {0x0000,  /* Bank  0: Basic ROM           */
                               0x0000,  /* Bank  1: Chosen extended ROM */
                               0x0000,  /* Bank  2: Reset ROM           */
                               0x8000,  /* Bank  3: Page 0   C000-FFFF  */
                               0x4000,  /* Bank  4: Page 1  +8000-BFFF  */
                               0x8000,  /* Bank  5: Page 2  +C000-FFFF  */
                               0x8000,  /* Bank  6: Page 3   C000-FFFF  */
                               0x8000,  /* Bank  7: Page 4   C000-FFFF  */
                               0x0000,  /* Bank  8: Page 5  +4000-7FFF  */
                               0x8000,  /* Bank  9: Page 6   C000-FFFF  */
                               0x0000,  /* Bank 10: Page 7   4000-7FFF  */
                               0x0000}; /* Bank 11: Multiface ROM       */
  word       BankStart128[] = {0x0000,  /* Bank  0: Basic ROM           */
                               0x0000,  /* Bank  1: Chosen extended ROM */
                               0x0000,  /* Bank  2: Reset ROM           */
                               0x8000,  /* Bank  3: Page 0  +C000-FFFF  */
                               0x8000,  /* Bank  4: Page 1   C000-FFFF  */
                               0x4000,  /* Bank  5: Page 2  +8000-BFFF  */
                               0x8000,  /* Bank  6: Page 3   C000-FFFF  */
                               0x8000,  /* Bank  7: Page 4   C000-FFFF  */
                               0x0000,  /* Bank  8: Page 5  +4000-7FFF  */
                               0x8000,  /* Bank  9: Page 6   C000-FFFF  */
                               0x0000,  /* Bank 10: Page 7   4000-7FFF  */
                               0x0000}; /* Bank 11: Multiface ROM       */
                              /* *** All 16384 bytes back! *** */
  word       UsedBankStart;

  _ResidentSize = 0;
  IBank = Bank + 3;                                                                                       /* Convert page to bank */
  FileNotFound = TRUE;
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return (TRUE);
  FileNotFound = FALSE;
  FileLength = filelength (Fh);
  if (Z80Version == 145)
  {
    if (lseek (Fh, Offset, SEEK_SET) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return (FALSE); }
    if ((NRead = read (Fh, _BufferCrunched, (word)(FileLength - Offset))) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (FALSE); }
    close (Fh);
    if (Z80145SnapshotHeader.Stat1 & 0x20)                                                                 /* Block is crunched ? */
    {
      if (!DecrunchZ80Block (_BufferCrunched, NRead, _BufferDecrunched, (word)49152))                       /* Then decrunch it! */
      { ErrorRequester (-1, -1, "@Close", NULL, NULL, _InvalidZ80File, FullPath); return (FALSE); }
    }
    else if (FileLength - Offset != (word)49152)
    { ErrorRequester (-1, -1, "@Close", NULL, NULL, "Wrong filesize of file %s", FullPath); return (FALSE); }
    else
      memcpy (_BufferDecrunched, _BufferCrunched, (word)49152);                     /* Move the block to the correct buffer first */
    _ResidentBank = 0;
    _ResidentSize = (word)49152;
    return (TRUE);
  }

  Snap128K = Z80201SnapshotHeader.HWMode >= 3 ? TRUE : FALSE;
  Offset += sizeof (struct Z80201SnapshotHeader_s);
  if (Z80Version == 300)
  {
    Offset += sizeof (struct Z80300SnapshotHeader_s);
    Snap128K = Z80201SnapshotHeader.HWMode >= 4 ? TRUE : FALSE;
  }
  if (Snap128K)
    UsedBankStart = BankStart128[IBank];
  else
    UsedBankStart = BankStart48[IBank];
  if (lseek (Fh, Offset, SEEK_SET) == -1)
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return (FALSE); }
  if ((NRead = read (Fh, &BlockInfo, 3)) == -1)
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (FALSE); }
  while (NRead == 3)                                                                                       /* More block follow ? */
  {
    if (BlockInfo.Number != IBank)                                                                           /* Requested block ? */
    {                                                                                                            /* No, just skip */
      if (BlockInfo.Length == 0xFFFF)                                                     /* Z80 v3.05: This block not crunched ? */
        Offset += 16384 + sizeof (struct BlockInfo_s);                                          /* Then count exactly 16384 bytes */
      else
        Offset += BlockInfo.Length + sizeof (struct BlockInfo_s);
      if (Offset > FileLength)
      { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _InvalidZ80File, FullPath); return (FALSE); }
      if (lseek (Fh, Offset, SEEK_SET) == -1)
      { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return (FALSE); }
      if ((NRead = read (Fh, &BlockInfo, 3)) == -1)
      { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (FALSE); }
    }
    else                                                                                                                  /* YES! */
    {
      if (BlockInfo.Length == 0xFFFF)                                                     /* Z80 v3.05: This block not crunched ? */
      {
        if ((NRead = read (Fh, _BufferDecrunched, 16384)) != 16384)                        /* Read non-crunched block into memory */
        { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (FALSE); }
        close (Fh);
      }
      else
      {
        if ((NRead = read (Fh, _BufferCrunched, BlockInfo.Length)) != BlockInfo.Length)        /* Read crunched block into memory */
        { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (FALSE); }
        close (Fh);
        if (!DecrunchZ80Block (_BufferCrunched, NRead, _BufferDecrunched + UsedBankStart, 16384))
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, _InvalidZ80File, FullPath); return (FALSE); }
      }
      _ResidentBank = Bank;
      _ResidentSize = 16384;
      return (TRUE);
    }
  }
  return (FALSE);
}

static bool _GetMemoryImageFromSNA (char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the window where the piture should go, `StartX', `StartY' hold the top-left edge and `FullPath'     */
/*         holds the full path to the .SNA file.                                                                                  */
/* Post  : The entire image has been read into memory.                                                                            */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  int        Fh;
  dword      FileLength;
  dword      Offset = 0;                                                            /* .SNP files start off with the memory image */
  word       NRead;

  if (FileType == FT_SNA)
    Offset = sizeof (struct MMSnapshotHeader_s);                                       /* .SNA files have the header at the start */
  else if (FileType == FT_SEM)
    Offset = 6;                                                               /* .SEM files have the ID string 'SPEC1\0' in front */
  else if (FileType == FT_SP)
  {
    Offset = sizeof (struct SPSnapshotHeader_s);                                    /* .SP files have the header in front as well */
    memset (_BufferDecrunched, 0, (word)49152);                                                 /* The snap length can be varying */
  }
  _ResidentSize = 0;
  FileNotFound = TRUE;
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return (TRUE);
  FileNotFound = FALSE;
  FileLength = filelength (Fh);
  if (FileType == FT_SP)
  {
    if ((NRead = read (Fh, &SPSnapshotHeader, sizeof (struct SPSnapshotHeader_s))) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (FALSE); }
    if (FileLength < SPSnapshotHeader.Length + Offset)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, "Invalid SP file %s", FullPath); return (FALSE); }
    if ((NRead = read (Fh, _BufferDecrunched + SPSnapshotHeader.Start - 16384, SPSnapshotHeader.Length)) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (FALSE); }
  }
  else
  {
    if (FileLength < (dword)49152 + Offset)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, "Invalid SNA file %s", FullPath); return (FALSE); }
    if (lseek (Fh, Offset, SEEK_SET) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return (FALSE); }
    if ((NRead = read (Fh, _BufferDecrunched, (word)(FileLength - Offset))) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (FALSE); }
  }
  close (Fh);
  _ResidentBank = 0;
  _ResidentSize = (word)49152;
  return (TRUE);
}

static void _WriteScrShotFile (byte *Buffer)

/**********************************************************************************************************************************/
/* Pre   : `Buffer' points to the 6912 bytes screen buffer. The `ScrShotPath' must have been determined.                          */
/* Post  : The buffer has been written as a screenshot file.                                                                      */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  int Fh;

  if ((Fh = open (ScrShotPath, O_BINARY | O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1)
  { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error creating file %s", ScrShotPath); return; }
  if (write (Fh, Buffer, 6912) == -1)                                                                           /* Write the file */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, "Error writing to file %s", ScrShotPath); return; }
  close (Fh);
  SoundErrorBeeper (BEEP_SHORT);
}

static void _TBTimedFlash (struct WindowInfo *Window)

/**********************************************************************************************************************************/
/* Pre   : A spectrum screen has to be set up and drawn already.                                                                  */
/* Post  : This is the timed flash handler. Each run toggles the flashing parts.                                                  */
/* Import: PutSpectrumScreen.                                                                                                     */
/**********************************************************************************************************************************/

{
  Window = Window;
  FlashState = !FlashState;
  PutSpectrumScreen (_ScreenBuffer + _BlockOffset, TBWindow.StartX + 4, TBWindow.StartY + _TBY);
}

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE EXPORTED SCREEN LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

void SpectrumScreenToGIF (char *FileName, bool FullName, short StartX, short StartY)

/**********************************************************************************************************************************/
/* Pre   : `FileName' is the PCName field of the displayed Spectrum screen, `StartX', `StartY' hold the top-left coordinate at    */
/*         screen where the spectrum image is displayed.                                                                          */
/*         if `FullName' is TRUE, `FileName' holds the full path and no translation is needed.                                    */
/* Post  : A 256x192x16 GIF-file has been made.                                                                                   */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  byte  register  Q;                                                                                                   /* Counter */
  byte            ErrCode;
  dword          *PP;                                                                                          /* Palette Pointer */
  char            GIFFullPath[MAXDIR];
  char            TestPath[MAXDIR];
  char            PathBuffer[MAXDIR];
  bool            GIFPathExists;
  unsigned        Dummy;

  HideMouse ();
  if (!FullName)
  {
    if (DBConfig.GIFPath[0] != '\0')                                                                 /* Has a path been defined ? */
    {
      if (DBConfig.GIFPath[0] == RELPATH)                                                          /* Path relative to the game ? */
        MakeFullPath (GIFFullPath, GamePath, DBConfig.GIFPath + 1);
      else
        strcpy (GIFFullPath, DBConfig.GIFPath);
      _fullpath (TestPath, GIFFullPath, MAXDIR);
      HWEAbort = TRUE;                                          /* Return immediately if the next functions cause a hardware trap */
      HWEOccured = FALSE;
      _dos_setdrive (ToNumber (TestPath[0]), &Dummy);                                                         /* Go to this drive */
      getcwd (PathBuffer, MAXDIR);
      GIFPathExists = !(chdir (TestPath + 2));                                                    /* Can I go to that directory ? */
      if (HWEOccured)                                                                     /* Not found if a hardware trap occured */
        GIFPathExists = FALSE;
      HWEAbort = FALSE;
      if (!GIFPathExists)                                                                   /* Does the directory exist already ? */
        if (!QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Create Directory",
            "The GIF directory\n    %s\ndoes not exist yet.\nDo you want to create it now ?", TestPath))
          if (mkdir (TestPath))                                                                    /* Try to create the directory */
            ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Could not create directory:\n%s", strerror (errno));
          else
            GIFPathExists = TRUE;
      chdir (PathBuffer);
      _dos_setdrive (CurDrv, &Dummy);
      chdir (CurPath);                                                                             /* Restore drive and directory */
      if (!GIFPathExists)                                                                           /* GIF path creation failed ? */
        return;
      MakeFullPath (GIFFullPath, TestPath, FileName);
    }
    else
      strcpy (GIFFullPath, FileName);
    strcpy (GIFFullPath + strlen (GIFFullPath) - 4, ".GIF");                        /* Extension was 3 chars for all known types! */
  }
  else
    strcpy (GIFFullPath, FileName);
  if (TestFileOverwrite (GIFFullPath))
    return;
  if ((ErrCode = GIFCreate (GIFFullPath, 320, 240, 16, 6)) != GIF_OK)                              /* Choose a bigger screen size */
  { ErrorRequester (-1, -1, NULL, NULL, NULL, "Cannot create GIF file (%s)", GIFstrerror (ErrCode)); return; }
  PP = Screen.CurrentPal;
  for (Q = 0 ; Q < Screen.NumColors ; Q ++, PP ++)
    GIFSetColor (Q, (byte)(*PP & 0x000000FF), (byte)((*PP & 0x0000FF00) >> 8), (byte)((*PP & 0x00FF0000) >> 16));
  if ((ErrCode = GIFCompressImage (StartX, StartY, 256, 192)) != GIF_OK)
  { ErrorRequester (-1, -1, NULL, NULL, NULL, "Cannot compress GIF image (%s)", GIFstrerror (ErrCode)); GIFClose ();
    unlink (GIFFullPath); return; }
  if ((ErrCode = GIFClose ()) != GIF_OK)
  { ErrorRequester (-1, -1, NULL, NULL, NULL, "Cannot close GIF file (%s)", GIFstrerror (ErrCode)); unlink (GIFFullPath); return; }
  ShowMouse ();
  SoundErrorBeeper (BEEP_SHORT);
}

void PutSpectrumScreen (byte *ScreenBuffer, short HorOff, short VerOff)

/**********************************************************************************************************************************/
/* Pre   : `ScreenBuffer' points to the 6912 bytes spectrum screen, `HorOff' holds the horizontal, `VerOff' holds the vertical    */
/*         pixel coordinate to put the spectrum screen on the VGA display; `RemapBrightBlack' is TRUE if the ULA bug should be    */
/*         used (bright and dark black both resolve to dark black).                                                               */
/* Post  : The spectrum screen has been put on the screen.                                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  byte            PixelMask;
  byte            PaletteMask;
  byte            PixelData;
  byte            ScreenBlockIndex;
  byte            ScreenBlockCount;
  word            PlaneDataIndex;
  word            ScreenDataIndex;
  word  register  PaletteIndex;
  byte           *StartAddress;                                           /* The first address in display memory to be considered */
  byte           *Address;                                              /* The current address in display memory being considered */
  short register  CntY;                                                                              /* Vertical scanline counter */
  short register  CntX;                                                                                /* Horizontal byte counter */
  word  register  CntPlane;                                                      /* Binary representation of the current bitplane */
  byte  register  ReadPlane;                                                                           /* Linear bitplane counter */
  byte            LeftMask;
  byte            ShiftRight;                                                                        /* Bit-shift within the byte */

  StartAddress = Screen.DisplayStart + 80 * VerOff + HorOff / 8;
  ShiftRight = HorOff % 8;
  LeftMask = 0xFF >> ShiftRight;

  _SpectrumPalette[8] = (DBConfig.RemapBrightBlack ? DBLACK : LBLACK);

  ScreenContainsFlash = FALSE;
  PaletteIndex = PlaneDataIndex = 0;
  for (ScreenBlockCount = 0 ; ScreenBlockCount < 3 ; ScreenBlockCount ++)              /* Spectrum screen is composed of 3 blocks */
    for (CntY = 0 ; CntY < 8 ; CntY ++)                                                        /* 8 character lines in each block */
    {
      ScreenDataIndex = ScreenBlockCount * 2048 + CntY * 32;
      for (ScreenBlockIndex = 0 ; ScreenBlockIndex < 8 ; ScreenBlockIndex ++)                  /* Each character has 8 scan lines */
      {
        for (CntX = 0 ; CntX < 32 ; CntX ++)
        {
          PaletteMask = *(ScreenBuffer + 6144 + PaletteIndex + CntX);
          if (PaletteMask & 0x80)
            ScreenContainsFlash = TRUE;
          if ((PaletteMask & 0x80) && FlashState)
            PixelMask = _SpectrumPalette[(PaletteMask & 0x78) >> 3];
          else
            PixelMask = _SpectrumPalette[(PaletteMask & 0x07) | ((PaletteMask & 0x40) >> 3)];
          PixelData = *(ScreenBuffer + ScreenDataIndex + CntX);
          for (CntPlane = 0, ReadPlane = 1; CntPlane < Screen.NumPlanes ; CntPlane ++, ReadPlane <<= 1)
            _ConvertBuffer[CntPlane][PlaneDataIndex] = (PixelMask & ReadPlane ? PixelData : 0);
          if ((PaletteMask & 0x80) && FlashState)
            PixelMask = _SpectrumPalette[(PaletteMask & 0x07) | ((PaletteMask & 0x40) >> 3)];
          else
            PixelMask = _SpectrumPalette[(PaletteMask & 0x78) >> 3];
          for (CntPlane = 0, ReadPlane = 1; CntPlane < Screen.NumPlanes ; CntPlane ++, ReadPlane <<= 1)
            _ConvertBuffer[CntPlane][PlaneDataIndex] |= (PixelMask & ReadPlane ? ~PixelData : 0);
          PlaneDataIndex ++;
        }
        ScreenDataIndex += 256;                                                       /* One spectrum scan line down is 256 bytes */
        PlaneDataIndex ++;                                                                          /* Skip past the padding byte */
      }
      PaletteIndex += 32;                                                                     /* Palette is `correctly' addressed */
    }

  if (ShiftRight != 0)
    for (ReadPlane = 0 ; ReadPlane < Screen.NumPlanes ; ReadPlane ++)
      for (CntY = 0 ; CntY < 192 ; CntY ++)
      {
        _ConvertBuffer[ReadPlane][CntY * 33 + 32] = 0;
        for (CntX = 31 ; CntX >= 0 ; CntX --)
          _ConvertBuffer[ReadPlane][CntPlane] = (_ConvertBuffer[ReadPlane][CntPlane = CntY * 33 + CntX + 1] >> ShiftRight) |
                                                 (_ConvertBuffer[ReadPlane][CntPlane - 1] << (8 - ShiftRight));
        _ConvertBuffer[ReadPlane][CntY * 33] >>= ShiftRight;
      }

  HideMouseRange (HorOff, VerOff, HorOff + 255, VerOff + 191);
  StartScreenUpdate (HorOff, VerOff, HorOff + 255, VerOff + 191);
  for (ReadPlane = 0 ; ReadPlane < Screen.NumPlanes ; ReadPlane ++)                                       /* Handle all bitplanes */
  {
    Address = StartAddress;
    SelectPlane (ReadPlane);
    for (CntY = 0 ; CntY < 192 ; CntY ++)
    {
      *(Address ++) = (*(Address) & ~LeftMask) | (_ConvertBuffer[ReadPlane][CntY * 33] & LeftMask);
      for (CntX = 1 ; CntX < 32 ; CntX ++)
        *(Address ++) = _ConvertBuffer[ReadPlane][CntY * 33 + CntX];
      if (ShiftRight == 0)
        Address += 48;
      else
      {
        *(Address ++) = (*(Address) & LeftMask) | (_ConvertBuffer[ReadPlane][CntY * 33 + CntX] & ~LeftMask);
        Address += 47;
      }
    }
  }
  StopScreenUpdate ();
  ShowMouse ();
}

void DisplayScreenFromProgram (struct ComboInfo *Window, comboent EntryNumber, char *FullPath, char *PCName)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the .TAP index window, `EntryNumber' holds the selected entry number, `FullPath' holds the full     */
/*         path to the .TAP file, `PCName' holds the PCName field of the selected game.                                           */
/*         Depending on the filetype (TAP/snap), the screen extraction is different.                                              */
/* Post  : The selected entry has been converted to a screen and put in a seperate window, together with a 'Make GIF' button.     */
/*         If this button has been pressed, a GIF image has been created. The user must explicitely use the close gadget.         */
/* Import: PutSpectrumScreen, SpectrumScreenToGIF.                                                                                */
/**********************************************************************************************************************************/

{
  struct MicrodriveRecord_s *MDriveRecord;
  char              Offset[6] = "0";
  struct ButtonInfo PMkGIF   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 19, 210, 10, DBLACK, DWHITE, NULL, 1, "Make @GIF",
                                NULL};
  struct ButtonInfo PMkSCR   = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 106, 210, 10, DBLACK, DWHITE, NULL, 2, "Make @SCR",
                                NULL};
  struct ButtonInfo PDecOff  = {BT_CPUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 189, 210, 1, DBLACK, DWHITE, NULL, 3, "\003", NULL};
  struct ButtonInfo PSetOff  = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 201, 210, 5, DBLACK, DWHITE, NULL, 4,
                                NULL, NULL};
  struct ButtonInfo PIncOff  = {BT_CPUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 238, 210, 1, DBLACK, DWHITE, NULL, 5, "\004", NULL};
  struct ButtonInfo *ButtonP;
  struct ComboEntry NEntry;
  byte              *ImageBuffer;
  bool              Done;
  bool              Redraw = TRUE;
  bool              SizeUnknown = TRUE;
  int               Fh;
  word              BlockLength;
  dword             COffset;
  word              MaxOffset;
  bool              MDRBlocks[128];                                                                      /* 128 * 512 bytes = 64K */
  int               Cnt;
  int               NumBlocks;
  byte              BlockID;
  byte              RByte;
  word              RWord;
  dword             RLong;
  char              BlockName[11];

  TBWindow.Header = (FileType == FT_TAP ? _TBWHeaderTAP : (FileType == FT_MDR ? _TBWHeaderMDR : _TBWHeaderSnap));
  PSetOff.Contents = Offset;
  AddButton (&TBWindow, &PMkGIF, TRUE);                                     /* Window MUST be created first, as the _ScreenBuffer */
  AddButton (&TBWindow, &PMkSCR, FALSE);                                             /* is global now and used to draw the window */
  AddButton (&TBWindow, &PDecOff, FALSE);
  AddButton (&TBWindow, &PSetOff, FALSE);
  AddButton (&TBWindow, &PIncOff, FALSE);
  DrawWindow (&TBWindow, NULL, WING_CLOSE, TRUE);
  SurroundScreen (&TBWindow, 2, 2);
  GetComboEntry (Window, &NEntry, EntryNumber);
  memset (_ScreenBuffer, 0, 6144);                                                                            /* DisplayFile part */
  memset (_ScreenBuffer + 6144, 0x38, 768);                                                                    /* Attributes file */
  while (Redraw)
  {
    _BlockOffset = atoi (Offset);                                                                          /* Read offset setting */
    if (SizeUnknown)                                                                                     /* Only at the first run */
      if (FileType == FT_TAP)                                                                          /* Read TAP files directly */
      {
        if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
        { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); DestroyWindow (&TBWindow); return; }
        if (lseek (Fh, NEntry.Identifier, SEEK_SET) == -1)                                       /* Wind tape to correct position */
        { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno));
          DestroyWindow (&TBWindow); return; }
        if (read (Fh, &BlockLength, 2) == -1)                                                   /* Read length of following block */
        { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
          DestroyWindow (&TBWindow); return; }
        BlockLength -= 2;                                                                   /* Exclude flag and block parity byte */
        if (BlockLength > (word)49152)
          BlockLength = (word)49152;
        if (BlockLength <= 6912)
          MaxOffset = 0;
        else
          MaxOffset = BlockLength - 6912;
        SizeUnknown = FALSE;
        if (lseek (Fh, (dword)NEntry.Identifier + _BlockOffset + 3, SEEK_SET) == -1)             /* Wind tape to correct position */
        { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno));
          DestroyWindow (&TBWindow); return; }
        if (read (Fh, _ScreenBuffer, BlockLength) == -1)                                     /* Read the entire block into memory */
        { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
          DestroyWindow (&TBWindow); return; }
        close (Fh);
      }
      else if (FileType == FT_MDR)                                                                     /* Read MDR files directly */
      {
        if ((MDriveRecord = (struct MicrodriveRecord_s *)malloc (sizeof (struct MicrodriveRecord_s))) == NULL)
        { ErrorRequester (-1, -1, "@Stop", NULL, NULL, _ErrOutOfMemory, sizeof (struct MicrodriveRecord_s));
          DestroyWindow (&TBWindow); return; }
        for (Cnt = 0 ; Cnt < 128 ; Cnt ++)                                                         /* Clear the done-blocks table */
          MDRBlocks[Cnt] = FALSE;
        if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
        { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); DestroyWindow (&TBWindow); return; }
        if (lseek (Fh, NEntry.Identifier, SEEK_SET) == -1)                                       /* Read the header (first) block */
        { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno));
          DestroyWindow (&TBWindow); return; }
        if (read (Fh, MDriveRecord, sizeof (struct MicrodriveRecord_s)) == -1)                                  /* Read the block */
        { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
          DestroyWindow (&TBWindow); return; }
        memcpy (BlockName, MDriveRecord->RecNam, 10);                                                         /* Store block name */
        BlockName[10] = '\0';
        BlockLength = MDriveRecord->DLength;
        NumBlocks = (BlockLength / 512) + (BlockLength % 512 ? 1 : 0);                                        /* Number of blocks */
        if (lseek (Fh, (dword)0, SEEK_SET) == -1)                                                         /* Go back to the start */
        { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno));
          DestroyWindow (&TBWindow); return; }
        for (Cnt = 0 ; Cnt < 254 ; Cnt ++)
        {
          if (read (Fh, MDriveRecord, sizeof (struct MicrodriveRecord_s)) == -1)                               /* Read each block */
          { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
            DestroyWindow (&TBWindow); return; }
          if (MDriveRecord->RecNum < 128 && MDriveRecord->RecLen <= 512)                                   /* Acceptable record ? */
            if (!memcmp (MDriveRecord->RecNam, BlockName, 10))                                        /* A record of this block ? */
            {                                                                            /* Copy datapart in the correct location */
              if (MDriveRecord->RecNum == 0)                                                                     /* First block ? */
                memcpy (_ScreenBuffer, (byte *)&MDriveRecord->DBlock, MDriveRecord->RecLen);
              else
                memcpy (_ScreenBuffer + (dword)503 + (dword)512 * (MDriveRecord->RecNum - 1), (byte *)&MDriveRecord->DType,
                          MDriveRecord->RecLen);
              MDRBlocks[MDriveRecord->RecNum] = TRUE;
            }
        }
        close (Fh);
        for (Cnt = 0 ; Cnt < NumBlocks && MDRBlocks[Cnt] ; Cnt ++)
          ;
        if (Cnt != NumBlocks)                                                                            /* Did we miss a block ? */
        { ErrorRequester (-1, -1, "@Close", NULL, NULL, "Missing record number %d in file\n  %s", Cnt, FullPath);
          DestroyWindow (&TBWindow); return; }
        if (BlockLength > (word)49152)
          BlockLength = (word)49152;
        else if (BlockLength <= 6912)
          MaxOffset = 0;
        else
          MaxOffset = BlockLength - 6912;
        SizeUnknown = FALSE;
      }
      else if (FileType == FT_TZX)                                                                     /* Read TZX files directly */
      {
        if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
        { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); DestroyWindow (&TBWindow); return; }
        if (lseek (Fh, NEntry.Identifier, SEEK_SET) == -1)                                       /* Wind tape to correct position */
        { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno));
          DestroyWindow (&TBWindow); return; }
        if (read (Fh, &BlockID, 1) == -1)                                                                        /* Read block ID */
        { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
          DestroyWindow (&TBWindow); return; }
        switch (BlockID)
        {
          case 0x10 : read (Fh, &RWord, 2); read (Fh, &RWord, 2); BlockLength = RWord - 2;                 /* (Normal Speed Data) */
                      read (Fh, &RByte, 1); break;                                                          /* Skip the flag byte */
          case 0x11 : read (Fh, &RWord, 2); read (Fh, &RWord, 2); read (Fh, &RWord, 2);                    /* (Custom Speed Data) */
                      read (Fh, &RWord, 2); read (Fh, &RWord, 2); read (Fh, &RWord, 2);
                      read (Fh, &RByte, 1); read (Fh, &RWord, 2); read (Fh, &RLong, 3);
                      BlockLength = (RLong & 0x00FFFFFF); read (Fh, &RByte, 1); break;                      /* Skip the flag byte */
          case 0x14 : read (Fh, &RWord, 2); read (Fh, &RWord, 2); read (Fh, &RByte, 1);                            /* (Pure Data) */
                      read (Fh, &RWord, 2); read (Fh, &RLong, 3); BlockLength = (RLong & 0x00FFFFFF); break;
        }
        if (BlockLength > (word)49152)
          BlockLength = (word)49152;
        if (BlockLength <= 6912)
          MaxOffset = 0;
        else
          MaxOffset = BlockLength - 6912;
        SizeUnknown = FALSE;
        if (read (Fh, _ScreenBuffer, BlockLength) == -1)                                     /* Read the entire block into memory */
        { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
          DestroyWindow (&TBWindow); return; }
        close (Fh);
      }
      else                                                                                 /* Snaps are handled a bit differently */
      {
        if (!GetMemoryImage (FullPath, AutoBank ((word)16384, ""), &ImageBuffer, &BlockLength))
          return;
        if ((FileType == FT_Z80 || FileType == FT_SLT) && Z80Version > 145)
        {
          if (!GetMemoryImage (FullPath, AutoBank ((word)32768, ""), &ImageBuffer, &BlockLength))      /* We want all banks there */
            return;
          if (!GetMemoryImage (FullPath, AutoBank ((word)49152, ""), &ImageBuffer, &BlockLength))
            return;
        }
        BlockLength = (word)49152;
        MaxOffset = (word)49152 - 6912;
        SizeUnknown = FALSE;
      }
    _TBY = 4;
    PutSpectrumScreen (_ScreenBuffer + _BlockOffset, TBWindow.StartX + 4, TBWindow.StartY + 4);
    ReleaseTimedHandler (_TBTimedFlash, &TBWindow);
    if (ScreenContainsFlash && DBConfig.EmulateFlash)
      RegisterTimedHandler (DBConfig.SlowFlash ? 960 : 320, _TBTimedFlash, &TBWindow);
    Done = FALSE;
    while (!Done)
    {
      switch (HandleWindow (&TBWindow, &ButtonP))
      {
        case WINH_RNOHAND : switch (ButtonP->ReactNum)
                            {
                              case 1 : SpectrumScreenToGIF (PCName, FALSE, TBWindow.StartX + 4, TBWindow.StartY + 4);
                                       break;                                                                       /* GIF button */
                              case 2 : if (TestScrShotDirExists ())                                                 /* SCR button */
                                         if (!TestFileOverwrite (ScrShotPath))
                                           _WriteScrShotFile (_ScreenBuffer + _BlockOffset);
                                       break;
                            }
                            break;
        case WINH_PNOHAND : switch (ButtonP->ReactNum)
                            {
                              case 3 : if (_BlockOffset > 0)                                                     /* DecOff button */
                                       {
                                         sprintf (Offset, "%d", _BlockOffset - 1);
                                         UpdateButtonText (&TBWindow, &PSetOff, BF_FITTXT);
                                         Done = TRUE;                                                                  /* Redraw! */
                                       }
                                       break;
                              case 5 : if (_BlockOffset < MaxOffset)                                             /* IncOff button */
                                       {
                                         sprintf (Offset, "%d", _BlockOffset + 1);
                                         UpdateButtonText (&TBWindow, &PSetOff, BF_FITTXT);
                                         Done = TRUE;                                                                  /* Redraw! */
                                       }
                                       break;
                            }
                            break;
        case WINH_EDITEND : if (ButtonP->Flags & BF_FITINPUT)                                               /* Has been changed ? */
                            {
                              COffset = atol (Offset);
                              if (COffset > MaxOffset)
                                _BlockOffset = MaxOffset;
                              else
                                _BlockOffset = COffset;
                              sprintf (Offset, "%d", _BlockOffset);
                              UpdateButtonText (&TBWindow, &PSetOff, BF_FITTXT);
                              Done = TRUE;                                                                             /* Redraw! */
                            }
                            break;
        case WINH_CLOSE   : Done = TRUE;
                            Redraw = FALSE;
      }
      if (GlobalKey == K_ESC)                                                                                  /* <ESC> stops too */
      {
        Done = TRUE;
        Redraw = FALSE;
      }
      MouseStatus ();
    }
  }
  DestroyWindow (&TBWindow);
  TestScrShotGhosted ();
}

void DisplayZ80Screendump (char *FullPath, char *FileName)

/**********************************************************************************************************************************/
/* Pre   : `FullPath' holds the path to the .SCR file to view, `FileName' is a copy to the filename itself.                       */
/* Post  : The selected Z80 screendump has been put in a seperate window, together with a 'Make GIF' button.                      */
/*         If this button has been pressed, a GIF image has been created. The user must explicitely use the close gadget.         */
/* Import: PutSpectrumScreen, SpectrumScreenToGIF.                                                                                */
/**********************************************************************************************************************************/

{
  char              WindowHeader[40];
  struct WindowInfo CWindow  = {185, 150, 264, 232, 264, 232, 264, 232, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, NULL};
  struct ButtonInfo POk      = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 100, 210, 10, DBLACK, DWHITE, NULL, 1, "Make @GIF",
                                NULL};
  struct ButtonInfo *ButtonP;
  char              GIFName[MAXDIR];
  bool              Done     = FALSE;
  int               Fh;

  CWindow.Header = WindowHeader;
  sprintf (WindowHeader, "Z80 screendump: [%s]", FileName);
  AddButton (&CWindow, &POk, TRUE);                                         /* Window MUST be created first, as the _ScreenBuffer */
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);                                     /* is global now and used to draw the window */
  SurroundScreen (&CWindow, 2, 2);
  memset (_ScreenBuffer, 0, 6144);                                                                                /* Display file */
  memset (_ScreenBuffer + 6144, 0x38, 768);                                                                    /* Attributes file */
  strcpy (GIFName, FullPath);
  strcpy (GIFName + strlen (GIFName) - 4, ".GIF");
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
  { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); DestroyWindow (&CWindow); return; }
  if (read (Fh, _ScreenBuffer, 6912) == -1)                                                  /* Read the entire block into memory */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
    DestroyWindow (&CWindow); return; }
  close (Fh);
  PutSpectrumScreen (_ScreenBuffer, CWindow.StartX + 4, CWindow.StartY + 4);
  while (!Done)
  {
    switch (HandleWindow (&CWindow, &ButtonP))
    {
      case WINH_RNOHAND : SpectrumScreenToGIF (GIFName, TRUE, CWindow.StartX + 4, CWindow.StartY + 4);              /* GIF Button */
                          break;
      case WINH_CLOSE   : Done = TRUE;
    }
    if (GlobalKey == K_ESC)                                                                                    /* <ESC> stops too */
      Done = TRUE;
    MouseStatus ();
  }
  DestroyWindow (&CWindow);
}

bool PutScreenFromTAP (struct WindowInfo *Window, short StartX, short StartY, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the window where the piture should go, `StartX', `StartY' hold the top-left edge and `FullPath'     */
/*         holds the full path to the .TAP file.                                                                                  */
/* Post  : The first (headerless) screen that was found in the tape file has been put on the screen.                              */
/*         If no screen was found, FALSE is returned.                                                                             */
/* Import: PutSpectrumScreen.                                                                                                     */
/**********************************************************************************************************************************/

{
  int        Fh;
  dword      FileLength;
  dword      Offset         = 0;
  word       SubOffset      = 0;
  word       NeededSize     = 6912;

  FileNotFound = TRUE;
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return (TRUE);                                                                                       /* Return: no new errors */
  FileNotFound = FALSE;
  FileLength = filelength (Fh);
  if (FileType == FT_TAP)                                                      /* (.TAP files include flag + parity in the count) */
    NeededSize += 2;
  while (SubOffset != NeededSize)                                    /* Search for 6912 (length of displaymemory) + Flag + Parity */
  {
    if (lseek (Fh, Offset, SEEK_SET) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return (TRUE); }
    if (read (Fh, &SubOffset, 2) == -1)                                                         /* Read length of following block */
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (TRUE); }
    if (SubOffset != NeededSize)                                                                                /* Not a screen ? */
    {
      Offset += SubOffset + 2;                                                                        /* Build up a new fileindex */
      if (Offset > FileLength)                                                                           /* End of file reached ? */
      { close (Fh); return (FALSE); }                                                                       /* Return the `error' */
    }
  }
  if (read (Fh, _ScreenBuffer, 1) == -1)                                                           /* Read and drop the flag byte */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (TRUE); }
  if (read (Fh, _ScreenBuffer, 6912) == -1)                                                  /* Read the entire block into memory */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (TRUE); }
  close (Fh);
  PutSpectrumScreen (_ScreenBuffer, Window->StartX + StartX, Window->StartY + StartY);
  return (TRUE);                                                                                         /* Return: screen dumped */
}

bool PutScreenFromTZX (struct WindowInfo *Window, short StartX, short StartY, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the window where the piture should go, `StartX', `StartY' hold the top-left edge and `FullPath'     */
/*         holds the full path to the .TZX file.                                                                                  */
/* Post  : The first (headerless) screen that was found in the tape file has been put on the screen.                              */
/*         If no screen was found, FALSE is returned.                                                                             */
/* Import: PutSpectrumScreen, _FindScreenInTZXFile.                                                                               */
/**********************************************************************************************************************************/

{
  int        Fh;

  FileNotFound = TRUE;
  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return (TRUE);                                                                                       /* Return: no new errors */
  FileNotFound = FALSE;
  if (!_FindScreenInTZXFile (Fh))
  { close (Fh); return (FALSE); }
  if (read (Fh, _ScreenBuffer, 6912) == -1)                                                  /* Read the entire block into memory */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return (TRUE); }
  close (Fh);
  PutSpectrumScreen (_ScreenBuffer, Window->StartX + StartX, Window->StartY + StartY);
  return (TRUE);                                                                                         /* Return: screen dumped */
}

void ViewTZXCIScreen (struct ComboInfo *Window, comboent EntryNumber, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the .TAP index window, `EntryNumber' holds the selected entry number, `FullPath' holds the full     */
/*         path to the .TZX file.                                                                                                 */
/* Post  : The selected Custom Info - Spectrum Screen block has been displayed in a Screen-style window.                          */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct WindowInfo  CWindow  = {185, 110, 264, 312, 264, 312, 264, 312, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                 WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "TZX Spectrum Screen Block"};
  struct WindowInfo  GWindow  = {224, 114, 180, 70, 180, 70, 180, 70, 1, WINB_IN, DBLACK, DWHITE, NULL, WINF_TWRAP, NULL, NULL};
  struct ButtonInfo  PScr     = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 25, 290, 10, DBLACK, DWHITE, NULL, 1, "Make @SCR",
                                 NULL};
  struct ButtonInfo  PGif     = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 100, 290, 10, DBLACK, DWHITE, NULL, 2, "Make @GIF",
                                 NULL};
  struct ButtonInfo  POk      = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 175, 290, 10, DBLACK, DWHITE, NULL, 3, "@Ok",
                                 NULL};
  struct ButtonInfo *ButtonP;
  struct ComboEntry  NEntry;
  char               GIFName[MAXDIR];
  char               GenDesc[256];
  char              *GWalk;
  bool               Done     = FALSE;
  FILE              *Fp;
  dword              BlockLength;
  dword              FileIndex;
  byte               DescLength;

  GetComboEntry (Window, &NEntry, EntryNumber);
  if ((Fp = fopen (FullPath, "rb")) == NULL)
  { ErrorRequester (-1, -1, "@Stop", NULL, NULL, "Error opening file %s", FullPath); return; }
  FileIndex = NEntry.Identifier + 17;
  if (fseek (Fp, FileIndex, SEEK_SET) == -1)                                                     /* Wind tape to correct position */
  { fclose (Fp); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
  if (fread (&BlockLength, 1, 4, Fp) == -1)                                                     /* Read length of following block */
  { fclose (Fp); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  if (fread (&DescLength, 1, 1, Fp) == -1)                                                          /* Read length of description */
  { fclose (Fp); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  if (DescLength == 0)
    strcpy (GenDesc, "Loading Screen");
  else
  {
    if (fread (GenDesc, 1, DescLength, Fp) == -1)                                                             /* Read description */
    { fclose (Fp); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
    GenDesc[DescLength] = '\0';
    for (GWalk = GenDesc ; *GWalk ; GWalk ++)
      if (*GWalk == 0x0D)
        *GWalk = 0x0A;
  }
  FileIndex += DescLength + 6;                                                                        /* (Skip the border colour) */
  if (fseek (Fp, FileIndex, SEEK_SET) == -1)                                                     /* Wind tape to correct position */
  { fclose (Fp); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
  BlockLength -= (DescLength + 2);
  if (BlockLength != 6912)
  { ErrorRequester (-1, -1, NULL, NULL, NULL, "Invalid Spectrum Screen block!"); return; }
  AddButton (&CWindow, &POk, TRUE);
  AddButton (&CWindow, &PScr, FALSE);                                       /* Window MUST be created first, as the _ScreenBuffer */
  AddButton (&CWindow, &PGif, FALSE);
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);                                     /* is global now and used to draw the window */
  DrawWindow (&GWindow, &CWindow, 0, TRUE);
  Wprintf (&GWindow, 0, 0, GenDesc);
  if (fread (_ScreenBuffer, 1, 6912, Fp) == -1)                                                           /* Read the screen data */
  { fclose (Fp); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno));
    DestroyWindow (&CWindow); return; }
  SurroundScreen (&CWindow, 2, 82);
  strcpy (GIFName, FullPath);
  strcpy (GIFName + strlen (GIFName) - 4, ".GIF");
  _BlockOffset = 0;
  _TBY = 84;
  PutSpectrumScreen (_ScreenBuffer, CWindow.StartX + 4, CWindow.StartY + _TBY);
  if (ScreenContainsFlash && DBConfig.EmulateFlash)
    RegisterTimedHandler (DBConfig.SlowFlash ? 960 : 320, _TBTimedFlash, &TBWindow);
  while (!Done)
  {
    switch (HandleWindow (&CWindow, &ButtonP))
    {
      case WINH_RNOHAND : switch (ButtonP->ReactNum)
                          {
                            case 1 :  _WriteScrShotFile (_ScreenBuffer); break;                                     /* (Make SCR) */
                            case 2 : SpectrumScreenToGIF (GIFName, TRUE, CWindow.StartX + 4, CWindow.StartY + _TBY);/* (Make GIF) */
                                     break;
                            case 3 : Done = TRUE; break;                                                                  /* (Ok) */
                          }
                          break;
      case WINH_CLOSE   : Done = TRUE;
    }
    if (GlobalKey == K_ESC)                                                                                    /* <ESC> stops too */
      Done = TRUE;
    MouseStatus ();
  }
  if (ScreenContainsFlash && DBConfig.EmulateFlash)
    ReleaseTimedHandler (_TBTimedFlash, &TBWindow);
  DestroyWindow (&GWindow);
  DestroyWindow (&CWindow);
  GlobalKey = 0x0000;
}

bool DecrunchZ80Block (byte *BufferIn, word BlLength, byte *BufferOut, word OutLength)

/**********************************************************************************************************************************/
/* Pre   : `BufferIn' points to the crunched spectrum block, `BlLength' holds the length, `BufferOut' points to the result.       */
/*         `OutLength' is the expected length after decrunching.                                                                  */
/*         The version of Z80 file must have been determined.                                                                     */
/* Post  : The block has been decrunched from `BufferIn' to `BufferOut'. The return value is TRUE if all went well.               */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  word register IndexIn;
  word register IndexOut       = 0;
  word register RunTimeLength;

  if (Z80Version == 145)
    BlLength -= 4;                                                                           /* Do not `decrunch' the end marker! */
  for (IndexIn = 0 ; IndexIn < BlLength ; IndexIn ++)                                       /* Decrunch it into the second buffer */
  {
    if (IndexOut >= OutLength)                                                                           /* Past maximum length ? */
      return (FALSE);                                                                           /* Then there's an error for sure */
    if (*(BufferIn + IndexIn) != 0xED)                                                                  /* Start of crunch code ? */
      *(BufferOut + (IndexOut ++)) = *(BufferIn + IndexIn);                                                      /* No, just copy */
    else
      if (*(BufferIn + IndexIn + 1) != 0xED)                                                                  /* Crunch confirm ? */
        *(BufferOut + (IndexOut ++)) = *(BufferIn + IndexIn);                                                    /* No, just copy */
      else                                                                                                      /* Yes, decrunch! */
      {
        RunTimeLength = *(BufferIn + IndexIn + 2);                                                             /* Run time length */
        while (RunTimeLength -- > 0 && IndexOut < OutLength)
          *(BufferOut + (IndexOut ++)) = *(BufferIn + IndexIn + 3);
        IndexIn += 3;
      }
  }
  if (Z80Version == 145)
  {
    if (IndexOut != OutLength)
      return (FALSE);
    if (*(BufferIn + IndexIn ++) != 0x00 || *(BufferIn + IndexIn ++) != 0xED ||
        *(BufferIn + IndexIn ++) != 0xED || *(BufferIn + IndexIn ++) != 0x00)
      return (FALSE);
    return (TRUE);
  }
  else
    return (IndexOut == OutLength);
}

void CrunchZ80Block (byte *BufferIn, word BlLength, byte *BufferOut, word *CrunchedLength)

/**********************************************************************************************************************************/
/* Pre   : `BufferIn' points to the uncrunched spectrum block, `BlLength' holds the length, `BufferOut' points to the result.     */
/*         `CrunchedLength' is the length after crunching.                                                                        */
/*         The version of Z80 file must have been determined.                                                                     */
/* Post  : The block has been crunched from `BufferIn' to `BufferOut'.                                                            */
/*         Z80 v3.05: If 16384 bytes (1 bank) must be crunched and the result is > 16383 bytes, the block is returned directly    */
/*         with `CrunchedLength' set to 0xFFFF.                                                                                   */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  word register IndexIn;
  word register IndexOut       = 0;
  word register RunTimeLength;
  bool          CrunchIt       = FALSE;
  byte          RepeatedCode;

  for (IndexIn = 0 ; IndexIn < BlLength - 4 ; IndexIn ++)                                     /* Crunch it into the second buffer */
  {
    if (*(BufferIn + IndexIn) == 0xED &&                                                                     /* Exceptional cases */
        *(BufferIn + IndexIn + 1) == 0xED)
      CrunchIt = TRUE;
    else if (*(BufferIn + IndexIn) == *(BufferIn + IndexIn + 1) &&
             *(BufferIn + IndexIn) == *(BufferIn + IndexIn + 2) &&
             *(BufferIn + IndexIn) == *(BufferIn + IndexIn + 3) &&
             *(BufferIn + IndexIn) == *(BufferIn + IndexIn + 4) &&                                        /* At least 5 repeats ? */
             IndexIn > 0 && *(BufferIn + IndexIn - 1) != 0xED)                                         /* Not right after a 0xED! */
      CrunchIt = TRUE;
    if (CrunchIt)                                                                                             /* Crunch a block ? */
    {
      CrunchIt = FALSE;
      RunTimeLength = 1;
      RepeatedCode = *(BufferIn + IndexIn);
      while (RunTimeLength < 255 && RunTimeLength + IndexIn < BlLength && *(BufferIn + RunTimeLength + IndexIn) == RepeatedCode)
        RunTimeLength ++;
      *(BufferOut + (IndexOut ++)) = 0xED;
      *(BufferOut + (IndexOut ++)) = 0xED;
      *(BufferOut + (IndexOut ++)) = RunTimeLength;
      *(BufferOut + (IndexOut ++)) = RepeatedCode;
      IndexIn += RunTimeLength - 1;
    }
    else
      *(BufferOut + (IndexOut ++)) = *(BufferIn + IndexIn);                                                      /* No, just copy */
  }
  while (IndexIn < BlLength)
    *(BufferOut + (IndexOut ++)) = *(BufferIn + (IndexIn ++));                                                  /* Copy last ones */
  if (Z80Version == 145)                                                                       /* Version 1.45 needs an endmarker */
  {
    *(BufferOut + (IndexOut ++)) = 0x00;
    *(BufferOut + (IndexOut ++)) = 0xED;
    *(BufferOut + (IndexOut ++)) = 0xED;
    *(BufferOut + (IndexOut ++)) = 0x00;
  }
  else if (Z80Version >= 300 && BlLength == 16384 && IndexOut > 16383)                   /* Compressed larger than uncompressed ? */
  {
    memcpy (BufferOut, BufferIn, BlLength);                                                 /* Return the inputblock uncompressed */
    IndexOut = 0xFFFF;                                                                                  /* Signal: not compressed */
  }
  *CrunchedLength = IndexOut;
}

bool PutScreenFromZ80 (struct WindowInfo *Window, short StartX, short StartY, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the window where the piture should go, `StartX', `StartY' hold the top-left edge and `FullPath'     */
/*         holds the full path to the .Z80 file. The version of the Z80 file must have been determined.                           */
/* Post  : The screen is decrunched from the Z80 file and put on the VGA display.                                                 */
/*         The return value is FALSE if any error occured.                                                                        */
/* Import: PutSpectrumScreen.                                                                                                     */
/**********************************************************************************************************************************/

{
  if (!_GetMemoryImageFromZ80 (FullPath, 0x05))                                                             /* (Screen is page 5) */
    return (FALSE);
  if (FileNotFound)
    return (FALSE);
  PutSpectrumScreen (_ScreenBuffer, Window->StartX + StartX, Window->StartY + StartY);
  return (TRUE);
}

void PutScreenFromSNA (struct WindowInfo *Window, short StartX, short StartY, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the window where the piture should go, `StartX', `StartY' hold the top-left edge and `FullPath'     */
/*         holds the full path to the .SNA file.                                                                                  */
/* Post  : The screen is taken the SNA file and put on the VGA display.                                                           */
/* Import: PutSpectrumScreen.                                                                                                     */
/**********************************************************************************************************************************/

{
  if (!_GetMemoryImageFromSNA (FullPath) || FileNotFound)
    return;
  PutSpectrumScreen (_ScreenBuffer, Window->StartX + StartX, Window->StartY + StartY);
  return;
}

void PutScreenFromSCR (struct WindowInfo *Window, short StartX, short StartY, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the window where the piture should go, `StartX', `StartY' hold the top-left edge and `FullPath'     */
/*         holds the full path to the .SCR file.                                                                                  */
/* Post  : The screen has been displayed.                                                                                         */
/* Import: PutSpectrumScreen.                                                                                                     */
/**********************************************************************************************************************************/

{
  int        Fh;
  dword      FileLength;
  word       NRead;

  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return;
  FileLength = filelength (Fh);
  if (FileLength != 6912)
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, "Invalid SCR file %s", FullPath); return; }
  if ((NRead = read (Fh, _ScreenBuffer, (word)6912)) == -1)
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  close (Fh);
  PutSpectrumScreen (_ScreenBuffer, Window->StartX + StartX, Window->StartY + StartY);
  return;
}

void PutScreenFromSLT (struct WindowInfo *Window, short StartX, short StartY, char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the window where the piture should go, `StartX', `StartY' hold the top-left edge and `FullPath'     */
/*         holds the full path to the .SCR file.                                                                                  */
/* Post  : The screen has been displayed.                                                                                         */
/* Import: PutSpectrumScreen, DecrunchZ80Block.                                                                                   */
/**********************************************************************************************************************************/

{
  int        Fh;
  word       NRead;

  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return;
  if (lseek (Fh, SLTMap.TableEnd + SLTMap.OffsetScrShot, SEEK_SET) == -1)              /* Move the pointer to the screenshot data */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
  if (SLTMap.HeaderScrShot.Length < 6912)                                                                  /* Block is crunched ? */
  {
    if ((NRead = read (Fh, &_ConvertBuffer, (word)SLTMap.HeaderScrShot.Length)) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
    if (!DecrunchZ80Block ((byte *)&_ConvertBuffer, NRead, _ScreenBuffer, 6912))                             /* Then decrunch it! */
    { ErrorRequester (-1, -1, "@Close", NULL, NULL, _InvalidZ80File, FullPath); return; }
  }
  else
    if ((NRead = read (Fh, _ScreenBuffer, (word)SLTMap.HeaderScrShot.Length)) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  close (Fh);
  PutSpectrumScreen (_ScreenBuffer, Window->StartX + StartX, Window->StartY + StartY);
  return;
}

void CreateScreenFromZ80 (char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `FullPath' holds the full path to the .Z80 file.                                                                       */
/* Post  : The screen is decrunched from the Z80 file and written as screenshot file.                                             */
/* Import: _WriteScrShotFile.                                                                                                     */
/**********************************************************************************************************************************/

{
  if (!_GetMemoryImageFromZ80 (FullPath, 0x05) || FileNotFound)                                             /* (Screen is page 5) */
    return;
  _WriteScrShotFile (_ScreenBuffer);
  return;
}

void CreateScreenFromTAP (char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `FullPath' holds the full path to the .TAP file.                                                                       */
/* Post  : The first (headerless) screen that was found in the tape file has been written as screenshot file.                     */
/* Import: _WriteScrShotFile.                                                                                                     */
/**********************************************************************************************************************************/

{
  int        Fh;
  dword      FileLength;
  dword      Offset         = 0;
  word       SubOffset      = 0;
  word       NeededSize     = 6912;

  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return;                                                                                              /* Return: no new errors */
  FileLength = filelength (Fh);
  if (FileType == FT_TAP)
    NeededSize += 2;
  while (SubOffset != NeededSize)                                    /* Search for 6912 (length of displaymemory) + Flag + Parity */
  {
    if (lseek (Fh, Offset, SEEK_SET) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
    if (read (Fh, &SubOffset, 2) == -1)                                                         /* Read length of following block */
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
    if (SubOffset != NeededSize)                                                                                /* Not a screen ? */
    {
      Offset += SubOffset + 2;                                                                        /* Build up a new fileindex */
      if (Offset > FileLength)                                                                           /* End of file reached ? */
      { close (Fh); return; }                                                                               /* Return the `error' */
    }
  }
  if (read (Fh, &SubOffset, 1) == -1)                                                                       /* Drop the flag byte */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  if (read (Fh, _ScreenBuffer, 6912) == -1)                                                  /* Read the entire block into memory */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  close (Fh);
  _WriteScrShotFile (_ScreenBuffer);
}

void CreateScreenFromTZX (char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `FullPath' holds the full path to the .TAP file.                                                                       */
/* Post  : The first (headerless) screen that was found in the tape file has been written as screenshot file.                     */
/* Import: _WriteScrShotFile.                                                                                                     */
/**********************************************************************************************************************************/

{
  int        Fh;

  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return;                                                                                              /* Return: no new errors */
  if (!_FindScreenInTZXFile (Fh))
  { close (Fh); return; }
  if (read (Fh, _ScreenBuffer, 6912) == -1)                                                  /* Read the entire block into memory */
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  close (Fh);
  _WriteScrShotFile (_ScreenBuffer);
}

void CreateScreenFromSNA (char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `FullPath' holds the full path to the .SNA file.                                                                       */
/* Post  : The screen is taken the SNA file and written as screenshot file.                                                       */
/* Import: _WriteScrShotFile.                                                                                                     */
/**********************************************************************************************************************************/

{
  if (!_GetMemoryImageFromSNA (FullPath) || FileNotFound)
    return;
  _WriteScrShotFile (_ScreenBuffer);
  return;
}

void CreateScreenFromSLT (char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : `FullPath' holds the full path to the .SLT file.                                                                       */
/* Post  : The screenshot is taken from the SLT file and written as a seperate screenshot file.                                   */
/* Import: _WriteScrShotFile, DecrunchZ80Block.                                                                                   */
/**********************************************************************************************************************************/

{
  int        Fh;
  word       NRead;

  if ((Fh = open (FullPath, O_BINARY | O_RDONLY)) == -1)
    return;                                                                                              /* Return: no new errors */
  if (lseek (Fh, SLTMap.TableEnd + SLTMap.OffsetScrShot, SEEK_SET) == -1)
  { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileSearchError, FullPath, strerror (errno)); return; }
  if (SLTMap.HeaderScrShot.Length < 6912)                                                                  /* Block is crunched ? */
  {
    if ((NRead = read (Fh, &_ConvertBuffer, (word)SLTMap.HeaderScrShot.Length)) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
    if (!DecrunchZ80Block ((byte *)&_ConvertBuffer, NRead, _ScreenBuffer, 6912))                             /* Then decrunch it! */
    { ErrorRequester (-1, -1, "@Close", NULL, NULL, _InvalidZ80File, FullPath); return; }
  }
  else
    if ((NRead = read (Fh, _ScreenBuffer, (word)SLTMap.HeaderScrShot.Length)) == -1)
    { close (Fh); ErrorRequester (-1, -1, "@Close", NULL, NULL, _FileReadError, FullPath, strerror (errno)); return; }
  close (Fh);
  _WriteScrShotFile (_ScreenBuffer);
}

bool GetMemoryImage (char *FullPath, byte Bank, byte **Image, word *ImageSize)

/**********************************************************************************************************************************/
/* Pre   : `FullPath' holds the full path to the snapshot file, `Bank' is the requested spectrum page (0-7).                      */
/* Post  : The requested page has been loaded into memory and decrunched. A pointer to byte 16384 has been returned in `Image'    */
/*         and the length of the read block in `ImageSize'. TRUE is returned if all went well.                                    */
/* Import: _GetMemoryImageFromZ80, _GetMemoryImageFromSNA.                                                                        */
/**********************************************************************************************************************************/

{
  switch (FileType)
  {
    case FT_SLT :
    case FT_Z80 : if (strcmp (FullPath, _ResidentFile) ||                                                     /* Different file ? */
                      _ResidentBank != Bank)                                                                   /* Already there ? */
                  {
                    strcpy (_ResidentFile, FullPath);
                    if (!_GetMemoryImageFromZ80 (FullPath, Bank) || FileNotFound)
                      return (FALSE);
                  }
                  *Image = _ScreenBuffer;
                  *ImageSize = _ResidentSize;
                  return (TRUE);
    case FT_SNA :
    case FT_SNP :
    case FT_SEM :
    case FT_SP  : if (strcmp (FullPath, _ResidentFile))                                                       /* Different file ? */
                  {
                    strcpy (_ResidentFile, FullPath);
                    if (!_GetMemoryImageFromSNA (FullPath) || FileNotFound)
                      return (FALSE);
                  }
                  *Image = _ScreenBuffer;
                  *ImageSize = _ResidentSize;
                  return (TRUE);
    default     : return (FALSE);
  }
}

void AllocateScreenBuffer (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The _ScreenBuffer is allocated.                                                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  if ((_ScreenBuffer = (byte *)malloc ((word)49152)) == NULL)
    FatalError (1, _ErrOutOfMemory, (word)49152);

  _BufferCrunched = (byte *)&_ConvertBuffer;                                                                 /* Hack for DBPOKES */
  _BufferDecrunched = (byte *)_ScreenBuffer;
}
