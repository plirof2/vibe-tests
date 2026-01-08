/* CONVWARA - Convert ZX-Rainbow database to SGD database */
/*            Compile with 'QCL /AS CONVRAIN.C'           */

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

#define DPMAIN     0
#define DPPROGNAME 1
#define DPPRODUCER 2
#define DPPROGTYPE 3
#define DPPROGPATH 4
#define DPLANGUAGE 5
#define DPMAX      6

struct DatabasePaths_s
{
  char Identifier[30];
  char FileName[68];
  word RecordLength;
  bool FoundFileName;
  bool FoundRecordLength;
  int  Fh;
} DatabasePaths[DPMAX] = {{ "Database",             "", 256, FALSE, TRUE,  -1},          /* (Database has fixed recordlength 256) */
                          { "Database-Progs",       "", 0,   FALSE, FALSE, -1},
                          { "Database-Producers",   "", 0,   FALSE, FALSE, -1},
                          { "Database-Progtypes",   "", 0,   FALSE, FALSE, -1},
                          { "Database-Directories", "", 0,   FALSE, FALSE, -1},
                          { "Database-Languages",   "", 0,   FALSE, FALSE, -1}};

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
  char Language[4];
  char Score[4];
} OutputRecord;

byte InputRecord[256];                                                                            /* One record in 'DATABASE.TBL' */
byte AdditionRecord[256];                                                                      /* One record in extension '*.TBL' */

int  FhOut        = -1;

word NumAdded = 0;

void CloseAll (void)
{
  short DPIndex;

  if (FhOut >= 0)
  { close (FhOut); FhOut = -1; }
  fcloseall ();
  for (DPIndex = 0 ; DPIndex < DPMAX ; DPIndex ++)                                                      /* See if we got them all */
    if (DatabasePaths[DPIndex].Fh >= 0)
    { close (DatabasePaths[DPIndex].Fh); DatabasePaths[DPIndex].Fh = -1; }
}

int NextString (FILE *Fp, char *String, bool FindEqual)
{
  char          Ch;
  char register Cnt;
  bool          Ready = FALSE;
  bool          SkipSpaces;

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
      SkipSpaces = isspace (Ch);
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
      do
      {
        fscanf (Fp, "%c", &Ch);
        if (feof (Fp))
        { fprintf (stderr, "Read config: Unexpected end of configuration file\n"); exit (1); }
        if (Ch == '\n')
        { fprintf (stderr, "Read config: Unexpected end of line in configuration file\n"); exit (1); }
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

void ReadConfig (void)
{
  FILE          *Fp;
  char           Item[128];
  char           String[128];
  int            Len;
  int            ReadingType = -1;
  short register DPIndex;
  bool           Done;
  
  if ((Fp = fopen ("RAINBOW.CFG", "r")) == NULL)
    if ((Fp = fopen ("SPECPIC.CFG", "r")) == NULL)
    { perror ("Open ZX-Rainbow configuration file"); exit (1); }
  Len = NextString (Fp, Item, FALSE);
  while (!feof (Fp))
  {
    if (Item[0] == '[' && Item[Len - 1] == ']')                                                             /* Block identifier ? */
    {
      Item[Len - 1] = '\0';
      Done = FALSE;
      for (DPIndex = 0 ; !Done && DPIndex < DPMAX ; DPIndex ++)
        if (!stricmp (Item + 1, DatabasePaths[DPIndex].Identifier))                                /* One of the blocks we need ? */
        {
          ReadingType = DPIndex;
          Done = TRUE;
        }
      if (!Done)
        ReadingType = -1;                                                               /* Signal: we don't care about this block */
    }
    else
    {
      if (Item[Len - 1] == '=')                                                                      /* Find and strip equal sign */
        Item[Len - 1] = '\0';
      else
        if (!NextString (Fp, String, TRUE))
        { fprintf (stderr, "Read config: Missing \'=\' sign in option %s", Item); fclose (Fp); exit (1); }
      Len = NextString (Fp, String, FALSE);                                                                      /* Read argument */
      switch (ReadingType)
      {
        case DPMAIN     : if (!stricmp (Item, "<Mainfile-path>"))
                          {
                            strcpy (DatabasePaths[DPMAIN].FileName, String);
                            DatabasePaths[DPMAIN].FoundFileName = TRUE;
                          }
                          break;
        case DPPROGNAME :
        case DPPRODUCER :
        case DPPROGTYPE :
        case DPPROGPATH :
        case DPLANGUAGE : if (!stricmp (Item, "<Path>"))
                          {
                            strcpy (DatabasePaths[ReadingType].FileName, String);
                            DatabasePaths[ReadingType].FoundFileName = TRUE;
                          }
                          else if (!stricmp (Item, "<Recordsize>"))
                          {
                            DatabasePaths[ReadingType].RecordLength = atoi (String);
                            DatabasePaths[ReadingType].FoundRecordLength = TRUE;
                          }
                          break;
        default         : break;
      }
    }
    Len = NextString (Fp, Item, FALSE);
  }
  fclose (Fp);
  for (DPIndex = 0 ; DPIndex < DPMAX ; DPIndex ++)                                                      /* See if we got them all */
  {
    if (!DatabasePaths[DPIndex].FoundFileName)
    { fprintf (stderr, "Read config: Missing filename for table [%s]\n", DatabasePaths[DPIndex].Identifier); exit (1); }
    if (!DatabasePaths[DPIndex].FoundRecordLength)
    { fprintf (stderr, "Read config: Missing recordsize for table [%s]\n", DatabasePaths[DPIndex].Identifier); exit (1); }
    String[0] = '[';
    strcpy (String + 1, DatabasePaths[DPIndex].Identifier);
    strcat (String, "]");
    printf ("Table %-22s has filename \"%s\" and recordsize %d\n", String, 
            DatabasePaths[DPIndex].FileName, DatabasePaths[DPIndex].RecordLength);
    if ((DatabasePaths[DPIndex].Fh = open (DatabasePaths[DPIndex].FileName, O_BINARY | O_RDONLY)) == -1)
    { fprintf (stderr, "Open table file %s: %s\n", DatabasePaths[DPIndex].FileName, strerror (errno)); CloseAll (); exit (1); }
  }
  putchar ('\n');
}

void InitAll (void)
{
  ReadConfig ();
  if ((FhOut = open ("RAINBASE.DAT", O_BINARY | O_RDWR | O_CREAT | O_APPEND, 0777)) == -1)
  { perror ("Open destination file"); CloseAll (); exit (1); }
  strcpy (OutputRecord.NumPlayers, "1");                                                          /* Setup defaults output fields */
  strcpy (OutputRecord.Together, " ");
  strcpy (OutputRecord.Joysticks, "     ");
  strcpy (OutputRecord.PathIndex, "255");
  strcpy (OutputRecord.FileSize, "      0");
  strcpy (OutputRecord.OrigScreen, " ");
  strcpy (OutputRecord.FloppyId, "   0");
  strcpy (OutputRecord.EmulOver, "-1");
  strcpy (OutputRecord.AYSound, " ");
  strcpy (OutputRecord.MultiLoad, " ");
  strcpy (OutputRecord.Score, "  0");
}

bool FindAdditionalData (int WhichDP, word Index, char *ResultString, short MaxLength)
{
  word ReadSize;

  *ResultString = '\0';
  if (Index == 0)
    return (TRUE);
  if (lseek (DatabasePaths[WhichDP].Fh, (long)(Index - 1) * DatabasePaths[WhichDP].RecordLength, SEEK_SET) == -1)
  { fprintf (stderr, "Seek additional file %s: %s\n", DatabasePaths[WhichDP].FileName, strerror (errno)); CloseAll (); exit (1); }
  ReadSize = read (DatabasePaths[WhichDP].Fh, AdditionRecord, DatabasePaths[WhichDP].RecordLength);
  if (ReadSize != DatabasePaths[WhichDP].RecordLength)
    return (FALSE);                                                                                 /* (Not found if end-of-file) */
  ReadSize = (word)*(AdditionRecord + 2);
  if ((short)ReadSize > MaxLength)
    ReadSize = MaxLength;
  strncpy (ResultString, AdditionRecord + 3, ReadSize);
  ResultString[ReadSize] = '\0';                                                                             /* Cut into a string */
  return (TRUE);
}

bool ReadNextRecord (void)
{
  if (read (DatabasePaths[DPMAIN].Fh, InputRecord, (unsigned)256) == 256)
    return (TRUE);
  else
    return (FALSE);
}

void WriteRecord (void)
{
  char Line[156];

  sprintf (Line, "%-36s %-4s %-36s %-3s %s %s %-5s %-12s %-7s %-3s %-7s %s %s %-4s %-2s %s %s %-3s %s\x0D\x0A",
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
  word ReadSize;
  FILE *PathFile;
  short register Cnt;
  int  FhPaths = -1;

  if ((FhPaths = open (DatabasePaths[DPPROGPATH].FileName, O_BINARY | O_RDONLY)) == -1)
  { perror ("Open paths table"); CloseAll (); exit (1); }
  while (read (FhPaths, AdditionRecord, DatabasePaths[DPPROGPATH].RecordLength) == DatabasePaths[DPPROGPATH].RecordLength)
  {
    ReadSize = (word)*(AdditionRecord + 2);
    if ((short)ReadSize > 67)
      ReadSize = 67;
    strncpy (TestPath, AdditionRecord + 3, ReadSize);
    TestPath[ReadSize] = '\0';
    AddPath ();
  }
  close (FhPaths);
  if (!(PathFile = fopen ("PATHS.INI", "a+t")))
  { perror ("Open paths file"); exit (1); }
  for (Cnt = 0 ; Cnt < NumKnownPaths ; Cnt ++)
    fprintf (PathFile, "GameDir                = %s\n", KnownPaths[Cnt]);
  fclose (PathFile);
}

int main (int argc, char **argv)
{
  char Wheel[] = "|/-\\";
  char ScanString[20];
  short register Cnt;
  bool Done;

  puts ("\nCONVRAIN - Convert ZX-Rainbow database files to SGD database files");
  puts ("v1.3       Written by M. van der Heide");
  puts ("           Copyright (C) 1997-1999 ThunderWare Research Center\n");
  InitAll ();
  while (ReadNextRecord ())
  {
    if ((*InputRecord & 0x1F) != 0x04 && (*InputRecord & 0x1F) != 0x07 && (*InputRecord & 0x1F) != 0x1F)
    {
      NumAdded ++;
      printf ("\rRecord: %05u  %c\r", NumAdded, Wheel[NumAdded & 0x03]);
      fflush (stdout);
      strcpy (OutputRecord.Language, "Eng");
      FindAdditionalData (DPPROGNAME, *((word *)(InputRecord + 29)), OutputRecord.Name, 36);
      FindAdditionalData (DPPRODUCER, *((word *)(InputRecord + 31)), OutputRecord.Publisher, 36);
      FindAdditionalData (DPPROGTYPE, *((word *)(InputRecord + 145)), OutputRecord.Type, 7);
      FindAdditionalData (DPLANGUAGE, *((word *)(InputRecord + 149)), OutputRecord.Language, 3);
      if (*((word *)(InputRecord + 143)) > 9999)
        strcpy (OutputRecord.Year, "1982");
      else if (*((word *)(InputRecord + 143)) == 0)
        strcpy (OutputRecord.Year, "    ");
      else
        sprintf (OutputRecord.Year, "%4d", *((word *)(InputRecord + 143)));
      if (*(InputRecord + 116) == 0x01 || *(InputRecord + 116) == 0x03 || *(InputRecord + 116) == 0x04)
        strcpy (OutputRecord.Memory, "128");
      else
        strcpy (OutputRecord.Memory, " 48");
      if (*((dword *)(InputRecord + 156)) & 0x000003C6)
        strcpy (OutputRecord.Improve, "Y");
      else
        strcpy (OutputRecord.Improve, " ");
      strcpy (OutputRecord.PCName, "            ");
      Cnt = *(InputRecord + 1);
      Done = FALSE;
      while (!Done && -- Cnt > 0)
        if (*(InputRecord + Cnt + 2) == '.')
        {
          strncpy (OutputRecord.PCName, InputRecord + 2, Cnt);
          strncpy (OutputRecord.PCName + 8, InputRecord + Cnt + 2, *(InputRecord + 1) - Cnt);
          OutputRecord.PCName[12] = '\0';
          Done = TRUE;
        }
      if (!Done)
      {
        strncpy (OutputRecord.PCName, InputRecord + 2, *(InputRecord + 1));
        OutputRecord.PCName[*(InputRecord + 1)] = '\0';
      }
      if (*((dword *)(InputRecord + 14)) > 9999999L)
        strcpy (OutputRecord.FileSize, "+");
      else
        sprintf (OutputRecord.FileSize, "%7ld", *((dword *)(InputRecord + 14)));
      WriteRecord ();
    }
  }
  CloseAll ();
  SavePaths ();
  printf ("Number of records processed: %u\n", NumAdded);
  exit (0);
}
