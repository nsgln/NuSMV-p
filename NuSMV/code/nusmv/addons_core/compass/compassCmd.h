/* ---------------------------------------------------------------------------


  This file is part of the ``addons_core.compass'' package.
  %COPYRIGHT%
  

-----------------------------------------------------------------------------*/

/*!
  \author Michele Dorigatti
  \brief Module header file for shell commands

  \todo: Missing description

*/


#ifndef __NUSMV_ADDONS_CORE_COMPASS_COMPASS_CMD_H__
#define __NUSMV_ADDONS_CORE_COMPASS_COMPASS_CMD_H__

#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/cinit/NuSMVEnv.h"

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
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/*!
  \brief Initializes the commands provided by this package

  
*/
void Compass_init_cmd(NuSMVEnv_ptr env);

/*!
  \brief Initializes the commands provided by this package

  
*/
void Compass_Cmd_quit(NuSMVEnv_ptr env);

/**AutomaticEnd***************************************************************/

#endif /* __NUSMV_ADDONS_CORE_COMPASS_COMPASS_CMD_H__ */
