/*
 * File:    debug.h
 * Purpose: Debug framework for fusecloop.
 * Copyright (c) 2007 Vitaly "_Vi" Shukela
 *
 * Contact Email: public_vi@tut.by
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef DEBUG_H
#define DEBUG_H

void dbg_init(int argc, char* argv[]);

/*#define DEBUG*/

#ifdef DEBUG
    #include <stdio.h>
    extern FILE *fdb;
    #define bprintf(fmt,args...) { fprintf(fdb,fmt,##args); fflush(fdb); }
    #define bfuncinfo(fmt,args...) \
	bprintf("%s:%d:%s: "fmt"\n",__FILE__,__LINE__,__func__,##args);
#else
    #define bprintf(fmt,args...) {}
    #define bfuncinfo(fmt,args...) {}
#endif

#endif
