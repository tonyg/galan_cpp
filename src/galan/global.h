/* gAlan - Graphical Audio Language
 * Copyright (C) 1999 Tony Garnock-Jones
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef Global_H
#define Global_H

#include <glib.h>
#include <gmodule.h>

#define GALAN_BEGIN_NAMESPACE		namespace Galan {
#define GALAN_END_NAMESPACE		}
#define GALAN_USE_NAMESPACE		using namespace Galan;

#define PUBLIC_SYMBOL extern "C" G_MODULE_EXPORT

#include <math.h>
#ifndef M_PI
/* M_PI appears to be missing with the egcs I used to compile on win32 for gAlan 0.1.3 */
# define M_PI		3.14159265358979323846	/* pi */
#endif

/**
 * Used like assert, except for non-fatal errors.
 *
 * @param test a test expression that, if it evaluates to false, will
 * cause the current subroutine to return.
 **/
#define RETURN_UNLESS(test)	G_STMT_START {			\
  if (!(test)) {						\
    g_warning("file %s line %d: failed RETURN_UNLESS `%s'",	\
	      __FILE__,						\
	      __LINE__,						\
	      #test						\
	      );						\
    return;							\
  }; }G_STMT_END

/**
 * Used like assert, except for non-fatal errors.
 *
 * @param test a test expression that, if it evaluates to false, will
 * cause the current subroutine to return 'val'.
 * @param val the value to return if the test fails.
 **/
#define RETURN_VAL_UNLESS(test, val)	G_STMT_START {		\
  if (!(test)) {						\
    g_warning("file %s line %d: failed RETURN_VAL_UNLESS `%s'",	\
	      __FILE__,						\
	      __LINE__,						\
	      #test						\
	      );						\
    return (val);						\
  }; }G_STMT_END

#ifndef NDEBUG
#define IFDEBUG(expr)	expr
#else
#define IFDEBUG(expr)
#endif

#endif
