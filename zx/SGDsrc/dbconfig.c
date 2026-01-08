/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBCONFIG.C                                                                                                      */
/* Description  : Configuration handler                                                                                           */
/* Version type : Program module                                                                                                  */
/* Last changed : 15-04-2001  10:20                                                                                               */
/* Update count : 193                                                                                                             */
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

#define __DBCONFIG_MAIN__

#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include "wbench32/wbench32.h"
#include "dbdbase.h"
#include "dbmenu.h"
#include "dbmain.h"
#include "dbconfig.h"

#pragma pack                ()

#include "speclogo.inc"

#define REMAP               "RemapBrightBlack"
#define DOFLASH             "EmulateFlash"
#define SLOWFLASH           "SlowFlash"
#define CONFQUIT            "ConfirmQuit"
#define CONFDEL             "ConfirmDelete"
#define CONFOVERW           "ConfirmOverwrite"
#define SYNCSLID            "FollowSlider"
#define EDITINS             "EditInsert"
#define BACKUPDB            "BackupDatabase"
#define VALIDATE            "ValidateAfterLoad"
#define ALWAYSCHEAT         "AlwaysCheat"
#define AUTOLOADTAP         "AutoLoadTapes"
#define BESTSCREEN          "PickBestScreen"
#define ALWSELECT           "AlwaysSelectGame"
#define STARTBUTTON         "StartInfoButton"
#define NOSOUNDS            "DisableSounds"
#define FORCESTICK          "ForceSnapJoystick"
#define EMULTYPE            "EmulatorType"
#define EMULPATH0           "EmulatorPathZ80"
#define EMULPATH1           "EmulatorPathJPP"
#define EMULPATH2           "EmulatorPathWAR"
#define EMULPATH3           "EmulatorPathX128"
#define EMULPATH4           "EmulatorPathWSPECEM"
#define EMULPATH5           "EmulatorPathZX"
#define EMULPATH6           "EmulatorPathSPANISH"
#define EMULPATH7           "EmulatorPathZX32"
#define EMULPATH8           "EmulatorPathR80"
#define EMULPATH9           "EmulatorPathRUSSIAN"
#define EMULPATH10          "EmulatorPathYUDIN"
#define EMULPATH11          "EmulatorPathRSPEC"
#define EMULOPTS0           "EmulatorOptionsZ80"
#define EMULOPTS1           "EmulatorOptionsJPP"
#define EMULOPTS2           "EmulatorOptionsWAR"
#define EMULOPTS3           "EmulatorOptionsX128"
#define EMULOPTS4           "EmulatorOptionsWSPECEM"
#define EMULOPTS5           "EmulatorOptionsZX"
#define EMULOPTS6           "EmulatorOptionsSPANISH"
#define EMULOPTS7           "EmulatorOptionsZX32"
#define EMULOPTS8           "EmulatorOptionsR80"
#define EMULOPTS9           "EmulatorOptionsRUSSIAN"
#define EMULOPTS10          "EmulatorOptionsYUDIN"
#define EMULOPTS11          "EmulatorOptionsRSPEC"
#define LINKEXT1            "LinkExtensionTAP"
#define LINKEXT2            "LinkExtensionZ80"
#define LINKEXT3            "LinkExtensionSNA"
#define LINKEXT4            "LinkExtensionMDR"
#define LINKEXT5            "LinkExtensionVOC"
#define LINKEXT6            "LinkExtensionSLT"
#define LINKEXT7            "LinkExtensionSNP"
#define LINKEXT8            "LinkExtensionSEM"
#define LINKEXT9            "LinkExtensionLTP"
#define LINKEXT10           "LinkExtensionTZX"
#define LINKEXT11           "LinkExtensionSP"
#define LINKEXT12           "LinkExtensionTRD"
#define LINKEXT13           "LinkExtensionDSK"
#define LINKEXT14           "LinkExtensionFDI"
#define EDITOR              "Editor"
#define ZXEDITOR            "ZX-Editor"
#define GIFVIEWER           "GIFViewerPath"
#define GIFOPTIONS          "GIFViewerOptions"
#define JPGVIEWER           "JPGViewerPath"
#define JPGOPTIONS          "JPGViewerOptions"
#define ALWZXEDIT           "AlwaysUseZX-Editor"
#define PRINTR              "Printer"
#define LNPPG               "LinesPerPage"
#define PFIELDS             "PrintedFields"
#define BLANKTO             "BlankerTimeOut"
#define DRAGOUTLN           "DragOutlineOnly"
#define RESZOUTLN           "ResizeOutlineOnly"
#define GREYGHOST           "GreyOutGhosting"
#define STABLEFLOP          "StableFloppies"
#define PRELOADRES          "PreLoadResources"
#define DISABLERIL          "DirectVideoControl"
#define SWAPMODE80          "SwapInTextMode"
#define QUITMODE80          "QuitInTextMode"
#define NEXTDISKID          "NextFloppyDiskId"
#define GMOUSE              "UseGeniusMouseHiding"
#define SWAPBUTT            "SwapMouseButtons"
#define EMUL3BUTT           "Emulate3rdMouseButton"
#define NOLOCHIDE           "NoLocalMouseHiding"
#define FORCEMOUSE          "ForceMouseRelease"
#define DCLICKDELAY         "DoubleClickInterval"
#define MOUSESPEED          "MouseSpeedSetting"
#define CPUSHDELAY          "InitialContPushDelay"
#define SNAPMOUSE           "SnapMouseDefault"
#define ADDTAPE             "Z80SnapNeedsTape"
#define ADDVOC              "Z80SnapNeedsVOC"
#define GIFPATH             "GIFPath"
#define COMPRESSOR          "Compressor"
#define COMPRESSOPT         "CompressOptions"
#define ONLYZ80TAP          "Z80TapesOnly"
#define VALEMULCHN          "ValidateAtEmulChange"
#define SORTPHONE           "PhonebookSorting"
#define NOSUBDIRS           "AllFilesInOneDir"
#define CUSTOMPAL           "UseCustomPalette"
#define CUSTOMCOL00         "CustomColor00"
#define CUSTOMCOL01         "CustomColor01"
#define CUSTOMCOL02         "CustomColor02"
#define CUSTOMCOL03         "CustomColor03"
#define CUSTOMCOL04         "CustomColor04"
#define CUSTOMCOL05         "CustomColor05"
#define CUSTOMCOL06         "CustomColor06"
#define CUSTOMCOL07         "CustomColor07"
#define CUSTOMCOL08         "CustomColor08"
#define CUSTOMCOL09         "CustomColor09"
#define CUSTOMCOL10         "CustomColor10"
#define CUSTOMCOL11         "CustomColor11"
#define CUSTOMCOL12         "CustomColor12"
#define CUSTOMCOL13         "CustomColor13"
#define CUSTOMCOL14         "CustomColor14"
#define CUSTOMCOL15         "CustomColor15"
#define LOCMASK             "GameDir"
#define ADDLANGUAGE         "AddLanguage"

/**********************************************************************************************************************************/
/* Define the static variables                                                                                                    */
/**********************************************************************************************************************************/

static struct _ConfOp_s                                                                 /* The commands available in a CONFIGFILE */
{
  char     *_OptionName;
  enum
  {
    OPT_BOOL,
    OPT_WORD,
    OPT_BYTE,
    OPT_SBYTE,
    OPT_DWORD,
    OPT_PATH,
    OPT_RPATH,
    OPT_TEXT,
    OPT_MASK,
    OPT_LANG,
  }         _OptionType;
  void     *_DataPtr;
};

static struct _ConfOp_s   _ConfOp[] = { { REMAP,       OPT_BOOL,  &DBConfig.RemapBrightBlack      },
                                        { DOFLASH,     OPT_BOOL,  &DBConfig.EmulateFlash          },
                                        { SLOWFLASH,   OPT_BOOL,  &DBConfig.SlowFlash             },
                                        { CONFQUIT,    OPT_BOOL,  &DBConfig.ConfirmQuit           },
                                        { CONFDEL,     OPT_BOOL,  &DBConfig.ConfirmDelete         },
                                        { CONFOVERW,   OPT_BOOL,  &DBConfig.ConfirmOverwrite      },
                                        { SYNCSLID,    OPT_BOOL,  &SystemDefaults.SyncSlider      },
                                        { EDITINS,     OPT_BOOL,  &SystemDefaults.EditInsert      },
                                        { BACKUPDB,    OPT_BOOL,  &DBConfig.AlwaysBackupDatabase  },
                                        { VALIDATE,    OPT_BOOL,  &DBConfig.ValidateAfterLoad     },
                                        { ALWAYSCHEAT, OPT_BOOL,  &DBConfig.AlwaysCheat           },
                                        { BESTSCREEN,  OPT_BOOL,  &DBConfig.PickBestScreen        },
                                        { ALWSELECT,   OPT_BOOL,  &DBConfig.AlwaysSelectGame      },
                                        { AUTOLOADTAP, OPT_BOOL,  &DBConfig.AutoLoadTapes         },
                                        { STARTBUTTON, OPT_BYTE,  &DBConfig.StartInfoButton       },
                                        { NOSOUNDS,    OPT_BOOL,  &SystemDefaults.Quiet           },
                                        { FORCESTICK,  OPT_BOOL,  &DBConfig.ForceSnapJoystick     },
                                        { EMULTYPE,    OPT_BYTE,  &DBConfig.EmulType              },
                                        { EMULPATH0,   OPT_PATH,   DBConfig.EmulPath[0]           },
                                        { EMULPATH1,   OPT_PATH,   DBConfig.EmulPath[1]           },
                                        { EMULPATH2,   OPT_PATH,   DBConfig.EmulPath[2]           },
                                        { EMULPATH3,   OPT_PATH,   DBConfig.EmulPath[3]           },
                                        { EMULPATH4,   OPT_PATH,   DBConfig.EmulPath[4]           },
                                        { EMULPATH5,   OPT_PATH,   DBConfig.EmulPath[5]           },
                                        { EMULPATH6,   OPT_PATH,   DBConfig.EmulPath[6]           },
                                        { EMULPATH7,   OPT_PATH,   DBConfig.EmulPath[7]           },
                                        { EMULPATH8,   OPT_PATH,   DBConfig.EmulPath[8]           },
                                        { EMULPATH9,   OPT_PATH,   DBConfig.EmulPath[9]           },
                                        { EMULPATH10,  OPT_PATH,   DBConfig.EmulPath[10]          },
                                        { EMULPATH11,  OPT_PATH,   DBConfig.EmulPath[11]          },
                                        { EMULOPTS0,   OPT_TEXT,   DBConfig.ExtraOpt[0]           },
                                        { EMULOPTS1,   OPT_TEXT,   DBConfig.ExtraOpt[1]           },
                                        { EMULOPTS2,   OPT_TEXT,   DBConfig.ExtraOpt[2]           },
                                        { EMULOPTS3,   OPT_TEXT,   DBConfig.ExtraOpt[3]           },
                                        { EMULOPTS4,   OPT_TEXT,   DBConfig.ExtraOpt[4]           },
                                        { EMULOPTS5,   OPT_TEXT,   DBConfig.ExtraOpt[5]           },
                                        { EMULOPTS6,   OPT_TEXT,   DBConfig.ExtraOpt[6]           },
                                        { EMULOPTS7,   OPT_TEXT,   DBConfig.ExtraOpt[7]           },
                                        { EMULOPTS8,   OPT_TEXT,   DBConfig.ExtraOpt[8]           },
                                        { EMULOPTS9,   OPT_TEXT,   DBConfig.ExtraOpt[9]           },
                                        { EMULOPTS10,  OPT_TEXT,   DBConfig.ExtraOpt[10]          },
                                        { EMULOPTS11,  OPT_TEXT,   DBConfig.ExtraOpt[11]          },
                                        { LINKEXT1,    OPT_SBYTE, &DBConfig.LinkExtension[0]      },
                                        { LINKEXT2,    OPT_SBYTE, &DBConfig.LinkExtension[1]      },
                                        { LINKEXT3,    OPT_SBYTE, &DBConfig.LinkExtension[2]      },
                                        { LINKEXT4,    OPT_SBYTE, &DBConfig.LinkExtension[3]      },
                                        { LINKEXT5,    OPT_SBYTE, &DBConfig.LinkExtension[4]      },
                                        { LINKEXT6,    OPT_SBYTE, &DBConfig.LinkExtension[5]      },
                                        { LINKEXT7,    OPT_SBYTE, &DBConfig.LinkExtension[6]      },
                                        { LINKEXT8,    OPT_SBYTE, &DBConfig.LinkExtension[7]      },
                                        { LINKEXT9,    OPT_SBYTE, &DBConfig.LinkExtension[8]      },
                                        { LINKEXT10,   OPT_SBYTE, &DBConfig.LinkExtension[9]      },
                                        { LINKEXT11,   OPT_SBYTE, &DBConfig.LinkExtension[10]     },
                                        { LINKEXT12,   OPT_SBYTE, &DBConfig.LinkExtension[11]     },
                                        { LINKEXT13,   OPT_SBYTE, &DBConfig.LinkExtension[12]     },
                                        { LINKEXT14,   OPT_SBYTE, &DBConfig.LinkExtension[13]     },
                                        { EDITOR,      OPT_PATH,   DBConfig.EditPath              },
                                        { ZXEDITOR,    OPT_PATH,   DBConfig.ZXEditPath            },
                                        { GIFVIEWER,   OPT_PATH,   DBConfig.GIFViewerPath         },
                                        { JPGVIEWER,   OPT_PATH,   DBConfig.JPGViewerPath         },
                                        { GIFOPTIONS,  OPT_TEXT,   DBConfig.GIFViewerOptions      },
                                        { JPGOPTIONS,  OPT_TEXT,   DBConfig.JPGViewerOptions      },
                                        { ALWZXEDIT,   OPT_BOOL,  &DBConfig.AlwaysUseZXEdit       },
                                        { PRINTR,      OPT_BYTE,  &DBConfig.Printer               },
                                        { LNPPG,       OPT_BYTE,  &DBConfig.LinesPerPage          },
                                        { PFIELDS,     OPT_TEXT,   DBConfig.FieldsString          },
                                        { BLANKTO,     OPT_WORD,  &DBConfig.BlankerTimeOut        },
                                        { DRAGOUTLN,   OPT_BOOL,  &SystemDefaults.DragOutline     },
                                        { RESZOUTLN,   OPT_BOOL,  &SystemDefaults.ResizeOutline   },
                                        { GREYGHOST,   OPT_BOOL,  &SystemDefaults.GreyOutGhosting },
                                        { STABLEFLOP,  OPT_BOOL,  &DBConfig.AssumeStableFloppies  },
                                        { PRELOADRES,  OPT_BOOL,  &DBConfig.PreLoadResources      },
                                        { DISABLERIL,  OPT_BOOL,  &DBConfig.DirectVideoControl    },
                                        { SWAPMODE80,  OPT_BOOL,  &SystemDefaults.ForceSwapMode80 },
                                        { QUITMODE80,  OPT_BOOL,  &SystemDefaults.ForceQuitMode80 },
                                        { NEXTDISKID,  OPT_WORD,  &NextFloppyId                   },
                                        { GMOUSE,      OPT_BOOL,  &Mouse.UseGeniusScreenUpdate    },
                                        { NOLOCHIDE,   OPT_BOOL,  &Mouse.DisableLocalHiding       },
                                        { FORCEMOUSE,  OPT_BOOL,  &Mouse.ForceMouseRelease        },
                                        { SWAPBUTT,    OPT_BOOL,  &Mouse.SwapLeftAndRightButton   },
                                        { EMUL3BUTT,   OPT_BOOL,  &Mouse.EmulateMiddleMouseButton },
                                        { SNAPMOUSE,   OPT_BOOL,  &Mouse.SnapToDefaultButton      },
                                        { DCLICKDELAY, OPT_WORD,  &Mouse.DoubleClickInterval      },
                                        { MOUSESPEED,  OPT_TEXT,   DBConfig.MouseSpeedString      },
                                        { CPUSHDELAY,  OPT_WORD,  &Mouse.CPushDelay               },
                                        { ADDTAPE,     OPT_TEXT,   DBConfig.SnapNeedsTAP          },
                                        { ADDVOC,      OPT_TEXT,   DBConfig.SnapNeedsVOC          },
                                        { GIFPATH,     OPT_RPATH,  DBConfig.GIFPath               },
                                        { COMPRESSOR,  OPT_PATH,   DBConfig.Compressor            },
                                        { COMPRESSOPT, OPT_TEXT,   DBConfig.CompressOptions       },
                                        { ONLYZ80TAP,  OPT_BOOL,  &DBConfig.OnlyZ80Tapes          },
                                        { VALEMULCHN,  OPT_BOOL,  &DBConfig.ValidateAtEmulChange  },
                                        { SORTPHONE,   OPT_BOOL,  &DBConfig.UsePhonebookSorting   },
                                        { NOSUBDIRS,   OPT_BOOL,  &DBConfig.AllFilesInOneDir      },
                                        { CUSTOMPAL,   OPT_BOOL,  &DBConfig.UseCustomPalette      },
                                        { CUSTOMCOL00, OPT_DWORD, &DBConfig.CustomColor00         },
                                        { CUSTOMCOL01, OPT_DWORD, &DBConfig.CustomColor01         },
                                        { CUSTOMCOL02, OPT_DWORD, &DBConfig.CustomColor02         },
                                        { CUSTOMCOL03, OPT_DWORD, &DBConfig.CustomColor03         },
                                        { CUSTOMCOL04, OPT_DWORD, &DBConfig.CustomColor04         },
                                        { CUSTOMCOL05, OPT_DWORD, &DBConfig.CustomColor05         },
                                        { CUSTOMCOL06, OPT_DWORD, &DBConfig.CustomColor06         },
                                        { CUSTOMCOL07, OPT_DWORD, &DBConfig.CustomColor07         },
                                        { CUSTOMCOL08, OPT_DWORD, &DBConfig.CustomColor08         },
                                        { CUSTOMCOL09, OPT_DWORD, &DBConfig.CustomColor09         },
                                        { CUSTOMCOL10, OPT_DWORD, &DBConfig.CustomColor10         },
                                        { CUSTOMCOL11, OPT_DWORD, &DBConfig.CustomColor11         },
                                        { CUSTOMCOL12, OPT_DWORD, &DBConfig.CustomColor12         },
                                        { CUSTOMCOL13, OPT_DWORD, &DBConfig.CustomColor13         },
                                        { CUSTOMCOL14, OPT_DWORD, &DBConfig.CustomColor14         },
                                        { CUSTOMCOL15, OPT_DWORD, &DBConfig.CustomColor15         },
                                        { ADDLANGUAGE, OPT_LANG,   NULL                           },
                                        { LOCMASK,     OPT_MASK,   NULL                           } };

#define NumOps              (sizeof (_ConfOp) / sizeof (struct _ConfOp_s))

/**********************************************************************************************************************************/
/* Define the static routines                                                                                                     */
/**********************************************************************************************************************************/

static int        _NextString     (FILE *Fp, char *String, bool FindEQual, bool SkipEOL);
static void       _GetNewFileName (char *Operation, char *OldName);

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE INTERNAL GRAPHICS INTERFACE LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

static int _NextString (FILE *Fp, char *String, bool FindEqual, bool SkipEOL)

/**********************************************************************************************************************************/
/* Pre   : 'Fp' points to the open CONFIGFILE, 'String' holds the address of the string to be read, 'FindEqual' is TRUE if the    */
/*         first character to be found must be a '=' sign, 'SkipEOL' is TRUE if '\n' characters may be skipped.                   */
/* Post  : Any string is read. If it started with a '#' (COMMENT character), then the rest of the line has been ignored. The      */
/*         function does not return before a string was read WITHOUT this character or EOF has been reached. The result of the    */
/*         function is the length of the read string, or 0 if EOF was encountered.                                                */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  char          Ch;
  char register Cnt;
  bool          Ready = FALSE;
  bool          SkipSpaces;
  bool          SkipInitialSpaces;

  String[0] = '\0';
  while (!Ready)                                                                            /* Read until a valid string is found */
  {
    SkipSpaces = TRUE;
    while (SkipSpaces)
    {
      fscanf (Fp, "%c", &Ch);                                                                 /* Read until no white-spaces found */
      if (feof (Fp))
      {
        String[0] = '\0';                                                                                         /* Or until EOF */
        return (0);
      }
      SkipSpaces = (isspace (Ch) || Ch == -1);                         /* (Overcome bug in v1.31 - default string -1 i.s.o. '\0') */
      if (Ch == '\n' && !SkipEOL)                                               /* EOL character found which may not be skipped ? */
      {
        String[0] = '\0';                                                                        /* Then return the EOL character */
        return (0);
      }                                                                               /* (Otherwise it is a whitespace character) */
    }
    if (Ch == '#')                                                                  /* First character is the COMMENT character ? */
    {
      do
      {
        fscanf (Fp, "%c", &Ch);                                                                   /* Ignore until end of the line */
        if (feof (Fp))
        {
          String[0] = '\0';                                                                                       /* Or until EOF */
          return (0);
        }
      }
      while (Ch != '\n');
      if (!SkipEOL)                                                                            /* Do not skip the EOL character ? */
      {
        String[0] = '\0';                                                                        /* Then return the EOL character */
        return (0);
      }
    }
    else
      Ready = TRUE;
  }
  Cnt = 0;
  Ready = FALSE;
  if (FindEqual)
    return (Ch == '=');
  while (!Ready)
  {
    while (!isspace (Ch) && Ch != '"')                                                             /* Trap quoted argument(part)s */
    {
      String[Cnt ++] = Ch;
      fscanf (Fp, "%c", &Ch);                                                                     /* Read until first white-space */
      if (feof (Fp))
      {
        String[Cnt] = '\0';                                                                                       /* Or until EOF */
        return (Cnt);
      }
    }
    if (Ch == '"')                                                                                          /* Handle quoted part */
    {
      SkipInitialSpaces = TRUE;
      do
      {
        fscanf (Fp, "%c", &Ch);
        if (feof (Fp))
          FatalError (11, "Config: Unexpected end of configuration file");
        if (Ch == '\n')
          FatalError (11, "Config: Unexpected end of line in configuration file");
        if (SkipInitialSpaces)
          if (!isspace (Ch) && Ch != -1)                               /* (Overcome bug in v1.31 - default string -1 i.s.o. '\0') */
          {
            SkipInitialSpaces = FALSE;
            String[Cnt ++] = Ch;
          }
          else                                                                                             /* (Skip the culprit!) */
            ;
        else
          String[Cnt ++] = Ch;
      }
      while (Ch != '"');
      Cnt --;
      fscanf (Fp, "%c", &Ch);                                                                                 /* Read first after */
      if (feof (Fp))
      {
        String[Cnt] = '\0';                                                                                       /* Or until EOF */
        return (Cnt);
      }
    }
    else
      Ready = TRUE;
  }
  String[Cnt] = '\0';
  return (Cnt);
}

static void _GetNewFileName (char *Operation, char *OldName)

/**********************************************************************************************************************************/
/* Pre   : `Operation' is a string for the file requester header, `OldName' is the current path name.                             */
/* Post  : A new filename has been asked with a FileRequester, which is returned in `OldName'.                                    */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  char           Path[MAXDIR + 1];
  char           File[13];
  char           Mask[13] = "*.*";
  short register Cnt;

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
  if (FileRequester (180, 50, 48, 29, 20, Path, File, Mask, Operation, TRUE, FALSE))
    MakeFullPath (OldName, Path, File);
  if (MaxFloppyIdChanged)
    WriteConfig ();
}

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE EXPORTED LIBRARY ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

void ReadConfig (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : If the configuration file is found, then it has been read out. Otherwise, one is created.                              */
/*         If one (or all) are not found, then they are initialized with the defaults.                                            */
/*         If a character '#' is encountered, then the rest of this line is ignored (comment).                                    */
/* Import: _NextString.                                                                                                           */
/**********************************************************************************************************************************/

{
  FILE         *Fp;
  char          Item[256];
  char         *String;
  char          S[256];
  bool          Handled;
  byte register CntOp;
  byte register N;
  int           Len;
  bool          PathError;
  bool          Finished = FALSE;
  byte          Val;
  byte          ValNo;
  char         *Ch;
  short         HorizontalMickeys;
  short         VerticalMickeys;
  short         DoubleSpeedThreshold;

  
  if (IsFloppy (ProgramDrive[0]))
    RequestFloppy (ProgramDrive[0], ResourceFloppyId);
  DBConfig.RemapBrightBlack = TRUE;
  DBConfig.EmulateFlash = TRUE;
  DBConfig.SlowFlash = FALSE;
  DBConfig.ConfirmQuit = TRUE;
  DBConfig.ConfirmDelete = TRUE;
  DBConfig.ConfirmOverwrite = TRUE;
  SystemDefaults.EditInsert = FALSE;
  DBConfig.AlwaysBackupDatabase = TRUE;
  DBConfig.ValidateAfterLoad = FALSE;
  DBConfig.AlwaysCheat = FALSE;
  DBConfig.PickBestScreen = FALSE;
  DBConfig.AutoLoadTapes = FALSE;
  DBConfig.StartInfoButton = 0;
  SystemDefaults.Quiet = FALSE;
  DBConfig.EmulType = ET_Z80;
  for (N = 0 ; N < MAXETYPE ; N ++)
    DBConfig.EmulPath[N][0] = DBConfig.ExtraOpt[N][0] = '\0';
  for (N = 0 ; N < MAXFTYPE ; N ++)
    DBConfig.LinkExtension[N] = ET_NONE;
  DBConfig.EditPath[0] = DBConfig.ZXEditPath[0] = DBConfig.GIFPath[0] = DBConfig.SnapNeedsTAP[0] = DBConfig.SnapNeedsVOC[0] =
  DBConfig.GIFViewerPath[0] = DBConfig.GIFViewerOptions[0] = DBConfig.JPGViewerPath[0] = DBConfig.JPGViewerOptions[0] = '\0';
  DBConfig.ForceSnapJoystick = FALSE;
  DBConfig.Printer = 1;
  DBConfig.LinesPerPage = 64;
  strcpy (DBConfig.FieldsString, "ABCDEFGHJKLI");                                       /* (The fields as of SGD v1.25 and below) */
  DBConfig.AlwaysUseZXEdit = FALSE;
  DBConfig.BlankerOn = TRUE;
  DBConfig.BlankerTimeOut = 600;
  DBConfig.AssumeStableFloppies = TRUE;
  DBConfig.DirectVideoControl = TRUE;
  DBConfig.PreLoadResources = FALSE;
  NextFloppyId = 1;
  DBConfig.OnlyZ80Tapes = TRUE;
  DBConfig.ValidateAtEmulChange = FALSE;
  DBConfig.AlwaysSelectGame = TRUE;
  DBConfig.UsePhonebookSorting = TRUE;
  DBConfig.AllFilesInOneDir = FALSE;
  DBConfig.Compressor[0] = '\0';
  DBConfig.CompressOptions[0] = '\0';
  DBConfig.UseCustomPalette = FALSE;
  DBConfig.CustomColor00 = 0x00000000;   /* VGA  0: DBLACK   */
  DBConfig.CustomColor01 = 0x002F0000;   /* VGA  1: DBLUE    */
  DBConfig.CustomColor02 = 0x0000002F;   /* VGA  4: DRED     */
  DBConfig.CustomColor03 = 0x002F002F;   /* VGA  5: DMAGENTA */
  DBConfig.CustomColor04 = 0x00002F00;   /* VGA  2: DGREEN   */
  DBConfig.CustomColor05 = 0x002F2F00;   /* VGA  3: DCYAN    */
  DBConfig.CustomColor06 = 0x00002F2F;   /* VGA  6: DYELLOW  */
  DBConfig.CustomColor07 = 0x002A2A2A;   /* VGA  7: DWHITE   */
  DBConfig.CustomColor08 = 0x001A1A1A;   /* VGA  8: LBLACK   */
  DBConfig.CustomColor09 = 0x003F0000;   /* VGA  9: LBLUE    */
  DBConfig.CustomColor10 = 0x0000003F;   /* VGA 12: LRED     */
  DBConfig.CustomColor11 = 0x003F003F;   /* VGA 13: LMAGENTA */
  DBConfig.CustomColor12 = 0x00003F00;   /* VGA 10: LGREEN   */
  DBConfig.CustomColor13 = 0x003F3F00;   /* VGA 11: LCYAN    */
  DBConfig.CustomColor14 = 0x00003F3F;   /* VGA 14: LYELLOW  */
  DBConfig.CustomColor15 = 0x003F3F3F;   /* VGA 15: LWHITE   */
  DBConfig.NumAddedLanguages = 0;
  DBConfig.NumLocs = 0;
  DBConfig.FileLocMapping = NULL;
  sprintf (DBConfig.MouseSpeedString, "%u,%u,%u",Mouse.HorizontalMickeys, Mouse.VerticalMickeys, Mouse.DoubleSpeedThreshold);
  if ((Fp = fopen (ConfigFile, "r")) == NULL)                                                      /* Configuration file exists ? */
  {
    WriteConfig ();                                                                                             /* No, create one */
    return;
  }
  Len = _NextString (Fp, Item, FALSE, TRUE);                                        /* Read-ahead first string: must be a keyword */
  while (!feof (Fp))
  {
    if (Item[Len - 1] == '=')
      Item[Len - 1] = '\0';
    else
      if (!_NextString (Fp, S, TRUE, FALSE))
        FatalError (11, "Config: Missing \'=\' sign in option %s", Item);
    Handled = FALSE;
    for (CntOp = 0 ; !Handled && CntOp < NumOps ; CntOp ++)
      if (!stricmp (Item, _ConfOp[CntOp]._OptionName))
      {
        _NextString (Fp, Item, FALSE, FALSE);                                                                 /* Read the operand */
        if (Item[0] == '\0')                                                                                /* An empty operand ? */
        {
          if (_ConfOp[CntOp]._OptionType == OPT_BYTE || _ConfOp[CntOp]._OptionType == OPT_WORD ||
              _ConfOp[CntOp]._OptionType == OPT_BOOL || _ConfOp[CntOp]._OptionType == OPT_SBYTE)
             FatalError (11, "Config: Missing operand after switch %s", _ConfOp[CntOp]._OptionName);
        }
        else
          switch (_ConfOp[CntOp]._OptionType)                                                                 /* Parse the option */
          {
            case OPT_TEXT    : strcpy ((char *)_ConfOp[CntOp]._DataPtr, Item);
                               break;
            case OPT_RPATH   : *((char *)_ConfOp[CntOp]._DataPtr) = '\0';
                               if (Item[0] == RELPATH)
                               {
                                 *((char *)_ConfOp[CntOp]._DataPtr) = RELPATH;
                                 *((char *)_ConfOp[CntOp]._DataPtr + 1) = '\0';
                                 if (Item[1] == '\0')
                                   break;
                                 N = 1;
                               }
                               else
                                 N = 0;
                               String = TestName (Item + N, FALSE, &PathError);
                               if (PathError)
                                 FatalError (11, "Config: %s", String);
                               strcat ((char *)_ConfOp[CntOp]._DataPtr, String);
                               strupr ((char *)_ConfOp[CntOp]._DataPtr);
                               break;
            case OPT_PATH    : String = TestName (Item, FALSE, &PathError);
                               if (PathError)
                                 FatalError (11, "Config: %s", String);
                               strcpy ((char *)_ConfOp[CntOp]._DataPtr, String);
                               strupr ((char *)_ConfOp[CntOp]._DataPtr);
                               break;
            case OPT_BYTE    :
            case OPT_SBYTE   :
            case OPT_WORD    : N = 0;
                               if (!isdigit (Item[N]))
                                 if (Item[N ++] != '-')
                                   FatalError (11, "Config: Invalid number %s after switch %s", Item, _ConfOp[CntOp]._OptionName);
                               for ( ; N < strlen (Item) ; N ++)
                                 if (!isdigit (Item[N]))
                                   FatalError (11, "Config: Invalid number %s after switch %s", Item, _ConfOp[CntOp]._OptionName);
                               if (_ConfOp[CntOp]._OptionType == OPT_WORD)
                                 *((word *)_ConfOp[CntOp]._DataPtr) = atoi (Item);
                               else if (_ConfOp[CntOp]._OptionType == OPT_BYTE)
                                 *((byte *)_ConfOp[CntOp]._DataPtr) = atoi (Item);
                               else
                                 *((char *)_ConfOp[CntOp]._DataPtr) = atoi (Item);
                               break;
            case OPT_DWORD   : if (Item[0] != '0' || toupper (Item[1]) != 'X')
                                 FatalError (11, "Config: Invalid hex number %s after switch %s", Item, _ConfOp[CntOp]._OptionName);
                               *((dword *)_ConfOp[CntOp]._DataPtr) = 0;
                               for (N = 2 ; N < strlen (Item) ; N ++)
                                 if (!isxdigit (Item[N]))
                                   FatalError (11, "Config: Invalid hex number %s after switch %s", Item,
                                               _ConfOp[CntOp]._OptionName);
                                 else
                                 {
                                   *((dword *)_ConfOp[CntOp]._DataPtr) *= 16;
                                   if (isdigit (Item[N]))
                                     *((dword *)_ConfOp[CntOp]._DataPtr) += (Item[N] - '0');
                                   else
                                     *((dword *)_ConfOp[CntOp]._DataPtr) += (toupper (Item[N]) - 'A' + 10);
                                 }
                               break;
            case OPT_BOOL    : if (!stricmp (Item, "On") || !stricmp (Item, "TRUE") || !strcmp (Item, "1") || !stricmp (Item,"Yes"))
                                 *((bool *)_ConfOp[CntOp]._DataPtr) = TRUE;
                               else if (!stricmp (Item, "Off") || !stricmp (Item, "FALSE") || !strcmp (Item, "0") ||
                                        !stricmp (Item, "No"))
                                 *((bool *)_ConfOp[CntOp]._DataPtr) = FALSE;
                               else
                                 FatalError (11, "Config: Unknown boolean type %s", Item);
                               break;
            case OPT_LANG    : if (++ DBConfig.NumAddedLanguages == 1)
                               {
                                 if ((DBConfig.AddedLanguage = (struct Language_s *)malloc (sizeof (struct Language_s))) == NULL)
                                   FatalError (1, "Config: Out of memory");
                               }
                               else
                               {
                                 if ((DBConfig.AddedLanguage = (struct Language_s *)realloc (DBConfig.AddedLanguage,
                                               sizeof (struct Language_s) * DBConfig.NumAddedLanguages)) == NULL)
                                   FatalError (1, "Config: Out of memory");
                               }
                               if (strlen (Item) <= D_LANGUAGE)
                                 strcpy (DBConfig.AddedLanguage[DBConfig.NumAddedLanguages - 1].Language, Item);
                               else
                               {
                                 strncpy (DBConfig.AddedLanguage[DBConfig.NumAddedLanguages - 1].Language, Item, D_LANGUAGE);
                                 DBConfig.AddedLanguage[DBConfig.NumAddedLanguages - 1].Language[D_LANGUAGE] = '\0';
                               }
                               break;
            case OPT_MASK    : if (++ DBConfig.NumLocs == 1)
                               {
                                 if ((DBConfig.FileLocMapping =
                                               (struct FileLocMapping_s *)malloc (sizeof (struct FileLocMapping_s))) == NULL)
                                   FatalError (1, "Config: Out of memory");
                               }
                               else
                               {
                                 if ((DBConfig.FileLocMapping = (struct FileLocMapping_s *)realloc (DBConfig.FileLocMapping,
                                               sizeof (struct FileLocMapping_s) * DBConfig.NumLocs)) == NULL)
                                   FatalError (1, "Config: Out of memory");
                               }
                               String = TestName (Item, FALSE, &PathError);
                               if (PathError)
                                 FatalError (11, "Config: %s", String);
                               strcpy (DBConfig.FileLocMapping[DBConfig.NumLocs - 1].Path, String);
                               strupr (DBConfig.FileLocMapping[DBConfig.NumLocs - 1].Path);
                               break;
          }
        Handled = TRUE;
      }
    if (!Handled)                                                                         /* Read item is not one of the keywords */
      FatalError (11, "Config: Unknown keyword %s in configuration file", Item);
    _NextString (Fp, Item, FALSE, TRUE);                                                                      /* Read next option */
    if (DBConfig.BlankerTimeOut == (word)-1)
    {
      DBConfig.BlankerOn = FALSE;
      DBConfig.BlankerTimeOut = 600;
    }
  }
  fclose (Fp);
  DBConfig.BackupDatabase = DBConfig.AlwaysBackupDatabase;
  if (Mouse.UseGeniusScreenUpdate && Mouse.DisableLocalHiding)
    Mouse.UseGeniusScreenUpdate = FALSE;
  if (DBConfig.StartInfoButton > 3)
    FatalError (11, "Config: Unknown info button %d in \'%s\'", DBConfig.StartInfoButton, STARTBUTTON);
  Ch = DBConfig.MouseSpeedString;
  Val = 0;
  ValNo = 0;
  while (!Finished)                                           /* First parse the configuration setting to set the register values */
  {
    if (isdigit (*Ch))
      Val = Val * 10 + *Ch - '0';
    else if (*Ch == ',' || *Ch == '\0')
    {
      if (Val != 0)
        switch (ValNo)
        {
          case 0 : HorizontalMickeys = Val; break;
          case 1 : VerticalMickeys = Val; break;
          case 2 : DoubleSpeedThreshold = Val; break;
        }
      Val = 0;
      ValNo ++;
      if (*Ch == '\0')
        Finished = TRUE;
    }
    Ch ++;
  }
  SetMouseSpeed (HorizontalMickeys, VerticalMickeys, DoubleSpeedThreshold);
}

void WriteConfig (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The configuration has been written to disk.                                                                            */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  FILE          *Fp;
  byte register  Cnt;

  if (IsFloppy (ProgramDrive[0]))
    RequestFloppy (ProgramDrive[0], ResourceFloppyId);
  if ((Fp = fopen (ConfigFile, "w")) == NULL)
    FatalError (11, "Cannot write configuration file %s", ConfigFile);
  fprintf (Fp, "# %s%s - Configuration of the Spectrum Games Database\n", ProgramName, CONFIGEXT);
  fprintf (Fp, "# Copyright (C) 1995-2001 ThunderWare Research Center\n#\n");
  fprintf (Fp, "# These items are updated from the configuration window\n\n");
  fprintf (Fp, "%-22s = %s\n", REMAP,            DBConfig.RemapBrightBlack ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", DOFLASH,          DBConfig.EmulateFlash ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", SLOWFLASH,        DBConfig.SlowFlash ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", CONFQUIT,         DBConfig.ConfirmQuit ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", CONFDEL,          DBConfig.ConfirmDelete ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", CONFOVERW,        DBConfig.ConfirmOverwrite ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", SYNCSLID,         SystemDefaults.SyncSlider ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", EDITINS,          SystemDefaults.EditInsert ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", BACKUPDB,         DBConfig.AlwaysBackupDatabase ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", VALIDATE,         DBConfig.ValidateAfterLoad ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", ALWAYSCHEAT,      DBConfig.AlwaysCheat ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", BESTSCREEN,       DBConfig.PickBestScreen ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", ALWSELECT,        DBConfig.AlwaysSelectGame ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", AUTOLOADTAP,      DBConfig.AutoLoadTapes ? "On" : "Off");
  fprintf (Fp, "%-22s = %d\n", STARTBUTTON,      DBConfig.StartInfoButton);
  fprintf (Fp, "%-22s = %s\n", NOSOUNDS,         SystemDefaults.Quiet ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", FORCESTICK,       DBConfig.ForceSnapJoystick ? "On" : "Off");
  fprintf (Fp, "%-22s = %d\n", EMULTYPE,         DBConfig.EmulType);
  fprintf (Fp, "%-22s = %s\n", EMULPATH0,        DBConfig.EmulPath[0]);
  fprintf (Fp, "%-22s = %s\n", EMULPATH1,        DBConfig.EmulPath[1]);
  fprintf (Fp, "%-22s = %s\n", EMULPATH2,        DBConfig.EmulPath[2]);
  fprintf (Fp, "%-22s = %s\n", EMULPATH3,        DBConfig.EmulPath[3]);
  fprintf (Fp, "%-22s = %s\n", EMULPATH4,        DBConfig.EmulPath[4]);
  fprintf (Fp, "%-22s = %s\n", EMULPATH5,        DBConfig.EmulPath[5]);
  fprintf (Fp, "%-22s = %s\n", EMULPATH6,        DBConfig.EmulPath[6]);
  fprintf (Fp, "%-22s = %s\n", EMULPATH7,        DBConfig.EmulPath[7]);
  fprintf (Fp, "%-22s = %s\n", EMULPATH8,        DBConfig.EmulPath[8]);
  fprintf (Fp, "%-22s = %s\n", EMULPATH9,        DBConfig.EmulPath[9]);
  fprintf (Fp, "%-22s = %s\n", EMULPATH10,       DBConfig.EmulPath[10]);
  fprintf (Fp, "%-22s = %s\n", EMULPATH11,       DBConfig.EmulPath[11]);
  fprintf (Fp, "%-22s = \"%s\"\n", EMULOPTS0,    DBConfig.ExtraOpt[0]);
  fprintf (Fp, "%-22s = \"%s\"\n", EMULOPTS1,    DBConfig.ExtraOpt[1]);
  fprintf (Fp, "%-22s = \"%s\"\n", EMULOPTS2,    DBConfig.ExtraOpt[2]);
  fprintf (Fp, "%-22s = \"%s\"\n", EMULOPTS3,    DBConfig.ExtraOpt[3]);
  fprintf (Fp, "%-22s = \"%s\"\n", EMULOPTS4,    DBConfig.ExtraOpt[4]);
  fprintf (Fp, "%-22s = \"%s\"\n", EMULOPTS5,    DBConfig.ExtraOpt[5]);
  fprintf (Fp, "%-22s = \"%s\"\n", EMULOPTS6,    DBConfig.ExtraOpt[6]);
  fprintf (Fp, "%-22s = \"%s\"\n", EMULOPTS7,    DBConfig.ExtraOpt[7]);
  fprintf (Fp, "%-22s = \"%s\"\n", EMULOPTS8,    DBConfig.ExtraOpt[8]);
  fprintf (Fp, "%-22s = \"%s\"\n", EMULOPTS9,    DBConfig.ExtraOpt[9]);
  fprintf (Fp, "%-22s = \"%s\"\n", EMULOPTS10,   DBConfig.ExtraOpt[10]);
  fprintf (Fp, "%-22s = \"%s\"\n", EMULOPTS11,   DBConfig.ExtraOpt[11]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT1,         DBConfig.LinkExtension[0]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT2,         DBConfig.LinkExtension[1]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT3,         DBConfig.LinkExtension[2]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT4,         DBConfig.LinkExtension[3]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT5,         DBConfig.LinkExtension[4]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT6,         DBConfig.LinkExtension[5]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT7,         DBConfig.LinkExtension[6]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT8,         DBConfig.LinkExtension[7]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT9,         DBConfig.LinkExtension[8]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT10,        DBConfig.LinkExtension[9]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT11,        DBConfig.LinkExtension[10]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT13,        DBConfig.LinkExtension[12]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT12,        DBConfig.LinkExtension[11]);
  fprintf (Fp, "%-22s = %d\n", LINKEXT14,        DBConfig.LinkExtension[13]);
  fprintf (Fp, "%-22s = %s\n", EDITOR,           DBConfig.EditPath);
  fprintf (Fp, "%-22s = %s\n", ZXEDITOR,         DBConfig.ZXEditPath);
  fprintf (Fp, "%-22s = %s\n", ALWZXEDIT,        DBConfig.AlwaysUseZXEdit ? "On" : "Off");
  fprintf (Fp, "%-22s = %d\n", PRINTR,           DBConfig.Printer);
  fprintf (Fp, "%-22s = %d\n", LNPPG,            DBConfig.LinesPerPage);
  fprintf (Fp, "%-22s = \"%s\"\n", PFIELDS,      DBConfig.FieldsString);
  fprintf (Fp, "%-22s = %d\n", BLANKTO,          DBConfig.BlankerOn ? DBConfig.BlankerTimeOut : -1);
  fprintf (Fp, "\n# These items must be changed by hand");
  fprintf (Fp, "\n# General environment\n\n");
  fprintf (Fp, "%-22s = %s\n", DRAGOUTLN,        SystemDefaults.DragOutline ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", RESZOUTLN,        SystemDefaults.ResizeOutline ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", GREYGHOST,        SystemDefaults.GreyOutGhosting ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", STABLEFLOP,       DBConfig.AssumeStableFloppies ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", PRELOADRES,       DBConfig.PreLoadResources ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", DISABLERIL,       DBConfig.DirectVideoControl ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", SWAPMODE80,       SystemDefaults.ForceSwapMode80 ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", QUITMODE80,       SystemDefaults.ForceQuitMode80 ? "On" : "Off");
  fprintf (Fp, "%-22s = %d   # Don't change this one by hand!\n", NEXTDISKID,    NextFloppyId);
  fprintf (Fp, "\n# Mouse options\n\n");
  fprintf (Fp, "%-22s = %s\n", GMOUSE,           Mouse.UseGeniusScreenUpdate ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", NOLOCHIDE,        Mouse.DisableLocalHiding ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", FORCEMOUSE,       Mouse.ForceMouseRelease ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", SWAPBUTT,         Mouse.SwapLeftAndRightButton ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", EMUL3BUTT,        Mouse.EmulateMiddleMouseButton ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", SNAPMOUSE,        Mouse.SnapToDefaultButton ? "On" : "Off");
  fprintf (Fp, "%-22s = %ld\n", DCLICKDELAY,     Mouse.DoubleClickInterval);
  fprintf (Fp, "%-22s = %ld\n", CPUSHDELAY,      Mouse.CPushDelay);
  fprintf (Fp, "%-22s = %s\n", MOUSESPEED,       DBConfig.MouseSpeedString);
  fprintf (Fp, "\n# Special options\n\n");
  fprintf (Fp, "%-22s = \"%s\"\n", ADDTAPE,      DBConfig.SnapNeedsTAP);
  fprintf (Fp, "%-22s = \"%s\"\n", ADDVOC,       DBConfig.SnapNeedsVOC);
  fprintf (Fp, "%-22s = %s\n", ONLYZ80TAP,       DBConfig.OnlyZ80Tapes ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", VALEMULCHN,       DBConfig.ValidateAtEmulChange ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", SORTPHONE,        DBConfig.UsePhonebookSorting ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", NOSUBDIRS,        DBConfig.AllFilesInOneDir ? "On" : "Off");
  fprintf (Fp, "%-22s = %s\n", GIFPATH,          DBConfig.GIFPath);
  fprintf (Fp, "%-22s = %s\n", COMPRESSOR,       DBConfig.Compressor);
  fprintf (Fp, "%-22s = \"%s\"\n", COMPRESSOPT,  DBConfig.CompressOptions);
  fprintf (Fp, "%-22s = %s\n", GIFVIEWER,        DBConfig.GIFViewerPath);
  fprintf (Fp, "%-22s = \"%s\"\n", GIFOPTIONS,   DBConfig.GIFViewerOptions);
  fprintf (Fp, "%-22s = %s\n", JPGVIEWER,        DBConfig.JPGViewerPath);
  fprintf (Fp, "%-22s = \"%s\"\n", JPGOPTIONS,   DBConfig.JPGViewerOptions);
  fprintf (Fp, "\n# Added game languages\n\n");
  for (Cnt = 0 ; Cnt < DBConfig.NumAddedLanguages ; Cnt ++)
    fprintf (Fp, "%-22s = %s\n", ADDLANGUAGE,    DBConfig.AddedLanguage[Cnt].Language);
  fprintf (Fp, "\n# Change these only if you know what you're doing!\n\n");
  fprintf (Fp, "%-22s = %s\n", CUSTOMPAL,        DBConfig.UseCustomPalette ? "On" : "Off");
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL00, DBConfig.CustomColor00);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL01, DBConfig.CustomColor01);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL02, DBConfig.CustomColor02);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL03, DBConfig.CustomColor03);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL04, DBConfig.CustomColor04);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL05, DBConfig.CustomColor05);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL06, DBConfig.CustomColor06);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL07, DBConfig.CustomColor07);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL08, DBConfig.CustomColor08);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL09, DBConfig.CustomColor09);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL10, DBConfig.CustomColor10);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL11, DBConfig.CustomColor11);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL12, DBConfig.CustomColor12);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL13, DBConfig.CustomColor13);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL14, DBConfig.CustomColor14);
  fprintf (Fp, "%-22s = 0x%06lX\n", CUSTOMCOL15, DBConfig.CustomColor15);
  fprintf (Fp, "\n# \`%s\' entries are added automagically\n\n", LOCMASK);
  for (Cnt = 0 ; Cnt < DBConfig.NumLocs ; Cnt ++)
    fprintf (Fp, "%-22s = %s\n", LOCMASK,        DBConfig.FileLocMapping[Cnt].Path);
  fclose (Fp);
}

byte TestForNewMask (char *Path)

/**********************************************************************************************************************************/
/* Pre   : `Path' holds the (possibly) new path.                                                                                  */
/* Post  : The array of known GameDirs has been searched. If the entry was new, it is added and the config file is rewritten.     */
/*         The return value is the index number into the FileLocMapping array.                                                    */
/* Import: WriteConfig.                                                                                                           */
/**********************************************************************************************************************************/

{
  char          FullMask[MAXDIR + 1];
  byte register LocCnt;
  bool          Resolved = FALSE;

  for (LocCnt = 0 ; LocCnt < DBConfig.NumLocs && !Resolved ; LocCnt ++)
    if (_fullpath (FullMask, DBConfig.FileLocMapping[LocCnt].Path, MAXDIR) != NULL)
      Resolved = !strcmp (Path, FullMask);
  if (!Resolved)
  {
    if (++ DBConfig.NumLocs == 1)
    {
      if ((DBConfig.FileLocMapping = (struct FileLocMapping_s *)malloc (sizeof (struct FileLocMapping_s))) == NULL)
        FatalError (1, "Out of memory");
    }
    else
    {
      if ((DBConfig.FileLocMapping = (struct FileLocMapping_s *)realloc (DBConfig.FileLocMapping,
                    sizeof (struct FileLocMapping_s) * DBConfig.NumLocs)) == NULL)
        FatalError (1, "Out of memory");
    }
    strcpy (DBConfig.FileLocMapping[DBConfig.NumLocs - 1].Path, Path);
    WriteConfig ();
    UnghostMenuFindNew ();
  }
  else
    LocCnt --;
  return (LocCnt);
}

void ChangeConfig (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The configuration has been changed by the user.                                                                        */
/* Import: WriteConfig, ReadConfig, _GetNewFileName.                                                                              */
/**********************************************************************************************************************************/

{
  char      EmulType[11];
  char      LinkName[MAXFTYPE][11];
  char      SButType[11];
  char      BlankerVal[6];
  char      NumLines[4];
  char      PrintNum[2];
  char     *String;
  bool      Stop         = FALSE;
  bool      AcceptConfig = FALSE;
  bool      SaveConfig   = FALSE;
  bool      Finished     = FALSE;
  bool      PathError;
  int       SIndex;

  struct WindowInfo CWindow  = {62, 20, 516, 445, 516, 445, 516, 445, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "CONFIGURATION"};
  struct ButtonInfo PReload  = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 113, 420, 8, DBLACK, DWHITE, NULL, 2, "@Reload", NULL};
  struct ButtonInfo PUse     = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 231, 420, 8, DBLACK, DWHITE, NULL, 22, "@Use", NULL};
  struct ButtonInfo PSave    = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 349, 420, 8, DBLACK, DWHITE, NULL, 1, "@Save", NULL};
  struct ButtonInfo CRemap   = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 30, 1, DBLACK, DWHITE, NULL, 3,
                                &DBConfig.RemapBrightBlack, NULL};
  struct ButtonInfo CEmFlash = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 50, 1, DBLACK, DWHITE, NULL, 23,
                                &DBConfig.EmulateFlash, NULL};
  struct ButtonInfo CSlFlash = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 70, 1, DBLACK, DWHITE, NULL, 24,
                                &DBConfig.SlowFlash, NULL};
  struct ButtonInfo CCoQuit  = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 90, 1, DBLACK, DWHITE, NULL, 4,
                                &DBConfig.ConfirmQuit, NULL};
  struct ButtonInfo CCoDel   = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 110, 1, DBLACK, DWHITE, NULL, 5,
                                &DBConfig.ConfirmDelete, NULL};
  struct ButtonInfo CCoOver  = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 130, 1, DBLACK, DWHITE, NULL, 25,
                                &DBConfig.ConfirmOverwrite, NULL};
  struct ButtonInfo CFollow  = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 150, 1, DBLACK, DWHITE, NULL, 6,
                                &SystemDefaults.SyncSlider, NULL};
  struct ButtonInfo CEdInsrt = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 170, 1, DBLACK, DWHITE, NULL, 20,
                                &SystemDefaults.EditInsert, NULL};
  struct ButtonInfo CAlwBck  = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 190, 1, DBLACK, DWHITE, NULL, 21,
                                &DBConfig.AlwaysBackupDatabase, NULL};
  struct ButtonInfo CAlwVali = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 210, 1, DBLACK, DWHITE, NULL, 26,
                                &DBConfig.ValidateAfterLoad, NULL};
  struct ButtonInfo CAlwCht  = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 230, 1, DBLACK, DWHITE, NULL, 27,
                                &DBConfig.AlwaysCheat, NULL};
  struct ButtonInfo CPckBest = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 250, 1, DBLACK, DWHITE, NULL, 28,
                                &DBConfig.PickBestScreen, NULL};
  struct ButtonInfo CAlwSelt = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 270, 1, DBLACK, DWHITE, NULL, 33,
                                &DBConfig.AlwaysSelectGame, NULL};
  struct ButtonInfo CAutLoad = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 290, 1, DBLACK, DWHITE, NULL, 32,
                                &DBConfig.AutoLoadTapes, NULL};
  struct ButtonInfo CStrtBut = {BT_TEXT, BC_TEXT, BF_FWIDTH, 113, 310, 7, DBLACK, DWHITE, NULL, 29, NULL, NULL};
  struct ButtonInfo CStrtSel = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 162, 310, 1, DBLACK, DWHITE, NULL, 30, "\007", NULL};
  struct ButtonInfo CQuiet   = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 330, 1, DBLACK, DWHITE, NULL, 31,
                                &SystemDefaults.Quiet, NULL};
  struct ButtonInfo CForcJoy = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 350, 1, DBLACK, DWHITE, NULL, 36,
                                &DBConfig.ForceSnapJoystick, NULL};
  struct ButtonInfo CEmuType = {BT_TEXT, BC_TEXT, BF_FWIDTH, 328, 70, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL};
  struct ButtonInfo CEmuSel  = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 359, 70, 1, DBLACK, DWHITE, NULL, 10, "\012", NULL};
  struct ButtonInfo CEmuPath = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITDIR, 214, 110, 23, DBLACK, DWHITE, NULL, 11,
                                NULL, NULL};
  struct ButtonInfo CEmuPSel = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 359, 110, 1, DBLACK, DWHITE, NULL, 16, "\012", NULL};
  struct ButtonInfo CExtOpt  = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 214, 150, 25, DBLACK, DWHITE, NULL, 19,
                                NULL, NULL};
  struct ButtonInfo CEdtPath = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITDIR, 214, 230, 23, DBLACK, DWHITE, NULL, 18,
                                DBConfig.EditPath, NULL};
  struct ButtonInfo CEdtPSel = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 359, 230, 1, DBLACK, DWHITE, NULL, 34, "\012", NULL};
  struct ButtonInfo CZXEPath = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITDIR, 214, 270, 23, DBLACK, DWHITE, NULL, 18,
                                DBConfig.ZXEditPath, NULL};
  struct ButtonInfo CZXEPSel = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 359, 270, 1, DBLACK, DWHITE, NULL, 35, "\012", NULL};
  struct ButtonInfo CAlwZXEd = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 359, 290, 1, DBLACK, DWHITE, NULL, 31,
                                &DBConfig.AlwaysUseZXEdit, NULL};
  struct ButtonInfo CPrtNum  = {BT_TEXT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 346, 350, 1, DBLACK, DWHITE, NULL, 13, NULL, NULL};
  struct ButtonInfo CPrtSel  = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 359, 350, 1, DBLACK, DWHITE, NULL, 14, "\007", NULL};
  struct ButtonInfo CNumLin  = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 346, 370, 3, DBLACK, DWHITE, NULL, 15,
                                NULL, NULL};
  struct ButtonInfo CLnkEx[MAXFTYPE] = { {BT_TEXT, BC_TEXT, BF_FWIDTH, 458,  30, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458,  50, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458,  70, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458,  90, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458, 110, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458, 130, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458, 150, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458, 170, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458, 190, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458, 210, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458, 230, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458, 250, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458, 270, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL},
                                         {BT_TEXT, BC_TEXT, BF_FWIDTH, 458, 290, 4, DBLACK, DWHITE, NULL, 9, NULL, NULL} };
  struct ButtonInfo CLnkES[MAXFTYPE] = { {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489,  30, 1, DBLACK, DWHITE, NULL, 100,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489,  50, 1, DBLACK, DWHITE, NULL, 101,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489,  70, 1, DBLACK, DWHITE, NULL, 102,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489,  90, 1, DBLACK, DWHITE, NULL, 103,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489, 110, 1, DBLACK, DWHITE, NULL, 104,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489, 130, 1, DBLACK, DWHITE, NULL, 105,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489, 150, 1, DBLACK, DWHITE, NULL, 106,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489, 170, 1, DBLACK, DWHITE, NULL, 107,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489, 190, 1, DBLACK, DWHITE, NULL, 108,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489, 210, 1, DBLACK, DWHITE, NULL, 109,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489, 230, 1, DBLACK, DWHITE, NULL, 110,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489, 250, 1, DBLACK, DWHITE, NULL, 111,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489, 270, 1, DBLACK, DWHITE, NULL, 112,
                                          "\012", NULL},
                                         {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 489, 290, 1, DBLACK, DWHITE, NULL, 113,
                                          "\012", NULL} };
  struct ButtonInfo CBlankOn = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 489, 350, 1, DBLACK, DWHITE, NULL, 7,
                                &DBConfig.BlankerOn, NULL};
  struct ButtonInfo CBlankTO = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 464, 370, 5, DBLACK, DWHITE, NULL, 8,
                                NULL, NULL};
  struct ButtonInfo *ButtonP;
  short   register  Cnt;

  CStrtBut.Contents = SButType;
  CEmuType.Contents = EmulType;
  CPrtNum.Contents = PrintNum;
  CNumLin.Contents = NumLines;
  CBlankTO.Contents = BlankerVal;
  CLnkEx[0].Contents = LinkName[0];
  CLnkEx[1].Contents = LinkName[1];
  CLnkEx[2].Contents = LinkName[2];
  CLnkEx[3].Contents = LinkName[3];
  CLnkEx[4].Contents = LinkName[4];
  CLnkEx[5].Contents = LinkName[5];
  CLnkEx[6].Contents = LinkName[6];
  CLnkEx[7].Contents = LinkName[7];
  CLnkEx[8].Contents = LinkName[8];
  CLnkEx[9].Contents = LinkName[9];
  CLnkEx[10].Contents = LinkName[10];
  CLnkEx[11].Contents = LinkName[11];
  CLnkEx[12].Contents = LinkName[12];
  CLnkEx[13].Contents = LinkName[13];
  AddButton (&CWindow, &PUse, TRUE);
  AddButton (&CWindow, &PSave, FALSE);
  AddButton (&CWindow, &CRemap, FALSE);
  AddButton (&CWindow, &CEmFlash, FALSE);
  AddButton (&CWindow, &CSlFlash, FALSE);
  AddButton (&CWindow, &CCoQuit, FALSE);
  AddButton (&CWindow, &CCoDel, FALSE);
  AddButton (&CWindow, &CCoOver, FALSE);
  AddButton (&CWindow, &CFollow, FALSE);
  AddButton (&CWindow, &CEdInsrt, FALSE);
  AddButton (&CWindow, &CAlwBck, FALSE);
  AddButton (&CWindow, &CAlwVali, FALSE);
  AddButton (&CWindow, &CAlwCht, FALSE);
  AddButton (&CWindow, &CPckBest, FALSE);
  AddButton (&CWindow, &CAlwSelt, FALSE);
  AddButton (&CWindow, &CAutLoad, FALSE);
  AddButton (&CWindow, &CStrtBut, FALSE);
  AddButton (&CWindow, &CStrtSel, FALSE);
  AddButton (&CWindow, &CQuiet, FALSE);
  AddButton (&CWindow, &CForcJoy, FALSE);
  AddButton (&CWindow, &CEmuType, FALSE);
  AddButton (&CWindow, &CEmuSel, FALSE);
  AddButton (&CWindow, &CEmuPath, FALSE);
  AddButton (&CWindow, &CEmuPSel, FALSE);
  AddButton (&CWindow, &CExtOpt, FALSE);
  AddButton (&CWindow, &CEdtPath, FALSE);
  AddButton (&CWindow, &CEdtPSel, FALSE);
  AddButton (&CWindow, &CZXEPath, FALSE);
  AddButton (&CWindow, &CZXEPSel, FALSE);
  AddButton (&CWindow, &CAlwZXEd, FALSE);
  AddButton (&CWindow, &CPrtNum, FALSE);
  AddButton (&CWindow, &CPrtSel, FALSE);
  AddButton (&CWindow, &CNumLin, FALSE);
  for (Cnt = 0 ; Cnt < MAXFTYPE ; Cnt ++)
  {
    GetEmulatorName (DBConfig.LinkExtension[Cnt], LinkName[Cnt]);
    AddButton (&CWindow, &CLnkEx[Cnt], FALSE);
    AddButton (&CWindow, &CLnkES[Cnt], FALSE);
  }
  AddButton (&CWindow, &CBlankOn, FALSE);
  AddButton (&CWindow, &CBlankTO, FALSE);
  AddButton (&CWindow, &PReload, FALSE);
  sprintf (PrintNum, "%d", DBConfig.Printer);
  sprintf (NumLines, "%d", DBConfig.LinesPerPage);
  sprintf (BlankerVal, "%d", DBConfig.BlankerTimeOut);
  if (DBConfig.BlankerOn)
    CBlankTO.Flags &= ~BF_GHOSTED;
  else
    CBlankTO.Flags |= BF_GHOSTED;
  GetEmulatorName (DBConfig.EmulType, EmulType);
  CEmuPath.Contents = DBConfig.EmulPath[DBConfig.EmulType];
  CExtOpt.Contents = DBConfig.ExtraOpt[DBConfig.EmulType];
  switch (DBConfig.StartInfoButton)
  {
    case 0 : strcpy (SButType, "none"); break;
    case 1 : strcpy (SButType, "SCREEN"); break;
    case 2 : strcpy (SButType, "INFO"); break;
    case 3 : strcpy (SButType, "SCRSHOT"); break;
  }
  CEmuPath.MaxInputLength = MAXDIR - 1;
  CEdtPath.MaxInputLength = MAXDIR - 1;
  CExtOpt.MaxInputLength = 100;
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
  WDrawPicture (&CWindow, 230, 11, SpectrumLogo);
  WDrawShadowBox (&CWindow, 1, 0, 39, 30, "General Options");
  WDrawShadowBox (&CWindow, 5, 33, 17, 63, "Emulator");
  WDrawShadowBox (&CWindow, 19, 33, 31, 63, "Game Info Editors");
  WDrawShadowBox (&CWindow, 33, 33, 39, 63, "Printer");
  WDrawShadowBox (&CWindow, 1, 66, 31, 84, "Extension Links");
  WDrawShadowBox (&CWindow, 33, 66, 39, 84, "Blanker");
  Wprintf (&CWindow, 3,  2,  "{+1Remap Bright Black}N");
  Wprintf (&CWindow, 5,  2,  "{+1Emulate Flash}N");
  Wprintf (&CWindow, 7,  2,  "{+1Slow Flash}N");
  Wprintf (&CWindow, 9,  2,  "{+1Confirm Quit}N");
  Wprintf (&CWindow, 11, 2,  "{+1Confirm Delete}N");
  Wprintf (&CWindow, 13, 2,  "{+1Confirm Overwrite}N");
  Wprintf (&CWindow, 15, 2,  "{+1Follow Slider}N");
  Wprintf (&CWindow, 17, 2,  "{+1Edit Insert}N");
  Wprintf (&CWindow, 19, 2,  "{+1Always Backup}N");
  Wprintf (&CWindow, 21, 2,  "{+1Always Validate}N");
  Wprintf (&CWindow, 23, 2,  "{+1Always Cheat}N");
  Wprintf (&CWindow, 25, 2,  "{+1Pick Best Screen}N");
  Wprintf (&CWindow, 27, 2,  "{+1Always Select Game}N");
  Wprintf (&CWindow, 29, 2,  "{+1Auto-Load Tapes}N");
  Wprintf (&CWindow, 31, 2,  "{+1Start Button}N");
  Wprintf (&CWindow, 33, 2,  "{+1Disable Sounds}N");
  Wprintf (&CWindow, 35, 2,  "{+1Force Snap Joystick}N");
  Wprintf (&CWindow, 7,  35, "{+1Emulator Type}N");
  Wprintf (&CWindow, 9,  35, "{+1Emulator Path}N");
  Wprintf (&CWindow, 13, 35, "{+1Additional Switches}N");
  Wprintf (&CWindow, 21, 35, "{+1ASCII Editor Path}N");
  Wprintf (&CWindow, 25, 35, "{+1ZX-Editor Path}N");
  Wprintf (&CWindow, 29, 35, "{+1Always Use ZX-Editor}N");
  Wprintf (&CWindow, 35, 35, "{+1Printer Port      LPT}N");
  Wprintf (&CWindow, 37, 35, "{+1Lines Per Page}N");
  Wprintf (&CWindow,  3, 68, "{+1*.TAP}N");
  Wprintf (&CWindow,  5, 68, "{+1*.Z80}N");
  Wprintf (&CWindow,  7, 68, "{+1*.SNA}N");
  Wprintf (&CWindow,  9, 68, "{+1*.MDR}N");
  Wprintf (&CWindow, 11, 68, "{+1*.VOC}N");
  Wprintf (&CWindow, 13, 68, "{+1*.SLT}N");
  Wprintf (&CWindow, 15, 68, "{+1*.SNP}N");
  Wprintf (&CWindow, 17, 68, "{+1*.SEM}N");
  Wprintf (&CWindow, 19, 68, "{+1*.LTP}N");
  Wprintf (&CWindow, 21, 68, "{+1*.TZX}N");
  Wprintf (&CWindow, 23, 68, "{+1*.SP}N");
  Wprintf (&CWindow, 25, 68, "{+1*.TRD}N");
  Wprintf (&CWindow, 27, 68, "{+1*.DSK}N");
  Wprintf (&CWindow, 29, 68, "{+1*.FDI}N");
  Wprintf (&CWindow, 35, 68, "{+1Installed}N");
  Wprintf (&CWindow, 37, 68, "{+1Time-Out}N");
  while (!Finished)
  {
    while (!AcceptConfig)
    {
      Stop = FALSE;
      while (!Stop)
      {
        switch (HandleWindow (&CWindow, &ButtonP))
        {
          case WINH_BUTTBOOL: switch (ButtonP->ReactNum)
                              {
                                case 7  : if (DBConfig.BlankerOn)                                                   /* Blanker on */
                                            CBlankTO.Flags &= ~BF_GHOSTED;
                                          else
                                            CBlankTO.Flags |= BF_GHOSTED;
                                          DrawButton (&CWindow, &CBlankTO);
                                          if (DBConfig.BlankerOn)
                                            InstallBlanker (DBConfig.BlankerTimeOut, NULL);
                                          else
                                            DisableBlanker ();
                                          break;
                                case 21 : if (!NonexistingDatabase)                                              /* Always backup */
                                            DBConfig.BackupDatabase = DBConfig.AlwaysBackupDatabase;
                                          break;
                              }
                              break;
          case WINH_CLOSE   : Stop = TRUE;
                              Finished = TRUE;
                              break;
          case WINH_RNOHAND : switch (ButtonP->ReactNum)
                              {
                                case 1  : Stop = TRUE; SaveConfig = TRUE; break;                                          /* Save */
                                case 22 : Stop = TRUE; Finished = TRUE; break;                                             /* Use */
                                case 2  : if (DBConfig.NumLocs > 0)                      /* Reload; do we have mappings defined ? */
                                            free (DBConfig.FileLocMapping);                                /* Release them first! */
                                          if (DBConfig.NumAddedLanguages > 0)                           /* Languages were added ? */
                                            free (DBConfig.AddedLanguage);                             /* Release them first too! */
                                          ReadConfig ();
                                          sprintf (PrintNum, "%d", DBConfig.Printer);
                                          sprintf (NumLines, "%d", DBConfig.LinesPerPage);
                                          sprintf (BlankerVal, "%d", DBConfig.BlankerTimeOut);
                                          if (DBConfig.BlankerOn)
                                            CBlankTO.Flags &= ~BF_GHOSTED;
                                          else
                                            CBlankTO.Flags |= BF_GHOSTED;
                                          GetEmulatorName (DBConfig.EmulType, EmulType);
                                          CEmuPath.Contents = DBConfig.EmulPath[DBConfig.EmulType];
                                          CExtOpt.Contents = DBConfig.ExtraOpt[DBConfig.EmulType];
                                          switch (DBConfig.StartInfoButton)
                                          {
                                            case 0 : strcpy (SButType, "none"); break;
                                            case 1 : strcpy (SButType, "SCREEN"); break;
                                            case 2 : strcpy (SButType, "INFO"); break;
                                            case 3 : strcpy (SButType, "SCRSHOT"); break;
                                          }
                                          DrawButton (&CWindow, &CRemap);
                                          DrawButton (&CWindow, &CEmFlash);
                                          DrawButton (&CWindow, &CSlFlash);
                                          DrawButton (&CWindow, &CCoQuit);
                                          DrawButton (&CWindow, &CCoDel);
                                          DrawButton (&CWindow, &CCoOver);
                                          DrawButton (&CWindow, &CFollow);
                                          DrawButton (&CWindow, &CEdInsrt);
                                          DrawButton (&CWindow, &CAlwBck);
                                          DrawButton (&CWindow, &CAlwVali);
                                          DrawButton (&CWindow, &CAlwCht);
                                          DrawButton (&CWindow, &CPckBest);
                                          DrawButton (&CWindow, &CAlwSelt);
                                          DrawButton (&CWindow, &CAutLoad);
                                          DrawButton (&CWindow, &CQuiet);
                                          DrawButton (&CWindow, &CAlwZXEd);
                                          DrawButton (&CWindow, &CBlankOn);
                                          UpdateButtonText (&CWindow, &CStrtBut, BF_FITTXT);
                                          UpdateButtonText (&CWindow, &CBlankTO, BF_FITTXT);
                                          UpdateButtonText (&CWindow, &CEmuType, BF_FITTXT);
                                          UpdateButtonText (&CWindow, &CEmuPath, BF_FITDIR);
                                          UpdateButtonText (&CWindow, &CExtOpt, BF_FITTXT);
                                          UpdateButtonText (&CWindow, &CEdtPath, BF_FITDIR);
                                          UpdateButtonText (&CWindow, &CZXEPath, BF_FITDIR);
                                          UpdateButtonText (&CWindow, &CPrtNum, BF_FITTXT);
                                          UpdateButtonText (&CWindow, &CPrtSel, BF_FITTXT);
                                          UpdateButtonText (&CWindow, &CNumLin, BF_FITTXT);
                                          for (Cnt = 0 ; Cnt < MAXFTYPE ; Cnt ++)
                                          {
                                            GetEmulatorName (DBConfig.LinkExtension[Cnt], LinkName[Cnt]);
                                            UpdateButtonText (&CWindow, &CLnkEx[Cnt], BF_FITTXT);
                                          }
                                          break;
                                case 10 : DBConfig.EmulType = GetDropDownEmulator (&CWindow, 155, 86, DBConfig.EmulType, FALSE);
                                          GetEmulatorName (DBConfig.EmulType, EmulType);
                                          CEmuPath.Contents = DBConfig.EmulPath[DBConfig.EmulType];
                                          CExtOpt.Contents = DBConfig.ExtraOpt[DBConfig.EmulType];
                                          UpdateButtonText (&CWindow, &CEmuType, BF_FITTXT);
                                          UpdateButtonText (&CWindow, &CEmuPath, BF_FITDIR);
                                          UpdateButtonText (&CWindow, &CExtOpt, BF_FITTXT);
                                          break;
                                case 14 : if (++ DBConfig.Printer == 4)                                                /* LPT Nr. */
                                            DBConfig.Printer = 0;                                                     /* 0 = File */
                                          sprintf (PrintNum, "%d", DBConfig.Printer);
                                          DrawButton (&CWindow, &CPrtNum); break;
                                          break;
                                case 16 : _GetNewFileName ("Select emulator start file", CEmuPath.Contents);
                                          UpdateButtonText (&CWindow, &CEmuPath, BF_FITDIR);
                                          break;
                                case 19 : _GetNewFileName ("Select game info editor", CEdtPath.Contents);
                                          UpdateButtonText (&CWindow, &CEdtPath, BF_FITDIR);
                                          break;
                                case 30 : if (++ DBConfig.StartInfoButton == 4) DBConfig.StartInfoButton = 0;
                                          switch (DBConfig.StartInfoButton)
                                          {
                                            case 0 : strcpy (SButType, "none"); break;
                                            case 1 : strcpy (SButType, "SCREEN"); break;
                                            case 2 : strcpy (SButType, "INFO"); break;
                                            case 3 : strcpy (SButType, "SCRSHOT"); break;
                                          }
                                          UpdateButtonText (&CWindow, &CStrtBut, BF_FITTXT);
                                          break;
                                case 34 : _GetNewFileName ("Select ASCII Editor", CEdtPath.Contents);
                                          UpdateButtonText (&CWindow, &CEdtPath, BF_FITDIR);
                                          break;
                                case 35 : _GetNewFileName ("Select ZX-Editor", CZXEPath.Contents);
                                          UpdateButtonText (&CWindow, &CZXEPath, BF_FITDIR);
                                          break;
                                case 100:
                                case 101:
                                case 102:
                                case 103:
                                case 104:
                                case 105:
                                case 106:
                                case 107:
                                case 108:
                                case 109:
                                case 110:
                                case 111:
                                case 112:
                                case 113: Cnt = ButtonP->ReactNum - 100;
                                          DBConfig.LinkExtension[Cnt] = GetDropDownEmulator (&CWindow, 285, 46 + Cnt * 20,
                                                                                             DBConfig.LinkExtension[Cnt], TRUE);
                                          GetEmulatorName (DBConfig.LinkExtension[Cnt], LinkName[Cnt]);
                                          UpdateButtonText (&CWindow, &CLnkEx[Cnt], BF_FITTXT);
                                          break;
                              }
                              break;
          case WINH_EDITEND : if (ButtonP->Flags & BF_FITINPUT)                                        /* Contents were changed ? */
                                switch (ButtonP->ReactNum)
                                {
                                  case 8  : DBConfig.BlankerTimeOut = Blanker.TimeOut = atoi (BlankerVal);
                                            sprintf (BlankerVal, "%d", DBConfig.BlankerTimeOut);
                                            UpdateButtonText (&CWindow, &CBlankTO, BF_FITTXT);
                                            break;
                                  case 15 : DBConfig.LinesPerPage = atoi (NumLines);
                                            sprintf (NumLines, "%d", DBConfig.LinesPerPage);
                                            UpdateButtonText (&CWindow, &CNumLin, BF_FITTXT);
                                            break;
                                }
        }
        if (GlobalKey == K_ESC)
          Stop = Finished = TRUE;
        MouseStatus ();
      }
      AcceptConfig = FALSE;
      String = TestName (DBConfig.EmulPath[0], FALSE, &PathError);
      if (PathError)
        ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
      else
      {
        strcpy (DBConfig.EmulPath[0], String);
        String = TestName (DBConfig.EmulPath[1], FALSE, &PathError);
        if (PathError)
          ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
        else
        {
          strcpy (DBConfig.EmulPath[1], String);
          String = TestName (DBConfig.EmulPath[2], FALSE, &PathError);
          if (PathError)
            ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
          else
          {
            SIndex = strlen (String) - 1;
            while (-- SIndex >= 0 && String[SIndex] != '\\')
              ;
            if (String[0] != '\0' &&                                                                               /* Allow empty */
                stricmp (String + SIndex + 1, "SPEC48.EXE") &&
                stricmp (String + SIndex + 1, "SPEC128.EXE"))
              ErrorRequester (-1, -1, NULL, NULL, NULL,
                "ERROR: %s is not a valid startcommand for Warajevo.\n"
                "Choose either SPEC48.EXE or SPEC128.EXE", String);
            else
            {
              strcpy (DBConfig.EmulPath[2], String);
              String = TestName (DBConfig.EmulPath[3], FALSE, &PathError);
              if (PathError)
                ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
              else
              {
                strcpy (DBConfig.EmulPath[3], String);
                String = TestName (DBConfig.EmulPath[4], FALSE, &PathError);
                if (PathError)
                  ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
                else
                {
                  strcpy (DBConfig.EmulPath[4], String);
                  String = TestName (DBConfig.EmulPath[5], FALSE, &PathError);
                  if (PathError)
                    ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
                  else
                  {
                    strcpy (DBConfig.EmulPath[5], String);
                    String = TestName (DBConfig.EmulPath[6], FALSE, &PathError);
                    if (PathError)
                      ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
                    else
                    {
                      strcpy (DBConfig.EmulPath[6], String);
                      String = TestName (DBConfig.EmulPath[7], FALSE, &PathError);
                      if (PathError)
                        ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
                      else
                      {
                        strcpy (DBConfig.EmulPath[7], String);
                        String = TestName (DBConfig.EmulPath[8], FALSE, &PathError);
                        if (PathError)
                          ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
                        else
                        {
                          strcpy (DBConfig.EmulPath[8], String);
                          String = TestName (DBConfig.EmulPath[9], FALSE, &PathError);
                          if (PathError)
                            ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
                          else
                          {
                            strcpy (DBConfig.EmulPath[9], String);
                            String = TestName (DBConfig.EmulPath[10], FALSE, &PathError);
                            if (PathError)
                              ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
                            else
                            {
                              strcpy (DBConfig.EmulPath[10], String);
                              String = TestName (DBConfig.EmulPath[11], FALSE, &PathError);
                              if (PathError)
                                ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
                              else
                              {
                                strcpy (DBConfig.EmulPath[11], String);
                                String = TestName (DBConfig.EditPath, FALSE, &PathError);
                                if (PathError)
                                  ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
                                else
                                {
                                  strcpy (DBConfig.EditPath, String);
                                  String = TestName (DBConfig.ZXEditPath, FALSE, &PathError);
                                  if (PathError)
                                    ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR: %s", String);
                                  else
                                  {
                                    strcpy (DBConfig.ZXEditPath, String);
                                    AcceptConfig = TRUE;
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    if (SaveConfig)
    {
      WriteConfig ();
      AcceptConfig = SaveConfig = FALSE;
    }
  }
  DestroyWindow (&CWindow);
}

void AboutBox (bool MakeWindow)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : An `about'-box has been shown on the screen and closed by the user.                                                    */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct ButtonInfo POk      = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 79, 215, 8, DBLACK, DWHITE, NULL, 1, "@Ok", NULL};
  struct ButtonInfo *ButtonP;
  short             HandleRet;

  memset (&AboutWindow, 0, sizeof (struct WindowInfo));
  AboutWindow.StartX      = MakeWindow ? 186 : 215;
  AboutWindow.StartY      = MakeWindow ? 144 : 10;
  AboutWindow.Width       = AboutWindow.MinWidth = AboutWindow.MaxWidth = 210;
  AboutWindow.Height      = AboutWindow.MinHeight = AboutWindow.MaxHeight = MakeWindow ? 240 : 215;
  AboutWindow.BorderSize  = MakeWindow ? 1 : 3;
  AboutWindow.BorderFlags = MakeWindow ? (WINB_OUT | WINB_SURROUND) : WINB_NONE;
  AboutWindow.APen        = DBLACK;
  AboutWindow.BPen        = DWHITE;
  AboutWindow.Font        = NULL;
  AboutWindow.Flags       = MakeWindow ? (WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER) : WINF_XMSBUFFER;
  AboutWindow.HeaderFont  = NULL;
  AboutWindow.Header      = "About This Program";
  if (MakeWindow)
    AddButton (&AboutWindow, &POk, TRUE);
  DrawWindow (&AboutWindow, NULL, WING_CLOSE, TRUE);
  WDrawPicture (&AboutWindow, 40, 7, SpectrumLogo);
  Wprintf (&AboutWindow,  4, TEXT_CENTER,  "{+3Spectrum Games Database}N");
  Wprintf (&AboutWindow,  4, TEXT_MCENTER, "{+1Spectrum Games Database}N");
  Wprintf (&AboutWindow,  7, TEXT_CENTER,              "By");
  Wprintf (&AboutWindow,  8, TEXT_CENTER,   "{TMartijn van der Heide}T");
  Wprintf (&AboutWindow, 10, TEXT_CENTER,    "Version 2.05 - 09/12/01");
  Wprintf (&AboutWindow, 11, TEXT_CENTER,        "32-bit version");
  Wprintf (&AboutWindow, 13, TEXT_CENTER,    "Copyleft (c) 1995-2001");
  Wprintf (&AboutWindow, 14, TEXT_CENTER,  "ThunderWare Research Center");
  Wprintf (&AboutWindow, 16, TEXT_CENTER, "This program is free software");
  Wprintf (&AboutWindow, 18, TEXT_CENTER,  "See 'COPYING' for more info");
  if (!MakeWindow)
  {
    WDrawBox (&AboutWindow, FALSE, DBLACK, 0, 0, AboutWindow.Width - 1, AboutWindow.Height - 1);
    WDrawBox (&AboutWindow, FALSE, DBLACK, 1, 1, AboutWindow.Width - 2, AboutWindow.Height - 2);
    return;
  }
  while ((HandleRet = HandleWindow (&AboutWindow, &ButtonP)) != WINH_RNOHAND && HandleRet != WINH_CLOSE && GlobalKey != K_ESC)
    MouseStatus ();
  DestroyWindow (&AboutWindow);
}

void SysInfoBox (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : A `system info'-box has been shown on the screen and closed by the user.                                               */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct WindowInfo CWindow  = {184, 105, 216, 245, 216, 245, 216, 245, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                WINF_HUSE | WINF_HCENTER | WINF_ACTIVATE | WINF_XMSBUFFER, NULL, "System Information"};
  struct ButtonInfo POk      = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_KEYSELECT, 81, 220, 8, DBLACK, DWHITE, NULL, 1, "@Ok", NULL};
  struct ButtonInfo *ButtonP;
  short              HandleRet;
  short              LineAdd     = 2;
  short              CurLine;
  byte              *Version;
  char              *MouseType[] = { "Unknown", "Bus", "Serial", "InPort", "PS/2", "HP" };
  char               MSWinMode[6];
  char               MSWinVersion[8];

  if (MSWindowsRunning)
    LineAdd ++;
  if (SoundBlasterInfo.Detected)                                                                  /* Did we find a SoundBlaster ? */
    LineAdd += 3;
  CWindow.Height = CWindow.MinHeight = CWindow.MaxHeight = CWindow.Height + LineAdd * (CurrentFont->Height + 2);
  POk.StartY += (LineAdd * (CurrentFont->Height + 2));
  AddButton (&CWindow, &POk, TRUE);
  DrawWindow (&CWindow, NULL, WING_CLOSE, TRUE);
  CurLine = 1;
  Wprintf (&CWindow, CurLine ++, 1, "{+1OS}N       DOS Version : %d.%02d", _osmajor, _osminor);
  Wprintf (&CWindow, CurLine ++, 1, "         WB Version  : %d.%02d.%d", WBMajorVersion, WBMinorVersion, WBRevision);
  if (MSWindowsRunning)
  {
    Version = (byte *)&MSWindowsVersion;
    if (MSWindowsStopped)
      strcpy (MSWinMode, "(Dis)");
    else
      strcpy (MSWinMode, MSWindowsMode == 0x0002 ? "(Std)" : "(Enh)");
    if (MSWindowsVersion >= 0x0400)                                                       /* Windows95 is reported as version 4.0 */
    {
      if (MSWindowsVersion == 0xFFFF)                                                        /* The special value for WindowsNT ? */
        strcpy (MSWinVersion, "WinNT");
      if (MSWindowsVersion < 0x0403)
        strcpy (MSWinVersion, "Win95");
      else if (MSWindowsVersion < 0x0500)
        strcpy (MSWinVersion, "Win96");                                                                    /* A.k.a. OSR2 (v4.3+) */
      else
        strcpy (MSWinVersion, "Win98");                                       /* Assume Windows98 will be reported as version 5.0 */
      if (!MSWindowsStopped)
        MSWinMode[0] = '\0';                                                   /* (There's no such thing as Std or Enh Windows95) */
    }
    else
      sprintf (MSWinVersion, "%d.%d", *(Version + 1), *Version);
    Wprintf (&CWindow, CurLine ++, 1, "         MS Windows  : %s %s", MSWinVersion, MSWinMode);
  }
  CurLine ++;
  Wprintf (&CWindow,  CurLine ++, 1, "{+1DISPLAY}N  Color       : %s", Screen.ColorDisplay ? "Yes" : "No");
  Wprintf (&CWindow,  CurLine ++, 1, "         Start       : %05lX", Screen.DisplayStart);
  if (Screen.UseRIL)
    Wprintf (&CWindow,  CurLine ++, 1, "         RIL Version : %d.%02d", Screen.RILMajorVersion, Screen.RILMinorVersion);
  else
    Wprintf (&CWindow,  CurLine ++, 1, "         Control     : Direct");
  CurLine ++;
  Wprintf (&CWindow,  CurLine ++, 1, "{+1KEYBOARD}N Enhanced    : %s", EnhancedKeyboard ? "Yes" : "No");
  CurLine ++;
  Version = (byte *)&(Mouse.DriverVersion);
  Wprintf (&CWindow, CurLine ++, 1, "{+1MOUSE}N    Version     : %X.%02X", *(Version + 1), *Version);
  Wprintf (&CWindow, CurLine ++, 1, "         Buttons     : %d", Mouse.NumButtons);
  Wprintf (&CWindow, CurLine ++, 1, "         Type        : %s", MouseType[Mouse.MouseType]);
  if (Mouse.MouseIRQ == 0 || Mouse.MouseIRQ == 255)                                            /* IRQ 0 is reserved for PS/2 mice */
    Wprintf (&CWindow, CurLine ++, 1, "         IRQ         : (PS/2)");
  else
    Wprintf (&CWindow, CurLine ++, 1, "         IRQ         : %d", Mouse.MouseIRQ);
  CurLine ++;
  if (SoundBlasterInfo.Detected)
  {
    Wprintf (&CWindow, CurLine ++, 1, "{+1SOUND}N    BaseAddress : %03X", SoundBlasterInfo.BaseAddress);
    Wprintf (&CWindow, CurLine ++, 1, "         IRQ         : %d", SoundBlasterInfo.IRQ);
    Wprintf (&CWindow, CurLine ++, 1, "         DMA Channel : %d", SoundBlasterInfo.DMAChannel);
    Wprintf (&CWindow, CurLine ++, 1, "         Soft Mixer  : %s", SoundBlasterInfo.MixerType == SBMTYPE_NONE ? "None" :
                  (SoundBlasterInfo.MixerType == SBMTYPE_SBPRO ? "SBPro" :
                   (SoundBlasterInfo.MixerType == SBMTYPE_SB16 ? "SB16" : "Dunno")));
  }
  else
    Wprintf (&CWindow, CurLine ++, 1, "{+1SOUND}N    Device      : None");
  CurLine ++;
  if (!DPMIServerDetected)
    Wprintf (&CWindow, CurLine ++, 1, "{+1DPMI}N                 : None");
  else
  {
    Wprintf (&CWindow, CurLine ++, 1, "{+1DPMI}N     Version     : %d.%d", DPMIMajorVersion, DPMIMinorVersion);
    Wprintf (&CWindow, CurLine ++, 1, "         Int Mode    : %s", DPMIFlags & 0x02 ? "Real" : "V86");
    Wprintf (&CWindow, CurLine ++, 1, "         VMEM Support: %s", DPMIFlags & 0x04 ? "Yes" : "No");
  }
  CurLine ++;
  if (DPMIFlags & 0x04)
    Wprintf (&CWindow, CurLine ++, 1, "{+1VMEM}N     Total left  : %lu", XMSAllCore);
  else
    Wprintf (&CWindow, CurLine ++, 1, "{+1MEM}N      Total left  : %lu", _memavl);
  while ((HandleRet = HandleWindow (&CWindow, &ButtonP)) != WINH_RNOHAND && HandleRet != WINH_CLOSE && GlobalKey != K_ESC)
    MouseStatus ();
  DestroyWindow (&CWindow);
}
