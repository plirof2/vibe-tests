/* CONVWARA - Convert Warajevo emulator database to SGD database */
/*            Compile with 'QCL /AS CONVWARA.C'                  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include <sys/types.h>

typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned long   dword;
typedef char            bool;
#ifndef TRUE
#define TRUE            1
#define FALSE           0
#endif

#define RECORDLENGTH    149

struct Fields_s
{
  char *FieldName;
  byte  FieldSize;
  byte  FieldIndex;
} Fields[] = {{ "SKRACENO",   10, 0 },
              { "PUNOIME",    30, 0 }, /* Full Name        */
              { "PROIZVODJ",   4, 0 }, /* Producer (index) */
              { "VRSTA",       4, 0 }, /* Type (index)     */
              { "GODISTE",     4, 0 }, /* Year             */
              { "KOMPJUTER",   4, 0 }, /* Memory           */
              { "MATERJEZIK",  4, 0 }, /* Language (index) */
              { "TRAKA",      10, 0 }, /* Tape name        */
              { "POZICIJA",    3, 0 },
              { "DODATNI",    10, 0 },
              { "PROBLEMI",    5, 0 },
              { "SIMULOPC",   50, 0 },
              { "OPIS",       10, 0 }, /* Description      */
              { "PRIORITET",   1, 0 }};
byte NumFields = (sizeof (Fields) / sizeof (struct Fields_s));

char KnownPaths[255][68];
short NumKnownPaths = 0;
char TestPath[68];

struct OutputRecord_s
{
  char Name[37];
  char Year[5];
  char Publisher[37];
  char Memory[5];
  char NumPlayers[2];
  char Together[2];
  char Joysticks[6];
  char PCName[13];
  char Type[8];
  char PathIndex[4];
  char FileSize[8];
  char OrigScreen[2];
  char Improve[2];
  char FloppyId[5];
  char EmulOver[3];
  char AYSound[2];
  char MultiLoad[2];
  char Language[5];
  char Score[4];
} OutputRecord;

char InputRecord[RECORDLENGTH];                                                                   /* One record in 'SOFTWARE.DBF' */
char AdditionRecord[62];                                                 /* (Includes the seperator) One record in 'ADDITION.DBF' */

int  FhMain       = -1;
int  FhAdditional = -1;
int  FhOut        = -1;

word NumAdded = 0;

void CloseAll (void)
{
  if (FhMain >= 0)
  { close (FhMain); FhMain = -1; }
  if (FhAdditional >= 0)
  { close (FhAdditional); FhAdditional = -1; }
  if (FhOut >= 0)
  { close (FhOut); FhOut = -1; }
}

void InitAll (void)
{
  short register FieldCnt;
  short register TypeCnt;
  bool           Match;
  byte           Index;

  if ((FhMain = open ("SOFTWARE.DBF", O_BINARY | O_RDONLY)) == -1)
  { perror ("Open source file"); exit (1); }
  if ((FhAdditional = open ("ADDITION.DBF", O_BINARY | O_RDONLY)) == -1)
  { perror ("Open addition file"); CloseAll (); exit (1); }
  if ((FhOut = open ("WARABASE.DAT", O_BINARY | O_RDWR | O_CREAT | O_APPEND, 0777)) == -1)
  { perror ("Open destination file"); CloseAll (); exit (1); }
  AdditionRecord[61] = '\0';
  if (lseek (FhMain, (long)32, SEEK_SET) == -1)
  { perror ("Seek source file"); CloseAll (); exit (1); }
  for (FieldCnt = 0 ; FieldCnt < NumFields ; FieldCnt ++)
  {
    if (read (FhMain, InputRecord, (unsigned)32) != 32)
    { perror ("Read source file"); CloseAll (); exit (1); }
    for (TypeCnt = 0, Match = FALSE, Index = 0 ; !Match && TypeCnt < NumFields ; TypeCnt ++)
      if (strcmp (InputRecord, Fields[TypeCnt].FieldName))
        Index += Fields[TypeCnt].FieldSize;
      else
      {
        printf ("Found field [%-10s] = %3u\n",Fields[TypeCnt].FieldName,Index);
        Fields[TypeCnt].FieldIndex = Index;
        Match = TRUE;
      }
    if (!Match)
    { fprintf (stderr, "Unknown field: \"%s\"\n", InputRecord); CloseAll (); exit (1); }
  }
  if (read (FhMain, InputRecord, (unsigned)2) != 2)
  { perror ("Read source file"); CloseAll (); exit (1); }
  putchar ('\n');
  strcpy (OutputRecord.NumPlayers, "1");                                                          /* Setup defaults output fields */
  strcpy (OutputRecord.Together, " ");
  strcpy (OutputRecord.Joysticks, "     ");
  strcpy (OutputRecord.PathIndex, "255");
  strcpy (OutputRecord.FileSize, "      0");
  strcpy (OutputRecord.OrigScreen, " ");
  strcpy (OutputRecord.Improve, " ");
  strcpy (OutputRecord.FloppyId, "   0");
  strcpy (OutputRecord.EmulOver, "-1");
  strcpy (OutputRecord.AYSound, " ");
  strcpy (OutputRecord.MultiLoad, " ");
  strcpy (OutputRecord.Language, "Eng");
  strcpy (OutputRecord.Score, "  0");
}

bool FindAdditionalData (char SearchType, char *IdString)
{
  bool  Found = FALSE;
  short StrLength;

  StrLength = strlen (IdString);
  while (StrLength > 0 && IdString[StrLength - 1] == ' ')
    StrLength --;
  if (StrLength == 0)
    return (FALSE);
  StrLength = strlen (IdString);
  if (lseek (FhAdditional, (long)66, SEEK_SET) == -1)
  { perror ("Seek addition file"); CloseAll (); exit (1); }
  while (!Found)
  {
    if (read (FhAdditional, AdditionRecord, (unsigned)61) != 61)
      return (FALSE);                                                                               /* (Not found if end-of-file) */
    if (SearchType == 0)                                                                                  /* Looking for a file ? */
      Found = ((AdditionRecord[1] == 'T' || AdditionRecord[1] == 'S' || AdditionRecord[1] == 'M') &&
               !strncmp (AdditionRecord + 3, IdString, StrLength));
    else
      Found = (AdditionRecord[1] == SearchType && !strncmp (AdditionRecord + 2, IdString, StrLength));                 /* Got it! */
  }
  return (TRUE);
}

bool ReadNextRecord (void)
{
  unsigned ReadSize;

  if ((ReadSize = read (FhMain, InputRecord, (unsigned)1)) == 1)                                                /* Read seperator */
    if (InputRecord[0] == 0x1A)                                                                           /* End-of-file marker ? */
      return (FALSE);
  if ((ReadSize = read (FhMain, InputRecord, (unsigned)RECORDLENGTH)) == RECORDLENGTH)
    return (TRUE);
  fprintf (stderr, "WARNING: Unexpected end-of-file!\n");
  return (FALSE);
}

void WriteRecord (void)
{
  char Line[156];

  sprintf (Line, "%-36s %-4s %-36s %-3s %s %s %-5s %-12s %-7s %-3s %-7s %s %s %-4s %-2s %s %s %s %s\x0D\x0A",
           OutputRecord.Name, OutputRecord.Year, OutputRecord.Publisher, OutputRecord.Memory, OutputRecord.NumPlayers,
           OutputRecord.Together, OutputRecord.Joysticks, OutputRecord.PCName, OutputRecord.Type, OutputRecord.PathIndex,
           OutputRecord.FileSize, OutputRecord.OrigScreen, OutputRecord.Improve, OutputRecord.FloppyId, OutputRecord.EmulOver,
           OutputRecord.AYSound, OutputRecord.MultiLoad, OutputRecord.Language, OutputRecord.Score);
  if (write (FhOut, Line, 151) == -1)
  { perror ("Write to destination file"); CloseAll (); exit (1); }
}

void AddPath (void)
{
  short register Cnt;

  strupr (TestPath);
  for (Cnt = 0 ; Cnt < NumKnownPaths ; Cnt ++)
    if (!strcmp (KnownPaths[Cnt], TestPath))
      return;
  if (NumKnownPaths == 254)
  { fprintf (stderr, "Too many different paths found - max is 254\n"); CloseAll (); exit (1); }
  strcpy (KnownPaths[NumKnownPaths ++], TestPath);
}

void SavePaths (void)
{
  FILE *PathFile;
  short register Cnt;

  if (!(PathFile = fopen ("PATHS.INI", "a+t")))
  { perror ("Open paths file"); exit (1); }
  for (Cnt = 0 ; Cnt < NumKnownPaths ; Cnt ++)
    fprintf (PathFile, "GameDir                = %s\n", KnownPaths[Cnt]);
  fclose (PathFile);
}

int main (void)
{
  char Wheel[] = "|/-\\";
  char ScanString[20];
  short register Cnt;

  puts ("\nCONVWARA - Convert Warajevo database files to SGD database files");
  puts ("v1.2       Written by M. van der Heide");
  puts ("           Copyright (C) 1997-1999 ThunderWare Research Center\n");
  InitAll ();
  while (ReadNextRecord ())
  {
    NumAdded ++;
    printf ("\rRecord: %05u  %c\r", NumAdded, Wheel[NumAdded & 0x03]);
    fflush (stdout);
    strncpy (OutputRecord.Name, InputRecord + Fields[1].FieldIndex, 30); OutputRecord.Name[30] = '\0';
    strncpy (OutputRecord.Year, InputRecord + Fields[4].FieldIndex, 4); OutputRecord.Year[4] = '\0';
    strncpy (OutputRecord.Publisher, InputRecord + Fields[2].FieldIndex, 4); OutputRecord.Publisher[4] = '\0';
    strncpy (OutputRecord.Memory, InputRecord + Fields[5].FieldIndex, 4); OutputRecord.Memory[4] = '\0';
    strncpy (OutputRecord.PCName, InputRecord + Fields[7].FieldIndex, 10); OutputRecord.PCName[10] = '\0';
    strncpy (OutputRecord.Type, InputRecord + Fields[3].FieldIndex, 4); OutputRecord.Type[4] = '\0';
    strncpy (OutputRecord.Language, InputRecord + Fields[6].FieldIndex, 4); OutputRecord.Language[4] = '\0';
    if (FindAdditionalData ('P', OutputRecord.Publisher))
    {
      strncpy (OutputRecord.Publisher, AdditionRecord + 6, 36);
      OutputRecord.Publisher[36] = '\0';
    }
    if (FindAdditionalData ('V', OutputRecord.Type))
    {
      strncpy (OutputRecord.Type, AdditionRecord + 6, 7);
      OutputRecord.Type[7] = '\0';
    }
    if (FindAdditionalData ('J', OutputRecord.Language))
    {
      strncpy (OutputRecord.Language, AdditionRecord + 6, 3);
      OutputRecord.Language[3] = '\0';
    }
    else
      strcpy (OutputRecord.Language, "Eng");
    if (FindAdditionalData (0, OutputRecord.PCName + 1))
    {
      strncpy (OutputRecord.PCName, AdditionRecord + 3, 8);
      strcpy (OutputRecord.PCName + 8, ".");
      switch (AdditionRecord[1])
      {
        case 'T' : strcpy (OutputRecord.PCName + 8, ".TAP"); break;
        case 'S' : strcpy (OutputRecord.PCName + 8, ".Z80"); break;
        case 'M' : strcpy (OutputRecord.PCName + 8, ".MDR"); break;
        default  : strcpy (OutputRecord.PCName + 8, "."); break;
      }
      strupr (OutputRecord.PCName);
      strncpy (TestPath, AdditionRecord + 12, 35);
      Cnt = 34;
      while (Cnt > 0 && TestPath[Cnt] == ' ')
        TestPath[Cnt --] = '\0';
      if (Cnt > 0)
        AddPath ();
    }
    else
      OutputRecord.PCName[0]= '\0';
    if (!strcmp (OutputRecord.Memory, "16  "))
      strcpy (OutputRecord.Memory, " 16");
    else if (!strcmp (OutputRecord.Memory, "128 "))
      strcpy (OutputRecord.Memory, "128");
    else if (!strcmp (OutputRecord.Memory, "+2  "))
      strcpy (OutputRecord.Memory, " +2");
    else if (!strcmp (OutputRecord.Memory, "+3  "))
      strcpy (OutputRecord.Memory, " +3");
    else
      strcpy (OutputRecord.Memory, " 48");
    WriteRecord ();
  }
  CloseAll ();
  SavePaths ();
  printf ("Number of records processed: %u\n", NumAdded);
  exit (0);
}
