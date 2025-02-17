/* ---------------------------------------------------------------------------


   This file is part of the ``node'' package of NuSMV version 2.
   Copyright (C) 1998-2001 by CMU and FBK-irst.

   NuSMV version 2 is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   NuSMV version 2 is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

   For more information on NuSMV see <http://nusmv.fbk.eu>
   or email to <nusmv-users@fbk.eu>.
   Please report bugs to <nusmv-users@fbk.eu>.

   To contact the NuSMV development board, email to <nusmv@fbk.eu>. 

-----------------------------------------------------------------------------*/

/*!
  \author Marco Roveri
  \brief Pretty printing of formulas represented using node struct.

  This file conatins the code to perform pretty printing
   of a formula represented with a node struct.

*/


#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/node/nodeInt.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/node/printers/MasterPrinter.h"

#if NUSMV_HAVE_STRING_H
#include <string.h> /* for strdup */
#else
# ifndef strdup
char* strdup(const char*); /* forward declaration */
# endif
#endif

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
