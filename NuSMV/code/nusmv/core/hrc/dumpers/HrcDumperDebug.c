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
  \brief Implementation of class 'HrcDumperDebug'

  \todo: Missing description

*/


#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/ErrorMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/hrc/dumpers/HrcDumperDebug.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/hrc/dumpers/HrcDumperDebug_private.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/utils.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/error.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'HrcDumperDebug_private.h' for class 'HrcDumperDebug' definition. */

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

static void hrc_dumper_debug_finalize(Object_ptr object, void* dummy);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

HrcDumperDebug_ptr HrcDumperDebug_create(const NuSMVEnv_ptr env,
                                         FILE* fout)
{
  HrcDumperDebug_ptr self = ALLOC(HrcDumperDebug, 1);
  HRC_DUMPER_DEBUG_CHECK_INSTANCE(self);

  hrc_dumper_debug_init(self, env, fout);
  return self;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void hrc_dumper_debug_init(HrcDumperDebug_ptr self,
                           const NuSMVEnv_ptr env,
                           FILE* fout)
{
  /* base class initialization */
  hrc_dumper_init(HRC_DUMPER(self), env, fout);

  /* members initialization */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = hrc_dumper_debug_finalize;
  OVERRIDE(HrcDumper, dump_snippet) = hrc_dumper_debug_dump_snippet;
}

void hrc_dumper_debug_deinit(HrcDumperDebug_ptr self)
{
  /* members deinitialization */


  /* base class deinitialization */
  hrc_dumper_deinit(HRC_DUMPER(self));
}

void hrc_dumper_debug_dump_snippet(HrcDumper_ptr self,
                                   HrcDumperSnippet snippet,
                                   const HrcDumperInfo* info)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  const char* desc;

  switch (snippet) {
  case HDS_HRC_TOP:
    desc = "HDS_HRC_TOP";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_LIST_MODS:
    desc = "HDS_LIST_MODS";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_MOD:
    desc = "HDS_MOD";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_MOD_NAME:
    desc = "HDS_MOD_NAME";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_LIST_MOD_FORMAL_PARAMS:
    desc = "HDS_LIST_MOD_FORMAL_PARAMS";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_MOD_FORMAL_PARAM:
    desc = "HDS_MOD_FORMAL_PARAM";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_LIST_MOD_INSTANCES:
    desc = "HDS_LIST_MOD_INSTANCES";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_MOD_INSTANCE:
    desc = "HDS_MOD_INSTANCE";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_MOD_INSTANCE_VARNAME:
    desc = "HDS_MOD_INSTANCE_VARNAME";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_MOD_INSTANCE_MODNAME:
    desc = "HDS_MOD_INSTANCE_MODNAME";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_LIST_MOD_INSTANCE_ACTUAL_PARAMS:
    desc = "HDS_LIST_MOD_INSTANCE_ACTUAL_PARAMS";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_MOD_INSTANCE_ACTUAL_PARAM:
    desc = "HDS_MOD_INSTANCE_ACTUAL_PARAM";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_LIST_SYMBOLS:
    desc = "HDS_LIST_SYMBOLS";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_SYMBOL:
    desc = "HDS_SYMBOL";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_LIST_ASSIGNS:
    desc = "HDS_LIST_ASSIGNS";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_ASSIGN_INIT:
    desc = "HDS_ASSIGN_INIT";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_ASSIGN_INVAR:
    desc = "HDS_ASSIGN_INVAR";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_ASSIGN_NEXT:
    desc = "HDS_ASSIGN_NEXT";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_LIST_CONSTRAINTS:
    desc = "HDS_LIST_CONSTRAINTS";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_CONSTRAINT_INIT:
    desc = "HDS_CONSTRAINT_INIT";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_CONSTRAINT_INVAR:
    desc = "HDS_CONSTRAINT_INVAR";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_CONSTRAINT_TRANS:
    desc = "HDS_CONSTRAINT_TRANS";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_LIST_FAIRNESS:
    desc = "HDS_LIST_FAIRNESS";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_JUSTICE:
    desc = "HDS_JUSTICE";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_COMPASSION:
    desc = "HDS_COMPASSION";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_LIST_SPECS:
    desc = "HDS_LIST_SPECS";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_SPEC:
    desc = "HDS_SPEC";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_LIST_COMPILER_INFO:
    desc = "HDS_COMPILER_INFO";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_LIST_SYNTAX_ERRORS:
    desc = "HDS_LIST_SYNTAX_ERRORS";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  case HDS_ERROR:
    desc = "HDS_ERROR";
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("Begin ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR("End ");
    }
    _HRC_DUMP_STR_NL(desc);
    break;

  default:
    ErrorMgr_internal_error(errmgr, "Unexpected node %d", snippet);
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The HrcDumperDebug class virtual finalizer

  Called by the class destructor
*/
static void hrc_dumper_debug_finalize(Object_ptr object, void* dummy)
{
  HrcDumperDebug_ptr self = HRC_DUMPER_DEBUG(object);

  hrc_dumper_debug_deinit(self);
  FREE(self);
}



/**AutomaticEnd***************************************************************/

