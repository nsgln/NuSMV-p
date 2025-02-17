/* ---------------------------------------------------------------------------


  This file is part of the ``parser'' package.
  %COPYRIGHT%
  

-----------------------------------------------------------------------------*/

/*!
  \author Michele Dorigatti
  \brief Package initialization e deinitialization functions

  \todo: Missing description

*/

#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/parser/parserInt.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/parser/parser.h"

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

void Parser_Init(NuSMVEnv_ptr env)
{
  const OptsHandler_ptr opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  /* lib init */
  parser_free_parsed_syntax_errors(env);

  /* options */
  (void)OptsHandler_register_bool_option(opts,
                                   OPT_PARSER_IS_LAX,
                                   false,
                                   true /*public*/);
}

void Parser_Quit(NuSMVEnv_ptr env)
{
  /* lib quit */
  parser_free_parsed_syntax_errors(env);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/



