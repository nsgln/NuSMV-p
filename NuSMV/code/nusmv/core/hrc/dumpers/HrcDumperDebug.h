/* ---------------------------------------------------------------------------


  This file is part of the ``hrc.dumper'' package of NuSMV version 2. 
  Copyright (C) 2011 by FBK-irst. 

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
  \brief Public interface of class 'HrcDumperDebug'

  \todo: Missing description

*/



#ifndef __NUSMV_CORE_HRC_DUMPERS_HRC_DUMPER_DEBUG_H__
#define __NUSMV_CORE_HRC_DUMPERS_HRC_DUMPER_DEBUG_H__


#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/hrc/dumpers/HrcDumper.h" /* fix this */ 
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/utils.h"

/*!
  \struct HrcDumperDebug
  \brief Definition of the public accessor for class HrcDumperDebug

  
*/
typedef struct HrcDumperDebug_TAG*  HrcDumperDebug_ptr;

/*!
  \brief To cast and check instances of class HrcDumperDebug

  These macros must be used respectively to cast and to check
  instances of class HrcDumperDebug
*/
#define HRC_DUMPER_DEBUG(self) \
         ((HrcDumperDebug_ptr) self)

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define HRC_DUMPER_DEBUG_CHECK_INSTANCE(self) \
         (nusmv_assert(HRC_DUMPER_DEBUG(self) != HRC_DUMPER_DEBUG(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/*!
  \methodof HrcDumperDebug
  \brief The HrcDumperDebug class constructor

  The HrcDumperDebug class constructor

  \sa HrcDumper_destroy
*/
HrcDumperDebug_ptr HrcDumperDebug_create(const NuSMVEnv_ptr env,
                                                FILE* fout);


/**AutomaticEnd***************************************************************/



#endif /* __NUSMV_CORE_HRC_DUMPERS_HRC_DUMPER_DEBUG_H__ */
