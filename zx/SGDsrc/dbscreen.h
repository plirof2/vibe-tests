/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBSCREEN.H                                                                                                      */
/* Description  : Spectrum screen dump                                                                                            */
/* Version type : Module interface                                                                                                */
/* Last changed : 13-09-1998  9:35                                                                                                */
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

#ifndef DBSCREEN_H_INC
#define DBSCREEN_H_INC

#ifdef EXTERN
#undef EXTERN
#endif
#ifdef __DBSCREEN_MAIN__
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN bool ScreenContainsFlash;                                                    /* TRUE if the screen has flashing characters */
EXTERN bool FlashState;                                                                                     /* Flash state toggle */

bool DecrunchZ80Block          (byte *BufferIn, word BlLength, byte *BufferOut, word OutLength);
void CrunchZ80Block            (byte *BufferIn, word BlLength, byte *BufferOut, word *CrunchedLength);
void SpectrumScreenToGIF       (char *FileName, bool FullName, short StartX, short StartY);
void PutSpectrumScreen         (byte *ScreenBuffer, short HorOff, short VerOff);
void DisplayScreenFromProgram  (struct ComboInfo *Window, comboent EntryNumber, char *FullPath, char *PCName);
void DisplayZ80Screendump      (char *FullPath, char *FileName);
bool PutScreenFromTAP          (struct WindowInfo *Window, short StartX, short StartY, char *FullPath);
bool PutScreenFromTZX          (struct WindowInfo *Window, short StartX, short StartY, char *FullPath);
void ViewTZXCIScreen           (struct ComboInfo *Window, comboent EntryNumber, char *FullPath);
bool PutScreenFromZ80          (struct WindowInfo *Window, short StartX, short StartY, char *FullPath);
void PutScreenFromSNA          (struct WindowInfo *Window, short StartX, short StartY, char *FullPath);
void PutScreenFromSLT          (struct WindowInfo *Window, short StartX, short StartY, char *FullPath);
void PutScreenFromSCR          (struct WindowInfo *Window, short StartX, short StartY, char *FullPath);
void CreateScreenFromTAP       (char *FullPath);
void CreateScreenFromTZX       (char *FullPath);
void CreateScreenFromZ80       (char *FullPath);
void CreateScreenFromSNA       (char *FullPath);
void CreateScreenFromSLT       (char *FullPath);

bool GetMemoryImage            (char *FullPath, byte Bank, byte **Image, word *ImageSize);

void AllocateScreenBuffer      (void);

#endif
