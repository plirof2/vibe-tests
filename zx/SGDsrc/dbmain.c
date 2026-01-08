/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBMAIN.C                                                                                                        */
/* Description  : Spectrum Database main program                                                                                  */
/* Version type : Main program                                                                                                    */
/* Last changed : 15-04-2001  13:25                                                                                               */
/* Update count : 194                                                                                                             */
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

#define __DBMAIN_MAIN__

#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "wbench32/wbench32.h"
#include "dbfile.h"
#include "dbscreen.h"
#include "dbdbase.h"
#include "dbconfig.h"
#include "dbselect.h"
#include "dbmenu.h"
#include "dbpoke.h"
#include "dbmain.h"

#pragma pack                ()

/**********************************************************************************************************************************/
/* Define the user defined joystick keyvalues                                                                                     */
/**********************************************************************************************************************************/

char     *UDefTable[8][16] = {{ "<caps>", "\'Z\'", "", "\'X\'", "", "", "", "\'C\'", "", "", "", "", "", "", "", "\'V\'" },
                              { "\'A\'",  "\'S\'", "", "\'D\'", "", "", "", "\'F\'", "", "", "", "", "", "", "", "\'G\'" },
                              { "\'Q\'",  "\'W\'", "", "\'E\'", "", "", "", "\'R\'", "", "", "", "", "", "", "", "\'T\'" },
                              { "\'1\'",  "\'2\'", "", "\'3\'", "", "", "", "\'4\'", "", "", "", "", "", "", "", "\'5\'" },
                              { "\'0\'",  "\'9\'", "", "\'8\'", "", "", "", "\'7\'", "", "", "", "", "", "", "", "\'6\'" },
                              { "\'P\'",  "\'O\'", "", "\'I\'", "", "", "", "\'U\'", "", "", "", "", "", "", "", "\'Y\'" },
                              { "<ent>",  "\'L\'", "", "\'K\'", "", "", "", "\'J\'", "", "", "", "", "", "", "", "\'H\'" },
                              { "<spc>",  "<sym>", "", "\'M\'", "", "", "", "\'N\'", "", "", "", "", "", "", "", "\'B\'" }};

/**********************************************************************************************************************************/
/* Define the spectrum palette for VGA                                                                                            */
/**********************************************************************************************************************************/

dword     SpecPalette[16]   = { 0x00000000, 0x002F0000, 0x00002F00, 0x002F2F00, 0x0000002F, 0x002F002F, 0x00002F2F, 0x002A2A2A,
                                0x001A1A1A, 0x003F0000, 0x00003F00, 0x003F3F00, 0x0000003F, 0x003F003F, 0x00003F3F, 0x003F3F3F };

short     SpecColTable[16]  = { 0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15 };

/**********************************************************************************************************************************/
/* Define the RealSpectrum options                                                                                                */
/**********************************************************************************************************************************/

#define RS_NONE      -1
#define RS_ISTRD     -2
#define RS_JOYSTICK  -3
#define RS_ISMDR     -4
#define RS_MODEL     -5
#define RS_POKES     -6
#define RS_ISSNAPD   -7
#define RS_ISTAPED   -8
#define RS_ISDISKD   -9
#define RS_ISMDRD    -10
#define RS_ISSNAPF   -11
#define RS_ISTAPEF   -12
#define RS_TAKEOUT   -13
struct RSPEC_s
{
  char *Name;
  int   When;
  bool  Done;
} RSPECOptions[] = {{ "MGT-Enabled",   RS_NONE,     FALSE },
                    { "TRDOS-Enabled", RS_ISTRD,    FALSE },
                    { "Joystick1",     RS_JOYSTICK, FALSE },
                    { "IF1-Enabled",   RS_ISMDR,    FALSE },
                    { "MB02-Enabled",  RS_NONE,     FALSE },
                    { "Opus-Enabled",  RS_NONE,     FALSE },
                    { "D80-Enabled",   RS_NONE,     FALSE },
                    { "SpectrumModel", RS_MODEL,    FALSE },
                    { "PokesPath",     RS_POKES,    FALSE },
                    { "SnapsFolder",   RS_ISSNAPD,  FALSE },
                    { "Snapshot",      RS_ISSNAPF,  FALSE },
                    { "TapesFolder",   RS_ISTAPED,  FALSE },
                    { "Tape",          RS_ISTAPEF,  FALSE },
                    { "DisksFolder",   RS_ISDISKD,  FALSE },
                    { "MGT-Disk1",     RS_TAKEOUT,  FALSE },
                    { "MB02-Disk1",    RS_TAKEOUT,  FALSE },
                    { "Opus-Disk1",    RS_TAKEOUT,  FALSE },
                    { "D80-Disk1",     RS_TAKEOUT,  FALSE },
                    { "IDE-HD1",       RS_TAKEOUT,  FALSE },
                    { "TRDOS-Disk1",   FT_TRD,      FALSE },
                    { "Plus3-Disk1",   FT_DSK,      FALSE },
                    { "MdrvFolder",    RS_ISMDRD,   FALSE },
                    { "Mdrv1",         FT_MDR,      FALSE }};
short NumRSPECOptions = sizeof (RSPECOptions) / sizeof (struct RSPEC_s);

/**********************************************************************************************************************************/
/* Define the function prototypes                                                                                                 */
/**********************************************************************************************************************************/

int  HandleAddEntry  (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleDelEntry  (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleSave      (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleReload    (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandlePlay      (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleConfig    (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleShell     (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleScreen    (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleList      (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleScrShot   (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandlePrint     (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleInfo      (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleEditInfo  (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleInlay     (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleAbout     (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleSysInfo   (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleSelMem    (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleSelEmul   (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleSelMulti  (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleSelLang   (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleSelOrigin (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleSelFile   (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandlePoke      (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleEditPoke  (struct WindowInfo *Window, struct ButtonInfo *Button);
int  HandleHelp      (struct WindowInfo *Window, struct ButtonInfo *Button);

/**********************************************************************************************************************************/
/* Define the global variables                                                                                                    */
/**********************************************************************************************************************************/

char                    ConvGamePath[MAXDIR + 6];
char                    ConvDBasePath[MAXDIR + 6];
char                    SMemory[7];
char                    SEmulOverride[5];
char                    SMultiLoad[5];
char                    SScore[4];
struct FontInfo         SpecFont     = {"SPECTRUM", 8, 8};
struct FontInfo         UsedMenuFont = {"8X10", 8, 10};
struct WindowInfo       MainWindow   = {329, 27, 300, 440, 300, 440, 300, 440, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE,
                                        NULL, WINF_HUSE | WINF_HCENTER | WINF_BACKGROUND | WINF_TWRAP | WINF_TWSMART, NULL,
                                        "GAME INFORMATION"};
struct WindowInfo       OverlayWind  = {10, 27, 244, 440, 244, 440, 244, 440, 1, WINB_OUT | WINB_SURROUND, DBLACK, DWHITE, NULL,
                                        WINF_HUSE | WINF_HCENTER | WINF_BACKGROUND, NULL, "SELECTED GAME"};
struct ButtonInfo       TTotals      = {BT_TEXT, BC_TEXT, BF_FWIDTH | BF_FITTXT, 6, 400, 36, DBLACK, DWHITE, NULL, 0,
                                        TotalText, NULL};
struct ButtonInfo       TCriterion   = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_FITTXT, 18, 420, 34, DBLACK, DWHITE, NULL, 0,
                                        Criterion, NULL};
struct ButtonInfo       TDBName      = {BT_TEXT, BC_TEXT, BF_FWIDTH | BF_FITDIR, 1, 428, 47, DBLACK, DWHITE, NULL, 0,
                                        ConvDBasePath, NULL};
struct ButtonInfo       TDBChanged   = {BT_TEXT, BC_GADGET, BF_FWIDTH, 290, 428, 1, DRED, DWHITE, NULL, 0, "\023", NULL};
struct ButtonInfo       BName        = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 10, 36, DBLACK, DWHITE,
                                        NULL, 1, CurrentEntry.Name, NULL};
struct ButtonInfo       BPublisher   = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 30, 36, DBLACK, DWHITE,
                                        NULL, 2, CurrentEntry.Publisher, NULL};
struct ButtonInfo       BAuthor      = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 50, 36, DBLACK, DWHITE,
                                        NULL, 42, CurrentEntry.Author, NULL};
struct ButtonInfo       BYear        = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 70, 4, DBLACK, DWHITE,
                                        NULL, 3, CurrentEntry.Year, NULL};
struct ButtonInfo       BMemory      = {BT_TEXT, BC_TEXT, BF_FWIDTH | BF_FITTXT, 240, 70, 6, DBLACK, DWHITE,
                                        NULL, 0, SMemory, NULL};
struct ButtonInfo       BMemSel      = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 283, 70, 1, DBLACK, DWHITE,
                                        NULL, 4, "\012", HandleSelMem};
struct ButtonInfo       BPlayers     = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 90, 1, DBLACK, DWHITE,
                                        NULL, 5, CurrentEntry.Players, NULL};
struct ButtonInfo       BTogether    = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR | BF_GHOSTED, 85, 90, 1, DBLACK, DWHITE,
                                        NULL, 6, &CurrentEntry.Together, NULL};
struct ButtonInfo       BScore       = {BT_INPUT, BC_NUMBER, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 240, 90, 3, DBLACK, DWHITE,
                                        NULL, 41, SScore, NULL};
struct ButtonInfo       BPCName      = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITDIR, 72, 110, 12, DBLACK, DWHITE,
                                        NULL, 7, CurrentEntry.PCName, NULL};
struct ButtonInfo       BPCNSel      = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 151, 110, 1, DBLACK, DWHITE,
                                        NULL, 0, "\012", HandleSelFile};
struct ButtonInfo       BType        = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 240, 110, 7, DBLACK, DWHITE,
                                        NULL, 8, CurrentEntry.Type, NULL};
struct ButtonInfo       BStick1      = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 72, 130, 0, DBLACK, DWHITE,
                                        NULL, 9, &CurrentEntry.Sticks[0], NULL};
struct ButtonInfo       BStick2      = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 116, 130, 0, DBLACK, DWHITE,
                                        NULL, 10, &CurrentEntry.Sticks[1], NULL};
struct ButtonInfo       BStick3      = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 128, 130, 0, DBLACK, DWHITE,
                                        NULL, 11, &CurrentEntry.Sticks[2], NULL};
struct ButtonInfo       BStick4      = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 162, 130, 0, DBLACK, DWHITE,
                                        NULL, 12, &CurrentEntry.Sticks[3], NULL};
struct ButtonInfo       BStick5      = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 204, 130, 0, DBLACK, DWHITE,
                                        NULL, 13, &CurrentEntry.Sticks[4], NULL};
struct ButtonInfo       BLangSel     = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 97, 190, 1, DBLACK, DWHITE,
                                        NULL, 40, "\012", HandleSelLang};
struct ButtonInfo       BOrigScreen  = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 72, 150, 0, DBLACK, DWHITE,
                                        NULL, 30, &CurrentEntry.OrigScreen, NULL};
struct ButtonInfo       BOrigin      = {BT_TEXT, BC_TEXT, BF_FWIDTH | BF_FITTXT, 240, 150, 1, DBLACK, DWHITE,
                                        NULL, 0, &CurrentEntry.Origin, NULL};
struct ButtonInfo       BOriginSel   = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 253, 150, 1, DBLACK, DWHITE,
                                        NULL, 31, "\012", HandleSelOrigin};
struct ButtonInfo       BAYSound     = {BT_BOOL, BC_GADGET, BF_KEYSELECT | BF_BCLEAR, 72, 170, 0, DBLACK, DWHITE,
                                        NULL, 36, &CurrentEntry.AYSound, NULL};
struct ButtonInfo       BMultiLoad   = {BT_TEXT, BC_TEXT, BF_FWIDTH | BF_FITTXT, 240, 170, 4, DBLACK, DWHITE,
                                        NULL, 37, SMultiLoad, NULL};
struct ButtonInfo       BMultiSel    = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 271, 170, 1, DBLACK, DWHITE,
                                        NULL, 38, "\007", HandleSelMulti};
struct ButtonInfo       BLanguage    = {BT_INPUT, BC_TEXT, BF_FWIDTH | BF_KEYSELECT | BF_FITTXT, 72, 190, 3, DBLACK, DWHITE,
                                        NULL, 39, CurrentEntry.Language, NULL};
struct ButtonInfo       BEmulOver    = {BT_TEXT, BC_TEXT, BF_FWIDTH | BF_FITTXT, 240, 190, 4, DBLACK, DWHITE,
                                        NULL, 34, SEmulOverride, NULL};
struct ButtonInfo       BEmulSel     = {BT_PUSH, BC_GADGET, BF_FWIDTH | BF_KEYSELECT, 271, 190, 1, DBLACK, DWHITE,
                                        NULL, 35, "\012", HandleSelEmul};
struct ButtonInfo       BPath        = {BT_TEXT, BC_TEXT, BF_FWIDTH | BF_FITDIR, 72, 210, 27, DBLACK, DWHITE,
                                        NULL, 0, ConvGamePath, NULL};
struct ButtonInfo       BFileSize    = {BT_TEXT, BC_TEXT, BF_FWIDTH | BF_FITTXT, 246, 210, 7, DBLACK, DWHITE,
                                        NULL, 0, CurrentEntry.FileSize, NULL};
struct ButtonInfo       MAddEntry    = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 0, 8, DBLACK, DWHITE,
                                        NULL, 14, "@Add", HandleAddEntry};
struct ButtonInfo       MDelEntry    = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 18, 8, DBLACK, DWHITE,
                                        NULL, 15, "@Delete", HandleDelEntry};
struct ButtonInfo       MSave        = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 36, 8, DBLACK, DWHITE,
                                        NULL, 16, "@Save", HandleSave};
struct ButtonInfo       MReload      = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 54, 8, DBLACK, DWHITE,
                                        NULL, 17, "@Reload", HandleReload};
struct ButtonInfo       MPrint       = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 72, 8, DBLACK, DWHITE,
                                        NULL, 29, "Prin@t", HandlePrint};
struct ButtonInfo       MPlay        = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_BOLD, -64, 106, 8, DBLACK, DWHITE,
                                        NULL, 18, "@Play", HandlePlay};
struct ButtonInfo       MPoke        = {BT_PUSH, BC_TEXT, BF_FWIDTH | BF_BOLD, -64, 124, 8, DBLACK, DWHITE,
                                        NULL, 32, "Poke", HandlePoke};
struct ButtonInfo       MConfig      = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 158, 8, DBLACK, DWHITE,
                                        NULL, 19, "@Config", HandleConfig};
struct ButtonInfo       MShell       = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 176, 8, DBLACK, DWHITE,
                                        NULL, 20, "S@hell", HandleShell};
struct ButtonInfo       MQuit        = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 194, 8, DBLACK, DWHITE,
                                        NULL, 21, "@Quit", NULL};
struct ButtonInfo       MScreen      = {BT_TOGGLE, BC_TEXT, BF_FWIDTH, -64, 228, 8, DBLACK, DWHITE,
                                        NULL, 22, "Scr@een", HandleScreen};
struct ButtonInfo       MList        = {BT_TOGGLE, BC_TEXT, BF_FWIDTH, -64, 246, 8, DBLACK, DWHITE,
                                        NULL, 23, "@List", HandleList};
struct ButtonInfo       MScrShot     = {BT_TOGGLE, BC_TEXT, BF_FWIDTH, -64, 264, 8, DBLACK, DWHITE,
                                        NULL, 24, "ScrSh@ot", HandleScrShot};
struct ButtonInfo       MInlay       = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 298, 8, DBLACK, DWHITE,
                                        NULL, 42, "Inla@y", HandleInlay};
struct ButtonInfo       MInfo        = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 316, 8, DBLACK, DWHITE,
                                        NULL, 25, "@Info", HandleInfo};
struct ButtonInfo       MEditInfo    = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 334, 8, DBLACK, DWHITE,
                                        NULL, 26, "EdIn@fo", HandleEditInfo};
struct ButtonInfo       MEditPoke    = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 352, 8, DBLACK, DWHITE,
                                        NULL, 33, "EdPoke", HandleEditPoke};
struct ButtonInfo       MHelp        = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 386, 8, DBLACK, DWHITE,
                                        NULL, 36, "Help", HandleHelp};
struct ButtonInfo       MAbout       = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 404, 8, DBLACK, DWHITE,
                                        NULL, 27, "A@bout", HandleAbout};
struct ButtonInfo       MSysInfo     = {BT_PUSH, BC_TEXT, BF_FWIDTH, -64, 422, 8, DBLACK, DWHITE,
                                        NULL, 28, "SysInfo", HandleSysInfo};

struct ComboInfo        GameSelectWindow;
struct ComboInfo        TAPIndex;
  
bool                    HaveSoundcard        = TRUE;                                                   /* (Command line arg '-s') */
bool                    HaveMixer            = TRUE;                                                   /* (Command line arg '-m') */
bool                    ForcedInsert         = FALSE;
bool                    SpectrumScreen       = FALSE;
bool                    ScreenIsScrShot      = FALSE;                                       /* TRUE if a ScrShot is on the screen */
char                    CurrentPath[MAXDIR];
char                    PathBuffer[MAXDIR];
char                    LastAddPath[MAXDIR];
char                    LastAddFile[13];
bool                    LastAddFileSet       = FALSE;
char                    TypeBuffer[37]       = "\0";
char                    ShrTypeBuffer[37];
byte                    TypeIndex            = 0;
bool                    GameButtonsSetup     = FALSE;
bool                    PlayGamePoked        = FALSE;
bool                    BestScreenLoop       = FALSE;
bool                    EmulatorChanged      = FALSE;

void GetEmulatorName (short EmulatorType, char *Name)

/**********************************************************************************************************************************/
/* Pre   : `EmulatorType' is one of the ET_xxx values.                                                                            */
/* Post  : The 4-character emulator name is returned in `Name'.                                                                   */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  switch (EmulatorType)
  {
    case ET_NONE    : strcpy (Name, "none"); break;
    case ET_Z80     : strcpy (Name, "Z80"); break;
    case ET_JPP     : strcpy (Name, "JPP"); break;
    case ET_WAR     : strcpy (Name, "WARA"); break;
    case ET_X128    : strcpy (Name, "X128"); break;
    case ET_WSPECEM : strcpy (Name, "WSPE"); break;
    case ET_ZX      : strcpy (Name, "ZX"); break;
    case ET_SPANSPEC: strcpy (Name, "SPAN"); break;
    case ET_ZX32    : strcpy (Name, "ZX32"); break;
    case ET_R80     : strcpy (Name, "R80"); break;
    case ET_RUS     : strcpy (Name, "RUS"); break;
    case ET_YUD     : strcpy (Name, "YUD"); break;
    case ET_RSPEC   : strcpy (Name, "RSPE"); break;
  }
}

short GetDropDownEmulator (struct WindowInfo *HostWindow, short StartX, short StartY, short OldValue, bool AllowNone)

/**********************************************************************************************************************************/
/* Pre   : `HostWindow' points to the window to stick the drop-down menu to, `StartX', `StartY' hold the top-left coordinate      */
/*         relative to the HostWindow to put the menu, `OldValue' holds the previous value, `AllowNone' is TRUE if option 'none'  */
/*         should be available as well.                                                                                           */
/* Post  : A list of all supported emulators has been shown. The return value is the selected emulator.                           */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
#define EMUFWIDTH  33
static char d_EmulatorNames[MAXETYPE + 1][EMUFWIDTH + 1] = {{ "none                       - none" },
                                                            { "Z80                        - Z80 " },
                                                            { "JPP                        - JPP " },
                                                            { "Warajevo Spectrum Emulator - WARA" },
                                                            { "X128                       - X128" },
                                                            { "WSpecEm                    - WSPE" },
                                                            { "ZX                         - ZX  " },
                                                            { "Spanish Spectrum Emulator  - SPAN" },
                                                            { "ZX-32                      - ZX32" },
                                                            { "R80                        - R80 " },
                                                            { "Shalayev's Russian Emul    - RUS " },
                                                            { "Yudin's Russian Emul       - YUD " },
                                                            { "RealSpectrum               - RSPE" }};
  struct ComboInfo  PWindow;
  struct ComboEntry PEntry;
  struct ButtonInfo *ButtonP;
  short   register  Cnt;
  bool              Finished = FALSE;
  comboent          SelectedLine;
  short             ReturnVal;

  MakeComboWindow (&PWindow, HostWindow, COMT_SELECT, COMF_VSLIDER | COMF_SYNCSLIDER | COMF_KEYSELECT | COMF_EXCLUSIVE,
                   HostWindow->StartX + StartX, HostWindow->StartY + StartY, 0, 1024, EMUFWIDTH + 2, EMUFWIDTH,
                   MAXETYPE + (AllowNone ? 1 : 0), NULL, FALSE, 0, NULL, DBLACK, DWHITE, 0, 0);
  for (Cnt = (AllowNone ? 0 : 1) ; Cnt < MAXETYPE + 1 ; Cnt ++)
  {
    PEntry.Selected = 0;
    PEntry.Identifier = Cnt;
    strcpy (PEntry.Text, d_EmulatorNames[Cnt]);
    if (!InsertComboEntry (&PWindow, &PEntry, NULL, -1))
      FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
  }
  PWindow.LastSelected = OldValue + (AllowNone ? 1 : 0);
  PrintComboEntries (&PWindow);
  ReturnVal = OldValue;
  while (!Finished)
  {
    MouseStatus ();
    HandleWindow (&(PWindow.ComboWindow), &ButtonP);                                                  /* (Just read the keyboard) */
    if (HandleComboWindow (&PWindow, &SelectedLine) == WINH_LINEFIRST)
    {
      Finished = TRUE;
      ReturnVal = PWindow.LastSelected - (AllowNone ? 1 : 0);
      GlobalKey = 0x0000;
    }
    else
    {
      if (Mouse.Xx < PWindow.OverlayWindow.A_StartX || Mouse.Xx > PWindow.OverlayWindow.A_EndX ||
          Mouse.Yy < PWindow.OverlayWindow.A_StartY || Mouse.Yy > PWindow.OverlayWindow.A_EndY)
        if (Mouse.Left)                                                                    /* Left click outside the select box ? */
        {
          Finished = TRUE;
          DelayLeftMouseButton ();
        }
    }
    if (GlobalKey == K_ESC)
    {
      Finished = TRUE;
      GlobalKey = 0x0000;
    }
  }
  DestroyComboWindow (&PWindow, 0);
  GlobalKey = 0x0000;
  return (ReturnVal);
}

void SurroundScreen (struct WindowInfo *Window, short StartX, short StartY)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the window where the screen is, `StartX' and `StartY' should both be 2 pixels less than the screen. */
/* Post  : A shadow box has been drawn around the screen.                                                                         */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  short EndX;
  short EndY;

  EndX = StartX + 259;
  EndY = StartY + 195;
  WDrawLine (Window, SystemColors.ShadowDark, StartX, StartY, EndX, StartY);
  WDrawLine (Window, SystemColors.ShadowDark, StartX, StartY, StartX, EndY);
  WDrawLine (Window, SystemColors.ShadowLight, StartX, EndY, EndX, EndY);
  WDrawLine (Window, SystemColors.ShadowLight, EndX, StartY, EndX, EndY);
}

void GhostEmptyDbase (void)

/**********************************************************************************************************************************/
/* Pre   : The database is empty.                                                                                                 */
/* Post  : All appropriate buttons and menu entries have been ghosted. An empty entry has been presented to the user.             */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  if (TAPListing)
  {
    DestroyComboWindow (&TAPIndex, 0);
    TAPListing = FALSE;
  }
  ClearMainLower ();
  MPrint.Flags |= BF_GHOSTED;                                                          /* Cannot do this if the database is empty */
  MPlay.Flags |= BF_GHOSTED;
  MSave.Flags |= BF_GHOSTED;
  MDelEntry.Flags |= BF_GHOSTED;
  MScreen.Flags |= BF_GHOSTED;
  MList.Flags |= BF_GHOSTED;
  MScrShot.Flags |= BF_GHOSTED;
  MInfo.Flags |= BF_GHOSTED;
  MInlay.Flags |= BF_GHOSTED;
  MEditInfo.Flags |= BF_GHOSTED;
  MEditPoke.Flags |= BF_GHOSTED;
  MPoke.Flags |= BF_GHOSTED;
  UpdateButtonText (&MainWindow, &MPrint, 0);
  UpdateButtonText (&MainWindow, &MDelEntry, 0);
  UpdateButtonText (&MainWindow, &MPlay, 0);
  UpdateButtonText (&MainWindow, &MSave, 0);
  UpdateButtonText (&MainWindow, &MScreen, 0);
  UpdateButtonText (&MainWindow, &MList, 0);
  UpdateButtonText (&MainWindow, &MScrShot, 0);
  UpdateButtonText (&MainWindow, &MInlay, 0);
  UpdateButtonText (&MainWindow, &MInfo, 0);
  UpdateButtonText (&MainWindow, &MEditInfo, 0);
  UpdateButtonText (&MainWindow, &MEditPoke, 0);
  UpdateButtonText (&MainWindow, &MPoke, 0);
  memset (&CurrentEntry, 0, sizeof (CurrentEntry));
  ConvGamePath[0] = '\0';
  SMemory[0] = SMultiLoad[0] = SEmulOverride[0] = SScore[0] = '\0';
  UpdateButtonText (&MainWindow, &BName, BF_FITTXT);                                                     /* Update the MainWindow */
  UpdateButtonText (&MainWindow, &BPublisher, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BAuthor, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BYear, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BPlayers, BF_FITTXT);
  BTogether.Flags |= BF_GHOSTED;
  UpdateButtonText (&MainWindow, &BTogether, 0);
  UpdateButtonText (&MainWindow, &BScore, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BPCName, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BMemory, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BType, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BStick1, 0);
  UpdateButtonText (&MainWindow, &BStick2, 0);
  UpdateButtonText (&MainWindow, &BStick3, 0);
  UpdateButtonText (&MainWindow, &BStick4, 0);
  UpdateButtonText (&MainWindow, &BStick5, 0);
  UpdateButtonText (&MainWindow, &BOrigScreen, 0);
  UpdateButtonText (&MainWindow, &BOrigin, 0);
  UpdateButtonText (&MainWindow, &BAYSound, 0);
  UpdateButtonText (&MainWindow, &BMultiLoad, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BLanguage, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BEmulOver, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BPath, BF_FITDIR);
  UpdateButtonText (&MainWindow, &BFileSize, BF_FITTXT);
  ClearDBEntry (&CurrentEntry);
  GhostMenuEmptyDbase ();                                                               /* Ghost appropriate menu entries as well */
}

void UnghostDbase (void)

/**********************************************************************************************************************************/
/* Pre   : The database is no longer empty.                                                                                       */
/* Post  : All appropriate buttons and menu entries have been unghosted.                                                          */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MPlay.Flags &= ~BF_GHOSTED;                                                                 /* These options come available now */
  MSave.Flags &= ~BF_GHOSTED;
  MPrint.Flags &= ~BF_GHOSTED;
  MDelEntry.Flags &= ~BF_GHOSTED;
  MScreen.Flags &= ~BF_GHOSTED;
  MList.Flags &= ~BF_GHOSTED;
  MScrShot.Flags &= ~BF_GHOSTED;
  MInlay.Flags &= ~BF_GHOSTED;
  MInfo.Flags &= ~BF_GHOSTED;
  MEditInfo.Flags &= ~BF_GHOSTED;
  MEditPoke.Flags &= ~BF_GHOSTED;
  MPoke.Flags &= ~BF_GHOSTED;
  UpdateButtonText (&MainWindow, &MDelEntry, 0);
  UpdateButtonText (&MainWindow, &MSave, 0);
  UpdateButtonText (&MainWindow, &MPrint, 0);
  UpdateButtonText (&MainWindow, &MPlay, 0);
  UpdateButtonText (&MainWindow, &MScreen, 0);
  UpdateButtonText (&MainWindow, &MList, 0);
  UpdateButtonText (&MainWindow, &MScrShot, 0);
  UpdateButtonText (&MainWindow, &MInlay, 0);
  UpdateButtonText (&MainWindow, &MInfo, 0);
  UpdateButtonText (&MainWindow, &MEditInfo, 0);
  UpdateButtonText (&MainWindow, &MEditPoke, 0);
  UpdateButtonText (&MainWindow, &MPoke, 0);
  UnghostMenu ();                                                                            /* Activate the menu entries as well */
}

void RescueHandler (int ExitCode, char *ErrorMessage)

/**********************************************************************************************************************************/
/* Pre   : This is the pre-fatal anything-but-low-memory handler.                                                                 */
/* Post  : The database has been rescued if changed and the user accepted.                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  unsigned Dummy;

  ExitCode = ExitCode;
  StoreEntry ();
  if (DatabaseChanged)
  {
    SoundErrorBeeper (BEEP_LONG);
    if (!QuestionRequester (-1, -1, FALSE, FALSE, NULL, NULL, NULL, NULL, NULL, "Rescue Database",
        "%s\n\nTry to rescue changed database to %s ?", ErrorMessage, DBSAVE))
    {
      _dos_setdrive (CurDrv, &Dummy);                                                       /* Return to home drive and directory */
      chdir (CurPath);
      WriteDatabase (TRUE, TRUE);
      sprintf (ErrorMessage, "Database rescued in %s", DBSAVE);
    }
    else
      sprintf (ErrorMessage, "All changes are lost");
  }
}

void LowMemHandler (int MemoryType, char *ErrorMessage)

/**********************************************************************************************************************************/
/* Pre   : This is the pre-fatal low memory handler.                                                                              */
/* Post  : The database has been rescued if changed and the user accepted.                                                        */
/* Import: RescueHandler.                                                                                                         */
/**********************************************************************************************************************************/

{
  MemoryType = MemoryType;
  StoreEntry ();
  if (TAPListing)
    DestroyComboWindow (&TAPIndex, 0);
  DestroyComboWindow (&GameSelectWindow, 0);                   /* Destroy game select window to free a lot of conventional memory */
  RescueHandler (1, ErrorMessage);                                                                          /* (Memory error = 1) */
}

bool TestFileOverwrite (char *Path)

/**********************************************************************************************************************************/
/* Pre   : `Path' points to the full path of the file to tested for existence.                                                    */
/* Post  : If the file existed, the user has been asked for confirmation. FALSE if returned if it didn't exist or the user        */
/*         accepted to overwrite.                                                                                                 */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct stat    StatInfo;
  short register Cnt;

  if (!DBConfig.ConfirmOverwrite)                                                                                 /* Don't care ? */
    return (FALSE);
  Cnt = strlen (Path);
  while (-- Cnt > 0 && Path[Cnt] != '\\')                                                                    /* Find the filename */
    ;
  if (!stat (Path, &StatInfo))                                                                                   /* File exists ? */
    return (QuestionRequester (-1, -1, FALSE, FALSE, "@Ok", "@Cancel", NULL, NULL, NULL, "Confirm Overwrite",
            "File %s exists!\nReplace the existing file?", Path + Cnt + 1, StatInfo.st_size));
  else
    return (FALSE);
}

bool CompatibleFileType (void)

/**********************************************************************************************************************************/
/* Pre   : `FileType' must have been determined, `CurrentEntry' must hold this game.                                              */
/* Post  : TRUE is returned if this FileType is supported by the current emulator.                                                */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  char TestEmulType;

  if (FileType == FT_UNKNOWN)
    TestEmulType = (CurrentEntry.OverrideEmulator == ET_NONE ? DBConfig.EmulType : CurrentEntry.OverrideEmulator);
  else if (DBConfig.LinkExtension[FileType] == ET_NONE)
    TestEmulType = (CurrentEntry.OverrideEmulator == ET_NONE ? DBConfig.EmulType : CurrentEntry.OverrideEmulator);
  else
    TestEmulType = (CurrentEntry.OverrideEmulator == ET_NONE ? DBConfig.LinkExtension[FileType] : CurrentEntry.OverrideEmulator);
  switch (TestEmulType)
  {
    case ET_X128    : return (FileType == FT_Z80 || FileType == FT_SNA || FileType == FT_VOC || FileType == FT_MDR ||
                              FileType == FT_SLT || FileType == FT_TAP || FileType == FT_TZX || FileType == FT_TRD ||
                              FileType == FT_DSK);
    case ET_Z80     : return (FileType == FT_Z80 || FileType == FT_SNA || FileType == FT_VOC || FileType == FT_MDR ||
                              FileType == FT_SLT || FileType == FT_TAP || FileType == FT_TZX);
    case ET_JPP     : return (FileType == FT_SNA);
    case ET_WAR     : return (FileType == FT_Z80 || FileType == FT_SNA || FileType == FT_VOC || FileType == FT_MDR ||
                              FileType == FT_SLT || FileType == FT_TAP || FileType == FT_TZX || FileType == FT_SP ||
                              FileType == FT_LTP);
                              //(FileType == FT_TAP && !DBConfig.OnlyZ80Tapes));
    case ET_WSPECEM : return (FileType == FT_Z80 || FileType == FT_SNA || FileType == FT_VOC || FileType == FT_SEM ||
                              FileType == FT_SLT || FileType == FT_TAP || FileType == FT_SP);
    case ET_ZX      : return (FileType == FT_SNA || FileType == FT_SNP || FileType == FT_TAP || FileType == FT_LTP);
    case ET_SPANSPEC: return (FileType == FT_SP);
    case ET_ZX32    : return (FileType == FT_Z80 || FileType == FT_SNA || FileType == FT_TAP || FileType == FT_TZX ||
                              FileType == FT_DSK);
    case ET_R80     : return (FileType == FT_Z80 || FileType == FT_SNA || FileType == FT_TAP || FileType == FT_TZX ||
                              FileType == FT_TRD);
    case ET_RUS     : return (FileType == FT_Z80 || FileType == FT_SNA || FileType == FT_TRD);
    case ET_YUD     : return (FileType == FT_Z80 || FileType == FT_SNA || FileType == FT_TRD || FileType == FT_FDI);
    case ET_RSPEC   : return (FileType == FT_Z80 || FileType == FT_SNA || FileType == FT_MDR || FileType == FT_TAP ||
                              FileType == FT_TZX || FileType == FT_TRD || FileType == FT_DSK);
    default         : return (FALSE);
  }
}

void ValidateAnEntry (comboent EntryNumber, bool AdjustTable, bool AdjustScreen)

/**********************************************************************************************************************************/
/* Pre   : `EntryNumber' holds the number of the entry to be validated, or -1 if it is the current entry.                         */
/*         `AdjustTable' is TRUE if the PLAYBIT bits in the selectwindow must be adjusted as well.                                */
/* Post  : The requested entry has been validated.                                                                                */
/* Import: StoreEntry, CompatibleFileType.                                                                                        */
/**********************************************************************************************************************************/

{
  if (EntryNumber != -1)                                                                                 /* Get the entry first ? */
  {
    GetDBEntry (&CurrentEntry, DBLEN, EntryNumber);
    SelectedGame = EntryNumber;
  }
  if (AdjustTable)
  {
    GetComboEntry (&GameSelectWindow, &CurrentSEntry, SelectedGame);
    CurrentSEntry.Selected &= ~PLAYBIT;                                                        /* Turn off playbit for the moment */
  }
  if (!(GamePathFound = ResolveFileLocation (&CurrentEntry, GamePath, &EntryChanged)))                      /* Get full game path */
    FileType = FT_UNKNOWN;
  else
  {
    if (EntryChanged)
      SignalDBChanged (TRUE);
    MakeFullPath (FullPath, GamePath, CurrentEntry.PCName);
    if (IsFloppy (FullPath[0]) && DBConfig.AssumeStableFloppies)
      GetFileType (CurrentEntry.PCName, FALSE);
    else
      GetFileType (CurrentEntry.PCName, TRUE);
  }
  StoreEntry ();                                                                                    /* Store XMS entry if changed */
  if (AdjustTable)                                                                               /* Adjust playbit for this entry */
  {
    CurrentSEntry.Selected |= (CompatibleFileType () ? 0x00 : PLAYBIT);
    PutComboEntry (&GameSelectWindow, &CurrentSEntry, SelectedGame, AdjustScreen);
  }
}

bool ValidateEntries (bool All)

/**********************************************************************************************************************************/
/* Pre   : `All' is TRUE if all entries must be validated, FALSE if only the selected entries must be done.                       */
/* Post  : The chosen entries have been validated.                                                                                */
/* Import: ValidateAnEntry, SelectGame.                                                                                           */
/**********************************************************************************************************************************/

{
  struct PReqInfo   PReq;
  comboent register ECnt;
  comboent          OldSelectedGame;
  struct ComboEntry NEntry;

  OldSelectedGame = SelectedGame;
  MakePRequester (&PReq, -1, -1, TRUE, NULL, "Validating Database", "Validating %s ...", All ? "database" : "selected entries");
  for (ECnt = 0 ; ECnt <= GameSelectWindow.NumEntries ; ECnt ++)                                          /* Validate the entries */
  {
    GetComboEntry (&GameSelectWindow, &NEntry, ECnt);
    if (All || (NEntry.Selected & SELECTBIT))
      ValidateAnEntry (ECnt, TRUE, FALSE);
    if (!UpdatePRequester (&PReq, (double)ECnt * 100 / (GameSelectWindow.NumEntries + 1), NULL))
    { DestroyPRequester (&PReq); SelectEntry (&GameSelectWindow, OldSelectedGame); SelectGame (OldSelectedGame); return (FALSE); }
    MouseStatus ();
  }
  DestroyPRequester (&PReq);
  SelectEntry (&GameSelectWindow, OldSelectedGame);                                      /* Ensure selected game is still correct */
  SelectGame (OldSelectedGame);
  return (TRUE);
}

void TruncateEntrytable (bool Soft)

/**********************************************************************************************************************************/
/* Pre   : There are too many games in XMS to fit in the entrytable.                                                              */
/* Post  : The last 175 games have been removed from the list.                                                                    */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Soft = Soft;
  DatabaseTruncated = TRUE;
  if (QuestionRequester (-1, -1, FALSE, FALSE, "@Ok", "@Quit", NULL, NULL, NULL, "Confirm Truncate", "Too many games - "
      "will be truncated to %d. Continue ?", GameSelectWindow.NumEntries + 1))
    FatalError (11, "Aborted");
}

void SetupNewData (bool FirstRun)

/**********************************************************************************************************************************/
/* Pre   : A (new) database has been read into XMS. `FirstRun' is TRUE if the buttons have not been drawn yet.                    */
/* Post  : The database has been included in the GameSelectWindow and printed.                                                    */
/* Import: ReselectEntries, ReportTotal, SelectGame, UnghostDbase, CompatibleFileType.                                            */
/**********************************************************************************************************************************/

{
  struct PReqInfo PReq;

  SetMouseShape (DefaultWaitingMouseShape);
  if (TAPListing)
  {
    DestroyComboWindow (&TAPIndex, 0);
    TAPListing = FALSE;
  }
  if (FirstRun)
    MakeComboWindow (&GameSelectWindow, NULL, COMT_SELECT, COMF_NOBUFFER | COMF_VSLIDER | COMF_KEYSELECT, 16, 40,
                     DBase.NumEntries + 1, 0, 37, 36, 38, NULL, FALSE, 0, NULL, DBLACK, DWHITE, SELECTBIT, 0);
  else
    ReInitComboWindow (&GameSelectWindow, DBase.NumEntries + 1);
  DatabaseTruncated = FALSE;                                                                         /* Assume we can read it all */
  NewEntry.Identifier = 0;
  if (DBConfig.ValidateAfterLoad)
    MakePRequester (&PReq, -1, -1, TRUE, NULL, "Validating Database", "Validating database ...");
  for (SelectedGame = 0 ; SelectedGame <= DBase.NumEntries ; SelectedGame ++)                          /* Rebuild the entry table */
  {
    GetDBEntry (&CurrentEntry, DBLEN, SelectedGame);
    strcpy (NewEntry.Text, CurrentEntry.Name);
    if (DBConfig.ValidateAfterLoad)                                                   /* All files must be tested for existence ? */
    {
      ValidateAnEntry (-1, FALSE, FALSE);                                                               /* Validate current entry */
      if (!UpdatePRequester (&PReq, (double)SelectedGame * 100 / (DBase.NumEntries + 1), NULL))
      { DestroyPRequester (&PReq); SetMouseShape (DefaultNormalMouseShape); DBConfig.ValidateAfterLoad = FALSE; }
      MouseStatus ();
    }
    else                                                                                   /* Nope, don't test, just get the type */
    {
      GetFileType (CurrentEntry.PCName, FALSE);
      if (!strcmp (CurrentEntry.DirIndex, DI_UNKNOWN))
        FileType = FT_UNKNOWN;
    }
    NewEntry.Selected = CompatibleFileType () ? 0x00 : PLAYBIT;
    if (!InsertComboEntry (&GameSelectWindow, &NewEntry, NULL, -1))
      TruncateEntrytable (TRUE);
  }
  if (DBConfig.ValidateAfterLoad)
    DestroyPRequester (&PReq);
  NumMatches = 0;
  ReselectEntries (&GameSelectWindow);
  ReportTotal ();
  if (GameSelectWindow.NumEntries >= 0)
  {
    SelectEntry (&GameSelectWindow, 0);
    TDBName.APen = DatabaseTruncated ? DRED : DBLACK;
    UpdateButtonText (&MainWindow, &TDBName, BF_FITDIR);
    UnghostDbase ();
  }
  switch (DBConfig.StartInfoButton)
  {
    case 0 : break;                                                                                    /* Don't activate a button */
    case 1 : ForceButtonPress (&MainWindow, &MScreen); break;                                    /* Startup with 'Screen' pressed */
    case 2 : ForceButtonPress (&MainWindow, &MList); break;                                        /* Startup with 'List' pressed */
    case 3 : ForceButtonPress (&MainWindow, &MScrShot); break;                                  /* Startup with 'ScrShot' pressed */
  }
  SetMouseShape (DefaultNormalMouseShape);
}

void SetDatabaseName (char *FileName, bool UpdateButton)

/**********************************************************************************************************************************/
/* Pre   : `FileName' holds the (new) name for the database to be handled.                                                        */
/*         `UpdateButton' is TRUE if the button to which the name is linked should be updated as well.                            */
/* Post  : The `DatabaseName', `DatabaseBackupName' and `DatabaseTemp' variables have been set to this name.                      */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  char Drive[4];
  char Path[MAXDIR];
  char FName[9];
  char Ext[5];

  if (NewDatabase)
  {
    strcpy (DatabaseName, FileName);
    strcpy (ConvDBasePath, FileName);
  }
  else
  {
    if (_fullpath (DatabaseName, FileName, MAXDIR) == NULL)
      FatalError (7, "Invalid path %s", FileName);
    ConvertPath (DatabaseName, ConvDBasePath, DatabaseFloppyId);
  }
  _splitpath (DatabaseName, Drive, Path, FName, Ext);
  _makepath (DatabaseTemp, Drive, Path, FName, DBTEMP);
  _makepath (DatabaseBackupName, Drive, Path, FName, DBBACKUP);
  if (UpdateButton)
    UpdateButtonText (&MainWindow, &TDBName, BF_FITDIR);
}

void TestEmptyDbase (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : If the database was empty, an empty record is added and the changed field is copied into it.                           */
/* Import: SelectGame, ReportTotal, UnghostDbase, ReselectAnEntry.                                                                */
/**********************************************************************************************************************************/

{
  struct DBaseEntry_s GraceEntry;

  if (GameSelectWindow.NumEntries == -1)                                                                   /* Database is empty ? */
  {
    memcpy (&GraceEntry, &CurrentEntry, sizeof (struct DBaseEntry_s));                           /* This stores the changed field */
    AddDBEntry ();                                                                                         /* Add an entry in XMS */
    memcpy (&CurrentEntry, &GraceEntry, sizeof (struct DBaseEntry_s));                           /* This copies the changed field */
    strcpy (NewEntry.Text, CurrentEntry.Name);
    NewEntry.Selected = PLAYBIT;                                                                /* No name yet - cannot be played */
    if (!InsertComboEntry (&GameSelectWindow, &NewEntry, NULL, -1))                                 /* And one in the entry table */
      FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
    SelectedGame = 0;
    UnghostDbase ();                                                                       /* Activate the buttons and menu strip */
    ReselectAnEntry (&GameSelectWindow, SelectedGame, &CurrentEntry, FALSE);                             /* Match the empty entry */
    PrintComboEntries (&GameSelectWindow);
    SelectGame (SelectedGame);                                                                        /* Make it the current game */
    ReportTotal ();
  }
}

void SignalDBChanged (bool Changed)

/**********************************************************************************************************************************/
/* Pre   : `Changed' holds the new DatabaseChanged value.                                                                         */
/* Post  : The flag has been exported and the 'changed' button has been updated.                                                  */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  DatabaseChanged = Changed;
  TDBChanged.Contents[0] = Changed ? '\011' : '\023';
  UpdateButtonText (&MainWindow, &TDBChanged, 0);
}

int GetFileType (char *FileName, bool TestZ80)

/**********************************************************************************************************************************/
/* Pre   : `FileName' points to the name to be tested, `TestZ80' is TRUE if Z80 files should be checked for validity.             */
/* Post  : The global variable `FileType' has been set according to the spectrum file. This value is also returned.               */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  char register *PCNameExt;

  PCNameExt = FileName + strlen (FileName) - 4;
  if (!strcmp (PCNameExt, ".Z80"))                                                                              /* Determine type */
    if (TestZ80)                                                                                           /* Test the Z80 file ? */
      FileType = (GetZ80Version (FullPath) ? FT_Z80 : FT_UNKNOWN);
    else
      FileType = FT_Z80;
  else if (!strcmp (PCNameExt, ".SLT"))                                                                /* Super Level-loader Trap */
    if (TestZ80)                                                                                           /* Test the Z80 file ? */
    {
      FileType = FT_UNKNOWN;
      if (GetZ80Version (FullPath))
        if (GetSLTExtensionOffset (FullPath))
          FileType = FT_SLT;
    }
    else
      FileType = FT_SLT;
  else if (!strcmp (PCNameExt, ".SNA")) FileType = FT_SNA;
  else if (!strcmp (PCNameExt, ".TAP")) FileType = FT_TAP;
  else if (!strcmp (PCNameExt, ".MDR")) FileType = FT_MDR;
  else if (!strcmp (PCNameExt, ".VOC")) FileType = FT_VOC;
  else if (!strcmp (PCNameExt, ".SNP")) FileType = FT_SNP;
  else if (!strcmp (PCNameExt, ".SEM")) FileType = FT_SEM;
  else if (!strcmp (PCNameExt, ".LTP")) FileType = FT_LTP;
  else if (!strcmp (PCNameExt, ".TRD")) FileType = FT_TRD;
  else if (!strcmp (PCNameExt, ".DSK")) FileType = FT_DSK;
  else if (!strcmp (PCNameExt, ".FDI")) FileType = FT_FDI;
  else if (!strcmp (PCNameExt, ".TZX"))
    if (TestZ80)                                                                                           /* Test the TZX file ? */
      FileType = (GetTZXVersion (FullPath) ? FT_TZX : FT_UNKNOWN);
    else
      FileType = FT_Z80;
  else if (!strcmp (FileName + strlen (FileName) - 3, ".SP")) FileType = FT_SP;
  else                                    FileType = FT_UNKNOWN;
  return (FileType);
}

void ReportTotal (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The total number of game entries is printed below the entry table.                                                     */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  TypeBuffer[0] = '\0';
  TypeIndex = 0;
  sprintf (TotalText, "Entries: %-5d  Matches: %-5d %5d", GameSelectWindow.NumEntries + 1, NumMatches,
           GameSelectWindow.LastSelected + 1);
  UpdateButtonText (&OverlayWind, &TTotals, BF_FITTXT);
  if (NumMatches)
    UnghostMenuNoMatches ();
  else
    GhostMenuNoMatches ();
}

void ClearMainLower (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The part of the MainWindow below the input fields has been cleared.                                                    */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  WDrawBox (&MainWindow, TRUE, MainWindow.BPen, 0, 224, MainWindow.Width - 1, MainWindow.Height - 14);
}

void TestPlayGhosted (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The `Play' button has been (un-)ghosted depending on the `Selected' field of the game in the entry table.              */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  struct ComboEntry NEntry;

  GetComboEntry (&GameSelectWindow, &NEntry, SelectedGame);
  if (!GamePathFound || NEntry.Selected & PLAYBIT)
    MPlay.Flags |= BF_GHOSTED;
  else
    MPlay.Flags &= ~BF_GHOSTED;
  UpdateButtonText (&MainWindow, &MPlay, MPlay.Flags);
}

void TestInfoGhosted (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The `Info' button has been (un-)ghosted depending whether an info file was found for the current game.                 */
/*         All `Info' variables have been set up (InfoFound, InfoDirExists, InfoDir, InfoPath).                                   */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  int  Fh;
  unsigned Dummy;

  InfoFound = FALSE;
  InfoIsZXEdit = FALSE;
  MEditInfo.Flags &= ~BF_GHOSTED;                                                                           /* Assume we can edit */
  if (FileType == FT_SLT && SLTMap.InfoIncluded)                                               /* SLT files have info as datatype */
  {
    InfoFound = TRUE;
    MakeFullPath (InfoPath, GamePath, CurrentEntry.PCName);
  }
  else if (GamePathFound)         
  {
    if (DBConfig.AllFilesInOneDir)
      strcpy (InfoDir, GamePath);
    else
      MakeFullPath (InfoDir, GamePath, INFODIR);
    HWEAbort = TRUE;                                            /* Return immediately if the next functions cause a hardware trap */
    HWEOccured = FALSE;
    _dos_setdrive (ToNumber (InfoDir[0]), &Dummy);                                                            /* Go to this drive */
    getcwd (PathBuffer, MAXDIR);
    InfoDirExists = !(chdir (InfoDir + 2));                                                       /* Can I go to that directory ? */
    if (HWEOccured)                                                                       /* Not found if a hardware trap occured */
      InfoDirExists = FALSE;
    HWEAbort = FALSE;
    strcpy (InfoPath, InfoDir);                                                      /* Build a full info path from the directory */
    strcat (InfoPath, "\\");
    strcat (InfoPath, CurrentEntry.PCName);                                                          /* Add name of the game file */
    strcpy (InfoPath + strlen (InfoPath) - 3, ZXEDEXT);                                       /* Change extension to ZX-Edit file */
    if (InfoDirExists)                                                                   /* Continue only if the directory exists */
    {
      if ((Fh = open (InfoPath, O_BINARY | O_RDONLY)) != -1)                                                     /* File exists ? */
      {
        close (Fh);
        InfoFound = TRUE;
        InfoIsZXEdit = TRUE;
      }
      else
      {
        strcpy (InfoPath + strlen (InfoPath) - 3, INFOEXT);                                      /* Change extension to info file */
        if ((Fh = open (InfoPath, O_BINARY | O_RDONLY)) != -1)                                                   /* File exists ? */
        {
          close (Fh);
          InfoFound = TRUE;
        }
        else if (DBConfig.AlwaysUseZXEdit)                                                     /* We want to always use ZX-Edit ? */
        {
          strcpy (InfoPath + strlen (InfoPath) - 3, ZXEDEXT);                            /* Change extension back to ZX-Edit file */
          InfoIsZXEdit = TRUE;
        }
      }
    }
    else if (!DBConfig.AlwaysUseZXEdit)
      strcpy (InfoPath + strlen (InfoPath) - 3, INFOEXT);
    chdir (PathBuffer);
    _dos_setdrive (CurDrv, &Dummy);
    chdir (CurPath);                                                                               /* Restore drive and directory */
  }
  else
    MEditInfo.Flags |= BF_GHOSTED;                                                         /* Cannot edit if no path was resolved */
  if (!InfoFound)
    MInfo.Flags |= BF_GHOSTED;
  else
    MInfo.Flags &= ~BF_GHOSTED;
  if (DBConfig.EditPath[0] == '\0' || (FileType == FT_SLT && SLTMap.InfoIncluded))                /* Ghosted if no editor defined */
    MEditInfo.Flags |= BF_GHOSTED;                                                         /* (Not implemented yet for SLT files) */
  UpdateButtonText (&MainWindow, &MInfo, MInfo.Flags);
  UpdateButtonText (&MainWindow, &MEditInfo, MEditInfo.Flags);
}

void TestInlayGhosted (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The `Inlay' button has been (un-)ghosted depending whether a JPG/GIF file was found for the current game.              */
/*         All `Inlay' variables have been set up (InlayFound, InlayDirExists, InlayDir, InlayPath).                              */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  int  Fh;
  unsigned Dummy;

  InlayFound = FALSE;
  if (GamePathFound)         
  {
    if (DBConfig.AllFilesInOneDir)
      strcpy (InlayDir, GamePath);
    else
      MakeFullPath (InlayDir, GamePath, INLAYDIR);
    HWEAbort = TRUE;                                            /* Return immediately if the next functions cause a hardware trap */
    HWEOccured = FALSE;
    _dos_setdrive (ToNumber (InlayDir[0]), &Dummy);                                                           /* Go to this drive */
    getcwd (PathBuffer, MAXDIR);
    InlayDirExists = !(chdir (InlayDir + 2));                                                     /* Can I go to that directory ? */
    if (HWEOccured)                                                                       /* Not found if a hardware trap occured */
      InlayDirExists = FALSE;
    HWEAbort = FALSE;
    strcpy (InlayPath, InlayDir);                                                    /* Build a full info path from the directory */
    strcat (InlayPath, "\\");
    strcat (InlayPath, CurrentEntry.PCName);                                                         /* Add name of the game file */
    strcpy (InlayPath + strlen (InlayPath) - 3, INLAYEXT1);                                       /* Change extension to JPG file */
    if (InlayDirExists)                                                                  /* Continue only if the directory exists */
    {
      if ((Fh = open (InlayPath, O_BINARY | O_RDONLY)) != -1)                                                    /* File exists ? */
      {
        close (Fh);
        InlayFound = TRUE;
      }
      else
      {
        strcpy (InlayPath + strlen (InlayPath) - 3, INLAYEXT2);                                   /* Change extension to GIF file */
        if ((Fh = open (InlayPath, O_BINARY | O_RDONLY)) != -1)                                                  /* File exists ? */
        {
          close (Fh);
          InlayFound = TRUE;
        }
      }
    }
    chdir (PathBuffer);
    _dos_setdrive (CurDrv, &Dummy);
    chdir (CurPath);                                                                               /* Restore drive and directory */
  }
  if (!InlayFound)
    MInlay.Flags |= BF_GHOSTED;
  else
    MInlay.Flags &= ~BF_GHOSTED;
  UpdateButtonText (&MainWindow, &MInlay, MInlay.Flags);
}

void TestScrShotGhosted (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The `ScrShot' button has been (un-)ghosted depending whether a screenshot file was found for the current game.         */
/*         All `ScrShot' variables have been set up (ScrShotFound, ScrShotDirExists, ScrShotDir, ScrShotPath).                    */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  int  Fh;
  unsigned Dummy;

  ScrShotFound = FALSE;
  if (GamePathFound)
  {
    if (DBConfig.AllFilesInOneDir)
      strcpy (ScrShotDir, GamePath);
    else
      MakeFullPath (ScrShotDir, GamePath, SCRSHOTDIR);
    HWEAbort = TRUE;                                            /* Return immediately if the next functions cause a hardware trap */
    HWEOccured = FALSE;
    _dos_setdrive (ToNumber (ScrShotDir[0]), &Dummy);                                                         /* Go to this drive */
    getcwd (PathBuffer, MAXDIR);
    ScrShotDirExists = !(chdir (ScrShotDir + 2));                                                 /* Can I go to that directory ? */
    if (HWEOccured)                                                                       /* Not found if a hardware trap occured */
      ScrShotDirExists = FALSE;
    HWEAbort = FALSE;
    strcpy (ScrShotPath, ScrShotDir);                                                /* Build a full info path from the directory */
    strcat (ScrShotPath, "\\");
    strcat (ScrShotPath, CurrentEntry.PCName);                                                       /* Add name of the game file */
    strcpy (ScrShotPath + strlen (ScrShotPath) - 3, SCRSHOTEXT);                           /* Change extension to screenshot file */
  }
  if (FileType == FT_SLT && SLTMap.ScrShotIncluded)                                    /* SLT files have a screenshot as datatype */
    ScrShotFound = TRUE;
  else if (GamePathFound)         
    if (ScrShotDirExists)                                                                /* Continue only if the directory exists */
      if ((Fh = open (ScrShotPath, O_BINARY | O_RDONLY)) != -1)                                                  /* File exists ? */
      {
        close (Fh);
        ScrShotFound = TRUE;
      }
  if (!ScrShotFound)
    MScrShot.Flags |= BF_GHOSTED;
  else
    MScrShot.Flags &= ~BF_GHOSTED;
  UpdateButtonText (&MainWindow, &MScrShot, MScrShot.Flags);
  if (GamePathFound)
  {
    chdir (PathBuffer);
    _dos_setdrive (CurDrv, &Dummy);
    chdir (CurPath);                                                                               /* Restore drive and directory */
  }
}

void TestPokeGhosted (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The `Poke' button has been (un-)ghosted depending whether a poke file was found for the current game.                  */
/*         All `Poke' variables have been set up (static in dbpoke.c)                                                             */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  MEditPoke.Flags &= ~BF_GHOSTED;                                                                           /* Assume we can edit */
  if (GamePathFound)
  {
    if (TestPokesFileExists (FALSE))                                                                 /* No we have a pokes file ? */
      if (IsSnapshot (FileType))                                                                  /* And we're doing a snapshot ? */
      {
        MPoke.Flags &= ~BF_GHOSTED;                                                                             /* Unghost + bold */
        MPoke.Flags |= BF_BOLD;
      }
      else                                                                                                    /* (Not a snapshot) */
      {
        MPoke.Flags |= BF_GHOSTED;                                                                              /* Ghost + unbold */
        MPoke.Flags &= ~BF_BOLD;
      }
    else                                                                                                       /* (No pokes file) */
      MPoke.Flags |= BF_GHOSTED | BF_BOLD;                                                                        /* Ghost + bold */
    if (FileType == FT_SLT && SLTMap.PokesIncluded)                                              /* (Not implemented for SLT yet) */
      MEditPoke.Flags |= BF_GHOSTED;
  }
  else                                                                                                        /* (Game not found) */
  {
    MPoke.Flags |= BF_GHOSTED | BF_BOLD;
    MEditPoke.Flags |= BF_GHOSTED;
  }
  UpdateButtonText (&MainWindow, &MPoke, 0);
  UpdateButtonText (&MainWindow, &MEditPoke, MEditPoke.Flags);
}

bool TestAGame (comboent Num, bool TestZ80)

/**********************************************************************************************************************************/
/* Pre   : `Num' holds the game number in the entry table.                                                                        */
/* Post  : The `Selected' field of the game in the entry table has been set if the game cannot be played with the selected        */
/*         emulator type. If the field is now different, TRUE is returned.                                                        */
/* Import: CompatibleFileType.                                                                                                    */
/**********************************************************************************************************************************/

{
  struct ComboEntry NEntry;
  byte              OldSelected;

  GetComboEntry (&GameSelectWindow, &NEntry, Num);
  OldSelected = NEntry.Selected;
  NEntry.Selected &= ~PLAYBIT;
  GetFileType (CurrentEntry.PCName, TestZ80);
  if (CompatibleFileType ())                                                    /* File has a compatible type for this emulator ? */
  {
    if (EmulatorChanged && DBConfig.ValidateAtEmulChange)                                     /* Need to check the path as well ? */
      if (!TestFileLocation (&CurrentEntry))                                                    /* But the file cannot be found ? */
        NEntry.Selected |= PLAYBIT;                                                                     /* Game can not be played */
  }
  else
    NEntry.Selected |= PLAYBIT;                                                                         /* Game can not be played */
  PutComboEntry (&GameSelectWindow, &NEntry, Num, TRUE);
  return (NEntry.Selected != OldSelected);
}

void TestGames (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The `Selected' field of all games in the entry table has been set if the game cannot be played with the selected       */
/*         emulator type.                                                                                                         */
/* Import: TestAGame.                                                                                                             */
/**********************************************************************************************************************************/

{
  comboent register Cnt;

  SetMouseShape (DefaultWaitingMouseShape);
  for (Cnt = 0 ; Cnt <= GameSelectWindow.NumEntries ; Cnt ++)
  {
    GetDBEntry (&CurrentEntry, DBLEN, Cnt);
    TestAGame (Cnt, FALSE);
  }
  GetDBEntry (&CurrentEntry, DBLEN, SelectedGame);
  SetMouseShape (DefaultNormalMouseShape);
}

void StoreEntry (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : If the `CurrentEntry' has been changed, it is re-stored in XMS. If it was a new entry, it is re-sorted in the entry    */
/*         table as well.                                                                                                         */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  if (!EntryChanged || SelectedGame < 0)                                                         /* Not changed or empty database */
    return;
  PutDBEntry (&CurrentEntry, DBLEN, SelectedGame);                                                          /* Re-store the entry */
}

void SetupGameButtons (void)

/**********************************************************************************************************************************/
/* Pre   : None.                                                                                                                  */
/* Post  : The buttons that are unghosted after a lot of disk-I/O are handled here. This allows more relaxed floppy use.          */
/* Import: TestPlayGhosted, TestInfoGhosted, TestInlayGhosted, TestScrShotGhosted, TestPokeGhosted.                               */
/**********************************************************************************************************************************/

{
  if (GameButtonsSetup)                                                                                       /* Already set up ? */
    return;
  TestPlayGhosted ();
  TestInfoGhosted ();
  TestInlayGhosted ();
  TestScrShotGhosted ();
  TestPokeGhosted ();
  GameButtonsSetup = TRUE;
}

static void TimedFlash (struct WindowInfo *Window)

/**********************************************************************************************************************************/
/* Pre   : All used variables must have been set up for this particular game.                                                     */
/* Post  : This is the timed handler for handling of flash. Each run toggles the flashing parts.                                  */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (SpectrumScreen)
  {
    FlashState = !FlashState;
    PutSpectrumScreen (_BufferDecrunched, MainWindow.StartX + 21, MainWindow.StartY + 230);
  }
}

void SelectGame (comboent SelectedLine)

/**********************************************************************************************************************************/
/* Pre   : `SelectedLine' holds the entry number in the GameSelectWindow that has been selected.                                  */
/* Post  : The MainWindow has been adopted to reflect the current choise. The path to this game has been found.                   */
/* Import: StoreEntry, HandleScreen, HandleList, HandleScrShot, ClearMainLower.                                                   */
/**********************************************************************************************************************************/

{
  struct ComboEntry NEntry;
  byte              OldSelected;

  StoreEntry ();                                                                        /* Store any changes to the current entry */
  GetDBEntry (&CurrentEntry, DBLEN, SelectedLine);                                                         /* Fetch the new entry */
  SelectedGame = SelectedLine;
  UpdateButtonText (&MainWindow, &BName, BF_FITTXT);                                                     /* Update the MainWindow */
  UpdateButtonText (&MainWindow, &BPublisher, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BAuthor, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BYear, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BPlayers, BF_FITTXT);
  if (CurrentEntry.Players[0] != '1')
    BTogether.Flags &= ~BF_GHOSTED;
  UpdateButtonText (&MainWindow, &BTogether, 0);
  if (CurrentEntry.Score == 0)
    SScore[0] = '\0';
  else
    sprintf (SScore, "%d", CurrentEntry.Score);
  UpdateButtonText (&MainWindow, &BScore, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BPCName, BF_FITTXT);
  switch (CurrentEntry.Memory)
  {
    case  16 : strcpy (SMemory, "    16"); break;
    case 128 : strcpy (SMemory, "   128"); break;
    case 129 : strcpy (SMemory, "  USR0"); break;
    case 130 : strcpy (SMemory, "+2/+2A"); break;
    case 131 : strcpy (SMemory, "    +3"); break;
    case 176 : strcpy (SMemory, "48/128"); break;
    case 228 : strcpy (SMemory, "Pentag"); break;
    default  : strcpy (SMemory, "    48"); break;
  }
  UpdateButtonText (&MainWindow, &BMemory, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BType, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BStick1, 0);
  UpdateButtonText (&MainWindow, &BStick2, 0);
  UpdateButtonText (&MainWindow, &BStick3, 0);
  UpdateButtonText (&MainWindow, &BStick4, 0);
  UpdateButtonText (&MainWindow, &BStick5, 0);
  UpdateButtonText (&MainWindow, &BLanguage, BF_FITTXT);
  UpdateButtonText (&MainWindow, &BOrigScreen, 0);
  UpdateButtonText (&MainWindow, &BOrigin, 0);
  UpdateButtonText (&MainWindow, &BAYSound, 0);
  switch (CurrentEntry.MultiLoad)
  {
    case 1          : strcpy (SMultiLoad, "yes"); break;
    case 4          : strcpy (SMultiLoad, "48K"); break;
    default         : strcpy (SMultiLoad, "no"); break;
  }
  UpdateButtonText (&MainWindow, &BMultiLoad, BF_FITTXT);
  GetEmulatorName (CurrentEntry.OverrideEmulator, SEmulOverride);
  UpdateButtonText (&MainWindow, &BEmulOver, BF_FITTXT);
  if (CurrentEntry.Players[0] == '1')
    BTogether.Flags |= BF_GHOSTED;
  GetComboEntry (&GameSelectWindow, &NEntry, SelectedGame);
  OldSelected = NEntry.Selected;
  if (!(GamePathFound = ResolveFileLocation (&CurrentEntry, GamePath, &EntryChanged)))                      /* Get full game path */
  {
    if (EntryChanged)
    {
      SignalDBChanged (TRUE);
      if (OldSelected & PLAYBIT == 0x00)
      {
        NEntry.Selected |= PLAYBIT;
        PutComboEntry (&GameSelectWindow, &NEntry, SelectedGame, TRUE);
        PrintComboEntries (&GameSelectWindow);
      }
    }
    strcpy (ConvGamePath, "<Path not found>");
    FileType = FT_UNKNOWN;
  }
  else
  {
    if (EntryChanged)
    {
      SignalDBChanged (TRUE);
      if ((OldSelected & PLAYBIT == PLAYBIT) && CompatibleFileType ())
      {
        NEntry.Selected &= ~PLAYBIT;
        PutComboEntry (&GameSelectWindow, &NEntry, SelectedGame, TRUE);
        PrintComboEntries (&GameSelectWindow);
      }
    }
    MakeFullPath (FullPath, GamePath, CurrentEntry.PCName);
    ConvertPath (GamePath, ConvGamePath, CurrentEntry.DiskId);
    if (IsFloppy (FullPath[0]) && DBConfig.AssumeStableFloppies)
      GetFileType (CurrentEntry.PCName, FALSE);
    else
      GetFileType (CurrentEntry.PCName, TRUE);
  }
  UpdateButtonText (&MainWindow, &BPath, BF_FITDIR);
  UpdateButtonText (&MainWindow, &BFileSize, BF_FITTXT);
  if (GamePathFound)
  {
    if (FileType == FT_TAP)
      UnghostMenuFixTape ();
    else
      GhostMenuFixTape ();
    if (IsTape (FileType))
    {
      UnghostMenuCallTAPER ();
      GhostMenuViewBasic ();
      GhostMenuViewScreen ();
    }
    else if (IsTAPERSnapshot (FileType))
    {
      UnghostMenuCallTAPER ();
      UnghostMenuViewBasic ();
      UnghostMenuViewScreen ();
    }
    else
    {
      GhostMenuCallTAPER ();
      UnghostMenuViewBasic ();
      UnghostMenuViewScreen ();
    }
    if (FileType == FT_TZX)
    {
      UnghostMenuTZXOptions ();                                                                          /* Allow the TZX options */
      GhostMenuEditHardwareType ();                                                /* But no edit as long as no block is selected */
      GhostMenuEditArchiveInfo ();
      GhostMenuEditTZXPokes ();
    }
    else
      GhostMenuTZXOptions ();
  }
  GameButtonsSetup = FALSE;                                                       /* Signal: the other buttons must still be done */
  if (IsFloppy (FullPath[0]) && DBConfig.AssumeStableFloppies)
  {
    ClearMainLower ();
    MInlay.Flags &= ~BF_GHOSTED;                                                                             /* Allow all buttons */
    MInfo.Flags &= ~BF_GHOSTED;
    MScrShot.Flags &= ~BF_GHOSTED;
    MPlay.Flags &= ~BF_GHOSTED;
    UpdateButtonText (&MainWindow, &MPlay, MPlay.Flags);
    UpdateButtonText (&MainWindow, &MInfo, MInlay.Flags);
    UpdateButtonText (&MainWindow, &MInfo, MInfo.Flags);
    UpdateButtonText (&MainWindow, &MScrShot, MScrShot.Flags);
  }
  else
  {
    SetupGameButtons ();
    if (MScreen.Flags & BF_PRESSED)
      HandleScreen (&MainWindow, &MScreen);
    else if (MList.Flags & BF_PRESSED)
      HandleList (&MainWindow, &MList);
    else if (MScrShot.Flags & BF_PRESSED)
      HandleScrShot (&MainWindow, &MScrShot);
  }
}

int HandleAddEntry (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Add' button has been pressed (or released).                                                                       */
/* Post  : A new game entry has been inserted in both the entry table and the XMS table.                                          */
/* Import: SelectGame, ReportTotal, StoreEntry, UnghostDbase, ReselectAnEntry.                                                    */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (ForcedInsert || !(Button->Flags & BF_PRESSED))
  {
    StoreEntry ();                                                                          /* Re-store previously selected entry */
    AddDBEntry ();                                                                                         /* Add an entry in XMS */
    NewEntry.Text[0] = '\0';
    NewEntry.Selected = PLAYBIT;                                                                /* No name yet - cannot be played */
    if (!InsertComboEntry (&GameSelectWindow, &NewEntry, NULL, -1))                                 /* And one in the entry table */
      FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
    GetDBEntry (&CurrentEntry, DBLEN, GameSelectWindow.NumEntries);                                        /* Fetch the new entry */
    SelectedGame = SortEntry (&GameSelectWindow, &CurrentEntry, GameSelectWindow.NumEntries);              /* Sort alphanumerical */
    UnghostDbase ();                                                                       /* Activate the buttons and menu strip */
    ReselectAnEntry (&GameSelectWindow, SelectedGame, &CurrentEntry, FALSE);                             /* Match the empty entry */
    PrintComboEntries (&GameSelectWindow);
    SelectGame (SelectedGame);                                                                        /* Make it the current game */
    ReportTotal ();
  }
  return (WINH_NONE);
}

int HandleDelEntry (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Del' button has been pressed (or released).                                                                       */
/* Post  : The current entry has been deleted from both the entry table and the XMS table.                                        */
/* Import: StoreEntry, SelectGame, ReportTotal, GhostEmptyDbase.                                                                  */
/**********************************************************************************************************************************/

{
  struct ComboEntry NEntry;

  Window = Window;
  if (!(Button->Flags & BF_PRESSED) && SelectedGame >= 0)
  {
    StoreEntry ();
    if (DBConfig.ConfirmDelete)                                                               /* Confirmation needed: handle it ? */
      if (QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Delete",
                             "Are you sure you want to delete the entry\n\"%s\"?", CurrentEntry.Name))
        return (0);
    GetComboEntry (&GameSelectWindow, &NEntry, SelectedGame);
    if (NEntry.Selected & SELECTBIT)                                                             /* The game matched previously ? */
      NumMatches --;
    DelDBEntry (SelectedGame);                                                                        /* Remove the game from XMS */
    DeleteComboEntry (&GameSelectWindow, SelectedGame);                                               /* And from the entry table */
    SelectedGame = GameSelectWindow.LastSelected;                                            /* Re-select the last selected entry */
    if (SelectedGame > GameSelectWindow.NumEntries)                           /* It was the last entry (and now points to void) ? */
      SelectedGame --;                                                                                               /* Move back */
    PrintComboEntries (&GameSelectWindow);
    if (SelectedGame < 0)                                                                              /* Database is now empty ? */
      GhostEmptyDbase ();
    else
      SelectGame (SelectedGame);                                                                      /* Select next game in line */
    ReportTotal ();
  }
  return (WINH_NONE);
}

int HandleSave (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Write' button has been pressed (or released).                                                                     */
/* Post  : The database has been written out to disk.                                                                             */
/* Import: StoreEntry.                                                                                                            */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
    StoreEntry ();                                                                                           /* Store any changes */
    if (!DatabaseChanged)                                            /* Ask for comfirmation if the database has not been changed */
    {
      if (QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Unneeded Save", "Database has not "
                             "been changed! Save anyway?%s",
                             DatabaseTruncated ? "\n{BWARNING}B: Remember that the database has been truncated!" : ""))
        return (WINH_NONE);
    }
    else if (DatabaseTruncated)
    {
      if (QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Save of Truncated Database",
          "{BWARNING}B: Remember that the database has been truncated\n"
          "Are you sure you want to throw away these entries?"))
        return (WINH_NONE);
    }
    SaveDatabase (TRUE);                                                                                        /* Write database */
  }
  return (WINH_NONE);
}

int HandleReload (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Reload' button has been pressed (or released).                                                                    */
/* Post  : The database has been re-read from disk.                                                                               */
/* Import: SelectGame, SetupNewData.                                                                                              */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
    if (DatabaseChanged)                                                                                  /* Ask for confirmation */
    {
      if (QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Discard", "Are you sure you want to "
                             "throw away the changes?"))
        return (WINH_NONE);
    }
    else
    {
      if (QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Unneeded Reload", "Database has not "
                             "been changed! Reload anyway?"))
        return (WINH_NONE);
    }
    ReloadDatabase ();                                                                                    /* Re-read the database */
    SetupNewData (FALSE);
    if (GameSelectWindow.NumEntries >= 0)
      SelectGame (0);                                                                                   /* Auto-select first game */
  }
  return (WINH_NONE);
}

int HandlePlay (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Play' button has been pressed (or released) or the right mouse button was pressed.                                */
/* Post  : The emulator has been started with the given game.                                                                     */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  char           Params[300] = "\0";
  char           ExtraPath[MAXDIR];
  char           StartName[13];
  char           StartPath[MAXDIR];
  char           UsedGamePath[MAXDIR];
  char           UsedGameDir[MAXDIR];
  byte           UsedEmulType;
  char          *WaraEnv;
  unsigned       Dummy;
  bool           RootFile;
  bool           AlwaysCheatLink = FALSE;
  bool           JoysticksDone = FALSE;
  bool           Found;
  int            FhIn;
  int            FhOut;
  FILE          *WaraConfig;
  FILE          *WaraDefault;
  dword          FLength;
  short register Cnt;
  short register CntEnd;
  short register CntOpt;
  bool           IgnoreErrorReturn = FALSE;
  bool           UseSystem = FALSE;
  struct stat    StatInfo;
  bool           AddTAP = FALSE;
  bool           AddVOC = FALSE;

  Window = Window;
  if (!(Button->Flags & BF_PRESSED) && SelectedGame >= 0)
  {
    if (FileType == FT_UNKNOWN)
      UsedEmulType = (CurrentEntry.OverrideEmulator == ET_NONE ? DBConfig.EmulType : CurrentEntry.OverrideEmulator);
    else if (CurrentEntry.OverrideEmulator != ET_NONE)
      UsedEmulType = CurrentEntry.OverrideEmulator;
    else if (DBConfig.LinkExtension[FileType] != ET_NONE)
      UsedEmulType = DBConfig.LinkExtension[FileType];
    else
      UsedEmulType = DBConfig.EmulType;
    if (UsedEmulType == ET_WSPECEM || UsedEmulType == ET_ZX32)
    {
      if (UsedEmulType == ET_WSPECEM &&
          (MSWindowsVersion < 0x0300 || MSWindowsMode != 0x0003))               /* WSpecEm needs Windows running in enhanced mode */
      {
        ErrorRequester (-1, -1, NULL, NULL, "Need MS Windows", "MS Windows must be running in order to use %s",
                        UsedEmulType == ET_WSPECEM ? "WSpecEm" : "ZX-32");
        return (WINH_NONE);
      }
      else if (MSWindowsStopped)                                                                   /* Windows has been disabled ? */
      {
        ErrorRequester (-1, -1, NULL, NULL, "MS Windows Disabled", "MS Windows has been disabled. Re-run SGD without the "
                        "\'-W\' switch");
        return (WINH_NONE);
      }
      UseSystem = TRUE;
    }
    if (!PlayGamePoked && DBConfig.AlwaysCheat && !(MPoke.Flags & BF_GHOSTED))
      if (InsertGamePokes ())
      {
        AlwaysCheatLink = TRUE;
        GameIsPoked = TRUE;
      }
    if (GameIsPoked)
    {
      strcpy (UsedGameDir, TmpDirectory);
      if ((FileType == FT_Z80 || FileType == FT_SLT) && DBConfig.ForceSnapJoystick)        /* Do we need to adjust the snapshot ? */
      {
        for (Dummy = 0 ; Dummy < 4 && !JoysticksDone ; Dummy ++)                                      /* Setup preferred joystick */
          if (CurrentEntry.Sticks[Dummy])                                                              /* This one is an option ? */
          {
            switch (Dummy)
            {
              case 0 : Z80145SnapshotHeader.Stat2 = (Z80145SnapshotHeader.Stat2 & 0x3F) | 0x40; break;              /* (Kempston) */
              case 1 : Z80145SnapshotHeader.Stat2 = (Z80145SnapshotHeader.Stat2 & 0x3F) | 0x80; break;             /* (Intf 2 #1) */
              case 2 : Z80145SnapshotHeader.Stat2 = (Z80145SnapshotHeader.Stat2 & 0x3F) | 0xC0; break;             /* (Intf 2 #2) */
              case 3 : Z80145SnapshotHeader.Stat2 = (Z80145SnapshotHeader.Stat2 & 0x3F) | 0x00; break;                /* (Cursor) */
            }
            JoysticksDone = TRUE;                                                              /* Take only the first possibility */
          }
        if (JoysticksDone)
        {
          MakeFullPath (UsedGamePath, TmpDirectory, CurrentEntry.PCName);                           /* Reopen temporary game file */
          if ((FhOut = open (UsedGamePath, O_BINARY | O_RDWR)) == -1)                                              /* Can't do... */
          { ErrorRequester (-1, -1, NULL, NULL, "Cannot reopen file %s", UsedGamePath);
            if (AlwaysCheatLink)
            {
              GameIsPoked = FALSE;
              unlink (PokedGamePath);
            }
            return (WINH_NONE);
          }
          lseek (FhOut, 0, SEEK_SET);
          if (write (FhOut, &Z80145SnapshotHeader, sizeof (Z80145SnapshotHeader)) == -1)         /* Write updated snapshot header */
          { close (FhOut); ErrorRequester (-1, -1, "@Close", NULL, NULL, "Error writing to file %s", UsedGamePath);
            if (AlwaysCheatLink)
            {
              GameIsPoked = FALSE;
              unlink (PokedGamePath);
            }
            return (WINH_NONE);
          }
          close (FhOut);
        }
      }
    }
    else
    {
      if ((FileType == FT_Z80 || FileType == FT_SLT) && DBConfig.ForceSnapJoystick)        /* Do we need to adjust the snapshot ? */
      {
        for (Dummy = 0 ; Dummy < 4 && !JoysticksDone ; Dummy ++)                                      /* Setup preferred joystick */
          if (CurrentEntry.Sticks[Dummy])                                                              /* This one is an option ? */
          {
            switch (Dummy)
            {
              case 0 : Z80145SnapshotHeader.Stat2 = (Z80145SnapshotHeader.Stat2 & 0x3F) | 0x40; break;              /* (Kempston) */
              case 1 : Z80145SnapshotHeader.Stat2 = (Z80145SnapshotHeader.Stat2 & 0x3F) | 0x80; break;             /* (Intf 2 #1) */
              case 2 : Z80145SnapshotHeader.Stat2 = (Z80145SnapshotHeader.Stat2 & 0x3F) | 0xC0; break;             /* (Intf 2 #2) */
              case 3 : Z80145SnapshotHeader.Stat2 = (Z80145SnapshotHeader.Stat2 & 0x3F) | 0x00; break;                /* (Cursor) */
            }
            JoysticksDone = TRUE;                                                              /* Take only the first possibility */
          }
        if (JoysticksDone)                                                                             /* Joystick pre-selected ? */
        {                                                                                                          /* Then patch! */
          MakeFullPath (UsedGamePath, GamePath, CurrentEntry.PCName);                                  /* Open original game file */
          if (!RequestFloppy (UsedGamePath[0], CurrentEntry.DiskId))
            return (WINH_NONE);
          if ((FhIn = open (UsedGamePath, O_BINARY | O_RDONLY)) == -1)                                        /* Doesn't exist... */
          { ErrorRequester (-1, -1, NULL, NULL, "Cannot open file %s", UsedGamePath); return (WINH_NONE); }
          FLength = filelength (FhIn);
          MakeFullPath (UsedGamePath, TmpDirectory, CurrentEntry.PCName);                           /* Create temporary game file */
          if ((FhOut = open (UsedGamePath, O_BINARY | O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1)                    /* Can't do... */
          { close (FhIn); ErrorRequester (-1, -1, NULL, NULL, "Cannot create file %s", UsedGamePath); return (WINH_NONE); }
          CopyFilePart (FhIn, FhOut, FLength);                                                                /* Copy entire file */
          close (FhIn);
          if (lseek (FhOut, 0, SEEK_SET) == -1)
          { close (FhOut); unlink (UsedGamePath);
            ErrorRequester (-1, -1, "@Close", NULL, NULL, "Error seeking file %s", UsedGamePath); return (WINH_NONE); }
          if (write (FhOut, &Z80145SnapshotHeader, sizeof (Z80145SnapshotHeader)) == -1)
          { close (FhOut); unlink (UsedGamePath);
            ErrorRequester (-1, -1, "@Close", NULL, NULL, "Error writing to file %s", UsedGamePath); return (WINH_NONE); }
          close (FhOut);
          strcpy (UsedGameDir, TmpDirectory);
          GameIsPoked = TRUE;
          AlwaysCheatLink = TRUE;                                                             /* Signal: changed in THIS function */
          MakeFullPath (PokedGamePath, UsedGameDir, CurrentEntry.PCName);
        }
        else
          strcpy (UsedGameDir, GamePath);
      }
      else
        strcpy (UsedGameDir, GamePath);
    }
    MakeFullPath (UsedGamePath, UsedGameDir, CurrentEntry.PCName);
    if (!RequestFloppy (UsedGamePath[0], CurrentEntry.DiskId))
    {
      if (AlwaysCheatLink)
      {
        GameIsPoked = FALSE;
        unlink (PokedGamePath);
      }
      return (WINH_NONE);
    }
    else
      SetupGameButtons ();

    getcwd (PathBuffer, MAXDIR);
    if (DBConfig.EmulPath[UsedEmulType][0] != '\0')                                               /* An emulator path was given ? */
    {
      Cnt = strlen (DBConfig.EmulPath[UsedEmulType]);                                                  /* Split path and filename */
      while (-- Cnt >= 0 && DBConfig.EmulPath[UsedEmulType][Cnt] != '\\' && DBConfig.EmulPath[UsedEmulType][Cnt] != ':')
        ;
      RootFile = (Cnt == 2 && DBConfig.EmulPath[UsedEmulType][Cnt] == '\\');
      if (Cnt >= 0)                                                                               /* Is there a preceeding path ? */
      {
        strncpy (StartPath, DBConfig.EmulPath[UsedEmulType], Cnt);
        StartPath[Cnt] = '\0';
        if (StartPath[1] == ':')                                                                        /* Preceeded by a drive ? */
        {
          _dos_setdrive (ToNumber (StartPath[0]), &Dummy);                                                  /* Change drive first */
          getcwd (PathBuffer, MAXDIR);
          if (StartPath[2] != '\0')                                                                             /* Only a drive ? */
            chdir (StartPath + 2);                                                                 /* Change directory too if not */
          else if (RootFile)
            chdir ("\\");
        }
        else
          if (RootFile)
            chdir ("\\");
          else
            chdir (StartPath);                                                                      /* No drive, just change path */
      }
      strcpy (StartName, DBConfig.EmulPath[UsedEmulType] + Cnt + 1);
    }
    else                                                                                              /* (No emulator path given) */
      switch (UsedEmulType)
      {
        case ET_Z80     : strcpy (StartName, "Z80.EXE"); break;                                     /* Default is Z80.EXE for Z80 */
        case ET_JPP     : strcpy (StartName, "JPP.EXE"); break;                                     /* Default is JPP.EXE for JPP */
        case ET_WAR     : strcpy (StartName, "SPEC48.EXE"); break;                               /* Default is SPEC48.EXE for WAR */
        case ET_X128    : strcpy (StartName, "X128.EXE"); break;                                  /* Default is X128.EXE for X128 */
        case ET_WSPECEM : strcpy (StartName, "WSPECEM.EXE"); break;                         /* Default is WSPECEM.EXE for WSPECEM */
        case ET_ZX      : strcpy (StartName, "ZX.EXE"); break;                                        /* Default is ZX.EXE for ZX */
        case ET_SPANSPEC: strcpy (StartName, "SPEC386.EXE"); break;                /* Default is SPEC386.EXE for Spanish Spectrum */
        case ET_ZX32    : strcpy (StartName, "ZX32.EXE"); break;                                 /* Default is ZX32.EXE for ZX-32 */
        case ET_R80     : strcpy (StartName, "R80.EXE"); break;                                     /* Default is R80.EXE for R80 */
        case ET_RUS     : strcpy (StartName, "SPECTRUM.EXE"); break;              /* Default is SPECTRUM.EXE for Russian Spectrum */
        case ET_YUD     : strcpy (StartName, "ZX_EMUL.EXE"); break;        /* Default is ZX_EMUL.EXE for Yudin's Russian Spectrum */
        case ET_RSPEC   : strcpy (StartName, "REALSPEC.EXE"); break;                  /* Default is REALSPEC.EXE for REALSpectrum */
      }
    if (UsedEmulType == ET_Z80)                                                               /* Handle switches for the emulator */
    {
      if (!strcmp (StartName, "WINZ80.EXE"))                                                         /* Run the Windows version ? */
      {
        UseSystem = TRUE;                                                                   /* (As this is a Windows application) */
        strcpy (Params, UsedGamePath);                                                     /* Use game path as the parameter list */
      }
      else
      {
        if (!strcmp (StartName, "Z80STUB.EXE"))                                             /* (As this is a Windows application) */
          UseSystem = TRUE;
        else if (MSWindowsRunning && MSWindowsVersion >= 0x0400 && !strcmp (StartName, "Z80.EXE"))                    /* Win95+ ? */
          if (!stat ("Z80STUB.EXE", &StatInfo))                                                                  /* Stub exists ? */
          {
            strcpy (StartName, "Z80STUB.EXE");                                                                 /* Use it instead! */
            UseSystem = TRUE;
          }
        if (SystemDefaults.Quiet)                                                                        /* Quiet mode selected ? */
          strcat (Params, "-g ");
        if (CurrentEntry.Memory >= 128)                                                                            /* 128K game ? */
          strcat (Params, "-128 ");                                                                         /* Then use 128K mode */
        JoysticksDone = FALSE;
        for (Dummy = 0 ; Dummy < 4 && !JoysticksDone ; Dummy ++)                                      /* Setup preferred joystick */
          if (CurrentEntry.Sticks[Dummy])                                                              /* This one is an option ? */
          {
            switch (Dummy)
            {
              case 0 : strcat (Params, "-jk "); break;                                                              /* (Kempston) */
              case 1 : strcat (Params, "-ju43125 "); break;                                                        /* (Intf 2 #1) */
              case 2 : strcat (Params, "-j2 "); break;                                                             /* (Intf 2 #2) */
              case 3 : strcat (Params, "-jc "); break;                                                                /* (Cursor) */
            }
            JoysticksDone = TRUE;                                                              /* Take only the first possibility */
          }
        if (IsTape (FileType) && DBConfig.AutoLoadTapes)
        {
          if (FileType == FT_TAP)                                                  /* The option order MUST be like this for Z80! */
            strcat (Params, "-ti ");
          else if (FileType == FT_TZX)
            strcat (Params, "-tz ");
          else if (FileType == FT_VOC)
            strcat (Params, "-tv ");
          strcat (Params, UsedGamePath);                                                   /* Add game path to the parameter list */
          strcat (Params, " ");
          if (CurrentEntry.Memory >= 128)                                                                          /* 128K game ? */
            if (CurrentEntry.Memory == 129)                                                                      /* A USR0 game ? */
              MakeFullPath (ExtraPath, CurrentPath, "LOADUSR0.Z80");
            else
              MakeFullPath (ExtraPath, CurrentPath, "LOAD128.Z80");
          else
            MakeFullPath (ExtraPath, CurrentPath, "LOAD48.Z80");
          strcat (Params, ExtraPath);
        }
        else
        {
          if (FileType == FT_Z80 || FileType == FT_SNA)
          {
            if (DBConfig.SnapNeedsTAP[0] != '\0')                                                   /* Do we have to add a tape ? */
            {
              if (!strcmp (CurrentEntry.Name + strlen (CurrentEntry.Name) - strlen (DBConfig.SnapNeedsTAP), DBConfig.SnapNeedsTAP))
                AddTAP = TRUE;
            }
            if (!AddTAP && DBConfig.SnapNeedsVOC[0] != '\0')                                         /* Do we have to add a VOC ? */
            {
              if (!strcmp (CurrentEntry.Name + strlen (CurrentEntry.Name) - strlen (DBConfig.SnapNeedsVOC), DBConfig.SnapNeedsVOC))
                AddVOC = TRUE;
            }
            if (!IsFloppy (FullPath[0]) && CurrentEntry.MultiLoad && !AddTAP && !AddVOC)                     /* See for ourselves */
            {
              strcpy (ExtraPath, FullPath);
              strcpy (ExtraPath + strlen (ExtraPath) - 3, "TAP");                                    /* Change filename extension */
              if (!stat (ExtraPath, &StatInfo))                                                                  /* File exists ? */
                AddTAP = TRUE;
              else
              {
                strcpy (ExtraPath + strlen (ExtraPath) - 3, "VOC");                                  /* Change filename extension */
                if (!stat (ExtraPath, &StatInfo))                                                                /* File exists ? */
                  AddVOC = TRUE;
              }
            }
            if (AddTAP)
            {                                                                                /* This snapshot needs a tape file ? */
              strcat (Params, "-ti ");                                                                           /* Add tape file */
              strcat (Params, FullPath);                                                           /* !IN THE ORIGINAL DIRECTORY! */
              strcpy (Params + strlen (Params) - 3, "TAP");                                          /* Change filename extension */
              strcat (Params, " ");
            }
            else if (AddVOC)
            {                                                                                 /* This snapshot needs a VOC file ? */
              strcat (Params, "-tv ");                                                                            /* Add VOC file */
              strcat (Params, FullPath);
              strcpy (Params + strlen (Params) - 3, "VOC");                                          /* Change filename extension */
              strcat (Params, " ");
            }
          }
          else if (FileType == FT_TAP)                                                            /* Tapefiles use the switch -ti */
            strcat (Params, "-ti ");
          else if (FileType == FT_TZX)                                                            /* TZX files use the switch -tz */
            strcat (Params, "-tz ");
          else if (FileType == FT_MDR)
            strcat (Params, "-m 1 ");                                                        /* Insert cartridge files in drive 1 */
          else if (FileType == FT_VOC)
            strcat (Params, "-tv ");                                                              /* VOC files use the switch -tv */
          strcat (Params, UsedGamePath);                                                   /* Add game path to the parameter list */
        }
      }
    }
    else if (UsedEmulType == ET_WAR)
    {
      if (FileType == FT_SNA || FileType == FT_SLT || FileType == FT_LTP || FileType == FT_TZX ||           /* Non-native formats */
          FileType == FT_SP || FileType == FT_VOC)
      {
        strcpy (StartName, "WARAJEVO.EXE");                              /* Start through the main program to set auto-conversion */
        sprintf (Params, "/D%s ", UsedGameDir);
        strcat (Params, "/S");                                                           /* Signal: auto-convert to native format */
        strcat (Params, UsedGamePath);
        strcat (Params, " /C");                                                                    /* Activate "SPECSIM.CFG" file */
        if ((WaraEnv = getenv ("WARAJEVO")) == NULL)
          strcpy (ExtraPath, "SPECSIM.CFG");
        else
        {
          if (WaraEnv[1] == ' ')                                                                      /* Preceeded by a "[G|T]" ? */
          {
            Dummy = 1;
            while (WaraEnv[++ Dummy] == ' ')                                                             /* Skip any extra spaces */
              ;
            if (!*WaraEnv)                                                                                      /* Nothing left ? */
              strcpy (ExtraPath, "SPECSIM.CFG");
            else
              MakeFullPath (ExtraPath, WaraEnv + Dummy, "SPECSIM.CFG");
          }
          else
            MakeFullPath (ExtraPath, WaraEnv, "SPECSIM.CFG");
        }
        if ((WaraConfig = fopen (ExtraPath, "wt")) != NULL)                                          /* Create "SPECSIM.CFG" file */
        {
          if ((WaraDefault = fopen ("DEFAULT.CFG", "rt")) != NULL)                             /* Is there a "DEFAULT.CFG" file ? */
          {
            while (fgets (UsedGamePath, MAXDIR - 1, WaraDefault))                              /* Copy the entire file over first */
              fputs (UsedGamePath, WaraConfig);
            fclose (WaraDefault);
            fputc ('\n', WaraConfig);                                                            /* (Just to be on the safe side) */
          }
          if (CurrentEntry.Memory >= 128)                                                                   /* Write memory model */
          {
            fputs ("/&E1 ", WaraConfig);
            if (CurrentEntry.Memory == 130)                                                                           /* (128/+2) */
              fputs ("/2 ", WaraConfig);
          }
          else
            fputs ("/&E0 ", WaraConfig);
          JoysticksDone = FALSE;
          for (Dummy = 0 ; Dummy < 4 && !JoysticksDone ; Dummy ++)                                    /* Setup preferred joystick */
            if (CurrentEntry.Sticks[Dummy])                                                            /* This one is an option ? */
            {
              switch (Dummy)
              {
                case 0 : fputs ("/JK ", WaraConfig); break;                                                         /* (Kempston) */
                case 1 : fputs ("/JS ", WaraConfig); break;                                                        /* (Intf 2 #1) */
                case 2 : fputs ("/JB ", WaraConfig); break;                                                        /* (Intf 2 #2) */
                case 3 : fputs ("/JC ", WaraConfig); break;                                                           /* (Cursor) */
              }
              JoysticksDone = TRUE;                                                            /* Take only the first possibility */
            }
          if (strlen (DBConfig.ExtraOpt[UsedEmulType]) > 0)
            fputs (DBConfig.ExtraOpt[UsedEmulType], WaraConfig);                                         /* Add any extra options */
          fputc ('\n', WaraConfig);
          fclose (WaraConfig);
        }
      }
      else
      {
        if (strcmp (StartName, "SPEC128.EXE"))                                                  /* Always use the 128K emulator ? */
          if (CurrentEntry.Memory >= 128)                                                                    /* Nope. 128K game ? */
            strcpy (StartName, "SPEC128.EXE");
          else
            strcpy (StartName, "SPEC48.EXE");
        JoysticksDone = FALSE;
        for (Dummy = 0 ; Dummy < 4 && !JoysticksDone ; Dummy ++)                                      /* Setup preferred joystick */
          if (CurrentEntry.Sticks[Dummy])                                                              /* This one is an option ? */
          {
            switch (Dummy)
            {
              case 0 : strcat (Params, "/JK "); break;                                                              /* (Kempston) */
              case 1 : strcat (Params, "/JS "); break;                                                             /* (Intf 2 #1) */
              case 2 : strcat (Params, "/JB "); break;                                                             /* (Intf 2 #2) */
              case 3 : strcat (Params, "/JC "); break;                                                                /* (Cursor) */
            }
            JoysticksDone = TRUE;                                                              /* Take only the first possibility */
          }
        if (FileType == FT_Z80)
        {
          if (DBConfig.SnapNeedsTAP[0] != '\0')                                                     /* Do we have to add a tape ? */
          {
            if (!strcmp (CurrentEntry.Name + strlen (CurrentEntry.Name) - strlen (DBConfig.SnapNeedsTAP), DBConfig.SnapNeedsTAP))
              AddTAP = TRUE;
          }
          if (!IsFloppy (FullPath[0]) && CurrentEntry.MultiLoad && !AddTAP)                                  /* See for ourselves */
          {
            strcpy (ExtraPath, FullPath);
            strcpy (ExtraPath + strlen (ExtraPath) - 3, "TAP");                                      /* Change filename extension */
            if (!stat (ExtraPath, &StatInfo))                                                                    /* File exists ? */
              AddTAP = TRUE;
          }
          if (AddTAP)
          {                                                                                  /* This snapshot needs a tape file ? */
            sprintf (Params + strlen (Params), "/D%s ", GamePath);                          /* !TAP IS IN THE ORIGINAL DIRECTORY! */
            strcat (Params, "/T");                                                                               /* Add tape file */
            strcat (Params, CurrentEntry.PCName);
            strcpy (Params + strlen (Params) - 3, "TAP");                                            /* Change filename extension */
            strcat (Params, " /R");
            if (GameIsPoked)
              strcat (Params, UsedGamePath);                                         /* Provide full path to the patched Z80 file */
            else
              strcat (Params, CurrentEntry.PCName);
          }
          else
          {
            sprintf (Params + strlen (Params), "/D%s ", UsedGameDir);
            strcat (Params, "/R");                                                                                /* Use Z80 file */
            strcat (Params, CurrentEntry.PCName);
          }
        }
        else
        {
          sprintf (Params + strlen (Params), "/D%s ", UsedGameDir);
          switch (FileType)
          {
            case FT_TAP : if (DBConfig.AutoLoadTapes && CurrentEntry.Memory != 130)               /* (Doesn't work for +2 games!) */
                          {
                            if (CurrentEntry.Memory >= 128)                                                        /* 128K game ? */
                              if (CurrentEntry.Memory == 129)                                                    /* A USR0 game ? */
                                MakeFullPath (ExtraPath, CurrentPath, "LOADUSR0.Z80");
                              else
                                MakeFullPath (ExtraPath, CurrentPath, "LOAD128.Z80");
                            else
                              MakeFullPath (ExtraPath, CurrentPath, "LOAD48.Z80");
                            strcat (Params, "/R");
                            strcat (Params, ExtraPath);
                            strcat (Params, " /T");
                          }
                          else
                            strcat (Params, "/T");
                          break;
            case FT_MDR : strcat (Params, "/#1;");                                                                /* Use MDR file */
                          break;
          }
          strcat (Params, CurrentEntry.PCName);
        }
      }
    }
    else if (UsedEmulType == ET_X128)
    {
      if (CurrentEntry.Memory >= 128)                                                                             /* 128K+ game ? */
      {
        if (CurrentEntry.Memory == 130)                                                                               /* +2/+2A ? */
          strcat (Params, "/P2 ");                                                                            /* Then use +2 mode */
        else if (CurrentEntry.Memory == 131)                                                                              /* +3 ? */
          strcat (Params, "/P3 ");                                                                            /* Then use +3 mode */
        else if (CurrentEntry.Memory == 228)                                                                        /* Pentagon ? */
          strcat (Params, "/pentagon ");                                                                /* Then use pentagon mode */
        else
          strcat (Params, "/128 ");                                                                         /* Else use 128K mode */
      }
      else
        strcat (Params, "/48 ");
      if (SystemDefaults.Quiet)                                                                          /* Quiet mode selected ? */
        strcat (Params, "/quiet ");
      if (FileType == FT_TRD || FileType == FT_DSK)                                                            /* TRD disk file ? */
      {
        strcat (Params, "/a ");                                                                  /* 'Insert' the disk in drive A: */
        strcat (Params, UsedGamePath);
      }
      else if (IsTape (FileType) && DBConfig.AutoLoadTapes &&
               CurrentEntry.Memory != 130 &&
               CurrentEntry.Memory != 131 &&
               CurrentEntry.Memory != 228)                                         /* (Doesn't work for +2/+2A, +3 and Pentagon!) */
      {
        if (CurrentEntry.Memory >= 128)                                                                            /* 128K game ? */
          if (CurrentEntry.Memory == 129)                                                                        /* A USR0 game ? */
            MakeFullPath (ExtraPath, CurrentPath, "LOADUSR0.Z80");
          else
            MakeFullPath (ExtraPath, CurrentPath, "LOAD128.Z80");
        else
          MakeFullPath (ExtraPath, CurrentPath, "LOAD48.Z80");
        strcat (Params, ExtraPath);
        strcat (Params, " ");
        strcat (Params, UsedGamePath);
      }
      else
      {
        strcat (Params, UsedGamePath);
        if (FileType == FT_Z80 || FileType == FT_SNA)
        {
          if (DBConfig.SnapNeedsTAP[0] != '\0')                                                     /* Do we have to add a tape ? */
          {
            if (!strcmp (CurrentEntry.Name + strlen (CurrentEntry.Name) - strlen (DBConfig.SnapNeedsTAP), DBConfig.SnapNeedsTAP))
              AddTAP = TRUE;
          }
          if (!AddTAP && DBConfig.SnapNeedsVOC[0] != '\0')                                           /* Do we have to add a VOC ? */
          {
            if (!strcmp (CurrentEntry.Name + strlen (CurrentEntry.Name) - strlen (DBConfig.SnapNeedsVOC), DBConfig.SnapNeedsVOC))
              AddVOC = TRUE;
          }
          if (!IsFloppy (FullPath[0]) && CurrentEntry.MultiLoad && !AddTAP && !AddVOC)                       /* See for ourselves */
          {
            strcpy (ExtraPath, FullPath);
            strcpy (ExtraPath + strlen (ExtraPath) - 3, "TAP");                                      /* Change filename extension */
            if (!stat (ExtraPath, &StatInfo))                                                                    /* File exists ? */
              AddTAP = TRUE;
            else
            {
              strcpy (ExtraPath + strlen (ExtraPath) - 3, "VOC");                                    /* Change filename extension */
              if (!stat (ExtraPath, &StatInfo))                                                                  /* File exists ? */
                AddVOC = TRUE;
            }
          }
          if (AddTAP)
          {                                                                                  /* This snapshot needs a tape file ? */
            strcat (Params, " ");
            strcat (Params, FullPath);                                                             /* !IN THE ORIGINAL DIRECTORY! */
            strcpy (Params + strlen (Params) - 3, "TAP");                                            /* Change filename extension */
          }
          else if (AddVOC)
          {                                                                                   /* This snapshot needs a VOC file ? */
            strcat (Params, " ");
            strcat (Params, FullPath);
            strcpy (Params + strlen (Params) - 3, "VOC");                                            /* Change filename extension */
          }
        }
      }
      IgnoreErrorReturn = TRUE;                                                      /* X128 generates a bogus error return value */
    }
    else if (UsedEmulType == ET_RSPEC)
    {
      if ((WaraConfig = fopen ("REALSPEC.BAK", "wt")) != NULL)                                      /* Create "REALSPEC.BAK" file */
      {
        if ((WaraDefault = fopen ("REALSPEC.INI", "rt")) != NULL)                             /* Is there a "REALSPEC.INI" file ? */
        {
          while (fgets (UsedGamePath, MAXDIR - 1, WaraDefault))                                /* Copy the entire file over first */
            fputs (UsedGamePath, WaraConfig);
          fclose (WaraDefault);
        }
        fclose (WaraConfig);
      }
      if ((WaraConfig = fopen ("REALSPEC.INI", "wt")) != NULL)                            /* Now create a new "REALSPEC.INI" file */
      {
        if ((WaraDefault = fopen ("REALSPEC.BAK", "rt")) != NULL)                                /* by parsing the created backup */
        {
          while (fgets (UsedGamePath, MAXDIR - 1, WaraDefault))                                /* Copy the entire file over first */
          {
            if (strlen (UsedGamePath) > 3)                                            /* (Do not inspect too short (empty) lines) */
            {
              Found = FALSE;
              if (UsedGamePath[0] == '#')                                                                 /* Line commented out ? */
              {
                Cnt = 1;
                while (isspace (UsedGamePath[Cnt]))
                  Cnt ++;                                                                  /* Find first non-whitespace character */
              }
              else
                Cnt = 0;
              for (CntOpt = 0 ; !Found && CntOpt < NumRSPECOptions ; CntOpt ++)
              {
                if (!strnicmp (UsedGamePath + Cnt, RSPECOptions[CntOpt].Name, strlen (RSPECOptions[CntOpt].Name)))
                  if (UsedGamePath[Cnt + strlen (RSPECOptions[CntOpt].Name)] == ':')
                  {
                    CntEnd = Cnt + strlen (RSPECOptions[CntOpt].Name) + 1;                                 /* Point after the ':' */
                    while (isspace (UsedGamePath[CntEnd]))
                      CntEnd ++;                                                           /* Find first non-whitespace character */
                    while (!isspace (UsedGamePath[CntEnd]) && UsedGamePath[CntEnd] != '#')
                      CntEnd ++;                                                               /* Find first whitespace character */
                    Found = TRUE;
                    if (RSPECOptions[CntOpt].When == RS_TAKEOUT)
                    {
                      if (Cnt == 0)
                        fputs ("# ", WaraConfig);
                      CntEnd = 0;                                                                            /* (Dup entire line) */
                    }
                    else
                    {
                      if (RSPECOptions[CntOpt].When > 0)
                      {
                        if (FileType == RSPECOptions[CntOpt].When)
                          fprintf (WaraConfig, "%s: %s", RSPECOptions[CntOpt].Name, CurrentEntry.PCName);
                        else
                        {
                          if (Cnt == 0)
                            fputs ("# ", WaraConfig);
                          CntEnd = 0;
                        }
                      }
                      else
                      {
                        switch (RSPECOptions[CntOpt].When)
                        {
                          case RS_NONE     : fprintf (WaraConfig, "%s: 0", RSPECOptions[CntOpt].Name);
                                             break;
                          case RS_JOYSTICK : fprintf (WaraConfig, "%s: ", RSPECOptions[CntOpt].Name);
                                             JoysticksDone = FALSE;
                                             for (Dummy = 0 ; Dummy < 4 && !JoysticksDone ; Dummy ++) /* Setup preferred joystick */
                                               if (CurrentEntry.Sticks[Dummy])                         /* This one is an option ? */
                                               {
                                                 switch (Dummy)
                                                 {
                                                   case 0 : fputc ('0', WaraConfig); JoysticksDone = TRUE; break;   /* (Kempston) */
                                                   case 1 : fputc ('1', WaraConfig); JoysticksDone = TRUE; break;      /* (I2 #1) */
                                                   case 2 : fputc ('2', WaraConfig); JoysticksDone = TRUE; break;      /* (I2 #2) */
                                                   default: break;                                    /* (Cursor not implemented) */
                                                 }
                                               }
                                             if (!JoysticksDone)
                                               fputc ('2', WaraConfig);                                   /* Default to Intf 2 #2 */
                                             break;
                          case RS_MODEL    : fprintf (WaraConfig, "%s: ", RSPECOptions[CntOpt].Name);
                                             switch (CurrentEntry.Memory)
                                             {
                                               case  16 : fputc ('0', WaraConfig); break;
                                               case  48 : fputc ('1', WaraConfig); break;
                                               case 128 : fputc ('2', WaraConfig); break;
                                               case 130 : fputc ('4', WaraConfig); break;
                                               case 131 : fputc ('5', WaraConfig); break;
                                               case 228 : fputc ('7', WaraConfig); break;
                                               default  : fputc ('1', WaraConfig); break;                     /* (Default is 48K) */
                                             }
                                             break;
                          case RS_POKES    : fprintf (WaraConfig, "%s: %s\\POKES", RSPECOptions[CntOpt].Name, UsedGameDir);
                                             break;
                          case RS_ISSNAPD  : if (IsSnapshot (FileType))
                                               fprintf (WaraConfig, "%s: %s", RSPECOptions[CntOpt].Name, UsedGameDir);
                                             else
                                             {
                                               if (Cnt == 0)
                                                 fputs ("# ", WaraConfig);
                                               CntEnd = 0;
                                             }
                                             break;
                          case RS_ISSNAPF  : if (IsSnapshot (FileType))
                                               fprintf (WaraConfig, "%s: %s", RSPECOptions[CntOpt].Name, CurrentEntry.PCName);
                                             else
                                             {
                                               if (Cnt == 0)
                                                 fputs ("# ", WaraConfig);
                                               CntEnd = 0;
                                             }
                                             break;
                          case RS_ISTAPED  : if (IsTape (FileType))
                                               fprintf (WaraConfig, "%s: %s", RSPECOptions[CntOpt].Name, UsedGameDir);
                                             else
                                             {
                                               if (Cnt == 0)
                                                 fputs ("# ", WaraConfig);
                                               CntEnd = 0;
                                             }
                                             break;
                          case RS_ISTAPEF  : if (IsTape (FileType))
                                               fprintf (WaraConfig, "%s: %s", RSPECOptions[CntOpt].Name, CurrentEntry.PCName);
                                             else
                                             {
                                               if (Cnt == 0)
                                                 fputs ("# ", WaraConfig);
                                               CntEnd = 0;
                                             }
                                             break;
                          case RS_ISDISKD  : if (IsDisk (FileType))
                                               fprintf (WaraConfig, "%s: %s", RSPECOptions[CntOpt].Name, UsedGameDir);
                                             else
                                             {
                                               if (Cnt == 0)
                                                 fputs ("# ", WaraConfig);
                                               CntEnd = 0;
                                             }
                                             break;
                          case RS_ISMDRD   : if (FileType == FT_MDR)
                                               fprintf (WaraConfig, "%s: %s", RSPECOptions[CntOpt].Name, UsedGameDir);
                                             else
                                             {
                                               if (Cnt == 0)
                                                 fputs ("# ", WaraConfig);
                                               CntEnd = 0;
                                             }
                                             break;
                          case RS_ISTRD    : fprintf (WaraConfig, "%s: %d", RSPECOptions[CntOpt].Name, FileType == FT_TRD ? 1 : 0);
                                             break;
                          case RS_ISMDR    : fprintf (WaraConfig, "%s: %d", RSPECOptions[CntOpt].Name, FileType == FT_MDR ? 1 : 0);
                                             break;
                        }
                      }
                    }
                  }
              }
              if (Found)
              {
                RSPECOptions[CntOpt].Done = TRUE;
                fputs (UsedGamePath + CntEnd, WaraConfig);                              /* Dup the remained (comment) of the line */
              }
              else                                                                                      /* Dup non-matching lines */
                fputs (UsedGamePath, WaraConfig);
            }
            else
              fputs (UsedGamePath, WaraConfig);
          }
          fclose (WaraDefault);
        }
        fclose (WaraConfig);
      }
      if (SystemDefaults.Quiet)                                                                          /* Quiet mode selected ? */
        strcat (Params, "-nosound ");
      
      if (IsTape (FileType) && DBConfig.AutoLoadTapes &&
          CurrentEntry.Memory != 130 &&
          CurrentEntry.Memory != 131 &&
          CurrentEntry.Memory != 228)                                              /* (Doesn't work for +2/+2A, +3 and Pentagon!) */
      {
        if (CurrentEntry.Memory >= 128)                                                                            /* 128K game ? */
          if (CurrentEntry.Memory == 129)                                                                        /* A USR0 game ? */
            MakeFullPath (ExtraPath, CurrentPath, "LOADUSR0.Z80");
          else
            MakeFullPath (ExtraPath, CurrentPath, "LOAD128.Z80");
        else
          MakeFullPath (ExtraPath, CurrentPath, "LOAD48.Z80");
        strcat (Params, ExtraPath);
      }
    }
    else if (UsedEmulType == ET_ZX)
    {
      if (SystemDefaults.Quiet)                                                                          /* Quiet mode selected ? */
        strcat (Params, "/snd- ");
      JoysticksDone = FALSE;
      for (Dummy = 0 ; Dummy < 4 && !JoysticksDone ; Dummy ++)                                        /* Setup preferred joystick */
        if (CurrentEntry.Sticks[Dummy])                                                                /* This one is an option ? */
        {
          switch (Dummy)
          {
            case 0 : strcat (Params, "/kpd:4 "); break;                                                             /* (Kempston) */
            case 1 : strcat (Params, "/kpd:2 "); break;                                                            /* (Intf 2 #1) */
            case 2 : strcat (Params, "/kpd:3 "); break;                                                            /* (Intf 2 #2) */
            case 3 : strcat (Params, "/kpd:1 "); break;                                                               /* (Cursor) */
          }
          JoysticksDone = TRUE;                                                                /* Take only the first possibility */
        }
      strcat (Params, UsedGamePath);
    }
    else if (UsedEmulType == ET_R80)
    {
      if (CurrentEntry.Memory >= 128)                                                                             /* 128K+ game ? */
      {
        if (CurrentEntry.Memory == 130)                                                                          /* +2/+2A game ? */
          strcat (Params, "-2 ");                                                                             /* Then use +2 mode */
        else if (CurrentEntry.Memory == 228)                                                                   /* Pentagon game ? */
          strcat (Params, "-pentagon ");                                                                /* Then use pentagon mode */
        else
          strcat (Params, "-128 ");                                                                         /* Else use 128K mode */
      }
      else
        strcat (Params, "-48 ");
      if (SystemDefaults.Quiet)                                                                          /* Quiet mode selected ? */
        strcat (Params, "-nosound ");
      JoysticksDone = FALSE;
      for (Dummy = 0 ; Dummy < 4 && !JoysticksDone ; Dummy ++)                                        /* Setup preferred joystick */
        if (CurrentEntry.Sticks[Dummy])                                                                /* This one is an option ? */
        {
          switch (Dummy)
          {
            case 0 : strcat (Params, "-cursors 2 "); break;                                                         /* (Kempston) */
            case 1 : strcat (Params, "-cursors 3 "); break;                                                        /* (Intf 2 #1) */
            case 2 : strcat (Params, "-cursors 4 "); break;                                                        /* (Intf 2 #2) */
            case 3 : strcat (Params, "-cursors 1 "); break;                                                           /* (Cursor) */
          }
          JoysticksDone = TRUE;                                                                /* Take only the first possibility */
        }
      strcat (Params, UsedGamePath);
    }
    else if (UsedEmulType == ET_RUS)
    {
      if (FileType == FT_Z80 || FileType == FT_SNA)
        strcat (Params, "-L");
      strcat (Params, UsedGamePath);
    }
    else
      strcat (Params, UsedGamePath);                                                       /* Add game path to the parameter list */
    if (strlen (DBConfig.ExtraOpt[UsedEmulType]) > 0)
    {
      strcat (Params, " ");
      strcat (Params, DBConfig.ExtraOpt[UsedEmulType]);                                                  /* Add any extra options */
    }
    SwapOut (TRUE, StartName, Params, NULL, NULL, UseSystem, FALSE, IgnoreErrorReturn);         /* Run the emulator with the game */
    chdir (PathBuffer);
    _dos_setdrive (CurDrv, &Dummy);
    chdir (CurPath);                                                                               /* Restore drive and directory */
    if (AlwaysCheatLink)
    {
      GameIsPoked = FALSE;
      unlink (PokedGamePath);
    }
  }
  return (WINH_NONE);
}

int HandleConfig (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Config' button has been pressed (or released).                                                                    */
/* Post  : The configuration has been changed.                                                                                    */
/* Import: TestGames, TestPlayGhosted, HandleScreen.                                                                              */
/**********************************************************************************************************************************/

{
  bool OldRemapBrightBlack;
  byte OldEmulType;

  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
    OldRemapBrightBlack = DBConfig.RemapBrightBlack;                                            /* Store some important variables */
    OldEmulType = DBConfig.EmulType;
    ChangeConfig ();                                                                /* Allow to make changes to the configuration */
    if (SystemDefaults.SyncSlider)                                                                           /* Sync slider bar ? */
      GameSelectWindow.Flags |= COMF_SYNCSLIDER;
    else
      GameSelectWindow.Flags &= ~COMF_SYNCSLIDER;
    if (GameSelectWindow.NumEntries >= 0)                                                              /* Database is not empty ? */
    {
      if (OldEmulType != DBConfig.EmulType)                                                   /* Emulator type has been changed ? */
      {
        EmulatorChanged = TRUE;                                                                /* Signal: test game paths as well */
        TestGames ();                                                                      /* Determine which games can be played */
        EmulatorChanged = FALSE;
        TestPlayGhosted ();
        PrintComboEntries (&GameSelectWindow);
      }
      if (OldRemapBrightBlack != DBConfig.RemapBrightBlack && (MScreen.Flags & BF_PRESSED))
      {
        SpecColTable[8] = DBConfig.RemapBrightBlack ? 8 : 0;
        HandleScreen (&MainWindow, &MScreen);                                 /* RemapBrightBlack changed and screen is displayed */
      }
      SelectGame (SelectedGame);                                                         /* Ensure the game info is still correct */
    }
  }
  return (WINH_NONE);
}

void CallTAPER (char *FullPath)

/**********************************************************************************************************************************/
/* Pre   : The `Call TAPER' menu option has been selected.                                                                        */
/* Post  : TAPER has been called with the selected game file as argument.                                                         */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  unsigned Dummy;
  char     SwapArgs[128] = { '\0' };

  if (!RequestFloppy (ProgramDrive[0], ResourceFloppyId))                          /* MUST be in the same directory as SGD itself */
    return;
  if (!HaveSoundcard)                                                                     /* SGD was started with a '-s' switch ? */
    strcpy (SwapArgs, "-s ");
  else if (!HaveMixer)                                                                    /* SGD was started with a '-m' switch ? */
    strcpy (SwapArgs, "-m ");
  strcat (SwapArgs, FullPath);
  SwapOut (TRUE, "TAPER.EXE", SwapArgs, NULL, NULL, FALSE, TRUE, FALSE);                                           /* Spawn TAPER */
  _dos_setdrive (CurDrv, &Dummy);                                                                       /* Restore drive and path */
  chdir (CurPath);
  SelectGame (SelectedGame);                                                                              /* Test for any changes */
}

int HandleShell (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Shell' button has been pressed (or released).                                                                     */
/* Post  : A shell has been started and shut down by the user.                                                                    */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  unsigned Dummy;

  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
    SwapOut (TRUE, "", "", NULL, "Enter 'EXIT' to return to the database\n\n", FALSE, FALSE, FALSE);                 /* Shell out */
    _dos_setdrive (CurDrv, &Dummy);                                                                     /* Restore drive and path */
    chdir (CurPath);
  }
  return (WINH_NONE);
}

int HandleScreen (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Screen' button has been pressed (or released).                                                                    */
/* Post  : The screen from the spectrum file has been shown on the display.                                                       */
/* Import: ClearMainLower, SurroundScreen.                                                                                        */
/**********************************************************************************************************************************/

{
  Window = Window;
  if ((Button->Flags & BF_PRESSED) && SelectedGame >= 0)
  {
    ForceButtonRelease (Window, &MList);                                           /* Screen or List can be pressed, but not both */
    ForceButtonRelease (Window, &MScrShot);
    if (DBConfig.PickBestScreen && ScrShotFound)
    {
      BestScreenLoop = TRUE;
      HandleScrShot (Window, Button);
      BestScreenLoop = FALSE;
      return (WINH_NONE);
    }
    if (!SpectrumScreen)                                                                          /* Already a screen displayed ? */
      ClearMainLower ();
    FlashState = FALSE;                                                                                       /* Initialize flash */
    ReleaseTimedHandler (TimedFlash, &MainWindow);
    if ((FileType == FT_TAP && DBConfig.EmulType == ET_WAR && !DBConfig.OnlyZ80Tapes) ||
        !RequestFloppy (FullPath[0], CurrentEntry.DiskId))
    {
      SpectrumScreen = FALSE;
      GhostMenuGIF ();
      GhostMenuSCR ();
      return (WINH_NONE);
    }
    else
      SetupGameButtons ();
    SpectrumScreen = TRUE;                                                                  /* Assume this operation will succeed */
    ScreenIsScrShot = FALSE;
    switch (FileType)                                                                        /* Find and draw the spectrum screen */
    {
      case FT_SLT :                                                                   /* SLT files start off with a Z80-type part */
      case FT_Z80 : if (!PutScreenFromZ80 (Window, 21, 230, FullPath))
                    {
                      ClearMainLower ();
                      Wprintf (Window, 31, TEXT_CENTER, "{+4<Error in %s file>}N", FileType == FT_Z80 ? "Z80" : "SLT");
                      SpectrumScreen = FALSE;
                    }
                    break;
      case FT_SNA :
      case FT_SNP :
      case FT_SEM :
      case FT_SP  : PutScreenFromSNA (Window, 21, 230, FullPath); break;
      case FT_TAP :
      case FT_LTP : if (!PutScreenFromTAP (Window, 21, 230, FullPath))
                    {
                      ClearMainLower ();
                      Wprintf (Window, 31, TEXT_CENTER, "{+4<No screen found>}N");
                      SpectrumScreen = FALSE;
                    }
                    break;
      case FT_TZX : if (!PutScreenFromTZX (Window, 21, 230, FullPath))
                    {
                      ClearMainLower ();
                      Wprintf (Window, 31, TEXT_CENTER, "{+4<No screen found>}N");
                      SpectrumScreen = FALSE;
                    }
                    break;
      case FT_MDR :
      case FT_VOC : ClearMainLower ();
                    SpectrumScreen = FALSE;
                    break;
      default     : ClearMainLower ();
                    SpectrumScreen = FALSE;
    }
    if (FileNotFound)
    {
      ClearMainLower ();
      Wprintf (Window, 31, TEXT_CENTER, "{+4<File not found>}N");
      SpectrumScreen = FALSE;
    }
    if (SpectrumScreen)
    {
      SurroundScreen (Window, 19, 228);
      if (ScreenContainsFlash && DBConfig.EmulateFlash)
        RegisterTimedHandler (DBConfig.SlowFlash ? 960 : 320, TimedFlash, &MainWindow);
    }
    if (SpectrumScreen)
    {
      UnghostMenuGIF ();
      UnghostMenuSCR ();
    }
    else
    {
      GhostMenuGIF ();
      GhostMenuSCR ();
    }
  }
  else                                                                                                /* Button is being released */
  {
    ClearMainLower ();
    SpectrumScreen = FALSE;
    ReleaseTimedHandler (TimedFlash, &MainWindow);
  }
  return (WINH_NONE);
}

int HandleList (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `List' button has been pressed (or released).                                                                      */
/* Post  : A listing of the spectrum file characteristics has been printed.                                                       */
/* Import: ClearMainLower.                                                                                                        */
/**********************************************************************************************************************************/

{
  char HWMode[21];
  bool ReadError = FALSE;

  Window = Window;
  if ((Button->Flags & BF_PRESSED) && SelectedGame >= 0)
  {
    GhostMenuGIF ();
    GhostMenuSCR ();
    ForceButtonRelease (Window, &MScreen);
    ForceButtonRelease (Window, &MScrShot);
    if ((FileType == FT_TAP && DBConfig.EmulType == ET_WAR && !DBConfig.OnlyZ80Tapes) ||
        !RequestFloppy (FullPath[0], CurrentEntry.DiskId))
      return (WINH_NONE);
    else
      SetupGameButtons ();
    if (FileType == FT_TAP || FileType == FT_LTP || FileType == FT_MDR || FileType == FT_TZX)
    {
      if (!TAPListing)
      {
        ClearMainLower ();
        MDRListing = (bool)(FileType == FT_MDR ? TRUE : FALSE);
        MakeComboWindow (&TAPIndex, NULL, COMT_SELECT, COMF_NOBUFFER | COMF_VSLIDER | COMF_SYNCSLIDER, 352, 264, 30, 256, 92, 30,
                         18, &SpecFont, FALSE, 0, NULL, DBLACK, DWHITE, 0, 0);
        TAPIndex.ComboWindow.ColorTable = SpecColTable;
        TAPIndex.BlinkRate = DBConfig.EmulateFlash ? (DBConfig.SlowFlash ? 960 : 320) : 0;
        TAPListing = TRUE;                                                          /* Add this combo window to the handling loop */
      }
      else
        ReInitComboWindow (&TAPIndex, 0);
    }
    else
    {
      if (TAPListing)
      {
        DestroyComboWindow (&TAPIndex, 0);
        TAPListing = FALSE;
      }
      ClearMainLower ();
    }
    switch (FileType)
    {
      case FT_SLT :
      case FT_Z80 : Wprintf (Window, 24, 3, "{+1Version          }N%.2f", (double)Z80Version / 100);
                    switch (Z80Version)
                    {
                      case 145 : strcpy (HWMode, "48K"); break;
                      case 201 : switch (Z80201SnapshotHeader.HWMode)
                                 {
                                   case 0 : strcpy (HWMode, "48K"); break;
                                   case 1 : strcpy (HWMode, "48K + Interface 1"); break;
                                   case 2 : strcpy (HWMode, "48K + SamRam"); break;
                                   case 3 : strcpy (HWMode, "128K"); break;
                                   case 4 : strcpy (HWMode, "128K + Interface 1"); break;
                                   default: strcpy (HWMode, "{+4<Unknown>}N");
                                 }
                                 break;
                      case 300 : switch (Z80201SnapshotHeader.HWMode)
                                 {
                                   case 0 : strcpy (HWMode, "48K"); break;
                                   case 1 : strcpy (HWMode, "48K + Interface 1"); break;
                                   case 2 : strcpy (HWMode, "48K + SamRam"); break;
                                   case 3 : strcpy (HWMode, "48K + M.G.T."); break;
                                   case 4 : strcpy (HWMode, "128K"); break;
                                   case 5 : strcpy (HWMode, "128K + Interface 1"); break;
                                   case 6 : strcpy (HWMode, "128K + M.G.T."); break;
                                   default: strcpy (HWMode, "{+4<Unknown>}N");
                                 }
                    }
                    Wprintf (Window, 25, 3, "{+1Hardware         }N%s", HWMode);
                    Wprintf (Window, 26, 3, "{+1Border           }N%d", (Z80145SnapshotHeader.Stat1 >> 1) & 0x07);
                    Wprintf (Window, 27, 3, "{+1Compressed       }N%s",
                             Z80Version != 145 || (Z80145SnapshotHeader.Stat1 & 0x20) ? "Yes" : "No");
                    Wprintf (Window, 28, 3, "{+1Issue 2 emu      }N%s", Z80145SnapshotHeader.Stat2 & 0x04 ? "Yes" : "No");
                    Wprintf (Window, 29, 3, "{+1R-reg emu        }N%s", 
                             Z80Version != 145 && (Z80201SnapshotHeader.Emulation & 0x01) ? "Yes" : "No");
                    Wprintf (Window, 30, 3, "{+1LDIR emu         }N%s",
                             Z80Version != 145 && (Z80201SnapshotHeader.Emulation & 0x02) ? "Yes" : "No");
                    Wprintf (Window, 31, 3, "{+1Double ints      }N%s", Z80145SnapshotHeader.Stat2 & 0x08 ? "Yes" : "No");
                    switch (Z80145SnapshotHeader.Stat2 & 0xC0)
                    {
                      case 0x00 : strcpy (HWMode, "Cursor"); break;
                      case 0x40 : strcpy (HWMode, "Kempston"); break;
                      case 0x80 : strcpy (HWMode, Z80Version == 300 ? "User Def" : "Interface 2 #1"); break;
                      case 0xC0 : strcpy (HWMode, "Interface 2 #2"); break;
                    }
                    Wprintf (Window, 32, 3, "{+1Joystick         }N%s", HWMode);
                    if (Z80Version == 300)
                    {
                      Wprintf (Window, 34, 3, "{+1User Def Keys:   }N");
                      Wprintf (Window, 35, 3, "{+1              Up }N%s",
                               UDefTable[Z80300SnapshotHeader.UKey3 & 0x0F][(Z80300SnapshotHeader.UKey3 >> 8) - 1]);
                      Wprintf (Window, 36, 3, "{+1            Down }N%s",
                               UDefTable[Z80300SnapshotHeader.UKey2 & 0x0F][(Z80300SnapshotHeader.UKey2 >> 8) - 1]);
                      Wprintf (Window, 37, 3, "{+1            Left }N%s",
                               UDefTable[Z80300SnapshotHeader.UKey0 & 0x0F][(Z80300SnapshotHeader.UKey0 >> 8) - 1]);
                      Wprintf (Window, 38, 3, "{+1           Right }N%s",
                               UDefTable[Z80300SnapshotHeader.UKey1 & 0x0F][(Z80300SnapshotHeader.UKey1 >> 8) - 1]);
                      Wprintf (Window, 39, 3, "{+1            Fire }N%s",
                               UDefTable[Z80300SnapshotHeader.UKey4 & 0x0F][(Z80300SnapshotHeader.UKey4 >> 8) - 1]);
                      if (!strcmp (UDefTable[Z80300SnapshotHeader.UKey0 & 0x0F][(Z80300SnapshotHeader.UKey0 >> 8) - 1], "\'1\'") &&
                          !strcmp (UDefTable[Z80300SnapshotHeader.UKey1 & 0x0F][(Z80300SnapshotHeader.UKey1 >> 8) - 1], "\'2\'") &&
                          !strcmp (UDefTable[Z80300SnapshotHeader.UKey2 & 0x0F][(Z80300SnapshotHeader.UKey2 >> 8) - 1], "\'3\'") &&
                          !strcmp (UDefTable[Z80300SnapshotHeader.UKey3 & 0x0F][(Z80300SnapshotHeader.UKey3 >> 8) - 1], "\'4\'") &&
                          !strcmp (UDefTable[Z80300SnapshotHeader.UKey4 & 0x0F][(Z80300SnapshotHeader.UKey4 >> 8) - 1], "\'5\'"))
                      Wprintf (Window, 40, 17, "(= Interface 2 #1)");
                    }
                    if (FileType == FT_SLT)
                    {
                      Wprintf (Window, 24, 30, "{+1Table   }N%07lu", SLTMap.TableStart);
                      Wprintf (Window, 25, 30, "{+1Entries }N%u", SLTMap.NumEntries);
                      Wprintf (Window, 26, 30, "{+1Data    }N%07lu", SLTMap.TableEnd);
                      if (SLTMap.InfoIncluded)
                        Wprintf (Window, 27, 30, "{+1Info    }N%07lu", SLTMap.OffsetInfo + SLTMap.TableEnd);
                      else
                        Wprintf (Window, 27, 30, "{+1Info    }N-");
                      if (SLTMap.ScrShotIncluded)
                        Wprintf (Window, 28, 30, "{+1ScrShot }N%07lu", SLTMap.OffsetScrShot + SLTMap.TableEnd);
                      else
                        Wprintf (Window, 28, 30, "{+1ScrShot }N-");
                      if (SLTMap.PokesIncluded)
                        Wprintf (Window, 29, 30, "{+1Pokes   }N%07lu", SLTMap.OffsetPokes + SLTMap.TableEnd);
                      else
                        Wprintf (Window, 29, 30, "{+1Pokes   }N-");
                    }
                    break;
      case FT_TAP :
      case FT_LTP : if (!InsertTAPIndex (&TAPIndex, FullPath))                                             /* Make a file catalog */
                      ReadError = TRUE;
                    else if (!FileNotFound)
                      PrintComboEntries (&TAPIndex);
                    break;
      case FT_TZX : if (!InsertTZXIndex (&TAPIndex, FullPath))                                             /* Make a file catalog */
                      ReadError = TRUE;
                    else if (!FileNotFound)
                      PrintComboEntries (&TAPIndex);
                    break;
      case FT_MDR : if (!InsertMDRIndex (&TAPIndex, FullPath))                                             /* Make a file catalog */
                      ReadError = TRUE;
                    else if (!FileNotFound)
                      PrintComboEntries (&TAPIndex);
                    break;
      default     : break;                                                                         /* (Nothing interesting known) */
    }
    if (ReadError)
    {
      if (TAPListing)
      {
        DestroyComboWindow (&TAPIndex, 0);
        TAPListing = FALSE;
      }
      ClearMainLower ();
      Wprintf (&MainWindow, 31, TEXT_CENTER, "{+4<Read error>}N");
    }
    if (FileNotFound)
      Wprintf (&MainWindow, 31, TEXT_CENTER, "{+4<File not found>}N");
  }
  else                                                                                                /* Button is being released */
  {
    if (TAPListing)
    {
      DestroyComboWindow (&TAPIndex, 0);
      TAPListing = FALSE;
    }
    ClearMainLower ();
  }
  return (WINH_NONE);
}

int HandleScrShot (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Screen' button has been pressed (or released).                                                                    */
/* Post  : The screen from the spectrum file has been shown on the display.                                                       */
/* Import: ClearMainLower, SurroundScreen.                                                                                        */
/**********************************************************************************************************************************/

{
  Window = Window;
  if ((Button->Flags & BF_PRESSED) && SelectedGame >= 0)
  {
    if (!BestScreenLoop)
    {
      ForceButtonRelease (Window, &MList);                                         /* Screen or List can be pressed, but not both */
      ForceButtonRelease (Window, &MScreen);
    }
    if (!SpectrumScreen)                                                                          /* Already a screen displayed ? */
      ClearMainLower ();
    FlashState = FALSE;                                                                                       /* Initialize flash */
    ReleaseTimedHandler (TimedFlash, &MainWindow);
    if (!RequestFloppy (FullPath[0], CurrentEntry.DiskId))
    {
      SpectrumScreen = FALSE;
      GhostMenuGIF ();
      GhostMenuSCR ();
      return (WINH_NONE);
    }
    else
      SetupGameButtons ();
    SpectrumScreen = TRUE;                                                                  /* Assume this operation will succeed */
    ScreenIsScrShot = TRUE;

    if (ScrShotFound)                                                                            /* Is there as screenshot file ? */
    {
      if (FileType == FT_SLT && SLTMap.ScrShotIncluded)                          /* Screenshots are inside the snap for SLT types */
      {
        MakeFullPath (PathBuffer, GamePath, CurrentEntry.PCName);
        PutScreenFromSLT (Window, 21, 230, PathBuffer);
      }
      else
        PutScreenFromSCR (Window, 21, 230, ScrShotPath);
      SurroundScreen (Window, 19, 228);
      if (ScreenContainsFlash && DBConfig.EmulateFlash)
        RegisterTimedHandler (DBConfig.SlowFlash ? 960 : 320, TimedFlash, &MainWindow);
    }
    else
    {
      ClearMainLower ();
      Wprintf (Window, 31, TEXT_CENTER, "{+4<No screenshot found>}N");
      SpectrumScreen = FALSE;
    }  
    if (SpectrumScreen)
      UnghostMenuGIF ();
    else
      GhostMenuGIF ();
    GhostMenuSCR ();                                                                  /* Creating a SCR from a SCR makes no sense */
  }
  else                                                                                                /* Button is being released */
  {
    ClearMainLower ();
    SpectrumScreen = FALSE;
    ReleaseTimedHandler (TimedFlash, &MainWindow);
  }
  return (WINH_NONE);
}

int HandlePrint (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Print' button has been pressed (or released).                                                                     */
/* Post  : The entire database has been printed on the printer.                                                                   */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
    PrintDatabase (TRUE);
  return (WINH_NONE);
}

int HandleAbout (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `About' button has been pressed (or released).                                                                     */
/* Post  : An about box has been shown to the user.                                                                               */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
    AboutBox (TRUE);
  return (WINH_NONE);
}

int HandleHelp (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : `Config.EditPath' is a non-empty string.                                                                               */
/* Post  : The `Config.EditPath' has been started on README.SGD (elementary help construction)                                    */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  char DocPath[MAXDIR];

  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
    if (!RequestFloppy (ProgramDrive[0], ResourceFloppyId))
      return (WINH_NONE);
    sprintf (DocPath, "%s%sREADME.SGD", ProgramDrive, ProgramPath);
    FileViewRequester (80, 100, 128, 80, 80, 25, DocPath, TRUE, "SGD Help");
  }
  return (WINH_NONE);
}

int HandleSysInfo (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `SysInfo' button has been pressed (or released).                                                                   */
/* Post  : A system info box has been shown to the user.                                                                          */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
    SysInfoBox ();
  return (WINH_NONE);
}

int HandlePoke (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Poke' button has been pressed (or released).                                                                      */
/* Post  : The game has been patched with the selected pokes, played and removed again.                                           */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
    if (InsertGamePokes ())
    {
      GameIsPoked = TRUE;
      PlayGamePoked = TRUE;
      HandlePlay (Window, Button);
      PlayGamePoked = FALSE;
      GameIsPoked = FALSE;
      unlink (PokedGamePath);
    }
  return (WINH_NONE);
}

int HandleEditPoke (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `EdPoke' button has been pressed (or released).                                                                    */
/* Post  : The game pokes database has been shown/updated.                                                                        */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
    EditGamePokes ();
    TestPokeGhosted ();
  }
  return (WINH_NONE);
}

int HandleInfo (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Info' button has been pressed (or released).                                                                      */
/* Post  : The info file (extension .TXT) has been viewed in a view requester.                                                    */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  char WHeader[D_NAME + 30];

  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
    if (!RequestFloppy (FullPath[0], CurrentEntry.DiskId))
      return (WINH_NONE);
    else
      SetupGameButtons ();
    if (InfoFound)
      if (InfoIsZXEdit)
        SpawnApplication ("ZX-Editor", DBConfig.ZXEditPath, NULL, InfoPath);
      else
      {
        sprintf (WHeader, "Game information: [%s]", CurrentEntry.Name);
        if (FileType == FT_SLT && SLTMap.InfoIncluded)
          ViewInfoFromSLT (100, 100, 128, 64, 64, 25, InfoPath, WHeader);
        else
          FileViewRequester (80, 100, 128, 80, 80, 25, InfoPath, FALSE, WHeader);
      }
  }
  return (WINH_NONE);
}

int HandleInlay (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Inlay' button has been pressed (or released).                                                                     */
/* Post  : The JPG/GIF file has been viewed using an external, spawned application.                                               */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  bool       Continue;
  byte       PicType;
  char      *UsedViewer;
  char      *UsedViewerOptions;
  char      *PCNameExt;

  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
    if (!RequestFloppy (FullPath[0], CurrentEntry.DiskId))
      return (WINH_NONE);
    else
      SetupGameButtons ();
    if (InlayFound)
    {
      PCNameExt = InlayPath + strlen (InlayPath) - 4;
      PicType = (byte)(strcmp (PCNameExt, "GIF") == 0 ? 0 : 1);
      UsedViewer = (PicType == 0x00 ? DBConfig.GIFViewerPath : DBConfig.JPGViewerPath);
      UsedViewerOptions = (PicType == 0x00 ? DBConfig.GIFViewerOptions : DBConfig.JPGViewerOptions);
      Continue = TRUE;
      if (UsedViewer[0] == '\0')                                                                           /* No viewer defined ? */
      {
        Continue = FALSE;
        if (!QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Picture Viewer Wizard",
                                "No %s viewer program path has not been set in your configuration.\n"
                                "Do you want to select it now?", PicType == 0x00 ? "GIF" : "JPG"))
          Continue = GetConfNewFileName (PicType == 0x00 ? "Select GIF Viewer Path" : "Select JPG Viewer Path", UsedViewer,
                                         UsedViewerOptions, "view a file");
      }
      if (Continue)
        SpawnApplication ("picture viewer", UsedViewer, UsedViewerOptions, InlayPath);                        /* Start the viewer */
    }
  }
  return (WINH_NONE);
}

int HandleEditInfo (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Edit info' button has been pressed (or released).                                                                 */
/* Post  : The info file (extension .TXT) has been edited in a seperate task. If the directory did not exist, it is created.      */
/* Import: TestInfoGhosted.                                                                                                       */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
    if (!RequestFloppy (FullPath[0], CurrentEntry.DiskId))
      return (WINH_NONE);
    else
      SetupGameButtons ();
    if (!InfoDirExists)                                                                     /* Does the directory exist already ? */
      if (!QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Create Directory",
          "The INFO directory\n    %s\ndoes not exist yet.\nDo you want to create it now ?", InfoDir))
        if (mkdir (InfoDir))                                                                       /* Try to create the directory */
          ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Could not create directory:\n%s", strerror (errno));
        else
          InfoDirExists = TRUE;
    if (InfoDirExists)                                                                        /* Does the directory exist (now) ? */
    {
      if (InfoIsZXEdit)
        SpawnApplication ("ZX-Editor", DBConfig.ZXEditPath, NULL, InfoPath);
      else
        SpawnApplication ("ASCII Editor", DBConfig.EditPath, NULL, InfoPath);
      TestInfoGhosted ();                                                      /* Unghost the `Info' button if a file was created */
    }
  }
  return (WINH_NONE);
}

int HandleSelMem (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Radio' button for the memory type has been pressed (or released).                                                 */
/* Post  : The memory type has been toggled between 16, 48, 48/128, 128, USR0, +2/+2A, +3 and Pentagon.                           */
/* Import: TestEmptyDbase.                                                                                                        */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
#define MAXMEM    8
    char              d_StdMem[MAXMEM][7] = { { "    16" },
                                              { "    48" },
                                              { "   128" },
                                              { "48/128" },
                                              { "  USR0" },
                                              { "+2/+2A" },
                                              { "    +3" },
                                              { "Pentag" } };

    struct ComboInfo  PWindow;
    struct ComboEntry SEntry;
    struct ButtonInfo *ButtonP;
    short   register  Cnt;
    bool              Finished = FALSE;
    comboent          SelectedLine;

    TestEmptyDbase ();
    MakeComboWindow (&PWindow, &MainWindow, COMT_SELECT, COMF_VSLIDER | COMF_SYNCSLIDER | COMF_KEYSELECT | COMF_EXCLUSIVE,
                     MainWindow.StartX + 241, MainWindow.StartY + 86, 0, 1024, 7, 6, MAXMEM, NULL, FALSE, 0,
                     NULL, DBLACK, DWHITE, 0, 0);
    SEntry.Selected = 0;
    for (Cnt = 0 ; Cnt < MAXMEM ; Cnt ++)                                                                   /* Build options list */
    {
      SEntry.Identifier = Cnt;
      strcpy (SEntry.Text, d_StdMem[Cnt]);
      if (!InsertComboEntry (&PWindow, &SEntry, NULL, -1))
        FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
    }
    switch (CurrentEntry.Memory)
    {
      case  16 : PWindow.LastSelected = 0; break;
      case  48 : PWindow.LastSelected = 1; break;
      case 128 : PWindow.LastSelected = 2; break;
      case 176 : PWindow.LastSelected = 3; break;
      case 129 : PWindow.LastSelected = 4; break;
      case 130 : PWindow.LastSelected = 5; break;
      case 131 : PWindow.LastSelected = 6; break;
      case 228 : PWindow.LastSelected = 7; break;
      default  : PWindow.LastSelected = 1; break;
    }
    PrintComboEntries (&PWindow);
    while (!Finished)
    {
      HandleWindow (&(PWindow.ComboWindow), &ButtonP);                                                /* (Just read the keyboard) */
      if (HandleComboWindow (&PWindow, &SelectedLine) == WINH_LINEFIRST)
      {
        Finished = TRUE;
        switch (PWindow.LastSelected)
        {
          case   0 : CurrentEntry.Memory = 16;  strcpy (SMemory, "    16"); break;
          case   1 : CurrentEntry.Memory = 48;  strcpy (SMemory, "    48"); break;
          case   2 : CurrentEntry.Memory = 128; strcpy (SMemory, "   128"); break;
          case   3 : CurrentEntry.Memory = 176; strcpy (SMemory, "48/128"); break;
          case   4 : CurrentEntry.Memory = 129; strcpy (SMemory, "  USR0"); break;
          case   5 : CurrentEntry.Memory = 130; strcpy (SMemory, "+2/+2A"); break;
          case   6 : CurrentEntry.Memory = 131; strcpy (SMemory, "    +3"); break;
          case   7 : CurrentEntry.Memory = 228; strcpy (SMemory, "Pentag"); break;
        }
        UpdateButtonText (&MainWindow, &BMemory, BF_FITTXT);
        EntryChanged = TRUE;
        SignalDBChanged (TRUE);
        if (ReselectAnEntry (&GameSelectWindow, SelectedGame, &CurrentEntry, FALSE))
        {
          PrintComboEntries (&GameSelectWindow);
          ReportTotal ();
        }
        GlobalKey = 0x0000;
      }
      else
      {
        if (Mouse.Xx < PWindow.OverlayWindow.A_StartX || Mouse.Xx > PWindow.OverlayWindow.A_EndX ||
            Mouse.Yy < PWindow.OverlayWindow.A_StartY || Mouse.Yy > PWindow.OverlayWindow.A_EndY)
          if (Mouse.Left)                                                                  /* Left click outside the select box ? */
          {
            Finished = TRUE;
            DelayLeftMouseButton ();
          }
      }
      if (GlobalKey == K_ESC)
      {
        Finished = TRUE;
        GlobalKey = 0x0000;
      }
      MouseStatus ();
    }
    DestroyComboWindow (&PWindow, 0);
    GlobalKey = 0x0000;
  }
  return (WINH_NONE);
}

int HandleSelEmul (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Radio' button for the emulator override has been pressed (or released).                                           */
/* Post  : The emulator override field has been increased.                                                                        */
/* Import: TestEmptyDbase, GetDropDownEmulator.                                                                                   */
/**********************************************************************************************************************************/

{
  bool  ReprintEntries = FALSE;
  short OldValue;

  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
    TestEmptyDbase ();
    if (SpectrumScreen && ScreenContainsFlash && DBConfig.EmulateFlash)
      ReleaseTimedHandler (TimedFlash, &MainWindow);
    OldValue = CurrentEntry.OverrideEmulator;
    CurrentEntry.OverrideEmulator = GetDropDownEmulator (&MainWindow, 67, 206, CurrentEntry.OverrideEmulator, TRUE);
    if (SpectrumScreen && ScreenContainsFlash && DBConfig.EmulateFlash)
      RegisterTimedHandler (DBConfig.SlowFlash ? 960 : 320, TimedFlash, &MainWindow);
    if (CurrentEntry.OverrideEmulator != OldValue)
    {
      GetEmulatorName (CurrentEntry.OverrideEmulator, SEmulOverride);
      UpdateButtonText (&MainWindow, &BEmulOver, BF_FITTXT);
      EntryChanged = TRUE;
      SignalDBChanged (TRUE);
      EmulatorChanged = TRUE;
      if (TestAGame (SelectedGame, TRUE))
        ReprintEntries = TRUE;
      EmulatorChanged = FALSE;
      if (ReselectAnEntry (&GameSelectWindow, SelectedGame, &CurrentEntry, FALSE))
      {
        ReprintEntries = TRUE;
        ReportTotal ();
      }
      if (ReprintEntries)
        PrintComboEntries (&GameSelectWindow);
    }
  }
  return (WINH_NONE);
}

int HandleSelMulti (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Radio' button for multi-load has been pressed (or released).                                                      */
/* Post  : The multi-load field has been increased.                                                                               */
/* Import: TestEmptyDbase.                                                                                                        */
/**********************************************************************************************************************************/

{
  bool ReprintEntries = FALSE;

  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
    TestEmptyDbase ();
    switch (CurrentEntry.MultiLoad)
    {
      case 1 : CurrentEntry.MultiLoad = 4; strcpy (SMultiLoad, "48K"); break;
      case 4 : CurrentEntry.MultiLoad = 0; strcpy (SMultiLoad, "no"); break;
      default: CurrentEntry.MultiLoad = 1; strcpy (SMultiLoad, "yes"); break;
    }
    UpdateButtonText (&MainWindow, &BMultiLoad, BF_FITTXT);
    EntryChanged = TRUE;
    SignalDBChanged (TRUE);
    if (TestAGame (SelectedGame, TRUE))
      ReprintEntries = TRUE;
    if (ReselectAnEntry (&GameSelectWindow, SelectedGame, &CurrentEntry, FALSE))
    {
      ReprintEntries = TRUE;
      ReportTotal ();
    }
    if (ReprintEntries)
      PrintComboEntries (&GameSelectWindow);
  }
  return (WINH_NONE);
}

int HandleSelLang (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The dropdown gadget for the game Language field has been pressed (or released).                                        */
/* Post  : The game language has been selected from the drop-down menu.                                                           */
/* Import: TestEmptyDbase.                                                                                                        */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
#define MAXLANGS  6
    char              d_StdLang[MAXLANGS][D_LANGUAGE + 1] = { { "   " }, { "Eng" }, { "Spa" }, { "Ger" }, { "Pol" }, { "Rus" } };

    struct ComboInfo  PWindow;
    struct ComboEntry SEntry;
    struct ButtonInfo *ButtonP;
    short   register  Cnt;
    bool              Finished = FALSE;
    comboent          SelectedLine;
    short             ChosenLang = 0;

    TestEmptyDbase ();
    if (SpectrumScreen && ScreenContainsFlash && DBConfig.EmulateFlash)
      ReleaseTimedHandler (TimedFlash, &MainWindow);
    MakeComboWindow (&PWindow, &MainWindow, COMT_SELECT, COMF_VSLIDER | COMF_SYNCSLIDER | COMF_KEYSELECT | COMF_EXCLUSIVE,
                     MainWindow.StartX + 73, MainWindow.StartY + 206, 0, 1024, D_LANGUAGE + 2, D_LANGUAGE, MAXLANGS, NULL, FALSE, 0,
                     NULL, DBLACK, DWHITE, 0, 0);
    SEntry.Selected = 0;
    for (Cnt = 0 ; Cnt < MAXLANGS ; Cnt ++)                                                   /* Start with the default languages */
    {
      SEntry.Identifier = Cnt;
      strcpy (SEntry.Text, d_StdLang[Cnt]);
      if (!InsertComboEntry (&PWindow, &SEntry, NULL, -1))
        FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
    }
    for (Cnt = 0 ; Cnt < DBConfig.NumAddedLanguages ; Cnt ++)                                   /* Add the user defined languages */
    {
      SEntry.Identifier = MAXLANGS + Cnt;
      strcpy (SEntry.Text, DBConfig.AddedLanguage[Cnt].Language);
      if (!InsertComboEntry (&PWindow, &SEntry, NULL, -1))
        FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
    }
    PWindow.LastSelected = ChosenLang;
    PrintComboEntries (&PWindow);
    while (!Finished)
    {
      MouseStatus ();
      HandleWindow (&(PWindow.ComboWindow), &ButtonP);                                                /* (Just read the keyboard) */
      if (HandleComboWindow (&PWindow, &SelectedLine) == WINH_LINEFIRST)
      {
        Finished = TRUE;
        ChosenLang = PWindow.LastSelected;
        GlobalKey = 0x0000;
      }
      else
      {
        if (Mouse.Xx < PWindow.OverlayWindow.A_StartX || Mouse.Xx > PWindow.OverlayWindow.A_EndX ||
            Mouse.Yy < PWindow.OverlayWindow.A_StartY || Mouse.Yy > PWindow.OverlayWindow.A_EndY)
          if (Mouse.Left)                                                                  /* Left click outside the select box ? */
          {
            Finished = TRUE;
            DelayLeftMouseButton ();
          }
      }
      if (GlobalKey == K_ESC)
      {
        Finished = TRUE;
        GlobalKey = 0x0000;
      }
    }
    DestroyComboWindow (&PWindow, 0);
    if (SpectrumScreen && ScreenContainsFlash && DBConfig.EmulateFlash)
      RegisterTimedHandler (DBConfig.SlowFlash ? 960 : 320, TimedFlash, &MainWindow);
    GlobalKey = 0x0000;
    if (ChosenLang > 0)
    {
      if (ChosenLang < MAXLANGS)                                                                        /* An internal language ? */
        strcpy (CurrentEntry.Language, d_StdLang[ChosenLang]);
      else                                                                                        /* (One of the added languages) */
        strcpy (CurrentEntry.Language, DBConfig.AddedLanguage[ChosenLang - MAXLANGS].Language);
      UpdateButtonText (&MainWindow, &BLanguage, BF_FITTXT);
      EntryChanged = TRUE;
      SignalDBChanged (TRUE);
      if (ReselectAnEntry (&GameSelectWindow, SelectedGame, &CurrentEntry, FALSE))
      {
        PrintComboEntries (&GameSelectWindow);
        ReportTotal ();
      }
    }
  }
  return (WINH_NONE);
}

int HandleSelOrigin (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The dropdown gadget for the game Origin field has been pressed (or released).                                          */
/* Post  : The games origin has been selected from the drop-down menu.                                                            */
/* Import: TestEmptyDbase.                                                                                                        */
/**********************************************************************************************************************************/

{
  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
#define MAXORIG   9
    char              d_StdOrigin[MAXORIG][32] = { { "(unknown)                      " },
                                                   { "Original release            - O" },
                                                   { "Re-release                  - R" },
                                                   { "Compilation release         - C" },
                                                   { "Magazine cover-tape version - M" },
                                                   { "Type-In                     - T" },
                                                   { "Hacked/cracked version      - H" },
                                                   { "Incomplete version          - I" },
                                                   { "Buggered/corrupt version    - B" } };

    struct ComboInfo  PWindow;
    struct ComboEntry SEntry;
    struct ButtonInfo *ButtonP;
    short   register  Cnt;
    bool              Finished = FALSE;
    comboent          SelectedLine;

    TestEmptyDbase ();
    if (SpectrumScreen && ScreenContainsFlash && DBConfig.EmulateFlash)
      ReleaseTimedHandler (TimedFlash, &MainWindow);
    MakeComboWindow (&PWindow, &MainWindow, COMT_SELECT, COMF_VSLIDER | COMF_SYNCSLIDER | COMF_KEYSELECT | COMF_EXCLUSIVE,
                     MainWindow.StartX + 61, MainWindow.StartY + 166, 0, 1024, 32, 31, MAXORIG, NULL, FALSE, 0,
                     NULL, DBLACK, DWHITE, 0, 0);
    SEntry.Selected = 0;
    for (Cnt = 0 ; Cnt < MAXORIG ; Cnt ++)                                                                  /* Build options list */
    {
      SEntry.Identifier = Cnt;
      strcpy (SEntry.Text, d_StdOrigin[Cnt]);
      if (!InsertComboEntry (&PWindow, &SEntry, NULL, -1))
        FatalError (1, _ErrOutOfMemory, sizeof (struct ComboEntry));
    }
    switch (CurrentEntry.Origin)
    {
      case 'O' : PWindow.LastSelected = 1; break;
      case 'R' : PWindow.LastSelected = 2; break;
      case 'C' : PWindow.LastSelected = 3; break;
      case 'M' : PWindow.LastSelected = 4; break;
      case 'T' : PWindow.LastSelected = 5; break;
      case 'H' : PWindow.LastSelected = 6; break;
      case 'I' : PWindow.LastSelected = 7; break;
      case 'B' : PWindow.LastSelected = 8; break;
      default  : PWindow.LastSelected = 0; break;
    }
    PrintComboEntries (&PWindow);
    while (!Finished)
    {
      HandleWindow (&(PWindow.ComboWindow), &ButtonP);                                                /* (Just read the keyboard) */
      if (HandleComboWindow (&PWindow, &SelectedLine) == WINH_LINEFIRST)
      {
        Finished = TRUE;
        switch (PWindow.LastSelected)
        {
          case   0 : CurrentEntry.Origin = ' '; break;
          case   1 : CurrentEntry.Origin = 'O'; break;
          case   2 : CurrentEntry.Origin = 'R'; break;
          case   3 : CurrentEntry.Origin = 'C'; break;
          case   4 : CurrentEntry.Origin = 'M'; break;
          case   5 : CurrentEntry.Origin = 'T'; break;
          case   6 : CurrentEntry.Origin = 'H'; break;
          case   7 : CurrentEntry.Origin = 'I'; break;
          case   8 : CurrentEntry.Origin = 'B'; break;
        }
        UpdateButtonText (&MainWindow, &BOrigin, BF_FITTXT);
        EntryChanged = TRUE;
        SignalDBChanged (TRUE);
        if (ReselectAnEntry (&GameSelectWindow, SelectedGame, &CurrentEntry, FALSE))
        {
          PrintComboEntries (&GameSelectWindow);
          ReportTotal ();
        }
        GlobalKey = 0x0000;
      }
      else
      {
        if (Mouse.Xx < PWindow.OverlayWindow.A_StartX || Mouse.Xx > PWindow.OverlayWindow.A_EndX ||
            Mouse.Yy < PWindow.OverlayWindow.A_StartY || Mouse.Yy > PWindow.OverlayWindow.A_EndY)
          if (Mouse.Left)                                                                  /* Left click outside the select box ? */
          {
            Finished = TRUE;
            DelayLeftMouseButton ();
          }
      }
      if (GlobalKey == K_ESC)
      {
        Finished = TRUE;
        GlobalKey = 0x0000;
      }
      MouseStatus ();
    }
    DestroyComboWindow (&PWindow, 0);
    GlobalKey = 0x0000;
    if (SpectrumScreen && ScreenContainsFlash && DBConfig.EmulateFlash)
      RegisterTimedHandler (DBConfig.SlowFlash ? 960 : 320, TimedFlash, &MainWindow);
  }
  return (WINH_NONE);
}

int HandleSelFile (struct WindowInfo *Window, struct ButtonInfo *Button)

/**********************************************************************************************************************************/
/* Pre   : The `Browse' button for the PCName has been pressed (or released).                                                     */
/* Post  : A file requester has been handled.                                                                                     */
/* Import: TestEmptyDbase, TestAGame, SelectGame.                                                                                 */
/**********************************************************************************************************************************/

{
  char Path[MAXDIR + 1];
  char File[13];
  char Mask[13] = "*.*";
  char OldIndex[D_DIRINDEX];
  bool ReprintEntries = FALSE;

  Window = Window;
  if (!(Button->Flags & BF_PRESSED))
  {
    TestEmptyDbase ();
    if (GamePathFound)
    {
      strcpy (Path, GamePath);
      strcpy (File, CurrentEntry.PCName);
    }
    else
    {
      strcpy (Path, LastAddPath);
      if (CurrentEntry.PCName[0] != '\0')
        strcpy (File, CurrentEntry.PCName);
      else
        strcpy (File, LastAddFileSet ? LastAddFile : CurrentEntry.PCName);
    }
    strcpy (OldIndex, CurrentEntry.DirIndex);
    if (FileRequester (180, 50, 48, 29, 20, Path, File, Mask, "Select a Spectrum File", TRUE, FALSE))
    {
      sprintf (CurrentEntry.DirIndex, "%d", TestForNewMask (Path));
      if (strcmp (File, CurrentEntry.PCName) || strcmp (CurrentEntry.DirIndex, OldIndex))
      {
        strcpy (CurrentEntry.PCName, File);
        strcpy (LastAddPath, Path);
        strcpy (GamePath, Path);
        strcpy (LastAddFile, File);
        if (IsFloppy (Path[0]))
          CurrentEntry.DiskId = CurrentFloppyId;
        LastAddFileSet = TRUE;
        MakeFullPath (FullPath, GamePath, CurrentEntry.PCName);
        EntryChanged = TRUE;
        SignalDBChanged (TRUE);
        if (TestAGame (SelectedGame, TRUE))
          ReprintEntries = TRUE;
        SelectGame (SelectedGame);
        ValidateAnEntry (SelectedGame, TRUE, TRUE);
        if (ReselectAnEntry (&GameSelectWindow, SelectedGame, &CurrentEntry, FALSE))
        {
          ReprintEntries = TRUE;
          ReportTotal ();
        }
        if (ReprintEntries)
          PrintComboEntries (&GameSelectWindow);
      }
    }
    if (MaxFloppyIdChanged)
      WriteConfig ();
  }
  return (WINH_NONE);
}

bool TestScrShotDirExists (void)

/**********************************************************************************************************************************/
/* Pre   : The `ScrShot' variables have been set up.                                                                              */
/* Post  : If the screenshot directory did not exist yet, it has been created. TRUE is returned if it exists (now).               */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  if (!ScrShotDirExists)                                                                    /* Does the directory exist already ? */
    if (!QuestionRequester (-1, -1, FALSE, FALSE, NULL, "@Cancel", NULL, NULL, NULL, "Confirm Create Directory",
        "The SCRSHOT directory\n    %s\ndoes not exist yet.\nDo you want to create it now ?", SCRSHOTDIR))
      if (mkdir (ScrShotDir))                                                                      /* Try to create the directory */
        ErrorRequester (-1, -1, NULL, NULL, NULL, "ERROR - Could not create directory:\n%s", strerror (errno));
      else
        ScrShotDirExists = TRUE;
  return (ScrShotDirExists);
}

void HandleCreateScrShot (void)

/**********************************************************************************************************************************/
/* Pre   : The `Create SCR' menu entry has been chosen.                                                                           */
/* Post  : A screenshot file has been created.                                                                                    */
/* Import: TestScrShotDirExists, TestScrShotGhosted, TestFileOverwrite.                                                           */
/**********************************************************************************************************************************/

{
  if (TestScrShotDirExists ())                                                                /* Does the directory exist (now) ? */
    if (!TestFileOverwrite (ScrShotPath))
    {
      if (ScreenIsScrShot)                                                                      /* The ScrShot button is active ? */
        CreateScreenFromSLT (FullPath);                                                       /* Otherwise this makes no sense... */
      else
      {
        switch (FileType)                                                                    /* Find and draw the spectrum screen */
        {
          case FT_SLT :
          case FT_Z80 : CreateScreenFromZ80 (FullPath); break;
          case FT_SNA :
          case FT_SNP :
          case FT_SEM : CreateScreenFromSNA (FullPath); break;
          case FT_TAP :
          case FT_LTP : CreateScreenFromTAP (FullPath); break;
          case FT_TZX : CreateScreenFromTZX (FullPath); break;
        }
        TestScrShotGhosted ();
      }
    }
}

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> MAIN PROGRAM <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

void main (int argc, char **argv)
{
  struct ButtonInfo *ButtonP;
  struct ComboEntry  NEntry;
  comboent register  Cnt;
  comboent           SelectedLine;
  bool               RequestQuit        = FALSE;
  bool               TimeToQuit         = FALSE;
  bool               CancelQuit         = TRUE;
  bool               Found;
  bool               DisableMSWin       = FALSE;
  int                ArgCnt             = 0;

  //if (DBELEN != DBLEN)                                                                          /* VERY important sanity check! */
    //FatalError (0, "DBELEN is %d bytes!", DBELEN);
  SWindow = &GameSelectWindow;                                                   /* Export the select window to the other modules */
  TWindow = &TAPIndex;
  SelectedGame = -1;                                                                           /* Initialize the global variables */
  FileType = FT_UNKNOWN;
  GamePathFound = FALSE;
  InfoFound = FALSE;
  InfoDirExists = FALSE;
  TAPListing = FALSE;
  MDRListing = FALSE;
  EntryChanged = DatabaseChanged = FALSE;                                                        /* Prepare for an empty database */
  DBase.NumEntries = -1;
  NewDatabase = FALSE;
  FlashState = FALSE;
  SetDatabaseName (DBNAME, FALSE);                                                    /* Prepare to use the default database name */
  CriterionEmpty = TRUE;
  Convert107 = FALSE;
  Convert109 = FALSE;
  Convert117 = FALSE;
  Convert121 = FALSE;
  Convert129 = FALSE;
  Convert130 = FALSE;
  Convert136 = FALSE;
  Convert202 = FALSE;
  ConvertImprove = FALSE;
  NoPercentageBar = FALSE;
  memset (&CurrentEntry, 0, sizeof (CurrentEntry));
  while (++ ArgCnt < argc)                                                                       /* Handle command line arguments */
    if (argv[ArgCnt][0] == '-' || argv[ArgCnt][0] == '/')                                                           /* A switch ? */
      switch (toupper (argv[ArgCnt][1]))
      {
        case 'C' : switch (argv[ArgCnt][2])
                   {
                     case '\0' : fprintf (stderr, "FATAL ERROR - Missing conversion type in switch \'C\'\n"); exit (1);
                     case '1'  : Convert107 = TRUE; break;
                     case '2'  : Convert109 = TRUE; break;
                     case '3'  : Convert117 = TRUE; break;
                     case '4'  : Convert121 = TRUE; break;
                     case '5'  : Convert129 = TRUE; break;
                     case '6'  : Convert130 = TRUE; break;
                     case '7'  : Convert136 = TRUE; break;
                     case '8'  : Convert202 = TRUE; break;
                     default   : fprintf (stderr, "FATAL ERROR - Unknown conversion type in switch \'%s\'\n", argv[ArgCnt] + 1);
                                 exit (1);
                   }
                   break;
        case 'P' : NoPercentageBar = TRUE; break;
        case 'W' : DisableMSWin = TRUE; break;
        case 'U' : break;                                                                                           /* (obsolete) */
        case 'S' : HaveSoundcard = FALSE; break;
        case 'M' : HaveMixer = FALSE; break;
        case '?' :
        case 'H' : fprintf (stderr, "SYNTAX: SGD [-C<1/2/3/4/5/6/7/8>] [-P] [-W] [-S] [-M] [<databasename>]\n");
                   fprintf (stderr, "        -C : Convert your database\n");
                   fprintf (stderr, "        -P : disable the load/save Percentage requester\n");
                   fprintf (stderr, "        -W : disable Windows \`multitasking\'\n");
                   fprintf (stderr, "        -S : do not check for a Soundcard\n");
                   fprintf (stderr, "        -M : do not check for a software Mixer on the soundcard\n");
                   fprintf (stderr, "See README.SGD and COPYING for details\n");
                   exit (0);
        default  : fprintf (stderr, "FATAL ERROR - Unknown switch \'%s\'\n", argv[ArgCnt] + 1);
                   exit (1);
      }
    else                                                                                        /* A database name has been given */
      SetDatabaseName (strupr (argv[ArgCnt]), FALSE);
  InitGUI (argv[0], LBLACK, SpecPalette, DisableMSWin, HaveSoundcard, HaveMixer);
  AllocateScreenBuffer ();
  _makepath (ConfigFile, ProgramDrive, ProgramPath, ProgramName, CONFIGEXT);              /* Build name of the configuration file */
  strcpy (CurrentPath, ProgramDrive);
  strcat (CurrentPath, ProgramPath);
  ReadConfig ();                                                                                        /* Read the configuration */
  SpecColTable[8] = DBConfig.RemapBrightBlack ? 8 : 0;
  if (DBConfig.DirectVideoControl)                                                         /* The RIL must not be used if found ? */
    Screen.UseRIL = FALSE;
  if (DBConfig.UseCustomPalette)                                                            /* User want his own custom palette ? */
  {
    SpecPalette[0]  = DBConfig.CustomColor00;   /* VGA  0: DBLACK   */
    SpecPalette[1]  = DBConfig.CustomColor01;   /* VGA  1: DBLUE    */
    SpecPalette[2]  = DBConfig.CustomColor04;   /* VGA  2: DGREEN   */
    SpecPalette[3]  = DBConfig.CustomColor05;   /* VGA  3: DCYAN    */
    SpecPalette[4]  = DBConfig.CustomColor02;   /* VGA  4: DRED     */
    SpecPalette[5]  = DBConfig.CustomColor03;   /* VGA  5: DMAGENTA */
    SpecPalette[6]  = DBConfig.CustomColor06;   /* VGA  6: DYELLOW  */
    SpecPalette[7]  = DBConfig.CustomColor07;   /* VGA  7: DWHITE   */
    SpecPalette[8]  = DBConfig.CustomColor08;   /* VGA  8: LBLACK   */
    SpecPalette[9]  = DBConfig.CustomColor09;   /* VGA  9: LBLUE    */
    SpecPalette[10] = DBConfig.CustomColor12;   /* VGA 10: LGREEN   */
    SpecPalette[11] = DBConfig.CustomColor13;   /* VGA 11: LCYAN    */
    SpecPalette[12] = DBConfig.CustomColor10;   /* VGA 12: LRED     */
    SpecPalette[13] = DBConfig.CustomColor11;   /* VGA 13: LMAGENTA */
    SpecPalette[14] = DBConfig.CustomColor14;   /* VGA 14: LYELLOW  */
    SpecPalette[15] = DBConfig.CustomColor15;   /* VGA 15: LWHITE   */
    RemapAllPalette (SpecPalette);                                                                 /* Activate the custom palette */
  }
  if (DBConfig.BlankerOn)
    InstallBlanker (DBConfig.BlankerTimeOut, NULL);
  else
    DisableBlanker ();
  strcpy (LastAddPath, CurPath);
  LastAddFile[0] = '\0';
  _UsedMenuFont = &UsedMenuFont;
  SpectrumFont = &SpecFont;                                                      /* Export the Spectrum font to the other modules */
  if (DBConfig.PreLoadResources)
  {
    SelectFont (&SpecFont, FONT_NORMAL);
    SelectFont (DefaultFont, FONT_NORMAL);
    SelectFont (DefaultFont, FONT_BOLD);
    SelectFont (DefaultFont, FONT_ITALICS);
    SelectFont (_UsedMenuFont, FONT_NORMAL);
  }

  AboutBox (FALSE);

  if (!ReadDatabase (FALSE, TRUE, FALSE))                                                           /* Read the database into XMS */
    FatalError (10, "Database found corrupt while reading entry %d", DBase.NumEntries + 1);
  if (CancelAutoConvert)                                                             /* Auto-Convert request has been cancelled ? */
    FatalError (11, "Auto-Conversion cancelled");
  SelectedAPen[1] = DRED;                                                       /* Non-playable games will be printed in dark red */
  SelectedBPen[2] = DCYAN;                                                 /* Games that match the criterion with blue background */
  SelectedAPen[3] = DRED;
  SelectedBPen[3] = DCYAN;
  RemapSpectrumUDGs (TRUE, TRUE, NULL);                                                                    /* Initialize the UDGs */

  DestroyWindow (&AboutWindow);

  AddButton (&OverlayWind, &TTotals, FALSE);                                                                 /* Set up the screen */
  AddButton (&OverlayWind, &TCriterion, FALSE);
  TCriterion.MaxInputLength = 256;
  NumMatches = 0;
  DrawWindow (&OverlayWind, NULL, 0, TRUE);
  Wprintf (&OverlayWind, 42, 1, "{BS}B");
  NewEntry.Selected = 0x00;
  if (DatabaseTruncated)
    MAddEntry.Flags |= BF_GHOSTED;
  GhostMenuCompress ();
  AddButton (&MainWindow, &BName, FALSE);
  AddButton (&MainWindow, &BPublisher, FALSE);
  AddButton (&MainWindow, &BAuthor, FALSE);
  AddButton (&MainWindow, &BYear, FALSE);
  AddButton (&MainWindow, &BMemory, FALSE);
  AddButton (&MainWindow, &BMemSel, FALSE);
  AddButton (&MainWindow, &BPlayers, FALSE);
  AddButton (&MainWindow, &BTogether, FALSE);
  AddButton (&MainWindow, &BScore, FALSE);
  AddButton (&MainWindow, &BPCName, FALSE);
  AddButton (&MainWindow, &BPCNSel, FALSE);
  AddButton (&MainWindow, &BType, FALSE);
  AddButton (&MainWindow, &BStick1, FALSE);
  AddButton (&MainWindow, &BStick2, FALSE);
  AddButton (&MainWindow, &BStick3, FALSE);
  AddButton (&MainWindow, &BStick4, FALSE);
  AddButton (&MainWindow, &BStick5, FALSE);
  AddButton (&MainWindow, &BOrigScreen, FALSE);
  AddButton (&MainWindow, &BOrigin, FALSE);
  AddButton (&MainWindow, &BOriginSel, FALSE);
  AddButton (&MainWindow, &BAYSound, FALSE);
  AddButton (&MainWindow, &BMultiLoad, FALSE);
  AddButton (&MainWindow, &BMultiSel, FALSE);
  AddButton (&MainWindow, &BLanguage, FALSE);
  AddButton (&MainWindow, &BLangSel, FALSE);
  AddButton (&MainWindow, &BEmulOver, FALSE);
  AddButton (&MainWindow, &BEmulSel, FALSE);
  AddButton (&MainWindow, &BPath, FALSE);
  AddButton (&MainWindow, &BFileSize, FALSE);
  AddButton (&MainWindow, &MAddEntry, FALSE);
  AddButton (&MainWindow, &MDelEntry, FALSE);
  AddButton (&MainWindow, &MSave, FALSE);
  AddButton (&MainWindow, &MReload, FALSE);
  AddButton (&MainWindow, &MPlay, FALSE);
  AddButton (&MainWindow, &MPoke, FALSE);
  AddButton (&MainWindow, &MConfig, FALSE);
  AddButton (&MainWindow, &MShell, FALSE);
  AddButton (&MainWindow, &MQuit, FALSE);
  AddButton (&MainWindow, &MScreen, FALSE);
  AddButton (&MainWindow, &MList, FALSE);
  AddButton (&MainWindow, &MScrShot, FALSE);
  AddButton (&MainWindow, &MInlay, FALSE);
  AddButton (&MainWindow, &MInfo, FALSE);
  AddButton (&MainWindow, &MEditInfo, FALSE);
  AddButton (&MainWindow, &MEditPoke, FALSE);
  AddButton (&MainWindow, &MPrint, FALSE);
  AddButton (&MainWindow, &MHelp, FALSE);
  AddButton (&MainWindow, &MAbout, FALSE);
  AddButton (&MainWindow, &MSysInfo, FALSE);
  AddButton (&MainWindow, &TDBName, FALSE);
  AddButton (&MainWindow, &TDBChanged, FALSE);
  DrawWindow (&MainWindow, NULL, 0, TRUE);
  NonexistingDatabase = (DBase.NumEntries == -1 ? TRUE : FALSE);                                  /* New (nonexisting) database ? */
  BAuthor.MaxInputLength = D_AUTHOR;                                                                    /* Enlarge typeable width */

  InstallMenu (&MainWindow);
  if (NonexistingDatabase)
  {
    GhostMenuBackup ();                                                                         /* Then there's nothing to backup */
    DBConfig.BackupDatabase = FALSE;
  }
  if (DBConfig.NumLocs == 0)                                                   /* No GamePath entries defined (newly installed) ? */
    GhostMenuFindNew ();                                                                    /* Then you cannot search directories */
  GhostMenuGIF ();                                                                            /* The 'Screen' button is unpressed */
  GhostMenuSCR ();                                                                           /* The 'ScrShot' button is unpressed */

  Wprintf (&MainWindow,  1,  0, "{+1Name}N");
  Wprintf (&MainWindow,  3,  0, "{+1Publisher}N");
  Wprintf (&MainWindow,  5,  0, "{+1Author(s)}N");
  Wprintf (&MainWindow,  7,  0, "{+1Year}N");
  Wprintf (&MainWindow,  7, 29, "{+1Memory}N");
  Wprintf (&MainWindow,  9,  0, "{+1# Players}N");
  Wprintf (&MainWindow,  9, 16, "{+1Together}N");
  Wprintf (&MainWindow,  9, 29, "{+1Score}N");
  Wprintf (&MainWindow, 11,  0, "{+1PC-Name}N");
  Wprintf (&MainWindow, 11, 29, "{+1Type}N");
  Wprintf (&MainWindow, 13,  0, "{+1Joysticks}N");
  Wprintf (&MainWindow, 13, 14, "{+1Kem}N");
  Wprintf (&MainWindow, 13, 23, "{+1I2}N");
  Wprintf (&MainWindow, 13, 29, "{+1Cur}N");
  Wprintf (&MainWindow, 13, 36, "{+1Rdf}N");
  Wprintf (&MainWindow, 15,  0, "{+1Orig Scr}N");
  Wprintf (&MainWindow, 15, 29, "{+1Origin}N");
  Wprintf (&MainWindow, 17,  0, "{+1AY-Sound}N");
  Wprintf (&MainWindow, 17, 29, "{+1MultiLoad}N");
  Wprintf (&MainWindow, 19,  0, "{+1Language}N");
  Wprintf (&MainWindow, 19, 29, "{+1Override}N");
  Wprintf (&MainWindow, 21,  0, "{+1Path}N");

  RegisterLowMemHandler (LowMemHandler);                                                       /* Set a low-memory pre-fatal trap */
  RegisterRescueHandler (RescueHandler);                                                       /* Trap all other pre-fatal errors */

  if (!NonexistingDatabase && GameSelectWindow.NumEntries >= 0 &&
      (Convert107 || Convert109 || Convert117 || Convert121 || Convert129 || Convert130 || Convert136 || Convert202 ||
       ConvertImprove))
  {                                                                                                /* Just converted a database ? */
    DBConfig.BackupDatabase = TRUE;                                                               /* Enforce creation of a backup */
    WriteDatabase (FALSE, TRUE);
    ErrorRequester (-1, -1, NULL, WBPics[2], "Conversion Succeeded", "Conversion succeeded for your database\n  %s\n\n"
                    "Your old database can be found in\n  %s", DatabaseName, DatabaseBackupName);
    Convert107 = Convert109 = Convert117 = Convert121 = Convert129 = Convert130 = Convert136 = Convert202 = ConvertImprove = FALSE;
  }

  SetupNewData (TRUE);
  if (SystemDefaults.SyncSlider)                                                                             /* Sync slider bar ? */
    GameSelectWindow.Flags |= COMF_SYNCSLIDER;
  else
    GameSelectWindow.Flags &= ~COMF_SYNCSLIDER;
  if (GameSelectWindow.NumEntries < 0)                                                                        /* Empty database ? */
  {
    SelectedLine = -1;
    GhostEmptyDbase ();                                                                               /* Ghost all action buttons */
  }
  else
  {
    SelectedLine = 0;
    ReportTotal ();
    SelectGame (0);                                                                                     /* Auto-select first game */
  }

  while (CancelQuit)
  {
    while (!TimeToQuit)                                                                                              /* MAIN LOOP */
    {
      switch (HandleComboWindow (&GameSelectWindow, &SelectedLine))                          /* Handle the selection combo window */
      {
        case WINH_VERMOVE   : if (DBConfig.AlwaysSelectGame)                                 /* Moved up/down, select that game ? */
                                if (SelectedGame != SelectedLine)                                       /* (Not already active ?) */
                                {
                                  SelectGame (SelectedLine);
                                  ReportTotal ();
                                }
                              break;
        case WINH_LINEFIRST : ReportTotal ();
                              SelectGame (SelectedLine);
                              break;
        case WINH_LINESEL   : HandlePlay (&MainWindow, &MPlay);                                   /* Double-click starts the game */
                              break;
        case WINH_SELTOGGLE : GetComboEntry (&GameSelectWindow, &NEntry, SelectedLine);                    /* Game selected now ? */
                              if (NEntry.Selected & SELECTBIT)
                                NumMatches ++;
                              else
                                NumMatches --;
                              SelectGame (SelectedLine);
                              ReportTotal ();
                              break;
        case WINH_MSELTOGGLE: NumMatches = 0;                                                              /* Recount all matches */
                              for (Cnt = 0 ; Cnt <= GameSelectWindow.NumEntries ; Cnt ++)
                              {
                                GetComboEntry (&GameSelectWindow, &NEntry, Cnt);
                                if (NEntry.Selected & SELECTBIT)
                                  NumMatches ++;
                              }
                              ReportTotal ();
                              break;
      }
      if (TAPListing)                                                                  /* Catalog from a TAP file on the screen ? */
        switch (HandleComboWindow (&TAPIndex, &SelectedLine))
        {
          case WINH_LINEFIRST : GetComboEntry (&TAPIndex, &NEntry, SelectedLine);
                                if ((FileType == FT_TAP || FileType == FT_LTP || FileType == FT_MDR || FileType == FT_TZX) &&
                                    (NEntry.Selected & EN_TYPEBITS) == BASICBLOCK)                       /* BASIC in a TAP file ? */
                                {
                                  UnghostMenuViewBasic ();                                   /* Then allow the menu strip as well */
                                  GhostMenuViewScreen ();
                                }
                                else
                                {
                                  GhostMenuViewBasic ();
                                  UnghostMenuViewScreen ();
                                }
                                if (FileType == FT_TZX && (NEntry.Selected & EN_TYPEBITS) == TZXHWTBLOCK)
                                  UnghostMenuEditHardwareType ();                             /* Appointing Hardware Type block ? */
                                else
                                  GhostMenuEditHardwareType ();
                                if (FileType == FT_TZX && (NEntry.Selected & EN_TYPEBITS) == TZXAIBLOCK)
                                  UnghostMenuEditArchiveInfo ();                               /* Appointing Archive Info block ? */
                                else
                                  GhostMenuEditArchiveInfo ();
                                if (FileType == FT_TZX && (NEntry.Selected & EN_TYPEBITS) == TZXCUSTPOK)
                                  UnghostMenuEditTZXPokes ();                                  /* Appointing Archive Info block ? */
                                else
                                  GhostMenuEditTZXPokes ();
                                break;
          case WINH_LINESEL   : GetComboEntry (&TAPIndex, &NEntry, SelectedLine);                   /* Double-click on an entry ? */
                                if ((NEntry.Selected & EN_TYPEBITS) == BASICBLOCK)                             /* BASIC or CODE ? */
                                  if (FileType == FT_TAP || FileType == FT_LTP)
                                    ViewBASICFromTAPBlock (TRUE, &TAPIndex, SelectedLine, FullPath);        /* View BASIC Listing */
                                  else if (FileType == FT_TZX)
                                    ViewBASICFromTZXBlock (TRUE, &TAPIndex, SelectedLine, FullPath);        /* View BASIC Listing */
                                  else
                                    ViewBASICFromMDRBlock (TRUE, &TAPIndex, SelectedLine, FullPath);        /* View BASIC Listing */
                                else if ((NEntry.Selected & EN_TYPEBITS) == TZXCUSTTXT)   /* TZX Custom Info - Instructions block */
                                  ViewTZXCIInstructions (&TAPIndex, SelectedLine, FullPath);                    /* View the block */
                                else if ((NEntry.Selected & EN_TYPEBITS) == TZXCUSTSCR)      /* TZX Custom Info - Spectrum Screen */
                                  ViewTZXCIScreen (&TAPIndex, SelectedLine, FullPath);                          /* View the block */
                                else if ((NEntry.Selected & EN_TYPEBITS) == TZXCUSTPIC)              /* TZX Custom Info - Picture */
                                  ViewTZXCIPicture (&TAPIndex, SelectedLine, FullPath);                         /* View the block */
                                else if ((NEntry.Selected & EN_TYPEBITS) == TZXCUSTPOK)                /* TZX Custom Info - Pokes */
                                  EditTZXPokesBlock (FALSE, &TAPIndex, SelectedLine, FullPath);                 /* Edit the block */
                                else if ((NEntry.Selected & EN_TYPEBITS) == TZXHWTBLOCK)               /* TZX Hardware Type block */
                                  EditHardwareTypeBlock (FALSE, &TAPIndex, SelectedLine, FullPath);             /* Edit the block */
                                else if ((NEntry.Selected & EN_TYPEBITS) == TZXAIBLOCK)                 /* TZX Archive Info block */
                                  EditArchiveInfoBlock (FALSE, &TAPIndex, SelectedLine, FullPath);              /* Edit the block */
                                else                                                                     /* Create picture window */
                                  DisplayScreenFromProgram (&TAPIndex, SelectedLine, FullPath, CurrentEntry.PCName);
        }
      switch (HandleWindow (&MainWindow, &ButtonP))                                      /* Handle the MainScreen and all buttons */
      {
        case WINH_MENUNOHAND: switch (((struct MenuEntry *)ButtonP)->ReactNum)
                              {
                                case 1 : SpectrumScreenToGIF (CurrentEntry.PCName, FALSE, MainWindow.StartX + 21,
                                                              MainWindow.StartY + 230);
                                         break;                                                                        /* MakeGif */
                                case 2 : RequestQuit = TRUE; break;                                                       /* Quit */
                                case 3 : HandleCreateScrShot (); break;                                                /* MakeSCR */
                                case 4 : if (FixTape ())                                                               /* FixTape */
                                         {
                                           if (TAPListing)
                                           {
                                             ReInitComboWindow (&TAPIndex, 0);
                                             InsertTAPIndex (&TAPIndex, FullPath);                     /* Make a new file catalog */
                                             PrintComboEntries (&TAPIndex);
                                           }
                                           ResolveFileLocation (&CurrentEntry, GamePath, &EntryChanged);     /* Get new file size */
                                           UpdateButtonText (&MainWindow, &BFileSize, BF_FITTXT);
                                           SignalDBChanged (TRUE);
                                         }
                                         break;
                                case 5 : if (FileType == FT_TAP || FileType == FT_LTP)                               /* ViewBasic */
                                           ViewBASICFromTAPBlock (TRUE, &TAPIndex, TAPIndex.LastSelected, FullPath);
                                         else if (FileType == FT_TZX)
                                           ViewBASICFromTZXBlock (TRUE, &TAPIndex, TAPIndex.LastSelected, FullPath);
                                         else if (FileType == FT_MDR)
                                           ViewBASICFromMDRBlock (TRUE, &TAPIndex, TAPIndex.LastSelected, FullPath);
                                         else
                                           ViewBASICFromSnap (TRUE, FullPath);
                                         break;
                                case 6 : DisplayScreenFromProgram (&TAPIndex, SelectedLine, FullPath, CurrentEntry.PCName);
                                         break;
                                case 7 : if (FileType == FT_TAP || FileType == FT_LTP)                                /* ViewDump */
                                           ViewBASICFromTAPBlock (FALSE, &TAPIndex, TAPIndex.LastSelected, FullPath);
                                         else if (FileType == FT_TZX)
                                           ViewBASICFromTZXBlock (FALSE, &TAPIndex, TAPIndex.LastSelected, FullPath);
                                         else if (FileType == FT_MDR)
                                           ViewBASICFromMDRBlock (FALSE, &TAPIndex, TAPIndex.LastSelected, FullPath);
                                         else if (IsSnapshot (FileType))
                                           ViewBASICFromSnap (FALSE, FullPath);
                                         break;
                                case 9 : CallTAPER (FullPath);                                                      /* Call TAPER */
                                         break;
                                case 12: EditHardwareTypeBlock (TRUE, &TAPIndex, TAPIndex.LastSelected, FullPath);
                                         break;                                                 /* Insert TZX Hardware Type block */
                                case 13: EditHardwareTypeBlock (FALSE, &TAPIndex, TAPIndex.LastSelected, FullPath);
                                         break;                                                   /* Edit TZX Hardware Type block */
                                case 14: EditArchiveInfoBlock (TRUE, &TAPIndex, TAPIndex.LastSelected, FullPath);
                                         break;                                                  /* Insert TZX Archive Info block */
                                case 15: EditArchiveInfoBlock (FALSE, &TAPIndex, TAPIndex.LastSelected, FullPath);
                                         break;                                                    /* Edit TZX Archive Info block */
                                case 16: EditTZXPokesBlock (TRUE, &TAPIndex, TAPIndex.LastSelected, FullPath);
                                         break;                                           /* Insert TZX Custom Info - Pokes block */
                                case 17: EditTZXPokesBlock (FALSE, &TAPIndex, TAPIndex.LastSelected, FullPath);
                                         break;                                             /* Edit TZX Custom Info - Pokes block */
                              }
                              break;
        case 100            : if (SpectrumScreen)                                                              /* Show Z80 Screen */
                                if (MScreen.Flags & BF_PRESSED)                                  /* Redraw the active game screen */
                                  HandleScreen (&MainWindow, &MScreen);
                                else if (MScrShot.Flags & BF_PRESSED)
                                  HandleScrShot (&MainWindow, &MScrShot);
                              break;
        case WINH_RNOHAND   : RequestQuit = TRUE; break;                           /* `Quit' is the only button without a handler */
        case WINH_BUTTBOOL  : EntryChanged = TRUE;                                                      /* Boolean button changed */
                              SignalDBChanged (TRUE);
                              TestEmptyDbase ();
                              if (ButtonP->ReactNum == 6 ||                                                 /* Together changed ? */
                                  ButtonP->ReactNum == 30 ||                                         /* Original Screen changed ? */
                                  ButtonP->ReactNum == 31 ||                                                  /* Origin changed ? */
                                  ButtonP->ReactNum == 36)                                                  /* AY-Sound changed ? */
                                if (ReselectAnEntry (&GameSelectWindow, SelectedGame, &CurrentEntry, FALSE))
                                {
                                  PrintComboEntries (&GameSelectWindow);
                                  ReportTotal ();
                                }
                              break;
        case WINH_EDITEND   : if (ButtonP->Flags & BF_FITINPUT)                              /* An edit button has been changed ? */
                              {
                                EntryChanged = TRUE;
                                SignalDBChanged (TRUE);
                                TestEmptyDbase ();
                                switch (ButtonP->ReactNum)                                               /* Find out which button */
                                {
                                  case 1 :                                                                              /* (Name) */
                                  case 2 :                                                                         /* (Publisher) */
                                  case 3 :                                                                              /* (Year) */
                                  case 42: SelectedGame = SortEntry (&GameSelectWindow, &CurrentEntry, SelectedGame); /* (Author) */
                                           SelectEntry (&GameSelectWindow, SelectedGame);
                                           break;
                                  case 5 : if (CurrentEntry.Players[0] == '1')                                   /* (No. players) */
                                             CurrentEntry.Together = FALSE;
                                           else
                                             BTogether.Flags &= ~BF_GHOSTED;
                                           UpdateButtonText (&MainWindow, &BTogether, 0);
                                           if (CurrentEntry.Players[0] == '1')
                                             BTogether.Flags |= BF_GHOSTED;
                                           break;
                                  case 7 : strupr (CurrentEntry.PCName);                                        /* (Game PC-name) */
                                           UpdateButtonText (&MainWindow, &BPCName, BF_FITTXT);
                                           if (TestAGame (SelectedGame, TRUE))
                                             PrintComboEntries (&GameSelectWindow);
                                           TestPlayGhosted ();
                                           break;
                                  case 41: CurrentEntry.Score = atoi (SScore);                                         /* (Score) */
                                           if (CurrentEntry.Score == 0)
                                             SScore[0] = '\0';
                                           else
                                             sprintf (SScore, "%d", CurrentEntry.Score);
                                           UpdateButtonText (&MainWindow, &BScore, BF_FITTXT);
                                           break;
                                  default: break;
                                }
                                if (ReselectAnEntry (&GameSelectWindow, SelectedGame, &CurrentEntry, FALSE))
                                {
                                  PrintComboEntries (&GameSelectWindow);
                                  ReportTotal ();
                                }
                              }
      }
      if (RequestQuit)
      {
        if (DBConfig.ConfirmQuit)
          if (QuestionRequester (-1, -1, FALSE, FALSE, NULL, NULL, NULL, NULL, NULL, "Confirm Quit",
              "Are you sure you want to quit?"))
            RequestQuit = FALSE;
        if (RequestQuit)
          TimeToQuit = TRUE;
      }
      if (HandleWindow (&OverlayWind, &ButtonP) == WINH_EDITEND)                                 /* Selection criterion changed ? */
      {
        StoreEntry ();                                                     /* Ensure that the current entry is considered as well */
        if (ParseCriterion (Criterion))                                                         /* Parse the new criterion string */
          if (ReselectEntries (&GameSelectWindow))
          {
            PrintComboEntries (&GameSelectWindow);
            ReportTotal ();
          }
      }
      if (GlobalKey != 0x0000 && GameSelectWindow.NumEntries >= 0)                     /* A key was pressed and not handled yet ? */
      {                                                                                    /* (only if the database is not empty) */
        //FatalError (999,"Key: %04X", GlobalKey);
        if ((GlobalKey & 0x00FF) >= 32 && !(GlobalShiftStatus & KS_AALT))                                            /* ASCII key */
        {
          if (TypeIndex < 36)
          {
            TypeBuffer[TypeIndex ++] = GlobalKey & 0x00FF;
            TypeBuffer[TypeIndex] = '\0';
            strcpy (TotalText, TypeBuffer);
            strcpy (ShrTypeBuffer, ShrinkString (TypeBuffer));
            UpdateButtonText (&OverlayWind, &TTotals, BF_FITTXT);
            Found = FALSE;
            for (Cnt = 0 ; !Found && Cnt <= GameSelectWindow.NumEntries ; Cnt ++)
            {
              GetComboEntry (&GameSelectWindow, &NEntry, Cnt);
              Found = (stricmp (ShrinkString (NEntry.Text), ShrTypeBuffer) >= 0 ? TRUE : FALSE);
            }
            SelectEntry (&GameSelectWindow, -- Cnt);
            SelectGame (Cnt);
          }
        }
        else if (GlobalKey == K_DEL)
          HandleDelEntry (&MainWindow, &MDelEntry);
        else if (GlobalKey == K_BCKSP)
        {
          if (TypeIndex > 0)
          {
            TypeBuffer[-- TypeIndex] = '\0';
            if (TypeIndex == 0)
            {
              ReportTotal ();
              Cnt = 1;
            }
            else
            {
              strcpy (TotalText, TypeBuffer);
              strcpy (ShrTypeBuffer, ShrinkString (TypeBuffer));
              UpdateButtonText (&OverlayWind, &TTotals, BF_FITTXT);
              Found = FALSE;
              for (Cnt = 0 ; !Found && Cnt <= GameSelectWindow.NumEntries ; Cnt ++)
              {
                GetComboEntry (&GameSelectWindow, &NEntry, Cnt);
                Found = (stricmp (ShrinkString (NEntry.Text), ShrTypeBuffer) >= 0 ? TRUE : FALSE);
              }
            }
            SelectEntry (&GameSelectWindow, -- Cnt);
            SelectGame (Cnt);
          }
        }
        else if (GlobalKey == K_ESC && Cnt != 0)
        {
          ReportTotal ();
          Cnt = 0;
          SelectEntry (&GameSelectWindow, Cnt);
          SelectGame (Cnt);
        }
        else if (GlobalKey == K_CRUP && (GlobalShiftStatus & KS_ACTRL) && SelectedGame >= 0)
        {
          Found = FALSE;
          for (Cnt = GameSelectWindow.LastSelected - 1 ; !Found && Cnt >= 0 ; Cnt --)
          {
            GetComboEntry (&GameSelectWindow, &NEntry, Cnt);
            if (NEntry.Selected & SELECTBIT)
            {
              Found = TRUE;
              SelectEntry (&GameSelectWindow, Cnt);
              if (DBConfig.AlwaysSelectGame)                                                                     /* Auto-select ? */
                if (SelectedGame != Cnt)                                                                /* (Not already active ?) */
                {
                  SelectGame (Cnt);
                  ReportTotal ();
                }
            }
          }
          if (!Found)
            SoundErrorBeeper (BEEP_SHORT);
        }
        else if (GlobalKey == K_CRDN && (GlobalShiftStatus & KS_ACTRL) && SelectedGame >= 0)
        {
          Found = FALSE;
          for (Cnt = GameSelectWindow.LastSelected + 1; !Found && Cnt <= GameSelectWindow.NumEntries ; Cnt ++)
          {
            GetComboEntry (&GameSelectWindow, &NEntry, Cnt);
            if (NEntry.Selected & SELECTBIT)
            {
              Found = TRUE;
              SelectEntry (&GameSelectWindow, Cnt);
              if (DBConfig.AlwaysSelectGame)                                                                     /* Auto-select ? */
                if (SelectedGame != Cnt)                                                                /* (Not already active ?) */
                {
                  SelectGame (Cnt);
                  ReportTotal ();
                }
            }
          }
          if (!Found)
            SoundErrorBeeper (BEEP_SHORT);
        }
      }
      MouseStatus ();
    }
    StoreEntry ();
    if (DatabaseChanged)
      switch (QuestionRequester (-1, -1, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL, "Confirm Quit", "Database has been changed! "
                                 "Write back?"))
      {
        case 0 : SaveDatabase (TRUE);                                                                                    /* (Yes) */
                 CancelQuit = FALSE;                                                                                 /* Full stop */
                 break;
        case 1 : CancelQuit = FALSE;                                                                                      /* (No) */
                 break;
        case 2 : TimeToQuit = FALSE;                                                                                  /* (Cancel) */
                 RequestQuit = FALSE;
                 break;
      }
    else
      CancelQuit = FALSE;                                                       /* Always step out if the database is not changed */
  }

  if (TAPListing)
    DestroyComboWindow (&TAPIndex, 0);
  DestroyComboWindow (&GameSelectWindow, 0);
  DestroyWindow (&MainWindow);
  EndGUI ();
  ReleaseFont (&SpecFont);
  exit (0);
}
