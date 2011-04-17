/*
    Higher-level definitions for building DVD authoring structures
*/
/*
 * Copyright (C) 2002 Scott Smith (trckjunky@users.sourceforge.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 */

#ifndef __DVDAUTHOR_H_
#define __DVDAUTHOR_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int default_video_format; /* defined in dvdcli.c */

typedef enum /* type of menu/title */
  { /* note assigned values cannot be changed */
    VTYPE_VTS = 0, /* title in titleset */
    VTYPE_VTSM = 1, /* menu in titleset */
    VTYPE_VMGM = 2, /* menu in VMG */
  } vtypes;

#define COLOR_UNUSED 0x1000000
  /* special value indicating unused colour-table entry, different from all
    possible 24-bit colour values */

/* types fully defined in da-internal.h */
struct menugroup;
struct pgcgroup;
struct pgc;
struct source;
struct cell;



void dvdauthor_vmgm_gen(struct menugroup *menus,const char *fbase);
void menugroup_add_pgcgroup(struct menugroup *mg,const char *lang,struct pgcgroup *pg);
struct menugroup *menugroup_new();
struct pgcgroup *pgcgroup_new(vtypes type);

#ifdef __cplusplus
}
#endif


#endif
