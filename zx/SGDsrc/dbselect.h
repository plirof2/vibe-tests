/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBSELECT.H                                                                                                      */
/* Description  : Database select handler                                                                                         */
/* Version type : Module interface                                                                                                */
/* Last changed : 09-05-1999  12:10                                                                                               */
/* Update count : 15                                                                                                              */
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

#ifndef DBSELECT_H_INC
#define DBSELECT_H_INC

#ifdef EXTERN
#undef EXTERN
#endif
#ifdef __DBSELECT_MAIN__
#define EXTERN
#else
#define EXTERN extern
#endif

#define CR_SEP              ','                                                   /* The special characters in a criterion string */
#define CR_WILD1            '*'
#define CV_WILD1            0x01
#define CR_WILD2            '\?'
#define CV_WILD2            0x02
#define CR_ESCAPE           '\\'

#define CO_GREATER          '>'                                                  /* Criterion operators and their bit-wise values */
#define CV_GREATER          0x01
#define CO_LESS             '<'
#define CV_LESS             0x02
#define CO_NOT              '!'
#define CV_NOT              0x04
#define CO_EQUAL            '='
#define CV_EQUAL            0x08

#define CB_TRUE             'Y'                                                        /* Possible values for a CT_BOOL criterion */
#define CB_FALSE            'N'

#define CA_SET              0                                 /* (Default) Select/deselect all entries according to the criterion */
#define CS_SET              '='
#define CA_ADD              1                                                     /* Only add entries, depending on the criterion */
#define CS_ADD              '+'
#define CA_DEL              2                                                /* Only unselect entries, depending on the criterion */
#define CS_DEL              '-'

#define MAXCRIT             18                                                                      /* Number of defined criteria */

/**********************************************************************************************************************************/
/* Define the global variables                                                                                                    */
/**********************************************************************************************************************************/

EXTERN char    Cr_Name[37];                                                        /* Value storage for a parsed criterion string */
EXTERN char    Cr_Publisher[37];
EXTERN char    Cr_Author[101];
EXTERN char    Cr_PubAuth[101];
EXTERN word    Cr_Year;
EXTERN word    Cr_Memory;
EXTERN word    Cr_Players;
EXTERN char    Cr_Together;
EXTERN char    Cr_PCName[37];
EXTERN char    Cr_Type[37];
EXTERN char    Cr_PlayBit;
EXTERN char    Cr_OrigScreen;
EXTERN char    Cr_Origin[2];
EXTERN word    Cr_DiskId;
EXTERN char    Cr_AYSound;
EXTERN char    Cr_MultiLoad;
EXTERN char    Cr_Language[37];
EXTERN word    Cr_Score;

struct CriterionInfo
{
  char     *Name;                                                                                     /* Two-character identifier */
  enum
  {
    CT_NUM,                                                                                                 /* ASCII word numeral */
    CT_BNUM,                                                                                               /* Binary byte numeral */
    CT_INUM,                                                                                               /* Binary word numeral */
    CT_ALPHA,                                                                                                   /* Alphanumerical */
    CT_PATH,                                                                                                          /* Filename */
    CT_BOOL,                                                                                                           /* Boolean */
    CT_FBOOL                                                                                                /* File exist boolean */
  }         Type;
  bool      Set;
  byte      Operator;
  byte      MaxLength;                                                                                    /* Length of Link field */
  void     *Value;
  void     *Link;
};

EXTERN bool        CriterionEmpty;
EXTERN comboent    NumMatches;                                                        /* Number of files that match the criterion */

/**********************************************************************************************************************************/
/* Define the function prototypes                                                                                                 */
/**********************************************************************************************************************************/

bool ParseCriterion  (char *CriterionString);
bool ReselectAnEntry (struct ComboInfo *Window, comboent Num, struct DBaseEntry_s *TestEntry, bool UpdateScreen);
bool ReselectEntries (struct ComboInfo *Window);

#endif
