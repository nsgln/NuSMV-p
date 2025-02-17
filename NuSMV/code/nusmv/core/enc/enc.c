/* ---------------------------------------------------------------------------


  This file is part of the ``enc'' package of NuSMV version 2.
  Copyright (C) 2003 by FBK-irst.

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
  \brief Package level code of package enc

  \todo: Missing description

*/


#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/StreamMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/Logger.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/node/NodeMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/ErrorMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/enc/enc.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/enc/encInt.h"

#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/dd/VarsHandler.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/parser/symbols.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/compile.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/ucmd.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/error.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/assoc.h"

#if NUSMV_HAVE_STRING_H
#include <string.h>
#endif

/*---------------------------------------------------------------------------*/
/* Macro definition                                                          */
/*---------------------------------------------------------------------------*/

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define  VARS_ORD_STR_INPUTS_BEFORE    "inputs_before"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define  VARS_ORD_STR_INPUTS_AFTER     "inputs_after"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define  VARS_ORD_STR_TOPOLOGICAL      "topological"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define  VARS_ORD_STR_INPUTS_BEFORE_BI "inputs_before_bi"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define  VARS_ORD_STR_INPUTS_AFTER_BI  "inputs_after_bi"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define  VARS_ORD_STR_TOPOLOGICAL_BI   "topological_bi"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define  VARS_ORD_STR_LEXICOGRAPHIC    "lexicographic"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define  VARS_ORD_STR_UNKNOWN

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define BDD_STATIC_ORDER_HEURISTICS_STR_NONE       "none"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define BDD_STATIC_ORDER_HEURISTICS_STR_BASIC      "basic"

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void
enc_construct_bdd_order_statically(NuSMVEnv_ptr env,
                                   FlatHierarchy_ptr flat_hierarchy,
                                   OrdGroups_ptr ord_groups);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

void Enc_init_encodings(NuSMVEnv_ptr env)
{
}

void Enc_init_bool_encoding(NuSMVEnv_ptr env)
{
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  OptsHandler_ptr opts;
  SymbTable_ptr st;
  BoolEnc_ptr bool_enc;

  opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  st = SYMB_TABLE(NuSMVEnv_get_value(env, ENV_SYMB_TABLE));

  if (!NuSMVEnv_has_value(env, ENV_BOOL_ENCODER)) {

    if (opt_verbose_level_gt(opts, 1)) {
      Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
      Logger_log(logger, "\nInitializing global boolean encoding...\n");
    }

    bool_enc = BoolEnc_create(st);
    NuSMVEnv_set_value(env, ENV_BOOL_ENCODER, bool_enc);

    if (opt_verbose_level_gt(opts, 1)) {
      Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
      Logger_log(logger, "Global boolean encoding initialized.\n");
    }
  }
  else {
    StreamMgr_print_output(streams,
            "Warning: Boolean encoding already initialized for this environment\n");
  }
}

void Enc_init_bdd_encoding(NuSMVEnv_ptr env, const char* input_order_file_name)
{
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  VarsHandler_ptr dd_vars_handler;
  SymbTable_ptr st;
  /* This variable has to be initialized with NULL otherwise it may
     take values different from NULL and it is assumed to be
     initialized while indeed it is not */
  OrdGroups_ptr ord_groups = ORD_GROUPS(NULL);
  BoolEnc_ptr bool_enc;
  BddEnc_ptr bdd_enc;
  OptsHandler_ptr opts;
  char * opts_order_file;

  nusmv_assert(NuSMVEnv_has_value(env, ENV_BOOL_ENCODER));
  nusmv_assert(!NuSMVEnv_has_value(env, ENV_BDD_ENCODER));

  dd_vars_handler = VARS_HANDLER(NuSMVEnv_get_value(env, ENV_DD_VARS_HANDLER));
  opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  st = SYMB_TABLE(NuSMVEnv_get_value(env, ENV_SYMB_TABLE));
  bool_enc = BOOL_ENC(NuSMVEnv_get_value(env, ENV_BOOL_ENCODER));

  opts_order_file = get_input_order_file(opts);

  if (!util_is_string_null(input_order_file_name)) {
    if (opt_verbose_level_gt(opts, 0)) {
      Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
      Logger_log(logger, "\nParsing the ordering file '");
      Logger_log(logger, "%s", input_order_file_name);
      Logger_log(logger, "'...\n");
    }

    ord_groups = enc_utils_parse_ordering_file(env,
                                               input_order_file_name,
                                               bool_enc);
  }
  else if (!util_is_string_null(opts_order_file)) {
    if (opt_verbose_level_gt(opts, 0)) {
      Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
      Logger_log(logger, "\nParsing the ordering file '");
      Logger_log(logger, "%s", opts_order_file);
      Logger_log(logger, "'...\n");
    }

    ord_groups = enc_utils_parse_ordering_file(env,
                                               opts_order_file,
                                               bool_enc);

  }
  else {
    /* there is no ordering file => use heuristics to construct the order */
    BddSohEnum bse = get_bdd_static_order_heuristics(opts);

    ord_groups = OrdGroups_create();

    if (bse != BDD_STATIC_ORDER_HEURISTICS_NONE) {

      if (opt_verbose_level_gt(opts, 0)) {
        Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
        Logger_log(logger,
                "\nHeuristics \"%s\" is going to be used to create var"
                "ordering statically\n",
                Enc_bdd_static_order_heuristics_to_string(bse));
      }

      if (!NuSMVEnv_has_value(env, ENV_FLAT_HIERARCHY)) {
        StreamMgr_print_error(streams,
                "Warning for addons writers: static BDD variables order heuristics is \n"
                "   set up but flatten hierarchy has not been constructed. Switch off\n"
                "   static order heuristics or provide the flatten hierarchy.\n"
                "   See docs on bdd_static_order_heuristics variable.\n");
      }
      else {
        FlatHierarchy_ptr fh =
          FLAT_HIERARCHY(NuSMVEnv_get_value(env, ENV_FLAT_HIERARCHY));
        enc_construct_bdd_order_statically(env, fh, ord_groups);
      }
    }
    else {
      /* heuristics are switched off => do nothing */
    }
  }

  if (opt_verbose_level_gt(opts, 1)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger, "\nInitializing global BDD encoding...\n");
  }

  /* ord_groups will belong to the environment's bdd_enc */
  bdd_enc = BddEnc_create(st, bool_enc, dd_vars_handler, ord_groups);

  NuSMVEnv_set_value(env, ENV_BDD_ENCODER, bdd_enc);

  if (!util_is_string_null(input_order_file_name)) {
    set_input_order_file(opts, (char*)input_order_file_name);
  }

  if (opt_verbose_level_gt(opts, 1)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger, "Global BDD encoding initialized.\n");
  }
}

void Enc_init_be_encoding(NuSMVEnv_ptr env)
{
  SymbTable_ptr st;
  BoolEnc_ptr bool_enc;
  OptsHandler_ptr opts;
  BeEnc_ptr be_enc;

  nusmv_assert(NuSMVEnv_has_value(env, ENV_BOOL_ENCODER));
  nusmv_assert(!NuSMVEnv_has_value(env, ENV_BE_ENCODER));

  opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  st = SYMB_TABLE(NuSMVEnv_get_value(env, ENV_SYMB_TABLE));
  bool_enc = BOOL_ENC(NuSMVEnv_get_value(env, ENV_BOOL_ENCODER));

  if (opt_verbose_level_gt(opts, 1)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger, "\nInitializing global BE encoding...\n");
  }

  be_enc = BeEnc_create(st, bool_enc);

  NuSMVEnv_set_value(env, ENV_BE_ENCODER, be_enc);

  if (opt_verbose_level_gt(opts, 1)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger, "Global BE encoding initialized.\n");
  }
}

void Enc_quit_encodings(NuSMVEnv_ptr env)
{
  if (NuSMVEnv_has_value(env, ENV_BDD_ENCODER)) {
    BddEnc_ptr bdd_enc = BDD_ENC(NuSMVEnv_remove_value(env, ENV_BDD_ENCODER));
    BddEnc_destroy(bdd_enc);
  }

  if (NuSMVEnv_has_value(env, ENV_BE_ENCODER)) {
    BeEnc_ptr be_enc = BE_ENC(NuSMVEnv_remove_value(env, ENV_BE_ENCODER));
    BeEnc_destroy(be_enc);
  }

  if (NuSMVEnv_has_value(env, ENV_BOOL_ENCODER)) {
    BoolEnc_ptr bool_enc = BOOL_ENC(NuSMVEnv_remove_value(env, ENV_BOOL_ENCODER));
    BoolEnc_destroy(bool_enc);
  }
}

void
Enc_append_bit_to_sorted_list(SymbTable_ptr symb_table,
                              NodeList_ptr sorted_list, node_ptr var,
                              node_ptr* sorting_cache)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(symb_table));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  node_ptr cache = *sorting_cache;
  int bit_number;
  ListIter_ptr iter;

  /* a var may be only a true bool var or a bit only */
  nusmv_assert(SymbTable_is_symbol_bool_var(symb_table, var) ||
               SymbType_calculate_type_size(SymbTable_get_var_type(symb_table, var)) == 1);

  /* NOTE about cache:
     In sorted list the vars are collected in groups. For example, there
     is a group corresponding to boolean vars, and there is a group for
     N-th bit vars.

     'cache' is a list. nodetype of every element is a number which
     correspond to the bit number of particular group and car is
     ListIter_ptr pointing to the last element of the group in
     sorted_list.  For boolean vars the number is SHRT_MAX.  If there is not
     element with particular number it means the list does not have such
     vars.

     'cache' are sorted by nodetype from higher to lower values (the
     same as in sorted list).
  */
  if (node_get_type(var) != BIT) bit_number = SHRT_MAX; /* a true boolean var */
  else {
    bit_number = NODE_TO_INT(cdr(var));
    nusmv_assert(bit_number < SHRT_MAX);/* implementation limit is not reached */
  }

  if (cache == Nil || node_get_type(cache) < bit_number) {
    /* add a new group at the beginning of both lists (cache and sorted list) */
    NodeList_prepend(sorted_list, var);
    iter = NodeList_get_first_iter(sorted_list);
    cache = new_node(nodemgr, bit_number, (node_ptr)iter, cache);
    *sorting_cache = cache;
    return;
  }

  /* find smaller group */
  while (cdr(cache) != Nil && node_get_type(cdr(cache)) >= bit_number) {
    cache = cdr(cache);
  }

  /* add the var */
  iter = LIST_ITER(car(cache));
  NodeList_insert_after(sorted_list, iter, var);
  iter = ListIter_get_next(iter);

  if (node_get_type(cache) == bit_number) {
    /* such group already exists => just update the iterator*/
    setcar(cache, (node_ptr)iter);
    return;
  }
  else {
    /* we are at the end of the list or before smaller group => create new group */
    nusmv_assert(node_get_type(cache) > bit_number);
    nusmv_assert(cdr(cache) == Nil || node_get_type(cdr(cache)) < bit_number);

    setcdr(cache, new_node(nodemgr, bit_number, (node_ptr)iter, cdr(cache)));
    return;
  }
}

const char* Enc_vars_ord_to_string(VarsOrdType vot)
{
  switch (vot) {

  case VARS_ORD_INPUTS_BEFORE: return VARS_ORD_STR_INPUTS_BEFORE;
  case VARS_ORD_INPUTS_AFTER: return VARS_ORD_STR_INPUTS_AFTER;
  case VARS_ORD_TOPOLOGICAL: return VARS_ORD_STR_TOPOLOGICAL;
  case VARS_ORD_INPUTS_BEFORE_BI: return VARS_ORD_STR_INPUTS_BEFORE_BI;
  case VARS_ORD_INPUTS_AFTER_BI: return VARS_ORD_STR_INPUTS_AFTER_BI;
  case VARS_ORD_TOPOLOGICAL_BI: return VARS_ORD_STR_TOPOLOGICAL_BI;
  case VARS_ORD_UNKNOWN:
  default:
    error_unreachable_code(); /* no other possible cases */
  }
  return NULL;
}

VarsOrdType Enc_string_to_vars_ord(const char* str, StreamMgr_ptr streams)
{
  if (strcmp(str, VARS_ORD_STR_INPUTS_BEFORE) == 0) {
    return VARS_ORD_INPUTS_BEFORE;
  }
  if (strcmp(str, VARS_ORD_STR_INPUTS_AFTER) == 0) {
    return VARS_ORD_INPUTS_AFTER;
  }
  if (strcmp(str, VARS_ORD_STR_TOPOLOGICAL) == 0) {
    return VARS_ORD_TOPOLOGICAL;
  }
  if (strcmp(str, VARS_ORD_STR_INPUTS_BEFORE_BI) == 0) {
    return VARS_ORD_INPUTS_BEFORE_BI;
  }
  if (strcmp(str, VARS_ORD_STR_INPUTS_AFTER_BI) == 0) {
    return VARS_ORD_INPUTS_AFTER_BI;
  }
  if (strcmp(str, VARS_ORD_STR_TOPOLOGICAL_BI) == 0) {
    return VARS_ORD_TOPOLOGICAL_BI;
  }
  /* deprecated feature */
  if (strcmp(str, VARS_ORD_STR_LEXICOGRAPHIC) == 0) {
    if (STREAM_MGR(NULL) != streams) {
      StreamMgr_print_error(streams,  "Warning: value \"" VARS_ORD_STR_LEXICOGRAPHIC
                            "\" is a deprecated feature. Use \"" VARS_ORD_STR_TOPOLOGICAL
                            "\" instead.\n");
    }
    return VARS_ORD_TOPOLOGICAL;
  }
  return VARS_ORD_UNKNOWN;
}

const char* Enc_get_valid_vars_ord_types()
{
  return
    VARS_ORD_STR_INPUTS_BEFORE ", " \
    VARS_ORD_STR_INPUTS_AFTER  ", " \
    VARS_ORD_STR_TOPOLOGICAL ", "\
    VARS_ORD_STR_INPUTS_BEFORE_BI ", " \
    VARS_ORD_STR_INPUTS_AFTER_BI  ", " \
    VARS_ORD_STR_TOPOLOGICAL_BI;
}

const char* Enc_bdd_static_order_heuristics_to_string(BddSohEnum value)
{
  switch (value) {
  case BDD_STATIC_ORDER_HEURISTICS_NONE: return BDD_STATIC_ORDER_HEURISTICS_STR_NONE;
  case BDD_STATIC_ORDER_HEURISTICS_BASIC: return BDD_STATIC_ORDER_HEURISTICS_STR_BASIC;
  default:
    error_unreachable_code(); /* no other possible cases */
  }
  return NULL;
}

BddSohEnum Enc_string_to_bdd_static_order_heuristics(const char* str)
{
  if (strcmp(str, BDD_STATIC_ORDER_HEURISTICS_STR_NONE) == 0) {
    return BDD_STATIC_ORDER_HEURISTICS_NONE;
  }
  if (strcmp(str, BDD_STATIC_ORDER_HEURISTICS_STR_BASIC) == 0) {
    return BDD_STATIC_ORDER_HEURISTICS_BASIC;
  }
  return BDD_STATIC_ORDER_HEURISTICS_ERROR;
}

const char* Enc_get_valid_bdd_static_order_heuristics()
{
  return
    BDD_STATIC_ORDER_HEURISTICS_STR_NONE ", " \
    BDD_STATIC_ORDER_HEURISTICS_STR_BASIC;
}

int Enc_clean_evaluation_cache(NuSMVEnv_ptr env,
                               BddEnc_ptr enc)
{
  UNUSED_PARAM(env);

  BddEnc_clean_evaluation_cache(enc);

  return 0;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The function is trying to construct a BDD var order using
  heuristics by analyzing the flattened model

  Shell variable vars_order_type is use to infer the initial order
  to begin the analysis. Then analyzing the flattened model and using heuristics
  the order is tried to improve.
  The kind of heuristics to be used is defined by shell variable
  bdd_static_order_heuristics.

  The final order is returned in ord_groups which has to be empty at
  the beginning.
*/
static void enc_construct_bdd_order_statically(NuSMVEnv_ptr env,
                                               FlatHierarchy_ptr flat_hierarchy,
                                               OrdGroups_ptr ord_groups)
{
  /* Discussion about the algorithm.  The only necessary thing to get
     is clusters computed by PredicateExtractor. The actual predicates
     are of no importance.  Unfortunately, it is more difficult to
     compute clusters only, without predicates. See
     PredicateExtractor.h for more into.

     Note: predicate give information only about scalar and word
     vars. Boolean vars interaction are completely ignored.
     A special function would be able to collect all the information */

  const OptsHandler_ptr opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  BoolEnc_ptr bool_enc = BOOL_ENC(NuSMVEnv_get_value(env, ENV_BOOL_ENCODER));
  SymbTable_ptr st = FlatHierarchy_get_symb_table(flat_hierarchy);
  PredicateExtractor_ptr extractor = PredicateExtractor_create(st, true);

  NodeList_ptr original_var_order;
  SymbTableType filter[3];
  SymbTableIter stiter;
  int i = 0;
  boolean isBitInterleaved = false;
  hash_ptr cache = new_assoc();

  nusmv_assert(flat_hierarchy != NULL);

  /* currently only one heuristics is implemented.
     NONE value has to be detected by the invoker of this function. */
  nusmv_assert(get_bdd_static_order_heuristics(opts) ==
               BDD_STATIC_ORDER_HEURISTICS_BASIC);

  /* create the original var order to begin with */
  original_var_order = NodeList_create();
  switch(get_vars_order_type(opts)) {
  case VARS_ORD_INPUTS_BEFORE_BI:
    isBitInterleaved = true;  /* no break here */
  case VARS_ORD_INPUTS_BEFORE:
    filter[0] = STT_INPUT_VAR;
    filter[1] = STT_STATE_VAR | STT_FROZEN_VAR;
    filter[2] = STT_NONE;
    break;

  case VARS_ORD_INPUTS_AFTER_BI:
    isBitInterleaved = true;    /* no break here */
  case VARS_ORD_INPUTS_AFTER:
    filter[0] = STT_STATE_VAR | STT_FROZEN_VAR;
    filter[1] = STT_INPUT_VAR;
    filter[2] = STT_NONE;
    break;

  case VARS_ORD_TOPOLOGICAL_BI:
    isBitInterleaved = true;   /* no break here */
  case VARS_ORD_TOPOLOGICAL:
    filter[0] = STT_VAR;
    filter[1] = STT_NONE;
    break;

  default:
    error_unreachable_code(); /* no other possible cases */
  }

  i = 0;
  while (filter[i] != STT_NONE) {
    SYMB_TABLE_FOREACH(st, stiter, filter[i]) {
      NodeList_append(original_var_order,
                      SymbTable_iter_get_symbol(st, &stiter));
    }
    ++i;
  }

  /* get rid of bits in the list */
  {
    ListIter_ptr iter = NodeList_get_first_iter(original_var_order);
    while (!ListIter_is_end(iter)) {
      node_ptr var = NodeList_get_elem_at(original_var_order, iter);
      ListIter_ptr tmp = iter;
      iter = ListIter_get_next(tmp);
      if (node_get_type(var) == BIT) {
        NodeList_remove_elem_at(original_var_order, tmp);
      }
    }
  }

  /* construct predicates from all the expressions of the hierarchy. */
  PredicateExtractor_compute_preds_from_hierarchy(extractor, flat_hierarchy);

  /* convert vars to bits and add the bits to the list.  The vars in
     cluster are added with bits interleaved.  Bits are added in the
     same order as the vars are met in the model. A set is added at
     the place of its highest var in the list. */
  while (NodeList_get_length(original_var_order) != 0) {
    NodeList_ptr subListVars = NodeList_create();
    NodeList_ptr subListBits = NodeList_create();
    ListIter_ptr subIter;
    node_ptr sorting_cache = Nil;

    node_ptr var = NodeList_get_elem_at(original_var_order,
                                        NodeList_get_first_iter(original_var_order));
    Set_t cluster = PredicateExtractor_get_var_cluster(extractor, var);

    if (NULL == cluster) { /* not grouped var */
      NodeList_append(subListVars, var);

      NodeList_remove_elem_at(original_var_order,
                              NodeList_get_first_iter(original_var_order));

    }
    else { /* a group of vars has to be added */
      /* remove all the vars from the original list and the hash */

      /* It is useless to continue iterating over the NodeList if
         all variables in the cluster have been already
         removed. Keep trace of this with this reverse counter */
      int missing = Set_GiveCardinality(cluster);

      subIter = NodeList_get_first_iter(original_var_order);
      while (!ListIter_is_end(subIter) && (missing > 0)) {
        node_ptr var = NodeList_get_elem_at(original_var_order, subIter);
        ListIter_ptr currIter = subIter;

        subIter = ListIter_get_next(subIter);

        if (Set_IsMember(cluster, (Set_Element_t)var)) {
          node_ptr tmp = NodeList_remove_elem_at(original_var_order, currIter);
          nusmv_assert(var == tmp);
          NodeList_append(subListVars, var);

          /* Found variable, decrease the missing counter */
          --missing;
        }
      }

      nusmv_assert(missing == 0);
    }

    /* Now prepare the cluster for insertion to the order list, i.e.
       divide the order on bits and interleave them if required.
       Here bit interleaving may or may not be used.
    */
    NODE_LIST_FOREACH(subListVars, subIter) {
      var = NodeList_get_elem_at(subListVars, subIter);
      if (SymbTable_is_symbol_bool_var(st, var)) {
        if (isBitInterleaved) {
          /* NB: for boolean interleaving actually may be of no importance? */
          Enc_append_bit_to_sorted_list(st, subListBits, var,
                                        &sorting_cache);
        }
        else NodeList_append(subListBits, var);
      }
      else {

        /* Skip variables that are not booleanizable */
        if (Compile_is_expr_booleanizable(st, var, false, cache)) {
          /* pushes the bits composing the scalar variable.
             Grouping is done by caller function */
          NodeList_ptr bits = BoolEnc_get_var_bits(bool_enc, var);
          ListIter_ptr bit_iter;
          NODE_LIST_FOREACH(bits,bit_iter) {
            node_ptr varBit = NodeList_get_elem_at(bits, bit_iter);
            if (isBitInterleaved) {
              Enc_append_bit_to_sorted_list(st, subListBits, varBit,
                                            &sorting_cache);
            }
            else NodeList_append(subListBits, varBit);
          }
          NodeList_destroy(bits);
        }

      }
    }

    /* append the bits to the global order list */
    NODE_LIST_FOREACH(subListBits, subIter) {
      /* Currently, every bit is inserted into the ordering individually.
         In future we may want to group the bits of one var as it is done
         in bdd_enc_sort_variables_and_groups_according */

      var = NodeList_get_elem_at(subListVars, subIter);

      /* every bit is inserted only once */
      nusmv_assert(-1 == OrdGroups_get_var_group(ord_groups, var));

      OrdGroups_add_variable(ord_groups, var,
                             OrdGroups_create_group(ord_groups));
    }

    free_list(nodemgr, sorting_cache);
    NodeList_destroy(subListBits);
    NodeList_destroy(subListVars);
  }

  NodeList_destroy(original_var_order);

  /* [AT] predicates computed for the hierarchy could be reused but
   * wasted currently.
   */
  PredicateExtractor_destroy(extractor);
  free_assoc(cache);

  /* end of static order computation */
  return;
}
