/* ---------------------------------------------------------------------------


  This file is part of the ``trace.exec'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK-irst.

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
  \author Marco Pensallorto
  \brief Private and protected interface of class 'BDDPartialTraceExecutor'

  This file can be included only by derived and friend classes

*/



#ifndef __NUSMV_CORE_TRACE_EXEC_BDDPARTIAL_TRACE_EXECUTOR_PRIVATE_H__
#define __NUSMV_CORE_TRACE_EXEC_BDDPARTIAL_TRACE_EXECUTOR_PRIVATE_H__


#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/trace/exec/BDDPartialTraceExecutor.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/trace/exec/PartialTraceExecutor.h" /* fix this */
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/trace/exec/PartialTraceExecutor_private.h" /* fix this */
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/utils.h"


/*!
  \brief BDDPartialTraceExecutor class definition derived from
               class PartialTraceExecutor

  

  \sa Base class PartialTraceExecutor
*/

typedef struct BDDPartialTraceExecutor_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(PartialTraceExecutor);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  BddFsm_ptr fsm;
  BddEnc_ptr enc;

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

} BDDPartialTraceExecutor;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */

/*!
  \methodof BDDPartialTraceExecutor
  \brief The BDDPartialTraceExecutor class private initializer

  The BDDPartialTraceExecutor class private initializer

  \sa BDDPartialTraceExecutor_create
*/
void bdd_partial_trace_executor_init(BDDPartialTraceExecutor_ptr self, const BddFsm_ptr fsm, BddEnc_ptr enc);

/*!
  \methodof BDDPartialTraceExecutor
  \brief The BDDPartialTraceExecutor class private deinitializer

  The BDDPartialTraceExecutor class private deinitializer

  \sa BDDPartialTraceExecutor_destroy
*/
void bdd_partial_trace_executor_deinit(BDDPartialTraceExecutor_ptr self);



#endif /* __NUSMV_CORE_TRACE_EXEC_BDDPARTIAL_TRACE_EXECUTOR_PRIVATE_H__ */
