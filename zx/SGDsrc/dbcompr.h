/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : DBCOMPR.H                                                                                                       */
/* Description  : Game compression handler for internet archives                                                                  */
/* Version type : Module interface                                                                                                */
/* Last changed : 19-05-1996  21:00                                                                                               */
/* Update count : 1                                                                                                               */
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

#ifndef DBCOMPR_H_INC
#define DBCOMPR_H_INC

#ifdef EXTERN
#undef EXTERN
#endif
#ifdef __DBCOMPR_MAIN__
#define EXTERN
#else
#define EXTERN extern
#endif

/**********************************************************************************************************************************/
/* Define the global variables                                                                                                    */
/**********************************************************************************************************************************/

/**********************************************************************************************************************************/
/* Define the function prototypes                                                                                                 */
/**********************************************************************************************************************************/

void CompressGames (bool All);

#endif
