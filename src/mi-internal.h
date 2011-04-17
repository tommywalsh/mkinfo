/*
    Lower-level definitions for building DVD authoring structures
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

#ifndef __DA_INTERNAL_H_
#define __DA_INTERNAL_H_

#include "common.h"


enum {VR_NONE=0,VR_NTSCFILM=1,VR_FILM=2,VR_PAL=3,VR_NTSC=4,VR_30=5,VR_PALFIELD=6,VR_NTSCFIELD=7,VR_60=8}; /* values for videodesc.vframerate */

typedef int64_t pts_t; /* timestamp in units of 90kHz clock */


struct colorinfo { /* a colour table for subpictures */
    int refcount; /* shared structure */
    int color[16];
};

struct videodesc { /* describes a video stream, info from a <video> tag */
    int vmpeg,vres,vformat,vaspect,vwidescreen,vframerate,vcaption;
};

struct audiodesc { /* describes an audio stream, info from an <audio> tag */
    int aformat;
    int aquant;
    int adolby;
    int achannels;
    int alangpresent;
    int asample;
    int aid;
    char lang[2];
    int acontent;
};

struct subpicdesc {
  /* describes a <subpicture> track at the pgcgroup level. This groups one or more
    streams, being alternative representations of the subpicture for different modes. */
    int slangpresent;
    char lang[2];
    int scontent;
    unsigned char idmap[4];
      /* stream ID for each of normal, widescreen, letterbox, and panscan respectively,
        (128 | id) if defined, else 0 */
};
struct cell {
  /* describes a cell within a source video file--generated from <cell> tags & "chapters"
    attributes, or by default if none of these */
    pts_t startpts,endpts;
    cell_chapter_types ischapter; // 1 = chapter&program, 2 = program only, 0 = neither
    int pauselen;
    int scellid; /* ID assigned to cell */
    int ecellid; /* ID assigned to next cell */
    struct vm_statement *commands;
};

struct source { /* describes an input video file, corresponding to a single <vob> directive */
    char *fname; /* name of file */
    int numcells; /* nr elements in cells */
    struct cell *cells; /* array */
    struct vob *vob; /* pointer to created vob */
};
struct audchannel { /* describes information collected from an audio stream */
    struct audpts *audpts; /* array */
    int numaudpts; /* used portion of audpts array */
    int maxaudpts; /* allocated size of audpts array */
    struct audiodesc ad,adwarn; // use for quant and channels
};


struct buttoninfo { /* describes a button within a single subpicture stream */
    int substreamid; /* substream ID as specified to spumux */
    bool autoaction; /* true for auto-action button */
    int x1,y1,x2,y2; /* button bounds */
    char *up,*down,*left,*right; /* names of neighbouring buttons */
    int grp;
};



struct pgc { /* describes a program chain corresponding to a <pgc> directive */
    int numsources; /* length of sources array */
    int numbuttons; /* length of buttons array */
    int numchapters,numprograms,numcells,pauselen;
    int entries; /* bit mask of applicable menu entry types, or, in a titleset, nonzero if non-title PGC */
    struct source **sources; /* array of <vob> directives seen */
    struct button *buttons; /* array */
    struct vm_statement *prei,*posti;
    struct colorinfo *colors;
    struct pgcgroup *pgcgroup; /* back-pointer to containing pgcgroup */
    unsigned char subpmap[32][4];
      /* per-PGC explicit mapping of subpicture streams to alternative display modes for same
        <subpicture> track. Each entry is (128 | id) if present; 127 if not present. */
};

struct pgcgroup { /* common info across a set of menus or a set of titles (<menus> and <titles> directives) */
    vtypes pstype; // 0 - vts, 1 - vtsm, 2 - vmgm
    struct pgc **pgcs; /* array[numpgcs] of pointers */
    int numpgcs;
    int allentries; /* mask of entry types present */
    int numentries; /* number of entry types present */
    struct vobgroup *vg; /* only for pstype==VTYPE_VTS, otherwise shared menugroup.vg field is used */
};

struct langgroup { /* contents of a <menus> directive */
    char lang[3]; /* value of the "lang" attribute */
    struct pgcgroup *pg;
};

struct menugroup { /* contents specific to all collections of <menus> directives, either VTSM or VMGM */
    int numgroups; /* length of groups array */
    struct langgroup *groups; /* array, one entry per <menus> directive */
    struct vobgroup *vg; /* common among all groups[i]->pg elements */
      /* fixme: I don't think this works right with multiple <menus> ,,, </menus> sections,
        which the XML does allow */
};

struct vobgroup { /* contents of a menuset or titleset (<menus> or <titles>) */
    int numaudiotracks; /* nr <audio> tags seen = size of used part of ad/adwarn arrays */
    int numsubpicturetracks; /* nr <subpicture> tags seen = size of used part of sp/spwarn arrays */
    int numvobs; /* size of vobs array */
    int numallpgcs; /* size of allpgcs array */
    struct pgc **allpgcs; /* array of pointers to PGCs */
    struct vob **vobs; /* array of pointers to VOBs */
    struct videodesc vd; /* describes the video stream, one <video> tag only */
    struct videodesc vdwarn; /* for saving attribute value mismatches */
    struct audiodesc ad[8]; /* describes the audio streams, one per <audio> tag */
    struct audiodesc adwarn[8]; /* for saving attribute value mismatches */
    struct subpicdesc sp[32]; /* describes the subpicture streams, one per <subpicture> tag */
    struct subpicdesc spwarn[32]; /* for saving attribute value mismatches */
};

struct vtsdef { /* describes a VTS */
    bool hasmenu;
    int numtitles; /* length of numchapters array */
    int *numchapters; /* number of chapters in each title */
    int numsectors;
    char vtssummary[0x300]; /* copy of VTS attributes (bytes 0x100 onwards of VTS IFO) */
    char vtscat[4]; /* VTS_CAT (copy of bytes 0x22 .. 0x25 of VTS IFO) */
};

// keeps TT_SRPT within 1 sector
#define MAXVTS 170

struct toc_summary {
    struct vtsdef vts[MAXVTS];
    int numvts;
};

struct workset {
    const struct toc_summary *titlesets;
    const struct menugroup *menus;
    const struct pgcgroup *titles;
};

void write4(unsigned char *p,unsigned int v);
void write2(unsigned char *p,unsigned int v);
unsigned int read2(const unsigned char *p);

int getratedenom(const struct vobgroup *va);
void TocGen(const struct workset *ws,const char *fname);

#endif
