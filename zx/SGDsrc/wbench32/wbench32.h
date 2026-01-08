/**********************************************************************************************************************************/
/* Module       : WBENCH32.H                                                                                                      */
/* Description  : Graphic User Interface (GUI) Design and handler                                                                 */
/* Lib file     : WBENCH32.LIB                                                                                                    */
/* Version type : Library interface                                                                                               */
/* Version num  : 2.71.8  32-bit protected mode                                                                                   */
/* Last changed : 08-06-1999  6:45                                                                                                */
/* OS type      : PC (DOS)                                                                                                        */
/* Compiler     : Watcom C32 Optimizing Compiler Version 10.x                                                                     */
/* Linker       : Watcom C Linker                                                                                                 */
/*                                                                                                                                */
/*                Copyright (C) 1995-1999 ThunderWare Research Center, written by Martijn van der Heide                           */
/*                                                                                                                                */
/**********************************************************************************************************************************/

#ifndef WBENCH32_H_INC
#define WBENCH32_H_INC

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <conio.h>

/* (Convert 16-bit MS C to 32-bit Watcom C) */
#include "unwatcom.h"

#define ToNumber(_X)     ((_X) - 'A' + 1)                                               /* Convert drive letter to number (A = 1) */
#define ToName(_X)       ((_X) + 'A' - 1)                                               /* Convert drive number to letter (A = 1) */
#define Div8(_X)         ((_X) - ((_X) % 8))                                                     /* Round down to a multiple of 8 */
#define IsFloppy(_X)     (((_X) != 0) && ((_X) - 'A' < NumFloppyDrives))            /* TRUE if the driveletter points to a floppy */

#define MAXDIR           256                                                                     /* Maximum length of a full path */
#define MAXBTEXT         257                                                                     /* Maximum text size of a button */
#define MAXMENUNAME      11                                                                      /* Maximum length of a menu name */
#define MAXMENUENTRYNAME 20                                                                /* Maximum length of a menu entry line */
#define MAXMENUENTRIES   32                                                                           /* Maximum entries per menu */

#define NUMPICS          5                                                           /* Number of predefined (requester) pictures */

#define MINSLIDERLENGTH  5                                                                      /* Minumum length of a slider bar */

#define ALTADD           '@'                                                               /* button hot-key assignment character */

/**********************************************************************************************************************************/
/* Add some variables types                                                                                                       */
/**********************************************************************************************************************************/

typedef char             bool;
typedef unsigned char    byte;
typedef unsigned short   word;
typedef unsigned int     dword;

#ifndef TRUE
#define TRUE             (bool)1
#define FALSE            (bool)0
#endif

/**********************************************************************************************************************************/
/* Define the colors in a 16-color VGA display; the first two are generic values                                                  */
/**********************************************************************************************************************************/

#define BPEN             (short)-2
#define APEN             (short)-1

#define DBLACK           0
#define DBLUE            1
#define DGREEN           2
#define DCYAN            3
#define DRED             4
#define DMAGENTA         5
#define DYELLOW          6
#define DWHITE           7
#define LBLACK           8
#define LBLUE            9
#define LGREEN           10
#define LCYAN            11
#define LRED             12
#define LMAGENTA         13
#define LYELLOW          14
#define LWHITE           15

/**********************************************************************************************************************************/
/* Define the BIOS keyboard values for the most important (editor) keys                                                           */
/**********************************************************************************************************************************/

#define K_ESC            0x011B                                                                                         /* Escape */
#define K_TAB            0x0F09                                                                                            /* Tab */
#define K_ENTER          0x1C0D                                                                                          /* Enter */
#define K_SPACE          0x3920                                                                                          /* Space */
#define K_BCKSP          0x0E08                                                                                      /* Backspace */
#define K_INSRT          0x5200                                                                                         /* Insert */
#define K_DEL            0x5300                                                                                         /* Delete */
#define K_HOME           0x4700                                                                                           /* Home */
#define K_END            0x4F00                                                                                            /* End */
#define K_PGUP           0x4900                                                                                        /* Page Up */
#define K_PGDN           0x5100                                                                                      /* Page Down */
#define K_CRUP           0x4800                                                                                      /* Cursor Up */
#define K_CRDN           0x5000                                                                                    /* Cursor Down */
#define K_CRLT           0x4B00                                                                                    /* Cursor Left */
#define K_CRRT           0x4D00                                                                                   /* Cursor Right */
#define K_PRTSC          0x3700                                                                                   /* Print Screen */
#define K_CTRLALTESC     0x0100                                                                            /* CTRL + ALT + Escape */
#define K_F1             0x3B00                                                                            /* F1 (usual help-key) */
#define K_F2             0x3C00                                                                                             /* F2 */
#define K_F3             0x3D00                                                                                             /* F3 */
#define K_F4             0x3E00                                                                                             /* F4 */
#define K_F5             0x3F00                                                                                             /* F5 */
#define K_F6             0x4000                                                                                             /* F6 */
#define K_F7             0x4100                                                                                             /* F7 */
#define K_F8             0x4200                                                                                             /* F8 */
#define K_F9             0x4300                                                                                             /* F9 */
#define K_F10            0x4400                                                                                            /* F10 */
#define K_F11            0x8500                                                                                            /* F11 */
#define K_F12            0x8600                                                                                            /* F12 */

/**********************************************************************************************************************************/
/* Define the BIOS shift values for `GlobalShiftStatus'. The highest 8 values are only returned for 101-key (enhanced) keyboards. */
/**********************************************************************************************************************************/

#define KS_RSHIFT        0x0001                                                                                    /* Right SHIFT */
#define KS_LSHIFT        0x0002                                                                                     /* Left SHIFT */
#define KS_ASHIFT        0x0003                                                                                      /* Any SHIFT */
#define KS_ACTRL         0x0004                                                                                       /* Any CTRL */
#define KS_AALT          0x0008                                                                                        /* Any ALT */
#define KS_ASCRLOCK      0x0010                                                                             /* SCROLL LOCK active */
#define KS_ANUMLOCK      0x0020                                                                                /* NUM LOCK active */
#define KS_ACPSLOCK      0x0040                                                                               /* CAPS LOCK active */
#define KS_AINSMODE      0x0080                                                                                /* INS mode active */

#define KS_LCTRL         0x0100                                                                                      /* Left CTRL */
#define KS_LALT          0x0200                                                                                       /* Left ALT */
#define KS_RCTRL         0x0400                                                                                     /* Right CTRL */
#define KS_RALT          0x0800                                                                                      /* Right ALT */
#define KS_PSCRLOCK      0x1000                                                                            /* SCROLL LOCK pressed */
#define KS_PNUMLOCK      0x2000                                                                               /* NUM LOCK pressed */
#define KS_PCPSLOCK      0x4000                                                                              /* CAPS LOCK pressed */
#define KS_PSYSREQ       0x8000                                                                                 /* SYSREQ pressed */

/**********************************************************************************************************************************/
/* Define the beep durations for the `SoundErrorBeeper' subroutine                                                                */
/**********************************************************************************************************************************/

#define BEEP_SHORT       0x02                                                                      /* Used in Question requesters */
#define BEEP_MEDIUM      0x08                                                                         /* Used in Error requesters */
#define BEEP_LONG        0x10                                                              /* Used by the `FatalError' subroutine */

/**********************************************************************************************************************************/
/* Define the flags for the `SelectFont' subroutine                                                                               */
/**********************************************************************************************************************************/

#define FONT_NORMAL      0x0001
#define FONT_BOLD        0x0002
#define FONT_ITALICS     0x0004

/**********************************************************************************************************************************/
/* Define the `Xco' substitutes for the `Wprintf' subroutine                                                                      */
/**********************************************************************************************************************************/

#define TEXT_CENTER      -1
#define TEXT_RIGHT       -2
#define TEXT_MCENTER     -3                                                                                     /* Merge contents */
#define TEXT_MRIGHT      -4

/**********************************************************************************************************************************/
/* Define the values for the `Flags' field in the `WindowInfo' structure                                                          */
/**********************************************************************************************************************************/

#define WINF_HUSE        0x0001                                                                             /* Use the headertext */
#define WINF_HCENTER     0x0002                                                                  /* Print the headertext centered */
#define WINF_HBOLD       0x0004                                                                   /* Print the headertext in bold */
#define WINF_NOBUFFER    0x0010                                                        /* Do not buffer the underlying screenpart */
#define WINF_XMSBUFFER   0x0020                                                              /* Move underlying screenpart to XMS */
#define WINF_BACKDROP    0x0040                                               /* Non-buffered, non-moveable, but can be activated */
#define WINF_BACKGROUND  0x0080                                               /* Window is part of the background - no properties */
#define WINF_TWRAP       0x0100                                                                   /* Wrap text onto the next line */
#define WINF_TWSMART     0x0200                                                  /* Wrap text smart, i.e. wrap on word boundaries */
#define WINF_TCLEAR      0x0400                                                     /* Clear each text line before printing on it */
#define WINF_WADJMDONE   0x0800                                     /* (Used by the system: min and max values already autosized) */
#define WINF_WADJUST     0x1000                 /* Adjust window size to hold an exact number of characters on X and Y directions */
#define WINF_ACTIVATE    0x2000                                                                     /* Activate window when drawn */
#define WINF_NOMOVE      0x4000                                                                           /* Window is unmoveable */
#define WINF_EXCLUSIVE   0x8000                                                                 /* Handle this window exclusively */

/**********************************************************************************************************************************/
/* Define the values for the `BorderFlags' field in the `WindowInfo' structure                                                    */
/**********************************************************************************************************************************/

#define WINB_ALL         0x01                                                            /* Use both an inner and an outer border */
#define WINB_NONE        0x02                                                                             /* Use no border at all */
#define WINB_IN          0x04                                                                        /* Use only the inner border */
#define WINB_OUT         0x08                                                                        /* Use only the outer border */
#define WINB_SURROUND    0x10                                       /* Surround the window with an extra 1-bit thick black border */
#define WINB_REVBORDER   0x20                                                                      /* Reverse border shadowcolors */

/**********************************************************************************************************************************/
/* Define the values for the system gadgets for the `DrawWindow' subroutine                                                       */
/**********************************************************************************************************************************/

#define WING_CLOSE       0x01
#define WING_RESIZEMAX   0x02

/**********************************************************************************************************************************/
/* Define the return values for the `HandleWindow' and `HandleComboWindow' subroutines                                            */
/**********************************************************************************************************************************/

#define WINH_NONE        -1                                                                            /* No button changed state */
#define WINH_EDITEND     -2                                                 /* An inputbutton in this window accepted a new value */
#define WINH_EDITKEY     -3                                                /* A key was consumed in an inputbutton in this window */
#define WINH_EDITESC     -4                                                                         /* An inputbutton was escaped */
#define WINH_BUTTBOOL    -5                                                                  /* A boolean button has been handled */
#define WINH_PNOHAND     -6                                                   /* A button was pressed, but no handler was defined */
#define WINH_RNOHAND     -7                                                  /* A button was released, but no handler was defined */
#define WINH_LINESEL     -8                                 /* (A COMT_SELECT combowindow): A selection has been made by the user */
#define WINH_LINEFIRST   -9                                      /* (A COMT_SELECT combowindow): First click on a selectable line */
#define WINH_HORMOVE     -10                                                 /* (A combowindow): Horizontal slider has been moved */
#define WINH_VERMOVE     -11                                                   /* (A combowindow): Vertical slider has been moved */
#define WINH_SELTOGGLE   -12                                         /* (A combowindow): A selectable entry has been (de)selected */
#define WINH_MSELTOGGLE  -13                                   /* (A combowindow): A selectable entry range has been (de)selected */
#define WINH_MENUACT     -14                                                                 /* The menu strip has been activated */
#define WINH_MENUREL     -15                                                                  /* The menu strip has been released */
#define WINH_MENUNOHAND  -16                              /* An item from the menu strip was selected, but no handler was defined */
#define WINH_MENUBOOL    -17                                                             /* A MEF_BOOL menu item has been toggled */
#define WINH_CLOSE       -18                                                                /* The close gadget has been released */
#define WINH_WINRESIZING -19                                                                                 /* Resizing a window */
#define WINH_WINRESIZE   -20                                                                 /* RESIZEMAX gadget or manual resize */
#define WINH_WINDRAGGING -21                                                                                 /* Dragging a window */
#define WINH_WINMOVED    -22                                                                            /* Done dragging a window */
#define WINH_DRAGDROPING -23                                                      /* Dragging entries between linked combowindows */
#define WINH_DRAGDROPPED -24                                                 /* Done dragging entries between linked combowindows */
#define WINH_SLIDERMOVE  -25                                                       /* (A SliderButton): The slider has been moved */

/**********************************************************************************************************************************/
/* Define the possible types of a Combowindow                                                                                     */
/**********************************************************************************************************************************/

#define COMT_SELECT      0x01                                                                       /* The user can select a line */
#define COMT_VIEW        0x02                                                                           /* The user can view only */

/**********************************************************************************************************************************/
/* Define the values for the `Flags' field in the `ComboInfo' structure                                                           */
/**********************************************************************************************************************************/

#define COMF_LAYIN       0x01                                                                    /* Reverse all subwindow borders */
#define COMF_VSLIDER     0x02                                                                            /* Add a vertical slider */
#define COMF_HSLIDER     0x04                                                                          /* Add a horizontal slider */
#define COMF_NOBUFFER    0x08            /* Do not buffer the overlay subwindow (you will not be able to remove the combowindow!) */
#define COMF_KEYSELECT   0x10                                                                /* Allow selection with the keyboard */
#define COMF_SYNCSLIDER  0x20                                                          /* Update combo window as the slider moves */
#define COMF_RESIZING    0x40                                                         /* Used when resizing a running combowindow */
#define COMF_EXCLUSIVE   0x80                                 /* (In combination with COMF_KEYSELECT): Doesn't need a host window */

/**********************************************************************************************************************************/
/* Name the bit-positions for the `Selected' field in the `ComboEntry' structure                                                  */
/**********************************************************************************************************************************/

#define ENT_NOSELECT     0x80                                                             /* If TRUE, the entry is not selectable */
#define ENT_NODROP       0x40                                                       /* If TRUE, cannot drop right after his entry */
#define ENT_HASBLINK     0x20                                              /* (System flag) The text contains blinking characters */
#define ENT_COLORBITS    0x03                                                /* The bits that index into the `SelectedxPen' array */

/**********************************************************************************************************************************/
/* Define the possible button types (The `Type' field in the `ButtonInfo' structure)                                              */
/**********************************************************************************************************************************/

#define BT_TEXT          0x01                                                                /* A text button (cannot be pressed) */
#define BT_PUSH          0x02                                                                             /* A normal push button */
#define BT_CPUSH         0x04                          /* A continuous push button (is always handled if the mouse keeps pressed) */
#define BT_INPUT         0x08                                                                                  /* An input button */
#define BT_TOGGLE        0x10                                                                                  /* A toggle button */
#define BT_BOOL          0x20                                                                                 /* A boolean button */
#define BT_FAKE          0x40                                            /* Not a real button; used for window->button conversion */
                                                                                          /* Combo windows have only this bit set */
                                                                                 /* InputBoxes have both BT_FAKE and BT_INPUT set */
#define BT_SLIDER        0x80                                                  /* Slider button in a combo window or SliderButton */

/**********************************************************************************************************************************/
/* Define the possible button contents types (The `ContentsType' field in the `ButtonInfo' structure)                             */
/**********************************************************************************************************************************/

#define BC_TEXT          0x01                                                                                    /* Contains text */
#define BC_PIC           0x02                                                                               /* Contains a picture */
#define BC_GADGET        0x03                                                                                /* Contains a gadget */
#define BC_NUMBER        0x04                                                               /* Input buttons: only numerical text */
#define BC_FLOAT         0x05                                                          /* Input buttons: only floating point text */
#define BC_ALPHA         0x06                                                                   /* Input buttons: only alpha text */
#define BC_HEX           0x07                                                             /* Input buttons: only hexadecimal text */
#define BC_STARTREDRAW   0x80                                                                 /* Input Buttons: redraw at startup */

/**********************************************************************************************************************************/
/* Define the possible button flags (The `Flags' field in the `ButtonInfo' structure)                                             */
/* Note: The flag `BF_FITINPUT' is for internal use only.                                                                         */
/**********************************************************************************************************************************/

#define BF_FWIDTH        0x0001                                                                              /* Use a fixed width */
#define BF_KEYSELECT     0x0002                                                    /* Allowed to select the button with TAB/ENTER */
#define BF_GHOSTED       0x0004                                                                               /* Button ghosted ? */
#define BF_BCLEAR        0x0008                                                  /* If BT_BOOL: print space i.s.o. cross if clear */
#define BF_NOSURROUND    0x0010                                             /* If pushable: do not use the extra black outer line */

#define BF_PRESSED       0x0020                                                /* (Set by the system): button is pressed/released */
#define BF_DPRESSED      0x0040                                                  /* (Set by the system): button is double-clicked */
#define BF_FITINPUT      0x0080                                        /* (Used by the system): Fit text starting from inputstart */
                                                                            /* Return edit buttons: contents have changed if TRUE */

#define BF_HDRHEIGHT     0x0100                                /* Adjust gadget button to height of the host window's header font */
#define BF_FNTHEIGHT     0x0200                                  /* Adjust gadget button to height of the host window's text font */
#define BF_BOLD          0x0400                                                               /* Print text button with bold font */
#define BF_FITDIR        0x0800                                                                           /* Fit a directory text */
#define BF_FITTXT        0x1000                                                            /* Fit text to the entire button width */
#define BF_PLAINBOX      0x2000                                                                /* Use a button box without shadow */
#define BF_FORCEIN       0x4000                                                             /* Force button to look as if pressed */
#define BF_FORCEOUT      0x8000                                                        /* Force button to come out of the display */

/**********************************************************************************************************************************/
/* Define the GIF return codes                                                                                                    */
/**********************************************************************************************************************************/

#define GIF_OK           0x00                                                                                        /* No errors */
#define GIF_ERRCREATE    0x01                                                                          /* Error creating GIF file */
#define GIF_ERRWRITE     0x02                                                                        /* Error writing to GIF file */
#define GIF_OUTMEM       0x03                                                                        /* Cannot allocate resources */
#define GIF_OUTMEM2      0x04                                                                        /* Cannot allocate resources */

/**********************************************************************************************************************************/
/* Define the SoundBlaster software mixer types                                                                                   */
/**********************************************************************************************************************************/

#define SBMTYPE_NONE     0                                                                                   /* No software mixer */
#define SBMTYPE_SBPRO    1                                                                                    /* Mixer type SBPro */
#define SBMTYPE_SB16     2                                                                                    /* Mixer type SB16+ */
#define SBMTYPE_AD1848   3                                                                         /* Mixer/sampling CODEC AD1848 */

/**********************************************************************************************************************************/
/* Define the SoundBlaster software mixer registers                                                                               */
/**********************************************************************************************************************************/

#define SBM_MASTERVOL    0x01                                                                     /* Master volume (output) level */
#define SBM_LINEINLEVEL  0x02                                                                            /* Line-in (input) level */
#define SBM_DACLEVEL     0x03                                                                               /* DAC (output) level */
#define SBM_TREBLELEVEL  0x04                                                                            /* Treble (output) level */
#define SBM_BASSLEVEL    0x05                                                                              /* Bass (output) level */
#define SBM_INPUTGAIN    0x06                                                                 /* Input gain multiplier (loudness) */
#define SBM_AUTOGAIN     0x07                                                                     /* Automatic Gain Control (AGC) */
#define SBM_OUTPUTGAIN   0x08                                                                /* Output gain multiplier (loudness) */
#define SBM_PCSPEAKER    0x09                                             /* (Only if PC Speaker connected to SoundBlaster board) */
#define SBM_INPUTCTRL    0x0A                                                                   /* Input control - SBI_xxx values */
#define SBM_OUTPUTCTRL   0x0B                                                                  /* Output control - SBO_xxx values */

/**********************************************************************************************************************************/
/* Define the SoundBlaster software mixer input control values                                                                    */
/**********************************************************************************************************************************/

#define SBI_MICON        0x01                                                                                           /* MIC on */
#define SBI_MICOFF       0x02                                                                                          /* MIC off */
#define SBI_CDON         0x03                                                                                            /* CD on */
#define SBI_CDOFF        0x04                                                                                           /* CD off */
#define SBI_LINEINON     0x05                                                                                        /* LineIn on */
#define SBI_LINEINOFF    0x06                                                                                       /* LineIn off */
#define SBI_RESTOREALL   0xFF                                                                 /* Generic - restore startup values */

/**********************************************************************************************************************************/
/* Define the SoundBlaster software mixer output control values                                                                   */
/**********************************************************************************************************************************/

#define SBO_MICON        0x01                                                                                           /* MIC on */
#define SBO_MICOFF       0x02                                                                                          /* MIC off */
#define SBO_CDON         0x03                                                                                            /* CD on */
#define SBO_CDOFF        0x04                                                                                           /* CD off */
#define SBO_LINEINON     0x05                                                                                        /* LineIn on */
#define SBO_LINEINOFF    0x06                                                                                       /* LineIn off */
#define SBO_FMON         0x07                                                                                            /* FM on */
#define SBO_FMOFF        0x08                                                                                           /* FM off */
#define SBO_RESTOREALL   0xFF                                                                 /* Generic - restore startup values */

/**********************************************************************************************************************************/
/* Define the SwapOut return codes (for the `SwapReturn' variable)                                                                */
/**********************************************************************************************************************************/

#define SWAP_OK         0                                                                              /* Swapped OK and returned */
#define SWAP_NOSHRINK   1                                                                     /* Could not shrink DOS memory size */
#define SWAP_NOSAVE     2                                                               /* Could not save program to XMS/EMS/disk */
#define SWAP_NOEXEC     3                                                                        /* Could not execute new program */
#define SWAP_TOOMANY    4                                                          /* Program has too many extra blocks allocated */

/**********************************************************************************************************************************/
/* If `SwapReturn' was not SWAP_OK (for the `ExecReturn' variable) - these are standard DOS error codes.                          */
/**********************************************************************************************************************************/

#define EXEC_NOFILE     ENOENT                                                    /* File not found -- couldn't find program_name */
#define EXEC_NOMEMORY   ENOMEM                                                         /* Insufficient memory to run program_name */
#define EXEC_ACCESS     EACCES                                                     /* Access denied -- couldn't open program_name */
#define EXEC_BADFORMAT  ENOEXEC                                                                     /* Format invalid -- unlikely */
#define EXEC_TOOLONG    0xFF                                                           /* Command line too long (128+ characters) */

/**********************************************************************************************************************************/
/* Define the possible report items in a hardware error (for the `HWEReportType' variable)                                        */
/**********************************************************************************************************************************/

#define HWER_CLASS       0x01                                                                               /* Report error class */
#define HWER_LOCUS       0x02                                                                               /* Report error locus */
#define HWER_LOCATION    0x04                                                                       /* Report error disk location */

/**********************************************************************************************************************************/
/* Define the pre-fatal low memory handler memory types                                                                           */
/**********************************************************************************************************************************/

#define ML_CONV          1                                                                      /* Run out of conventional memory */
#define ML_XMS           2                                                                       /* Run out of XMS handles/blocks */

/**********************************************************************************************************************************/
/* Define the menu strip flags                                                                                                    */
/**********************************************************************************************************************************/

#define MF_ACTIVE        0x0001                                                            /* (Set by the `HandleWindow' routine) */
#define MF_GHOSTED       0x0002                                                    /* If TRUE, all menu items are ghosted at once */
#define MF_IGNGHOSTED    0x0004                                                       /* If TRUE, ghosted entries are not printed */

/**********************************************************************************************************************************/
/* Define the menu entry flags                                                                                                    */
/**********************************************************************************************************************************/

#define MEF_GHOSTED      0x01                                                           /* If TRUE, the entry can not be selected */
#define MEF_KEYSELECT    0x02                                             /* If TRUE, the entry can be selected from the keyboard */
#define MEF_SEPERATOR    0x04                                         /* If TRUE, the entry is only a seperator (horizontal line) */
#define MEF_BOOL         0x08                                                                /* If TRUE, the entry can be toggled */
#define MEF_BOOLCLEAR    0x10                                          /* (If type MEF_BOOL): entry can be deselected by itself ? */
#define MEF_SUBMENU      0x20                                                                         /* (Assigned by the system) */

/**********************************************************************************************************************************/
/* Define the Font information structure                                                                                          */
/**********************************************************************************************************************************/

struct FontInfo
{
  /* User supplied part */
  char                    Name[9];                                                             /* Font filename, except extension */
  short                   Width;                                                                          /* Font width in pixels */
  short                   Height;                                                                        /* Font height in pixels */

  /* System supplied part */
  bool                    NormalResident;                                           /* TRUE if the normal font image is in memory */
  bool                    BoldResident;                                               /* TRUE if the bold font image is in memory */
  bool                    ItalicsResident;                                         /* TRUE if the italics font image is in memory */
  short                   DWidth;                                                                          /* Font width in bytes */
  byte                   *NormalData;                                                                         /* Font image areas */
  byte                   *BoldData;
  byte                   *ItalicsData;
};

/**********************************************************************************************************************************/
/* Define the Button information structure                                                                                        */
/**********************************************************************************************************************************/

struct ButtonInfo
{
  /* User supplied part */
  byte                    Type;                                                                                /* The button type */
  byte                    ContentsType;                                                                      /* The contents type */
  word                    Flags;                                                                              /* The button flags */
  short                   StartX;                                       /* The relative offset to the top-left of the host window */
  short                   StartY;                                              /* (Is converted after the `AddButton' subroutine) */
  short                   FWidth;                                                                              /* The fixed width */
  short                   APen;                                                                           /* The foreground color */
  short                   BPen;                                                                           /* The background color */
  struct FontInfo        *Font;                                                                       /* The font to use for text */
  short                   ReactNum;                                             /* Reaction number, supplied for user convenience */
  char                   *Contents;                                                                 /* The contents of the button */
  int                   (*Handler)(struct WindowInfo *Window, struct ButtonInfo *Button);                   /* The button handler */

  /* System supplied part */
  short                   EndX;                                                                 /* The rightmost pixel coordinate */
  short                   EndY;                                                                /* The bottommost pixel coordinate */
  struct ButtonInfo      *NextButton;                                             /* Used to chain all buttons to the host window */
  struct ButtonInfo      *PreviousButton;
  dword                   FirstPress;                                                        /* Timestamp for double-click timing */
  short                   MaxInputLength;                              /* The maximum contents length, only used for inputbuttons */
};

/**********************************************************************************************************************************/
/* Define the ALTInfo information structure                                                                                       */
/**********************************************************************************************************************************/

struct ALTInfo
{
  struct ALTInfo         *NextElement;                                                               /* Next element in the chain */
  struct ALTInfo         *PrevElement;                                                           /* Previous element in the chain */
  byte                    ASCIIKeyValue;                                                                  /* The key to be tested */
  struct ButtonInfo      *Button;                                                      /* The button that contains this short-key */
};

/**********************************************************************************************************************************/
/* Define the Window information structure                                                                                        */
/**********************************************************************************************************************************/

struct WindowInfo
{
  /* User supplied part */
  short                   StartX;                        /* Top-left pixel coordinates of the first printable pixel in the window */
  short                   StartY;
  short                   Width;                                                                        /* Window width in pixels */
  short                   Height;                                                                      /* Window Height in pixels */
  short                   MinWidth;                                                                /* Minimal width when resizing */
  short                   MinHeight;
  short                   MaxWidth;                                                                /* Maximal width when resizing */
  short                   MaxHeight;
  short                   BorderSize;                                                                   /* Border width in pixels */
  byte                    BorderFlags;                                                                       /* Border type flags */
  short                   APen;                                                                               /* Foreground color */
  short                   BPen;                                                                               /* Background color */
  struct FontInfo        *Font;                                                                       /* Font to be used for text */
  word                    Flags;                                                                                  /* Window flags */
  struct FontInfo        *HeaderFont;                                                           /* Font to be used for the header */
  char                   *Header;                                                                              /* The header text */

  /* System supplied part */
  struct                                                                                          /* The text coordinating system */
  {
    short                 CurTextX;                                                             /* The current character position */
    short                 CurTextY;                                                                 /* The current character line */
    short                 EndTextX;                                              /* One past the last possible character position */
    short                 EndTextY;                                                  /* One past the last possible character line */
    short                 Status;                                                        /* Printing status (bold, inverted, etc) */
    short                 CurAPen;                                                                    /* Current foreground color */
    short                 CurBPen;                                                                    /* Current background color */
  } TStat;
  short                   A_StartX;                                  /* The (absolute) contained screen part in pixel coordinates */
  short                   A_StartY;
  short                   A_EndX;
  short                   A_EndY;
  short                   H_StartX;                                         /* Contained header part relative to the A_xxx values */
  short                   H_StartY;
  short                   H_EndX;
  short                   H_EndY;
  short                   TABLength;                                                                           /* (Defaults to 8) */
  short                  *ColorTable;                                                                      /* Alternate color map */
  struct WindowInfo      *NextWindow;                                          /* Used to keep track of all windows on the screen */
  struct WindowInfo      *PreviousWindow;
  struct WindowInfo      *SubWindow;
  struct WindowInfo      *HostWindow;
  struct WindowInfo      *PreviousActive;
  struct ALTInfo         *ALTChain;                                                              /* The first button in the chain */
  struct ButtonInfo      *FirstButton;                                                           /* The first button in the chain */
  struct ButtonInfo      *LastButton;
  struct ButtonInfo      *KeyButton;                                           /* The key currently selectable with the ENTER key */
  struct ButtonInfo      *KeySelectButton;                                               /* The key currently in a keyselect loop */
  struct ButtonInfo      *DefaultButton;
  struct FontInfo        *MenuFont;                                                            /* The font used by the menu strip */
  struct MenuInfo        *FirstMenu;                                                          /* The first item in the menu strip */
  byte                    SystemGadgets;                                                              /* The added system gadgets */
  bool                    ResizeMin;                                           /* ResizeMax gadget does Max if FALSE, Min if TRUE */
  bool                    HandlingKeySelect;                                    /* TRUE during two consecutive keyselecting loops */
  byte                   *Buffer;                                                                   /* The underlying screen part */
};

/**********************************************************************************************************************************/
/* Define one entry for the combowindow entrytable                                                                                */
/**********************************************************************************************************************************/

struct ComboEntry
{
  dword                   Identifier;                                                            /* Supplied for user convenience */
  byte                    Selected;                      /* SET if the `Selection' colors should be used when printing this entry */
  char                    Text[257];                                                                     /* The text of that line */
};

typedef long              comboent;
#define                   MAXENTRIES     2147483647                            /* Maximum number of entries in a table (comboent) */

struct PrintAttributes
{
  short                   Status;                                                        /* Printing status (bold, inverted, etc) */
  short                   APen;                                                                               /* Foreground color */
  short                   BPen;                                                                               /* Background color */
};

/**********************************************************************************************************************************/
/* Define the Slider information structure                                                                                        */
/**********************************************************************************************************************************/

struct SliderInfo
{
  byte                    Orientation;                                                            /* COMF_VSLIDER or COMF_HSLIDER */
  struct WindowInfo       SliderWindow;                                                                      /* The slider window */
  struct ButtonInfo       PButtonInc;                                                                      /* The increase button */
  struct ButtonInfo       PButtonDec;                                                                      /* The decrease button */
  struct ButtonInfo       PButtonReset;                                 /* Reset to origin; only used if both sliders are defined */
  struct ButtonInfo       FakeButton;                                            /* Window->Button conversion of the sliderwindow */
  double                  SliderStart;                                 /* Starting pixel of the slider bar, relative to the start */
  double                  SliderLength;                                                         /* Length of the slider in pixels */
  double                  StepRate;                                                          /* Slider bar difference for 1 entry */
};

/**********************************************************************************************************************************/
/* Define the Combowindow information structure                                                                                   */
/* Creation should only be done with `MakeComboWindow'!                                                                           */
/**********************************************************************************************************************************/

struct ComboInfo
{
  byte                    Type;                                                                       /* COMT_VIEW or COMT_SELECT */
  byte                    Flags;                                                                             /* Combowindow flags */
  struct ComboInfo       *LinkedWindow;                                                         /* (When called LinkComboWindows) */
  struct WindowInfo       OverlayWindow;                                       /* Window on top of which all subwindows are drawn */
  struct WindowInfo       ComboWindow;                                                    /* The window where the entry text goes */
  struct SliderInfo      *VSlider;                                                                         /* The vertical slider */
  struct SliderInfo      *HSlider;                                                                       /* The horizontal slider */
  struct ButtonInfo       FakeButton;                                           /* Window->Button conversion of the `ComboWindow' */
  bool                    KeepUpdated;                                          /* TRUE if the contents should be kept up to date */
  time_t                  TimeStamp;                                                                          /* Last update time */
  time_t                  UpdateInterval;                                                           /* Update interval in seconds */
  void                  (*UpdateHandler)(struct ComboInfo *Window);                                   /* Handler to do the update */
  short                   MaxEntrySize;                                                  /* Maximum length of an entry.text field */
  short                   MaxDataWidth;                                                     /* Maximum found width of all entries */
  comboent                NumEntries;                                                              /* Total number of entries - 1 */
  struct ComboEntry     **EntryTable;                                                                             /* The viewport */
  short                   DHeight;                                                                   /* Displayed number of lines */
  comboent                DFirstLine;                                                   /* Entry number of the displayed top line */
  short                   DFirstRow;                                                 /* Index into the leftmost entry text column */
  short                   TextWidth;                                                    /* Width of the text window in characters */
  short                   TextHeight;                                                       /* Height of the text window in lines */
  comboent                LastSelected;                                                             /* Last selected entry number */
  short                   TableXMSHandle;                                                         /* XMS handle of the entrytable */
  short                   XMSEntrySize;                                                    /* Size of 1 entry in XMS (power of 4) */
  bool                    Sliding;                                                           /* TRUE if currently in sliding mode */
  bool                    LastToggled;                                             /* TRUE if the 'LastSelected' entry is toggled */
  byte                    SelectToggle;                                                 /* Bitmask to toggle Selected entry field */
  byte                    SelectExclude;                                               /* Bitmask when an entry cannot be toggled */
  bool                    ContainsBlink;                                               /* TRUE if the viewport has blinking parts */
  dword                   BlinkRate;                                                                         /* Speed of blinking */
};

/**********************************************************************************************************************************/
/* Intermediate structure to store information for drag & drop between linked combowindows                                        */
/* Valid for use when `HandleComboWindow' returned WINH_DRAGDROPPED (used while WINH_DRAGDROPING)                                 */
/**********************************************************************************************************************************/

extern struct
{
  struct ComboInfo     *SourceWindow;
  struct ComboInfo     *DestinationWindow;
  comboent              SourceEntry;                                                                         /* The dragged entry */
  comboent              DestinationEntry;                                                          /* Dragged to AFTER this entry */
  bool                  MultipleEntries;                                      /* TRUE if one of some selected entries was dragged */
  bool                  MoveEntries;                                                             /* TRUE for move, FALSE for copy */
}                       DraggedComboEntry;

/**********************************************************************************************************************************/
/* Define the Button information structure                                                                                        */
/**********************************************************************************************************************************/

struct SliderButton
{
  /* User supplied part */
  short                   StartX;                                       /* The relative offset to the top-left of the host window */
  short                   StartY;                                        /* (Is converted after the `AddSliderButton' subroutine) */
  short                   Width;                                                                            /* (Height always 13) */
  short                   APen;                                                                  /* Inner-color of the slide-line */
  short                   BPen;                                                                     /* Color of the slider itself */
  short                   ReactNum;                                             /* Reaction number, supplied for user convenience */
  word                    Flags;                                                                              /* The button flags */
  short                   NumSteps;
  short                  *Contents;                                                                              /* Current value */

  /* System supplied part */
  short                   EndX;                                                                 /* The rightmost pixel coordinate */
  short                   EndY;                                                                /* The bottommost pixel coordinate */
  struct ButtonInfo       FakeButton;                                             /* Box->Button conversion of the `SliderButton' */
  double                  SliderStart;                                 /* Starting pixel of the slider bar, relative to the start */
  double                  SliderLength;                                                         /* Length of the slider in pixels */
  double                  StepRate;                                                          /* Slider bar difference for 1 entry */
  bool                    Sliding;                                                           /* TRUE if currently in sliding mode */
  byte                   *SliderButton;                                           /* Picture of the button itself (chunky - 8x15) */
};

struct InputBoxInfo
{
  /* User supplied part */
  short                   StartX;                                       /* The relative offset to the top-left of the host window */
  short                   StartY;                                            /* (Is converted after the `AddInputBox' subroutine) */
  short                   Width;
  short                   Height;
  short                   APen;                                                                           /* The foreground color */
  short                   BPen;                                                                           /* The background color */
  struct FontInfo        *Font;                                                                       /* The font to use for text */
  short                   ReactNum;                                             /* Reaction number, supplied for user convenience */
  char                   *Contents;                                                                 /* The contents of the button */
  word                    Flags;                                                                              /* The button flags */
  short                   MaxInputLength;                                           /* The maximum contents length (0 = infinite) */
  short                   MaxNumLines;                                                  /* Maximum number of lines (0 = infinite) */

  /* System supplied part */
  short                   EndX;                                                                 /* The rightmost pixel coordinate */
  short                   EndY;                                                                /* The bottommost pixel coordinate */
  struct ButtonInfo       FakeButton;                                                 /* Box->Button conversion of the `InputBox' */
  short                   NumLines;                                                                  /* Total number of lines - 1 */
  char                  **LineTable;                                                                           /* The entry table */
};

/**********************************************************************************************************************************/
/* Define the Percentage Requester information structure                                                                          */
/**********************************************************************************************************************************/

struct PReqInfo
{
  struct WindowInfo       PReqWindow;                                                             /* The window where it all goes */
  struct ButtonInfo       PButton;                                                             /* `Button' for the percentage bar */
  struct ButtonInfo       PCancel;                                                               /* Button to cancel to operation */
  struct MouseShape      *OldMouseShape;
  bool                    ACancel;                                                         /* TRUE if the cancel button is in use */
  double                  Percentage;                                             /* The percentage to be written into the window */
};

/**********************************************************************************************************************************/
/* Define the Mouse information structure                                                                                         */
/**********************************************************************************************************************************/

struct MouseInfo
{
  int                     OldXx;                                       /* Pixel coordinates at the previous call to `MouseStatus' */
  int                     OldYy;
  int                     Xx;                                                                        /* Current pixel coordinates */
  int                     Yy;
  bool                    CoChange;                               /* TRUE if the coordinates have changed since the previous call */
  bool                    LeftStillPressed;                /* TRUE if the left mouse button was also pressed an the previous call */
  bool                    LeftChange;                      /* TRUE if the left mouse button changed state since the previous call */
  bool                    RightStillPressed;              /* TRUE if the right mouse button was also pressed an the previous call */
  bool                    RightChange;                    /* TRUE if the right mouse button changed state since the previous call */
  bool                    MiddleStillPressed;            /* TRUE if the middle mouse button was also pressed an the previous call */
  bool                    Left;                                                        /* Current status of the left mouse button */
  bool                    Middle;                                                    /* Current status of the middle mouse button */
  bool                    Right;                                                      /* Current status of the right mouse button */
  bool                    Hidden;                                                          /* TRUE if the mouse pointer is hidden */
  byte                    NumButtons;                                                                  /* Number of mouse buttons */
  word                    DriverVersion;                                                                  /* Mouse driver version */
  byte                    MouseType;                                                                                /* Mouse type */
  byte                    MouseIRQ;                                                                           /* Mouse IRQ number */
  dword                   DoubleClickInterval;                                   /* Inter-click time for double-click recognition */
  dword                   CPushDelay;                                                     /* Initial continuous push button delay */
  bool                    SnapToDefaultButton;                                   /* If TRUE, the mouse goes to the default button */
  bool                    SwapLeftAndRightButton;
  bool                    UseGeniusScreenUpdate;                         /* If TRUE, the Genius syntax is used for HideMouseRange */
  bool                    DisableLocalHiding;                                  /* If TRUE, will treat HideMouseRange as HideMouse */
  bool                    ForceMouseRelease;                             /* Will set the video mode even if driver version < 7.00 */
  bool                    EmulateMiddleMouseButton;                                     /* Emulate 3rd button on a 2-button mouse */
  word                    HorizontalMickeys;
  word                    VerticalMickeys;
  word                    DoubleSpeedThreshold;
};

/**********************************************************************************************************************************/
/* Define the Mouse shape structure                                                                                               */
/**********************************************************************************************************************************/

struct MouseShape
{
  short                   HotspotX;                                                                    /* Location of the hotspot */
  short                   HotspotY;
  word                    ScreenMask[16];                                                                /* The screen mask (AND) */
  word                    CursorMask[16];                                                                /* The cursor mask (XOR) */
};

/**********************************************************************************************************************************/
/* Define the Blanker information structure                                                                                       */
/**********************************************************************************************************************************/

struct BlankerInfo
{
  word                    TimeOut;                              /* Number of seconds to wait before activating the screen blanker */
  bool                    Active;                                                                /* Screen is currently blanked ? */
  time_t                  TimeStamp;                                                      /* Time of last keyboard/mouse activity */
  bool                    WakeUp;                                                        /* TRUE if the blanker should be stopped */
  void                  (*Blanker)(void);                                                                      /* Blanker routine */
  struct WindowInfo       Screen;                                          /* Included for the blanker routine: the entire screen */
};

/**********************************************************************************************************************************/
/* Define the Menu entry structure                                                                                                */
/**********************************************************************************************************************************/
 
struct MenuEntry
{
  /* User supplied part */
  word                    ActivationKey;                                  /* If MEF_KEYSELECT set: the key to activate this entry */
  short                   ReactNum;                                                              /* Supplied for user convenience */
  byte                    Flags;
  bool                   *BoolValue;                                                    /* If MEF_BOOL: This is the current value */
  dword                   MutualExclude;                              /* All entries that must be deselected if you pick this one */
  int                   (*Handler)(struct MenuEntry *Entry);                                                 /* The entry handler */
  char                    Text[MAXMENUENTRYNAME + 1];

  /* System supplied part */
  struct MenuInfo        *SubMenu;
};
  
/**********************************************************************************************************************************/
/* Define the Menu information structure                                                                                          */
/**********************************************************************************************************************************/
 
struct MenuInfo
{
  struct MenuInfo        *NextMenu;                                                             /* The menu's are linked together */
  struct MenuInfo        *PreviousMenu;
  short                   StartX;                                                                        /* Position of menu name */
  short                   EndX;                                                                           /* End of the menu name */
  word                    Flags;
  char                    MenuName[MAXMENUNAME];                                                          /* The name of the menu */
  short                   MaxDataWidth;                                                     /* Maximum found width of all entries */
  short                   TailLength;                                               /* Num. extra characters after the entry name */
  short                   NumEntries;                                                              /* Total number of entries - 1 */
  struct MenuEntry      **EntryTable;
};

/**********************************************************************************************************************************/
/* Define the System Defaults information structure                                                                               */
/**********************************************************************************************************************************/

struct SystemDefInfo
{
  bool                    EditInsert;                                                              /* Default: FALSE (overstrike) */
  bool                    DragOutline;                                  /* Default: TRUE (FALSE updates entire window constantly) */
  bool                    ResizeOutline;                                /* Default: TRUE (FALSE updates entire window constantly) */
  bool                    Quiet;                                                                  /* Default: FALSE (allow beeps) */
  bool                    GreyOutGhosting;                                                     /* Default: FALSE (inlay ghosting) */
  bool                    SyncSlider;                                                                      /* Follow slider bar ? */
  bool                    ForceSwapMode80;                                              /* Swapped video mode forced to MODE 80 ? */
  bool                    ForceQuitMode80;                                         /* Program quit video mode forced to MODE 80 ? */
};
  
/**********************************************************************************************************************************/
/* Define the System Colors information structure                                                                                 */
/**********************************************************************************************************************************/
 
struct SystemColInfo
{
  short                   ButtonOutline;                                                                       /* Default: DBLACK */
  short                   ShadowLight;                                                                         /* Default: LWHITE */
  short                   ShadowDark;                                                                          /* Default: LBLACK */
  short                   WindowSelect;                                                                         /* Default: DBLUE */
  short                   ButtonSelectionBorder;                                                               /* Default: DBLACK */
  short                   GhostedLight;                                                                        /* Default: LWHITE */
  short                   GhostedDark;                                                                         /* Default: LBLACK */
  short                   ReqAPen;                                                                             /* Default: DBLACK */
  short                   ReqBPen;                                                                             /* Default: DWHITE */
  short                   ReqOPen;                                                                             /* Default: DWHITE */
  short                   ComboSelectionBorder;                                                                  /* Default: APEN */
  short                   FileReqDirAPen;                                                                       /* Default: DBLUE */
  short                   FileReqSelBPen;                                                                       /* Default: DCYAN */
  short                   PReqPAPen;                                                                            /* Default: DBLUE */
  short                   PReqPBPen;                                                                             /* Default: BPEN */
  short                   MenuAPen;                                                                            /* Default: DWHITE */
  short                   MenuBPen;                                                                            /* Default: DBLACK */
  short                   MenuGhostedLight;                                                                    /* Default: DWHITE */
  short                   MenuGhostedDark;                                                                     /* Default: LBLACK */
};

/**********************************************************************************************************************************/
/* Define the screen information                                                                                                  */
/**********************************************************************************************************************************/

struct ScreenInfo
{
  short                   Width;
  short                   Height;
  dword                  *DefaultPal;                                                                      /* Default VGA palette */
  dword                  *CurrentPal;                                                                      /* Current VGA palette */
  byte                   *DisplayStart;                                                                /* Start of display memory */
  short                   NumColors;
  short                   NumPlanes;
  short                   ColorRes;                                                                   /* Number of bits per color */
  byte                    RILMajorVersion;                                           /* Version of the Register Interface Library */
  byte                    RILMinorVersion;
  bool                    ColorDisplay;                                                           /* TRUE if the display is color */
  bool                    UseRIL;                                                            /* TRUE if the RIL has been detected */
};

/**********************************************************************************************************************************/
/* Define the SoundBlaster information                                                                                            */
/**********************************************************************************************************************************/

struct SoftMixer_s
{
  byte                  MasterVolume;
  byte                  DACLevel;
  byte                  LineInLevel;
  byte                  InputGain;                                                                                   /* SB16 only */
  byte                  OutputGain;                                                                                  /* SB16 only */
  bool                  AutoGain;                                                                                    /* SB16 only */
  byte                  Treble;                                                                                      /* SB16 only */
  byte                  Bass;                                                                                        /* SB16 only */
  byte                  PCSpeakerLevel;                                                                              /* SB16 only */
  byte                  InputControl;                                                                          /* SBPro/SB16 only */
  byte                  OutputControl;                                                                               /* SB16 only */
};

struct SoundBlasterInfo_s
{
  char                 *EnvVarP;
  word                  BaseAddress;
  word                  AD1848BaseAddress;                                                              /* (Analog Devices CODEC) */
  byte                  IRQ;
  byte                  DMAChannel;
  byte                  DSPMajorVersion;
  byte                  DSPMinorVersion;
  byte                  MixerType;
  struct SoftMixer_s    Mixer;                                                                   /* Current software mixer values */
  struct SoftMixer_s    MixerMax;                                                            /* Maximum software mixer values + 1 */
  bool                  Detected;
  bool                  CanDoHighSpeedDMA;                                                         /* Some clones cannot do this! */
  bool                  DoingDMA;
};
extern struct SoundBlasterInfo_s SoundBlasterInfo;

extern byte                     *SB_DMAComplete;                                         /* Set by the SoundBlaster DMA interrupt */
                                                               /* When using AutoInit DMA, is FALSE when the first buffer half is */
                                                                      /* being played, TRUE when the second half is player (poll) */
extern byte                     *SB_BufferReady;                                               /* Set by the user in AutoInit DMA */
                                                                                        /* Cleared by the system on DMA-ready-INT */
extern bool                     *SB_DMAOverrun;            /* Set by the DMA-ready-INT when the buffer was not filled fast enough */

/**********************************************************************************************************************************/
/* Define some common error messages                                                                                              */
/**********************************************************************************************************************************/

extern char                     *_FileReadError;                                                 /* = "Error reading file %s: %s" */
extern char                     *_FileWriteError;                                                /* = "Error writing file %s: %s" */
extern char                     *_FileSearchError;                                             /* = "Error searching file %s: %s" */
extern char                     *_ErrOutOfMemory;                                 /* = "Out of memory - Cannot allocate %u bytes" */

/**********************************************************************************************************************************/
/* Define the global variables                                                                                                    */
/**********************************************************************************************************************************/

extern short                     WBMajorVersion;                                                         /* Version of Workbench! */
extern short                     WBMinorVersion;
extern short                     WBRevision;    

extern bool                      MSWindowsRunning;                                         /* TRUE if Microsoft Window is running */
extern bool                      MSWindowsStopped;                                  /* TRUE if Microsoft Windows has been stopped */
extern word                      MSWindowsVersion;
extern word                      MSWindowsMode;                                           /* 0x0002 = standard, 0x0003 = enhanced */

/* The hardware timer used in the ReadTimer, ElapsedTime and Delay routines has a resolution of 32-bits. Since the clock ticks at */
/* a rate of 1193180Hz, this resolution is just enough for 3600 seconds. Due to timer corrections that are required to overcome   */
/* bugs in Windows 98, fall-over through zero is undetectable. The following variable fixes this, by adding 8 extra bits to the   */
/* timer. This variable is globally readable and not part of the timer return value. It is initialized at 0 at Workbench! startup */

extern byte                      TimerHighByte;                                                       /* Timer overlap bits 33-40 */

extern byte                     *GlobalVideoPlaneBuffer;
extern byte                     *GlobalCriticalSectionBuffer;
extern byte                     *GlobalFileCopyBuffer;                                   /* MUST be exactly 16Kb if preallocated! */
extern word                      VBlankRegister;                                               /* Vertical Blank register (bit 3) */

extern word                      CurrentFloppyId;
extern word                      NextFloppyId;
extern bool                      FloppyIdChanged;
extern bool                      MaxFloppyIdChanged;
extern char                     *FloppyMarkName;
extern char                      ResourceFloppyId;                                          /* Used to load resources from floppy */

extern bool                      MenuStripActive;                                /* TRUE if the menu strip is currently on screen */
extern word                      MenuActivationKey;                                            /* If pressed, activate menu strip */

extern struct MouseInfo          Mouse;                                                                              /* The mouse */
extern struct MouseShape        *DefaultNormalMouseShape;                                             /* Mouse cursor information */
extern struct MouseShape        *DefaultWaitingMouseShape;
extern struct MouseShape        *CurrentMouseShape;

extern struct SystemDefInfo      SystemDefaults;                                                               /* System defaults */
extern struct SystemColInfo      SystemColors;                                                                   /* System colors */
extern struct ScreenInfo         Screen;                                                                    /* Screen information */
extern struct WindowInfo        *ActiveWindow;                                                         /* Currently active window */
extern bool                      EnhancedKeyboard;                      /* TRUE if an enhanced (101-keys) keyboard was identified */
extern int                       NumFloppyDrives;                                             /* Number of detected floppy drives */
extern dword                     DrivesMap;                                    /* Each set bit represents an available disk drive */
extern int                       CurDrv;                                                              /* The current drive number */
extern char                      CurPath[MAXDIR];                                                        /* The current directory */
extern char                      ProgramDrive[_MAX_DRIVE];                                              /* The startup drive name */
extern char                      ProgramPath[MAXDIR];                                                    /* The startup directory */
extern char                      ProgramName[9];                                                      /* The raw application name */
extern char                      TmpDirectory[MAXDIR];                                           /* Directory for temporary files */
extern short                     SelectedAPen[4];  /* Colors to be used when printing a combo entry with the `Selected' field set */
extern short                     SelectedBPen[4];
extern word                      GlobalKey;                                        /* Last pressed key, 0x0000 if already handled */
extern word                      GlobalShiftStatus;                                                 /* Last read key shift status */
extern word                      ALTKey;                                            /* Re-assigned `GlobalKey' if ALT was pressed */
extern struct FontInfo          *DefaultFont;                                             /* Pointer to the default (system) font */
extern struct FontInfo          *DefaultHFont;                                     /* Pointer to the default (system) header font */
extern struct FontInfo          *DefaultGadgets;                                          /* Pointer to the default gadget `font' */
extern struct FontInfo          *CurrentFont;                                                       /* Pointer to the active font */
extern struct FontInfo          *CurrentGadgets;                                           /* Pointer to the active gadget `font' */

extern byte                     *WBPics[NUMPICS];                                              /* Predefined (requester) pictures */

extern struct BlankerInfo        Blanker;                                                                          /* The blanker */

extern byte                      SwapReturn;                                                    /* Value returned by swap routine */
extern byte                      ExecReturn;                                            /* Value returned by external application */

extern bool                      HWEAbort;                                             /* Abort on critical errors (no requester) */
extern bool                      HWEOccured;                                              /* TRUE if a critical error has occured */
extern byte                      HWEReportType;                          /* Defines the reported items in the requester: HWER_xxx */

extern byte                      XMSError;                                                               /* Last XMS error number */
extern word                      XMSVersion;                                                          /* XMS Version number (BCD) */
extern word                      XMSRevision;                                               /* XMS internal revision number (BCD) */

extern bool                      DPMIServerDetected;
extern byte                      DPMIMajorVersion;
extern byte                      DPMIMinorVersion;
extern byte                      DPMIFlags;

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> THE INTERFACE ROUTINES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

/**********************************************************************************************************************************/
/* Mouse functions                                                                                                                */
/**********************************************************************************************************************************/

void  ShowMouse                (void);
void  HideMouse                (void);
void  HideMouseRange           (short StartX, short StartY, short EndX, short EndY);
void  MouseStatus              (void);
bool  ResetMouse               (void);
void  PositionMouse            (short Column, short Row);
void  SetMouseRange            (short MinCol, short MaxCol, short MinRow, short MaxRow);
void  SetMouseShape            (struct MouseShape *Shape);
void  SetMouseSpeed            (word HorizontalMickeys, word VerticalMickeys, word DoubleSpeedThreshold);
void  SnapMouseToButton        (struct WindowInfo *Window, struct ButtonInfo *Button);
void  ClearMouseEventTable     (void);
void  DelayLeftMouseButton     (void);
void  DelayRightMouseButton    (void);

/**********************************************************************************************************************************/
/* XMS interface                                                                                                                  */
/**********************************************************************************************************************************/

dword XMSAllAllocated          (void);
dword XMSAllUsed               (void);
dword XMSLargestCore           (void);
dword XMSAllCore               (void);
dword XMSAllAllocatedUMB       (void);
word  XMSLargestUMB            (void);
byte  XMSAlloc                 (short *Handle, dword BlockSize);
byte  XMSRealloc               (short Handle, dword BlockSize, bool ExpandOnly);
void *XMSAllocUMB              (dword BlockSize);
bool  XMSFreeUMB               (void *UMBPointer);
byte  XMSFree                  (short Handle);
void  FreeAllXMS               (void);
void  FreeAllUMB               (void);
byte  CopyToXMS                (dword BlockSize, short Handle, void *From, dword To);
byte  CopyFromXMS              (dword BlockSize, short Handle, dword From, void *To);
byte  CopyInXMS                (dword BlockSize, short SrcHandle, dword From, short DstHandle, dword To);
char *XMSstrerror              (byte ErrorCode);

/**********************************************************************************************************************************/
/* Menu strip                                                                                                                     */
/**********************************************************************************************************************************/

struct MenuInfo *AddMenu       (struct WindowInfo *Window, char *MenuName, word Flags);
bool  AddMenuEntry             (struct MenuInfo *Menu, struct MenuEntry *Entry);
void  ConvertToSubMenu         (struct MenuInfo *Menu, struct MenuEntry *Entry);
void  UpdateMenuWidths         (struct MenuInfo *Menu);

/**********************************************************************************************************************************/
/* Requesters                                                                                                                     */
/**********************************************************************************************************************************/

bool  ReadFloppyId             (char DriveLetter, bool CreateIfNeeded);
bool  MarkFloppy               (char DriveLetter, word *NextId, word FloppyId);
void  ConvertPath              (char *OldPath, char *NewPath, word FloppyId);
bool  RequestFloppy            (char DriveLetter, word FloppyId);

bool  InputRequester           (short StartX, short StartY, char *PositiveText, char *NegativeText, char *InputBuffer,
                                short MaxInput, bool ReadAlpha, bool ReadDir, char *Header, char *Prompt, ...);
byte  QuestionRequester        (short StartX, short StartY, bool Use3rdButton, bool Use4thButton, char *PositiveText,
                                char *NegativeText, char *CancelText, char *YesAllText, byte *CustomPicture, char *Header,
                                char *Question, ...);
void  ErrorRequester           (short StartX, short StartY, char *PositiveText, byte *CustomePicture, char *Header,
                                char *ErrorMessage, ...);
void  MakePRequester           (struct PReqInfo *PReq, short StartX, short StartY, bool AddCancel,
                                struct MouseShape *CustomWaitingCursor, char *Header, char *Format, ...);
bool  UpdatePRequester         (struct PReqInfo *PReq, double Percentage, char *Format, ...);
void  DestroyPRequester        (struct PReqInfo *PReq);
void  FileViewRequester        (short StartX, short StartY, short MaxEntrySize, short MaxEntryView, short TextWidth,
                                short TextHeight, char *FileName, bool PlainText, char *Header);
bool  FileRequester            (short StartX, short StartY, short MaxEntrysize, short TextWidth, short TextHeight,
                                char *Path, char *FileName, char *Mask, char *Header, bool MustExist, bool NeedDirectory);
void  MakeFullPath             (char *Result, char *Path, char *FileName);
char *GetTmpDirSetting         (void);
bool  MatchFileToMask          (char *FileName, char *Mask);
char *TestName                 (char *OldName, bool AllowWildcards, bool *ErrorReturn);
char  CopyFilePart             (int FhIn, int FhOut, long Length);
bool  CopyFile                 (char *OldPath, char *NewPath, bool *Multiple, bool *OverwriteAll, char *Message);
bool  MoveFile                 (char *OldPath, char *NewPath, bool *Multiple, bool *OverwriteAll);

/**********************************************************************************************************************************/
/* Workbench main functions                                                                                                       */
/**********************************************************************************************************************************/

void  SetINT8Handler           (word Ticks, void (interrupt *TimerHandler)(void));
dword ElapsedTime              (dword StartTime);
dword ReadTimer                (void);
void  Delay                    (dword NumMilliSeconds);
void  EnterCriticalSection     (void);
void  LeaveCriticalSection     (void);

void  InitGUI                  (char *StartupPath, short BkColor, dword *NewPalette, bool StopMSWindow,
                                bool HaveSoundcard, bool HaveMixer);
void  EndGUI                   (void);

void  UseXTKeyboard            (void);
void  PushKey                  (struct WindowInfo *Window, word Key, word ShiftStatus);

void  SoundErrorBeeper         (byte Duration);
void  FatalError               (int ExitCode, char *Format, ...);
char *StrError                 (int ErrorCode);

void  SelectFont               (struct FontInfo *Font, word Flags);
void  ReleaseFont              (struct FontInfo *Font);

void  AddButton                (struct WindowInfo *Window, struct ButtonInfo *Button, bool Default);
void  AddInputBox              (struct WindowInfo *Window, struct InputBoxInfo *InputBox, bool Default);
void  AddSliderButton          (struct WindowInfo *Window, struct SliderButton *SliderButton, bool Default);
void  DrawButton               (struct WindowInfo *Window, struct ButtonInfo *Button);
void  DrawInputBox             (struct WindowInfo *Window, struct InputBoxInfo *InputBox);
int   ForceButtonPress         (struct WindowInfo *Window, struct ButtonInfo *Button);
int   ForceButtonRelease       (struct WindowInfo *Window, struct ButtonInfo *Button);
void  ClearButton              (struct WindowInfo *Window, struct ButtonInfo *Button);
void  UpdateButtonText         (struct WindowInfo *Window, struct ButtonInfo *Button, word Flags);
void  UpdateInputBoxText       (struct WindowInfo *Window, struct InputBoxInfo *InputBox, word Flags, short WhichLine);

void  DrawWindow               (struct WindowInfo *Window, struct WindowInfo *HostWindow, byte AddGadgets, bool DrawButtons);
void  ClearWindow              (struct WindowInfo *Window);
void  ActivateWindow           (struct WindowInfo *Window);
void  DestroyWindow            (struct WindowInfo *Window);
int   HandleWindow             (struct WindowInfo *Window, struct ButtonInfo **Button);
void  WClearLine               (struct WindowInfo *Window, short Yco);
void  WDrawPixel               (struct WindowInfo *Window, short Color, short StartX, short StartY);
void  WDrawLine                (struct WindowInfo *Window, short Color, short StartX, short StartY, short EndX, short EndY);
void  WDrawBox                 (struct WindowInfo *Window, bool Fill, short Color, short StartX, short StartY, short EndX,
                                short EndY);
void  WDrawShadowLine          (struct WindowInfo *Window, short Yco);
void  WDrawShadowBox           (struct WindowInfo *Window, short Yco, short Xco, short Yend, short Xend, char *Header);
void  WDrawPicture             (struct WindowInfo *Window, short StartX, short StartY, byte *Picture);
bool  Wprintf                  (struct WindowInfo *Window, short Yco, short Xco, char *Format, ...);

void  ReInitComboWindow        (struct ComboInfo *Window, comboent NewMinEntries);
void  ClearComboWindow         (struct ComboInfo *Window);
void  GetComboAttributes       (struct ComboInfo *Window, struct PrintAttributes *Attributes);
bool  FitComboString           (struct ComboInfo *Window, struct PrintAttributes *Attributes, char *String, short MaxLen,
                                byte Selected, dword Identifier, bool NoSmartWrap);
bool  InsertComboEntry         (struct ComboInfo *Window, struct ComboEntry *Entry, struct PrintAttributes *Attributes,
                                comboent Location);
bool  DeleteComboEntry         (struct ComboInfo *Window, comboent Location);
void  MakeComboWindow          (struct ComboInfo *Window, struct WindowInfo *HostWindow, byte Type, byte Flags,
                                short StartX, short StartY, comboent MinEntries, word TableChunkSize, short MaxEntrySize,
                                short TextWidth, short TextHeight, struct FontInfo *Font, bool KeepUpdated, time_t UpdateInterval,
                                void (*UpdateHandler)(struct ComboInfo *Window), short APen, short BPen, byte SelectToggle,
                                byte SelectExclude);
void  PrintComboEntries        (struct ComboInfo *Window);
bool  GetComboEntry            (struct ComboInfo *Window, struct ComboEntry *Entry, comboent EntryNumber);
bool  PutComboEntry            (struct ComboInfo *Window, struct ComboEntry *Entry, comboent EntryNumber, bool UpdateScreen);
int   HandleComboWindow        (struct ComboInfo *Window, comboent *SetValue);
void  DestroyComboWindow       (struct ComboInfo *Window, byte Flags);
bool  SelectEntry              (struct ComboInfo *Window, comboent EntryNumber);
void  LinkComboWindows         (struct ComboInfo *Window1, struct ComboInfo *Window2);
void  UnLinkComboWindows       (struct ComboInfo *Window1, struct ComboInfo *Window2);

void  DisableBlanker           (void);
void  InstallBlanker           (word TimeOut, void (*UserBlanker)(void));
void  BlankerWakeUp            (void);

void  RegisterExitHandler      (int (*Handler)(int ExitCode));

void  RegisterLowMemHandler    (void (*Handler)(int MemoryType, char *ErrorMessage));

void  RegisterRescueHandler    (void (*Handler)(int ExitCode, char *ErrorMessage));

void  RegisterTimedHandler     (dword TimeInterval, void (*TimedHandler)(struct WindowInfo *Window), struct WindowInfo *LockWindow);
void  ReleaseTimedHandler      (void (*TimedHandler)(struct WindowInfo *Window), struct WindowInfo *LockWindow);
void  ReleaseAllTimedHandlers  (void);

void  DisableInterrupts        (void);
void  EnableInterrupts         (void);

/**********************************************************************************************************************************/
/* Memory functions                                                                                                               */
/**********************************************************************************************************************************/

void  DOS_memalloc             (word NumParas, word *Segment, word *Selector);
#pragma  aux DOS_memalloc = \
  "push  ecx"               \
  "push  edx"               \
  "mov   ax, 0100h"         \
  "int   31h"               \
  "pop   ebx"               \
  "mov   [ebx], dx"         \
  "pop   ebx"               \
  "mov   [ebx], ax"         \
  parm   [bx] [ecx] [edx]   \
  modify [ax ebx ecx edx];

void  DOS_memfree              (word Selector);
#pragma  aux DOS_memfree =  \
  "mov   ax, 0101h"         \
  "int   31h"               \
  parm   [dx]               \
  modify [ax dx];

#define WritePort(Port,Value)  outp(Port,Value)
#define ReadPort(Port)         inp(Port)

/**********************************************************************************************************************************/
/* Video functions                                                                                                                */
/**********************************************************************************************************************************/

void  RemapAllPalette          (dword *NewPalette);
void  SelectPlane              (byte Plane);
void  StartScreenUpdate        (short StartX, short StartY, short EndX, short EndY);
void  StopScreenUpdate         (void);

void  BlankScreen              (void);
void  UnblankScreen            (void);

void  SaveDisplayToXMS         (void);
void  RestoreDisplayFromXMS    (void);

short CopyImageToXMS           (short StartX, short StartY, short EndX, short EndY);
void  CopyImageFromXMS         (short Handle);

/**********************************************************************************************************************************/
/* Swap function                                                                                                                  */
/**********************************************************************************************************************************/

bool  SwapOut                  (bool ErrorReport, char *Progname, char *ProgParam, char *SwapFile, char *SwapText,
                                bool UseSystem, bool WorkbenchApplication, bool IgnoreErrorReturn);

/**********************************************************************************************************************************/
/* GIF creation functions                                                                                                         */
/**********************************************************************************************************************************/

byte  GIFCreate                (char *FileName, short Width, short Height, short NumColors, short ColorRes);
void  GIFSetColor              (byte ColorNum, byte Red, byte Green, byte Blue);
byte  GIFCompressImage         (short StartX, short StartY, short Width, short Height);
byte  GIFClose                 (void);
void  GIFMakePicture           (char *FileName, short StartX, short StartY, short Width, short Height, dword *Palette);
char *GIFstrerror              (byte ErrorCode);

/**********************************************************************************************************************************/
/* SoundBlaster functions                                                                                                         */
/**********************************************************************************************************************************/

void  SB_Speaker               (bool TurnOn);
void  SB_PlayByte              (byte SoundByte);
byte  SB_SampleByte            (void);
void  SB_PlayDMA               (byte *Buffer, word SampleRate, word Length);
void  SB_SampleDMA             (char *Buffer, word SampleRate, word Length);
void  SB_PlayAutoInitDMA       (byte *Buffer, word SampleRate, word Length);
void  SB_SampleAutoInitDMA     (byte *Buffer, word SampleRate, word Length);
void  SB_StopDMA               (void);
void  SB_Reset                 (void);
void  SB_WriteMixer            (byte MixerRegister, byte Value);

#ifdef __cplusplus
};
#endif
#endif
