/* ---------------------------------------------------------------------------


This file is part of the ``utils'' package of NuSMV version 2.
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
  \brief Error routines

  This file conatins the error routines. This file is
partitioned in two parts. The first contains general routines, the
second contains specific error routines.

*/


#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/StreamMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/node/printers/MasterPrinter.h"
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>

#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/error.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/utils.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/ustring.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/node/node.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/opt/opt.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/mc/mc.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/cinit/cinit.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/utils_io.h" /* for indent_node */
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/compile.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/parser/symbols.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/parser/psl/pslNode.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/bmc/bmcUtils.h" /* for Bmc_Utils_ConvertLoopFromInteger */
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/ErrorMgr.h"

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void error_out_of_memory(size_t size);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

void Error_init(NuSMVEnv_ptr env)
{
  ErrorMgr_ptr error_manager = ErrorMgr_create(env);

  NuSMVEnv_set_value(env, ENV_ERROR_MANAGER, error_manager);
}

void Error_quit(NuSMVEnv_ptr env)
{
  ErrorMgr_ptr err =
    ERROR_MGR(NuSMVEnv_remove_value(env, ENV_ERROR_MANAGER));
  ErrorMgr_destroy(err);
}

void init_memory(void)
{
#ifndef USE_MM
  MMoutOfMemory = (void (*)(size_t))error_out_of_memory;
#endif
}

/* TODO[AMa] May there be a better way to do this? */

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
static void error_out_of_memory(size_t size)
{
  (void) fprintf(stderr,
                 "\n##################################################\n");
  (void) fprintf(stderr,
                 "### Out of memory allocating %" PRIuPTR " bytes\n", size);
  (void) fprintf(stderr,
                 "##################################################\n");
  exit(1);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

