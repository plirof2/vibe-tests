/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBSELECT.C                                                                                                      */
/* Description  : Database select handler                                                                                         */
/* Version type : Program module                                                                                                  */
/* Last changed : 09-05-1999  12:15                                                                                               */
/* Update count : 21                                                                                                              */
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

#define __DBSELECT_MAIN__

#include <fcntl.h>
#include <io.h>
#include "wbench32/wbench32.h"
#include "dbdbase.h"
#include "dbconfig.h"
#include "dbfile.h"
#include "dbmain.h"
#include "dbselect.h"

#pragma pack                ()

/**********************************************************************************************************************************/
/* Define the selection criterion interface                                                                                       */
/**********************************************************************************************************************************/

static struct DBaseEntry_s  _SelectEntry;
static short                _SelectAction;
static char                 _PlayBitValue;
static struct CriterionInfo _CriteriaVar[MAXCRIT] = {
    { "NM", CT_ALPHA, FALSE, 0, D_NAME,       Cr_Name,         _SelectEntry.Name       },
    { "PU", CT_ALPHA, FALSE, 0, D_PUBLISHER,  Cr_Publisher,    _SelectEntry.Publisher  },
    { "YR", CT_NUM,   FALSE, 0, D_YEAR,      &Cr_Year,         _SelectEntry.Year       },
    { "ME", CT_BNUM,  FALSE, 0, D_MEMORY,    &Cr_Memory,      &_SelectEntry.Memory     },
    { "PL", CT_NUM,   FALSE, 0, 1,           &Cr_Players,      _SelectEntry.Players    },
    { "TO", CT_BOOL,  FALSE, 0, 1,           &Cr_Together,    &_SelectEntry.Together   },
    { "PC", CT_PATH , FALSE, 0, 12,           Cr_PCName,       _SelectEntry.PCName     },
    { "TP", CT_ALPHA, FALSE, 0, D_TYPE,       Cr_Type,         _SelectEntry.Type       },
    { "PB", CT_BOOL,  FALSE, 0, 1,           &Cr_PlayBit,     &_PlayBitValue           },
    { "OS", CT_BOOL,  FALSE, 0, 1,           &Cr_OrigScreen,  &_SelectEntry.OrigScreen },
    { "OR", CT_ALPHA, FALSE, 0, 1,            Cr_Origin,      &_SelectEntry.Origin     },
    { "FD", CT_INUM,  FALSE, 0, 3,           &Cr_DiskId,      &_SelectEntry.DiskId     },
    { "AY", CT_BOOL,  FALSE, 0, 1,           &Cr_AYSound,     &_SelectEntry.AYSound    },
    { "ML", CT_BOOL,  FALSE, 0, 1,           &Cr_MultiLoad,   &_SelectEntry.MultiLoad  },
    { "LN", CT_ALPHA, FALSE, 0, 3,           &Cr_Language,     _SelectEntry.Language   },
    { "SC", CT_INUM,  FALSE, 0, 3,           &Cr_Score,       &_SelectEntry.Score      },
    { "AU", CT_ALPHA, FALSE, 0, D_AUTHOR,     Cr_Author,       _SelectEntry.Author     },
    { "BY", CT_ALPHA, FALSE, 0, D_AUTHOR,     Cr_PubAuth,      _SelectEntry.Author     }};

/**********************************************************************************************************************************/
/* Define the static prototypes                                                                                                   */
/**********************************************************************************************************************************/

static bool _MatchString (char *String, char *Mask);

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE INTERNAL LIBRARY FUNCTIONS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

static bool _MatchString (char *String, char *Mask)

/**********************************************************************************************************************************/
/* Pre   : `String' points to the string to be matched against the string `Mask' (which may contain wildcards).                   */
/* Post  : The string has been matched. The return value is TRUE if the match was completed successfully.                         */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  int  IMask = 0;                                                                               /* Indices in 'Mask' and 'String' */
  int  IString;
  int  IiMask;                                                                                           /* Copy of these indices */
  int  IiString;
  int  LNMask;                                                                                   /* Length of 'Mask' and 'String' */
  int  LNString;
  bool WildLoop = FALSE;                                                                      /* TRUE if handling a '*' wild card */

  LNString = strlen (String);
  LNMask = strlen (Mask);
  if (LNString == 0 && LNMask == 1 && Mask[0] == CV_WILD1)                          /* Special case: empty field mathing with '*' */
    return (FALSE);
  for (IString = 0 ; IString < LNString ; IString ++)
    if (Mask[IMask] == CV_WILD1)
      if (++ IMask != LNMask)                                                                     /* '*' was the last character ? */
      {
        WildLoop = TRUE;
        IiString = -- IString;                                                              /* Do not update IString, keep copies */
        IiMask = IMask;
      }
      else
        return (TRUE);
    else
      if (Mask[IMask] == CV_WILD2)                                                                            /* A '?' wildcard ? */
      {
        if (++ IMask == LNMask)                                                                                  /* End of mask ? */
          if (IString != (LNString - 1))                                                     /* No match if not end of teststring */
            if (WildLoop)                                                                    /* Retrieve copies if handling a '*' */
            {
              IString = ++ IiString;
              IMask = IiMask;
            }
            else
              return (FALSE);
      }
      else                                                                                                     /* A 'normal' char */
        if (Mask[IMask] != toupper (String[IString]))
          if (WildLoop)                                                                      /* Retrieve copies if handling a '*' */
          {
            IString = ++ IiString;
            IMask = IiMask;
          }
          else
            return (FALSE);
        else                                                                                                 /* Character matches */
        {
          if (++ IMask == LNMask)                                                                                /* End of mask ? */
            if (IString != (LNString - 1))                                                                /* Also end of String ? */
              if (WildLoop)                                                                             /* Retrieve copies if not */
              {
                IString = ++ IiString;
                IMask = IiMask;
              }
              else
                return (FALSE);
        }
  if (IMask != LNMask)
    if (IMask == (LNMask - 1) && (Mask[IMask] == CV_WILD1))
      return (TRUE);
    else
      return (FALSE);
  return (TRUE);
}

/**********************************************************************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START OF THE EXPORTED LIBRARY FUNCTIONS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/**********************************************************************************************************************************/

bool ParseCriterion (char *CriterionString)

/**********************************************************************************************************************************/
/* Pre   : `CriterionString' points to the string to parse.                                                                       */
/* Post  : The (internal) _CriteriaVar structure array has been set up according to the criterion string.                         */
/*         The return value is FALSE if the string is invalid (This has already been reported to the user).                       */
/* Import: None.                                                                                                                  */
/**********************************************************************************************************************************/

{
  bool            Error       = FALSE;
  char           *StringWalk;
  char           *ValWalk;
  short register  Cnt;
  char            Type[3]     = "  ";
  char            Message[80] = "Unknown error";
  bool            Handled;
  bool            FirstBoolChar;
  int             StringLength;

  for (Cnt = 0 ; Cnt < MAXCRIT ; Cnt ++)
    _CriteriaVar[Cnt].Set = FALSE;
  StringWalk = CriterionString;
  if (CriterionEmpty = !*StringWalk)
    return (TRUE);
  switch (*StringWalk)
  {
    case CS_ADD : _SelectAction = CA_ADD; StringWalk ++; break;
    case CS_DEL : _SelectAction = CA_DEL; StringWalk ++; break;
    case CS_SET : _SelectAction = CA_SET; StringWalk ++; break;
    default     : _SelectAction = CA_SET; break;
  }
  if (CriterionEmpty = !*StringWalk)
    return (TRUE);
  while (!Error && *StringWalk)                                                                            /* 'Till end of string */
  {
    Type[0] = *(StringWalk ++);
    Type[1] = *(StringWalk ++);
    strupr (Type);
    Error = TRUE;
    Handled = FALSE;
    for (Cnt = 0 ; !Handled && Cnt < MAXCRIT ; Cnt ++)
      if (!strcmp (Type, _CriteriaVar[Cnt].Name))                                                                 /* Valid name ? */
      {
        Handled = TRUE;
        switch (_CriteriaVar[Cnt].Type)
        {
          case CT_PATH  :
          case CT_FBOOL :
          case CT_BOOL  : if (!(Error = ((*(StringWalk ++) != CO_EQUAL))))
                            *((word *)_CriteriaVar[Cnt].Value) = 0;
                          else
                            sprintf (Message, "%s: Unknown operator %c", Type, (*(-- StringWalk)));
                          break;
          case CT_ALPHA : switch (*(StringWalk ++))
                          {
                            case CO_NOT     : if (Error = (*(StringWalk ++) != CO_EQUAL))
                                                sprintf (Message, "%s: Unknown operator !%c", Type, *(-- StringWalk));
                                              _CriteriaVar[Cnt].Operator = CV_NOT;
                                              break;
                            case CO_LESS    : if (Error = (*(StringWalk ++) != CO_GREATER))    /* Exception: '<>' for 'not equal' */
                                                sprintf (Message, "%s: Unknown operator <%c", Type, *(-- StringWalk));
                                              _CriteriaVar[Cnt].Operator = CV_NOT;
                                              break;
                            case CO_EQUAL   : _CriteriaVar[Cnt].Operator = CV_EQUAL;
                                              Error = FALSE;
                                              break;
                            default         : Error = TRUE;
                                              sprintf (Message, "%s: Unknown operator %c", Type, (*(-- StringWalk)));
                                              break;
                          }
                          break;
          case CT_NUM   :
          case CT_BNUM  :
          case CT_INUM  : switch (*(StringWalk ++))
                          {
                            case CO_GREATER : if (*StringWalk == CO_EQUAL)
                                              {
                                                _CriteriaVar[Cnt].Operator = CV_GREATER | CV_EQUAL;
                                                StringWalk ++;
                                              }
                                              else
                                                _CriteriaVar[Cnt].Operator = CV_GREATER;
                                              Error = FALSE;
                                              break;
                            case CO_LESS    : if (*StringWalk == CO_EQUAL)
                                              {
                                                _CriteriaVar[Cnt].Operator = CV_LESS | CV_EQUAL;
                                                StringWalk ++;
                                              }
                                              else if (*StringWalk == CO_GREATER)
                                              {
                                                _CriteriaVar[Cnt].Operator = CV_NOT;
                                                StringWalk ++;
                                              }
                                              else
                                                _CriteriaVar[Cnt].Operator = CV_LESS;
                                              Error = FALSE;
                                              break;
                            case CO_NOT     : if (Error = (*(StringWalk ++) != CO_EQUAL))
                                                sprintf (Message, "%s: Unknown operator !%c", Type, *(-- StringWalk));
                                              _CriteriaVar[Cnt].Operator = CV_NOT;
                                              break;
                            case CO_EQUAL   : _CriteriaVar[Cnt].Operator = CV_EQUAL;
                                              Error = FALSE;
                                              break;
                            default         : Error = TRUE;
                                              sprintf (Message, "%s: Unknown operator %c", Type, (*(-- StringWalk)));
                                              break;
                          }
                          if (!Error)
                            *((word *)_CriteriaVar[Cnt].Value) = 0;
        }
        ValWalk = (char *)_CriteriaVar[Cnt].Value;
        FirstBoolChar = TRUE;
        StringLength = 0;
        while (!Error && *StringWalk && *StringWalk != CR_SEP)
          switch (_CriteriaVar[Cnt].Type)
          {
            case CT_FBOOL :
            case CT_BOOL  : if (FirstBoolChar)
                            {
                              FirstBoolChar = FALSE;
                              if (toupper (*StringWalk) != CB_TRUE && toupper (*StringWalk) != CB_FALSE)
                              {
                                Error = TRUE;
                                sprintf (Message, "%s is type bool, unknown value %c", Type, *StringWalk);
                              }
                              else
                                *ValWalk = toupper (*(StringWalk));
                            }
                            else
                            {
                              Error = TRUE;
                              sprintf (Message, "%s is type bool, only %c and %c allowed", Type, CB_TRUE, CB_FALSE);
                            }
                            StringWalk ++;
                            break;
            case CT_INUM  :
            case CT_BNUM  :
            case CT_NUM   : if (!isdigit (*StringWalk))
                            {
                              Error = TRUE;
                              sprintf (Message, "%s: Invalid token %c", Type, *StringWalk);
                            }
                            else 
                              *((word *)_CriteriaVar[Cnt].Value) = *((word *)_CriteriaVar[Cnt].Value) * 10 +
                                                                   (*(StringWalk ++) - '0');
                            break;
            case CT_ALPHA : if (++ StringLength <= _CriteriaVar[Cnt].MaxLength)
                              switch (*StringWalk)
                              {
                                case CR_ESCAPE : if (!*(++ StringWalk))
                                                 {
                                                   Error = TRUE;
                                                   strcpy (Message, "Unexpected end of string");
                                                 }
                                                 *(ValWalk ++) = *StringWalk;
                                                 break;
                                case CR_WILD1  : *(ValWalk ++) = CV_WILD1;
                                                 break;
                                case CR_WILD2  : *(ValWalk ++) = CV_WILD2;
                                                 break;
                                default        : *(ValWalk ++) = toupper (*StringWalk);
                              }
                            StringWalk ++;
                            break;
            case CT_PATH  : if (++ StringLength <= _CriteriaVar[Cnt].MaxLength)
                              *(ValWalk ++) = toupper (*StringWalk);
                            StringWalk ++;
          }
      if ((_CriteriaVar[Cnt].Type == CT_ALPHA || _CriteriaVar[Cnt].Type == CT_PATH) && !Error)
        *ValWalk = '\0';
      _CriteriaVar[Cnt].Set = TRUE;
    }
    if (*StringWalk == CR_SEP)                                                               /* Step over the seperator character */
      StringWalk ++;
    if (!Handled)
      sprintf (Message, "Unknown identifier %s", Type);
  }
  if (Error)
    ErrorRequester (-1, -1, NULL, NULL, NULL, "Invalid criterion string!\n%s", Message);
  return (!Error);
}

bool ReselectAnEntry (struct ComboInfo *Window, comboent Num, struct DBaseEntry_s *TestEntry, bool UpdateScreen)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the combo window that contains the entry, `Num' holds the entry number.                             */
/*         If `TestEntry' is not NULL, this entry is copied to _SelectEntry first.                                                */
/*         If `UpdateScreen' is TRUE, the SWindow viewport is updated on screen as well.                                          */
/* Post  : The requested entry has been tested against all criteria; the `Selected' field has been adjusted accordingly.          */
/*         The return value is TRUE if the `Selected' field changed.                                                              */
/* Import: _MatchString.                                                                                                          */
/**********************************************************************************************************************************/

{
  struct ComboEntry NEntry;
  short    register RCnt;
  word              IntValue1;
  word              IntValue2;
  bool              Fit = TRUE;
  byte              OldSelected;

  GetComboEntry (Window, &NEntry, Num);
  OldSelected = NEntry.Selected;
  if (_SelectAction == CA_SET)
  {
    if (OldSelected & SELECTBIT)                                                               /* Previously selected (matched) ? */
      NumMatches --;
    NEntry.Selected &= ~SELECTBIT;
  }
  if (TestEntry != NULL)                                                                                  /* An entry was given ? */
    memcpy (&_SelectEntry, TestEntry, sizeof (struct DBaseEntry_s));
  if (CriterionEmpty)                                                                                    /* No criterion set up ? */
    Fit = FALSE;                                                                                      /* Then we have no matches! */
  else
    for (RCnt = 0 ; Fit && RCnt < MAXCRIT ; RCnt ++)
      if (_CriteriaVar[RCnt].Set)                                                                           /* Criterion is set ? */
        if (RCnt == 8)                                                                       /* Test 'PlayBit' seperately by hand */
        {
          if (*((char *)_CriteriaVar[RCnt].Value) == CB_TRUE)
            Fit = NEntry.Selected & PLAYBIT ? FALSE : TRUE;
          else
            Fit = NEntry.Selected & PLAYBIT ? TRUE : FALSE;
        }
        else if (_CriteriaVar[RCnt].Type != CT_INUM && _CriteriaVar[RCnt].Type != CT_NUM && _CriteriaVar[RCnt].Type != CT_BOOL &&
                 !*((char *)_CriteriaVar[RCnt].Link) &&                                           /* Field is empty in database ? */
                 !(_CriteriaVar[RCnt].Operator & CV_NOT))                                                   /* Special: match NOT */
          Fit = FALSE;                                                                    /* Then the entry simply doesn't match! */
        else
          switch (_CriteriaVar[RCnt].Type)
          {
            case CT_FBOOL :
            case CT_BOOL  : if (*((char *)_CriteriaVar[RCnt].Value) == CB_TRUE)
                              Fit = (*((char *)_CriteriaVar[RCnt].Link) == TRUE) ? TRUE : FALSE;
                            else
                              Fit = (*((char *)_CriteriaVar[RCnt].Link) == TRUE) ? FALSE : TRUE;
                            break;
            case CT_ALPHA : Fit = _MatchString ((char *)_CriteriaVar[RCnt].Link, ((char *)_CriteriaVar[RCnt].Value));
                            if (!strcmp (_CriteriaVar[RCnt].Name, "BY") && !Fit)     /* SPECIAL: either Publisher or Author match */
                              Fit = _MatchString ((char *)_SelectEntry.Publisher, ((char *)_CriteriaVar[RCnt].Value));
                            if (_CriteriaVar[RCnt].Operator & CV_NOT)
                              Fit = !Fit;
                            break;
            case CT_PATH  : Fit = MatchFileToMask ((char *)_CriteriaVar[RCnt].Link, ((char *)_CriteriaVar[RCnt].Value));
                            break;
            case CT_NUM   :
            case CT_BNUM  :
            case CT_INUM  : IntValue2 = *((word *)_CriteriaVar[RCnt].Value);
                            if (_CriteriaVar[RCnt].Type == CT_NUM)
                              IntValue1 = atoi (((char *)_CriteriaVar[RCnt].Link));
                            else if (_CriteriaVar[RCnt].Type == CT_BNUM)
                              IntValue1 = *((byte *)_CriteriaVar[RCnt].Link);
                            else
                              IntValue1 = *((word *)_CriteriaVar[RCnt].Link);
                            if (IntValue1 == 0)                                                             /* Field contains 0 ? */
                              if (_CriteriaVar[RCnt].Operator != CV_EQUAL || IntValue2 != 0)         /* Are we looking for that ? */
                              {                                                                                   /* Nope, no fit */
                                Fit = FALSE;
                                break;
                              }
                            switch (_CriteriaVar[RCnt].Operator)
                            {
                              case CV_GREATER            : Fit = (IntValue1 > IntValue2 ? TRUE : FALSE); break;
                              case CV_LESS               : Fit = (IntValue1 < IntValue2 ? TRUE : FALSE); break;
                              case CV_NOT                : Fit = (IntValue1 != IntValue2 ? TRUE : FALSE); break;
                              case CV_EQUAL              : Fit = (IntValue1 == IntValue2 ? TRUE : FALSE); break;
                              case CV_GREATER | CV_EQUAL : Fit = (IntValue1 >= IntValue2 ? TRUE : FALSE); break;
                              case CV_LESS | CV_EQUAL    : Fit = (IntValue1 <= IntValue2 ? TRUE : FALSE); break;
                              default                    : Fit = FALSE;
                            }
          }
  if (Fit)
  {
    switch (_SelectAction)
    {
      case CA_SET : NEntry.Selected |= SELECTBIT;
                    NumMatches ++;
                    break;
      case CA_ADD : if (!(OldSelected & SELECTBIT))                                          /* This entry was not selected yet ? */
                    {
                      NEntry.Selected |= SELECTBIT;                                                              /* Select it now */
                      NumMatches ++;
                    }
                    break;
      case CA_DEL : if (OldSelected & SELECTBIT)                                              /* This entry was selected before ? */
                    {
                      NEntry.Selected &= ~SELECTBIT;                                                           /* Unselect it now */
                      NumMatches --;
                    }
                    break;
    }
  }
  PutComboEntry (Window, &NEntry, Num, UpdateScreen);
  return (NEntry.Selected != OldSelected);
}

bool ReselectEntries (struct ComboInfo *Window)

/**********************************************************************************************************************************/
/* Pre   : `Window' points to the combo window which entry table should be re-selected to a new critera set.                      */
/* Post  : All entries have been re-selected according to the criterion.                                                          */
/*         The return value is TRUE if (at least) one entry has a changed `Selected' field.                                       */
/* Import: SelectAnEntry.                                                                                                         */
/**********************************************************************************************************************************/

{
  comboent register ECnt;
  bool              Changed;

  for (ECnt = 0 ; ECnt <= DBase.NumEntries ; ECnt ++)
  {
    GetDBEntry (&_SelectEntry, DBLEN, ECnt);
    if (ReselectAnEntry (Window, ECnt, NULL, FALSE))
      Changed = TRUE;
  }
  return (Changed);
}
