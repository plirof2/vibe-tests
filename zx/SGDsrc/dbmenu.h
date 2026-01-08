/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBMENU.H                                                                                                        */
/* Description  : Menu stuff                                                                                                      */
/* Version type : Module interface                                                                                                */
/* Last changed : 06-10-1998  6:20                                                                                                */
/* Update count : 14                                                                                                              */
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

#ifndef DBMENU_H_INC
#define DBMENU_H_INC

#ifdef EXTERN
#undef EXTERN
#endif
#ifdef __DBMENU_MAIN__
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN struct FontInfo *_UsedMenuFont;

void GhostMenuEmptyDbase         (void);
void UnghostMenu                 (void);
void GhostMenuNoMatches          (void);
void UnghostMenuNoMatches        (void);
void GhostMenuGIF                (void);
void UnghostMenuGIF              (void);
void GhostMenuSCR                (void);
void UnghostMenuSCR              (void);
void GhostMenuBackup             (void);
void UnghostMenuBackup           (void);
void GhostMenuFindNew            (void);
void UnghostMenuFindNew          (void);
void GhostMenuCompress           (void);
void GhostMenuFixTape            (void);
void UnghostMenuFixTape          (void);
void GhostMenuCallTAPER          (void);
void UnghostMenuCallTAPER        (void);
void GhostMenuViewBasic          (void);
void UnghostMenuViewBasic        (void);
void GhostMenuViewScreen         (void);
void UnghostMenuViewScreen       (void);
void GhostMenuEditHardwareType   (void);
void UnghostMenuEditHardwareType (void);
void GhostMenuEditArchiveInfo    (void);
void UnghostMenuEditArchiveInfo  (void);
void GhostMenuEditTZXPokes       (void);
void UnghostMenuEditTZXPokes     (void);
void GhostMenuTZXOptions         (void);
void UnghostMenuTZXOptions       (void);
void InstallMenu                 (struct WindowInfo *Window);
void ForceNewDatabaseName        (bool All);

#endif
