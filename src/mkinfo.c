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

#include "config.h"

#include "compat.h"

#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>

#include "mkinfo.h"
#include "mi-internal.h"
#include "dvdvm.h"



// with this enabled, all 16 general purpose registers can be used, but
// prohibits certain convenience features, like multiple commands on a button
bool allowallreg = false;

/* video/audio/subpicture attribute keywords -- note they are all unique to allow
   xxx_ANY attribute setting to work */
static const char * const vmpegdesc[4]={"","mpeg1","mpeg2",0};
static const char * const vresdesc[6]={"","720xfull","704xfull","352xfull","352xhalf",0};
static const char * const vformatdesc[4]={"","ntsc","pal",0};
static const char * const vaspectdesc[4]={"","4:3","16:9",0};
static const char * const vwidescreendesc[5]={"","noletterbox","nopanscan","crop",0};
// taken from mjpegtools, also GPL
const static char * const vratedesc[16] = /* descriptions of frame-rate codes */
  {
    "0x0",
    "24000.0/1001.0 (NTSC 3:2 pulldown converted FILM)",
    "24.0 (NATIVE FILM)",
    "25.0 (PAL/SECAM VIDEO / converted FILM)",
    "30000.0/1001.0 (NTSC VIDEO)",
    "30.0",
    "50.0 (PAL FIELD RATE)",
    "60000.0/1001.0 (NTSC FIELD RATE)",
    "60.0",
    /* additional rates copied from FFmpeg, really just to fill out array */
    "15.0",
    "5.0",
    "10.0",
    "12.0",
    "15.0",
    "0xe",
    "0xf"
  };
static const char * const aformatdesc[6]={"","ac3","mp2","pcm","dts",0};
/* audio formats */
static const char * const aquantdesc[6]={"","16bps","20bps","24bps","drc",0};
static const char * const adolbydesc[3]={"","surround",0};
static const char * const alangdesc[4]={"","nolang","lang",0};
static const char * const achanneldesc[10]={"","1ch","2ch","3ch","4ch","5ch","6ch","7ch","8ch",0};
static const char * const asampledesc[4]={"","48khz","96khz",0};
/* audio sample rates */
static const char * const acontentdesc[6] =
  {"", "normal", "impaired", "comments1", "comments2", 0};
/* audio content types */

const char * const entries[9]={"","","title","root","subtitle","audio","angle","ptt",0};
/* entry menu types */

const char * const pstypes[3]={"VTS","VTSM","VMGM"};

static const int default_colors[16]={ /* default contents for new colour tables */
  COLOR_UNUSED,
  COLOR_UNUSED,
  COLOR_UNUSED,
  COLOR_UNUSED,

  COLOR_UNUSED,
  COLOR_UNUSED,
  COLOR_UNUSED,
  COLOR_UNUSED,

  COLOR_UNUSED,
  COLOR_UNUSED,
  COLOR_UNUSED,
  COLOR_UNUSED,

  COLOR_UNUSED,
  COLOR_UNUSED,
  COLOR_UNUSED,
  COLOR_UNUSED
};

static const int ratedenom[9]={0,90090,90000,90000,90090,90000,90000,90090,90000};
/* corresponding to vratedesc, adjustment to clock units per second
   to convert nominal to actual frame rate */
static const int evenrate[9]={0,    24,   24,   25,   30,   30,   50,   60,   60};
/* corresponding to vratedesc, nominal frame rate */

bool delete_output_dir = false;

static int getratecode(const struct vobgroup *va)
/* returns the frame rate code if specified, else the default. */
{
  /* HACK */
  default_video_format = VF_NTSC;

  if (va->vd.vframerate)
    return va->vd.vframerate;
  else if (va->vd.vformat || default_video_format)
    {
      /* fudge it for calls from menu PGC-generation routines with no video present */
      return (va->vd.vformat || default_video_format) == VF_PAL ? VR_PAL : VR_NTSC;
    }
  else
    {
#if defined(DEFAULT_VIDEO_FORMAT)
#    if DEFAULT_VIDEO_FORMAT == 1
      fprintf(stderr, "WARN: defaulting frame rate to NTSC\n");
      return VR_NTSC;
#    elif DEFAULT_VIDEO_FORMAT == 2
      fprintf(stderr, "WARN: defaulting frame rate to PAL\n");
      return VR_PAL;
#    endif
#else
      fprintf(stderr, "ERR:  cannot determine default frame rate--no video format specified\n");
      exit(1);
#endif
    } /*if*/
} /*getratecode*/

int getratedenom(const struct vobgroup *va)
/* returns the frame rate divider for the frame rate if specified, else the default. */
{
  return ratedenom[getratecode(va)];
} /*getratedenom*/

void write4(unsigned char *p,unsigned int v)
/* inserts a four-byte integer in big-endian format beginning at address p. */
{
  p[0]=(v>>24)&255;
  p[1]=(v>>16)&255;
  p[2]=(v>>8)&255;
  p[3]=v&255;
}

void write2(unsigned char *p,unsigned int v)
/* inserts a two-byte integer in big-endian format beginning at address p. */
{
  p[0]=(v>>8)&255;
  p[1]=v&255;
}

unsigned int read4(const unsigned char *p)
/* extracts a four-byte integer in big-endian format beginning at address p. */
{
  return (p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3];
}

unsigned int read2(const unsigned char *p)
/* extracts a two-byte integer in big-endian format beginning at address p. */
{
  return (p[0]<<8)|p[1];
}

#define ATTRMATCH(a) (attr==0 || attr==(a))
/* does the attribute code match either the specified value or the xxx_ANY value */

static char *makevtsdir(const char *s)
/* returns the full pathname of the VIDEO_TS subdirectory within s if non-NULL,
   else returns NULL. */
{
  static char fbuf[1000];

  if( !s )
    return 0;
  strcpy(fbuf,s);
  strcat(fbuf,"/VIDEO_TS");
  return strdup(fbuf);
}

static void ScanIfo(struct toc_summary *ts, const char *ifo)
/* scans another existing VTS IFO file and puts info about it
   into *ts for inclusion in the VMG. */
{
  static unsigned char buf[2048];
  struct vtsdef *vd;
  size_t dummy;
  int i,first;
  FILE * const h = fopen(ifo, "rb");
  if (!h)
    {
      fprintf(stderr, "ERR:  cannot open %s: %s\n", ifo, strerror(errno));
      exit(1);
    } /*if*/
  if (ts->numvts + 1 >= MAXVTS)
    {
      /* shouldn't occur */
      fprintf(stderr,"ERR:  Too many VTSs\n");
      exit(1);
    } /*if*/
  dummy = fread(buf, 1, 2048, h);
  vd = &ts->vts[ts->numvts]; /* where to put new entry */
  if (read4(buf + 0xc0) != 0) /* start sector of menu VOB */
    vd->hasmenu = true;
  else
    vd->hasmenu = false;
  vd->numsectors = read4(buf + 0xc) + 1; /* last sector of title set (last sector of BUP) */
  memcpy(vd->vtscat, buf + 0x22, 4); /* VTS category */
  memcpy(vd->vtssummary, buf + 0x100, 0x300); /* attributes of streams in VTS and VTSM */
  dummy = fread(buf, 1, 2048, h); // VTS_PTT_SRPT is 2nd sector
  // we only need to read the 1st sector of it because we only need the
  // pgc pointers
  vd->numtitles = read2(buf); /* nr titles */
  vd->numchapters = (int *)malloc(sizeof(int) * vd->numtitles);
  /* array of nr chapters in each title */
  first = 8 + vd->numtitles * 4; /* offset to VTS_PTT for first title */
  for (i = 0; i < vd->numtitles - 1; i++)
    {
      const int n = read4(buf + 12 + i * 4); /* offset to VTS_PTT for next title */
      vd->numchapters[i] = (n - first) / 4; /* difference from previous offset gives nr chapters for this title */
      first = n;
    } /*for*/
  vd->numchapters[i] = (read4(buf + 4) /* end address (last byte of last VTS_PTT) */ + 1 - first) / 4;
  /* nr chapters for last title */
  fclose(h);
  ts->numvts++;
} /*ScanIfo*/

static void forceaddentry(struct pgcgroup *va, int entry)
/* gives the first PGC in va the specified entry type, if this is not present already. */
{
  if (!va->numpgcs && true)
    return;
  if (!(va->allentries & entry)) /* entry not already present */
    {
      if (va->numpgcs)
        va->pgcs[0]->entries |= entry;
      va->allentries |= entry;
      va->numentries++;
    } /*if*/
} /*forceaddentry*/

static void initdir(const char * fbase)
/* creates the top-level DVD-video subdirectories within the output directory,
   if they don't already exist. */
{
  static char realfbase[1000];
  if (fbase)
    {
      if (mkdir(fbase, 0777) && errno != EEXIST)
        {
          fprintf(stderr, "ERR:  cannot create dir %s: %s\n", fbase, strerror(errno));
          exit(1);
        } /*if*/
      snprintf(realfbase, sizeof realfbase, "%s/VIDEO_TS", fbase);
      if (mkdir(realfbase, 0777) && errno != EEXIST)
        {
          fprintf(stderr, "ERR:  cannot create dir %s: %s\n", realfbase, strerror(errno));
          exit(1);
        } /*if*/
      snprintf(realfbase, sizeof realfbase, "%s/AUDIO_TS", fbase);
      if (mkdir(realfbase, 0777) && errno != EEXIST)
        {
          fprintf(stderr, "ERR:  cannot create dir %s: %s\n", realfbase, strerror(errno));
          exit(1);
        } /*if*/
    } /*if*/
  errno = 0;
} /*initdir*/

static struct vobgroup *vobgroup_new()
{
  struct vobgroup *vg=malloc(sizeof(struct vobgroup));
  memset(vg,0,sizeof(struct vobgroup));
  return vg;
}

static void pgcgroup_pushci(struct pgcgroup *p, bool warn)
/* shares colorinfo structures among all pgc elements that have sources
   which were allocated the same vob structures. */
{
  int i, j, ii, jj;
  for (i = 0; i < p->numpgcs; i++)
    {
      if (!p->pgcs[i]->colors)
        continue;
      for (j = 0; j < p->pgcs[i]->numsources; j++)
        {
          const struct vob * const v = p->pgcs[i]->sources[j]->vob;
          for (ii = 0; ii < p->numpgcs; ii++)
            for (jj = 0; jj < p->pgcs[ii]->numsources; jj++)
              if (v == p->pgcs[ii]->sources[jj]->vob)
                {
                  if (!p->pgcs[ii]->colors)
                    {
                      p->pgcs[ii]->colors = p->pgcs[i]->colors;
                      p->pgcs[ii]->colors->refcount++;
                    }
                  else if (p->pgcs[ii]->colors != p->pgcs[i]->colors && warn)
                    {
                      fprintf
                        (
                         stderr,
                         "WARN: Conflict in colormap between PGC %d and %d\n",
                         i, ii
                         );
                    } /*if*/
                } /*if; for; for*/
        } /*for*/
    } /*for*/
} /*pgcgroup_pushci*/

static void pgcgroup_createvobs(struct pgcgroup *p, struct vobgroup *v)
/* appends p->pgcs onto v->allpgcs and builds the struct vob arrays in the vobgroups. */
{
  v->allpgcs = (struct pgc **)realloc(v->allpgcs, (v->numallpgcs + p->numpgcs) * sizeof(struct pgc *));
  memcpy(v->allpgcs + v->numallpgcs, p->pgcs, p->numpgcs * sizeof(struct pgc *));
  v->numallpgcs += p->numpgcs;
  pgcgroup_pushci(p, false);
  pgcgroup_pushci(p, true);
} /*pgcgroup_createvobs*/

static void validatesummary(struct pgcgroup *va)
/* merges the info for all pgcs and validates the collected settings for a pgcgroup. */
{
  int i,allowedentries;
  bool err = false;

  switch (va->pstype)
    {
    case VTYPE_VTSM:
      allowedentries = 0xf8; /* all entry menu types allowed except title */
      break;
    case VTYPE_VMGM:
      allowedentries = 4; /* title entry menu type only */
      break;
    case VTYPE_VTS:
    default:
      allowedentries = 0; /* no entry menu types */
      break;
    } /*switch*/

  for( i=0; i<va->numpgcs; i++ ) {
    struct pgc *p=va->pgcs[i];
    if( !p->posti && p->numsources ) {
      struct source *s=p->sources[p->numsources-1];
      s->cells[s->numcells-1].pauselen=255;
    }
    if( va->allentries & p->entries ) {
      /* this pgc adds entry menus already seen */
      int j;

      for( j=0; j<8; j++ )
        if( va->allentries & p->entries & (1<<j) )
          fprintf(stderr,"ERR:  Multiple definitions for entry %s, 2nd occurance in PGC #%d\n",entries[j],i);
      err = true;
    }
    if (va->pstype != VTYPE_VTS && (p->entries & ~allowedentries) != 0)
      {
        /* disallowed entry menu types present--report them */
        int j;
        for (j = 0; j < 8; j++)
          if (p->entries & (~allowedentries) & (1 << j))
            fprintf
              (
               stderr,
               "ERR:  Entry %s is not allowed for menu type %s\n",
               entries[j],
               pstypes[va->pstype]
               );
        err = true;
      } /*if*/
    va->allentries|=p->entries;
    if( p->numsources ) {
      int j;
      bool first;
      first = true;
      for( j=0; j<p->numsources; j++ ) {
        if( !p->sources[j]->numcells )
          fprintf(stderr,"WARN: Source has no cells (%s) in PGC %d\n",p->sources[j]->fname,i);
        else if( first ) {
          if( p->sources[j]->cells[0].ischapter!=CELL_CHAPTER_PROGRAM ) {
            fprintf(stderr,"WARN: First cell is not marked as a chapter in PGC %d, setting chapter flag\n",i);
            p->sources[j]->cells[0].ischapter=CELL_CHAPTER_PROGRAM;
          }
          first = false;
        }
      }
    }
  }
  for( i=1; i<256; i<<=1 )
    if( va->allentries&i )
      va->numentries++;
  if(err)
    exit(1);
}

struct pgcgroup *pgcgroup_new(vtypes type)
{
  struct pgcgroup *ps=malloc(sizeof(struct pgcgroup));
  memset(ps,0,sizeof(struct pgcgroup));
  ps->pstype=type;
  return ps;
}

struct menugroup *menugroup_new()
{
  struct menugroup *mg=malloc(sizeof(struct menugroup));
  memset(mg,0,sizeof(struct menugroup));
  mg->vg=vobgroup_new();
  return mg;
}

void menugroup_add_pgcgroup(struct menugroup *mg, const char *lang, struct pgcgroup *pg)
{
  mg->groups = (struct langgroup *)realloc(mg->groups, (mg->numgroups + 1) * sizeof(struct langgroup));
  if (strlen(lang) != 2)
    {
      fprintf(stderr, "ERR:  Menu language '%s' is not two letters.\n", lang);
      exit(1);
    } /*if*/
  /* fixme: I don't check if there's already a langgroup with this language code? */
  mg->groups[mg->numgroups].lang[0] = tolower(lang[0]);
  mg->groups[mg->numgroups].lang[1] = tolower(lang[1]);
  mg->groups[mg->numgroups].lang[2] = 0;
  mg->groups[mg->numgroups].pg = pg;
  mg->numgroups++;
} /*menugroup_add_pgcgroup*/

void dvdauthor_vmgm_gen(struct menugroup *menus, const char *fbase)
/* generates a VMG, taking into account all already-generated titlesets. */
{
  DIR *d;
  struct dirent *de;
  char *vtsdir;
  int i;
  static struct toc_summary ts; /* static avoids having to initialize it! */
  static char fbuf[1000];
  static char ifonames[101][14];
  struct workset ws;

  if (!fbase) // can't really make a vmgm without titlesets
    return;
  ws.titlesets = &ts;
  ws.menus = menus;
  ws.titles = 0;
  for (i = 0; i < menus->numgroups; i++)
    {
      validatesummary(menus->groups[i].pg);
      pgcgroup_createvobs(menus->groups[i].pg, menus->vg);
      forceaddentry(menus->groups[i].pg, 4); /* entry=title */
    } /*for*/
  fprintf(stderr, "INFO: dvdauthor creating table of contents\n");
  initdir(fbase);
  // create base entry, if not already existing
  memset(&ts, 0, sizeof(struct toc_summary));
  vtsdir = makevtsdir(fbase);
  for (i = 0; i < 101; i++)
    ifonames[i][0] = 0; /* mark all name entries as unused */
  d = opendir(vtsdir);
  while ((de = readdir(d)) != 0)
    {
      /* look for existing titlesets */
      i = strlen(de->d_name);
      if
        (
         i == 12
         &&
         !strcasecmp(de->d_name + i - 6, "_0.IFO")
         &&
         !strncasecmp(de->d_name, "VTS_", 4)
         /* name is of form VTS_nn_0.IFO */
         )
        {
          i = (de->d_name[4] - '0') * 10 + (de->d_name[5] - '0');
          if (ifonames[i][0]) /* title set nr already seen!? will actually never happen */
            {
              fprintf(stderr, "ERR:  Two different names for the same titleset: %s and %s\n",
                      ifonames[i], de->d_name);
              exit(1);
            } /*if*/
          if (!i)
            {
              fprintf(stderr,"ERR:  Cannot have titleset #0 (%s)\n", de->d_name);
              exit(1);
            } /*if*/
          strcpy(ifonames[i], de->d_name);
        } /*if*/
    } /*while*/
  closedir(d);
  for (i = 1; i <= 99; i++)
    {
      if (!ifonames[i][0])
        break;
      snprintf(fbuf, sizeof fbuf, "%s/%s", vtsdir, ifonames[i]);
      fprintf(stderr, "INFO: Scanning %s\n",fbuf);
      ScanIfo(&ts, fbuf); /* collect info about existing titleset for inclusion in new VMG IFO */
    } /*for*/
  for (; i <= 99; i++) /* look for discontiguously-assigned title nrs (error) */
    if (ifonames[i][0])
      {
        fprintf(stderr, "ERR:  Titleset #%d (%s) does not immediately follow the last titleset\n",i,ifonames[i]);
        exit(1);
      } /*if; for*/
  if (!ts.numvts)
    {
      fprintf(stderr, "ERR:  No .IFO files to process\n");
      exit(1);
    } /*if*/


  /* (re)generate VMG IFO */
  snprintf(fbuf, sizeof fbuf, "%s/VIDEO_TS.IFO", vtsdir);
  TocGen(&ws, fbuf);
  snprintf(fbuf, sizeof fbuf, "%s/VIDEO_TS.BUP", vtsdir); /* same thing again, backup copy */
  TocGen(&ws, fbuf);
  for (i = 0; i < ts.numvts; i++)
    if (ts.vts[i].numchapters)
      free(ts.vts[i].numchapters);
  free(vtsdir);
} /*dvdauthor_vmgm_gen*/

