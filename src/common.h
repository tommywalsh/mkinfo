/*
    Common definitions needed across both authoring and unauthoring tools.
*/
/*
   Copyright (C) 2010 Lawrence D'Oliveiro <ldo@geek-central.gen.nz>.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
   USA
 */

#ifndef __DVDAUTHOR_COMMON_H_
#define __DVDAUTHOR_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum /* attributes of cell */
/* A "cell" is a  grouping of one or more VOBUs, which might have a single VM command attached.
    A "program" is a grouping of one or more cells; the significance is that skipping using
    the next/prev buttons on the DVD player remote is done in units of programs. Also with
    multiple interleaved angles, each angle goes in its own cell(s), but they must be within
    the same program.
    A program can also be marked as a "chapter" (aka "Part Of Title", "PTT"), which means it
    can be directly referenced via an entry in the VTS_PTT_SRPT table, which allows it
    to be linked from outside the current PGC.
    And finally, one or more programs are grouped into a "program chain" (PGC). This can
    have a VM command sequence to be executed at the start of the PGC, and another sequence
    to be executed at the end. It also specifies the actual audio and subpicture stream IDs
    corresponding to the stream attributes described in the IFO header for this menu/title.
    Each menu or title may consist of a single PGC, or a sequence of multiple PGCs.
    There is also a special "first play" PGC (FPC), which if present is automatically entered
    when the disc is inserted into the player. */
  {
    CELL_NEITHER = 0, /* neither of following specified */
    CELL_CHAPTER_PROGRAM = 1, /* cell has chapter attribute (implies program attribute) */
    CELL_PROGRAM = 2, /* cell has program attribute only */
  } cell_chapter_types;

#ifdef __cplusplus
}
#endif

#endif
