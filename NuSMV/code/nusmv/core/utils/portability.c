/* ---------------------------------------------------------------------------


  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 2005 by FBK-irst. 

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

  For more information of NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. 

-----------------------------------------------------------------------------*/

/*!
  \author Roberto Cavada
  \brief This module contains functions provided for portability
  reason.

  For some host a few features are not available. The
  idea of this module is to provide those features that are not
  available otherwise. Most of the code within this module is compiled
  and used conditionally, depending on features that must be provided.
  

*/



#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/portability.h"
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
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

#if !NUSMV_HAVE_MALLOC 

/*!
  \brief This function is used instead of malloc when 
  a GNU compatible malloc function is not available.

  

  \se None
*/

void* rpl_malloc(size_t size)
{
  if (size == 0) size = 1;
  return malloc(size);
}
#endif /* if not NUSMV_HAVE_MALLOC */


#if !NUSMV_HAVE_MALLOC 

/*!
  \brief This function is used instead of malloc when 
  a GNU compatible malloc function is not available.

  

  \se None
*/

void* rpl_realloc(void* ptr, size_t size)
{
  if (size == 0) size = 1;
  return realloc(ptr, size);
}
#endif /* if not NUSMV_HAVE_MALLOC */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

