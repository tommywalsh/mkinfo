/*
    interpretation of command-line options
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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include "mkinfo.h"
#include "compat.h"

int default_video_format = VF_NONE;

bool directory_has_ifo_file(const char* dirname)
{
  DIR *d;
  struct dirent *de;
  int len;
  char* buffer;

  len = strlen(dirname);
  if (dirname[len-1] == '/') --len;
  buffer = malloc(len+10);
  strncpy(buffer, dirname, len);
  strcat(buffer, "/VIDEO_TS");

  d = opendir(buffer);
  free(buffer);
  while ((de = readdir(d)) != 0)
    {
      if (strcasecmp(de->d_name, "VIDEO_TS.IFO") == 0) {
        return true;
      }
    }
  return false;
}

int main(int argc, char **argv)
{
  struct pgcgroup *va[1]; /* element 0 for doing menus, 1 for doing titles */
  struct menugroup *mg;

  /* Menus set to some default setup */
  memset(va, 0, sizeof(struct pgcgroup *));
  va[0] = pgcgroup_new(VTYPE_VTSM);
  mg = menugroup_new();
  menugroup_add_pgcgroup(mg, "en", va[0]);

  if (argc==2) {
    fprintf(stdout, "Checking directory %s\n", argv[1]);
    if (directory_has_ifo_file(argv[1])) {
      fprintf(stdout, "VIDEO_TS.IFO already present.  Doing nothing\n");
    } else {
      fprintf(stdout, "Processing directory\n");
      dvdauthor_vmgm_gen(mg, argv[1]);
    }
  } else {
    fprintf(stderr, "Usage foobar /path/to/dvddirectory\n");
  }
  return 0;
}
