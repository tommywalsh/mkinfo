/*
    dvdauthor mainline, interpretation of command-line options and parsing of
    dvdauthor XML control files
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

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>

#include "conffile.h"
#include "dvdauthor.h"
#include "readxml.h"
#include "rgb.h"

int default_video_format = VF_NONE;

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
    dvdauthor_vmgm_gen(0, mg, argv[1]);
  } else {
    fprintf(stderr, "Usage foobar /path/to/dvddirectory\n");
  }
  return 0;
}
