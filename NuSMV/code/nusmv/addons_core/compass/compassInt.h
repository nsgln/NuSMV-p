/* ---------------------------------------------------------------------------


  This file is part of the ``compass'' package of NuSMV version 2. 
  Copyright (C) 2008 by FBK-irst. 

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
  \author Roberto Cavada
  \brief The private internal header file of the
  <tt>compass</tt> addon.

  The <tt>compass</tt> implementation package

*/


#ifndef __NUSMV_ADDONS_CORE_COMPASS_COMPASS_INT_H__
#define __NUSMV_ADDONS_CORE_COMPASS_COMPASS_INT_H__

#include "NuSMV-2.6.0/NuSMV/code/nusmv/addons_core/compass/compass.h"

#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/dd/dd.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/opt/opt.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/compile.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/type_checking/TypeChecker.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/NodeList.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/enc/bdd/BddEnc.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern cmp_struct_ptr cmps;


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

void Compass_check_prob_list(TypeChecker_ptr tc, NodeList_ptr list);
void Compass_check_ap_list(TypeChecker_ptr tc, NodeList_ptr list);
add_ptr Compass_process_prob_list(BddEnc_ptr enc, NodeList_ptr list, add_ptr r);


#endif /* __COMPASS_H__ */
