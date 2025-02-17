/* ---------------------------------------------------------------------------


This file is part of the ``mc'' package of NuSMV version 2.
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
  \brief Witness and Debug generator for Fair CTL models.

  This file contains the code to find counterexamples
execution trace that shows a cause of the problem. Here are
implemented the techniques described in the CMU-CS-94-204 Technical
Report by E. Clarke, O. Grumberg, K. McMillan and X. Zhao.

*/


#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/OStream.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/StreamMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/Logger.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/node/NodeMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/ErrorMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/node/printers/MasterPrinter.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/mc/mc.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/mc/mcInt.h"

#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/ustring.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/parser/symbols.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/assoc.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/fsm/bdd/FairnessList.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/error.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/utils_io.h" /* for indent_node */
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/symb_table/ResolveSymbol.h"
/* Define this to enable trace explain debug */
/* #define EXPLAIN_TRACE_DEBUG */

/*--------------------------------------------------------------------------*/
/* Variable declarations                                                    */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* Static function prototypes                                               */
/*--------------------------------------------------------------------------*/
static node_ptr fairness_explain(BddFsm_ptr fsm, BddEnc_ptr enc,
                                 node_ptr pl, bdd_ptr f,
                                 JusticeList_ptr justice);

static node_ptr explain_recur(BddFsm_ptr, BddEnc_ptr enc,
                              node_ptr, node_ptr, node_ptr);

node_ptr explain_and(BddFsm_ptr, BddEnc_ptr enc,
                     node_ptr, node_ptr, node_ptr);

node_ptr explain_eval(BddFsm_ptr, BddEnc_ptr enc,
                      node_ptr, node_ptr, node_ptr);


static node_ptr
Extend_trace_with_state_input_pair(BddFsm_ptr fsm, BddEnc_ptr enc,
                                   node_ptr path,
                                   bdd_ptr starting_state,
                                   bdd_ptr next_states,
                                   const char * comment);

static node_ptr
Extend_trace_with_states_inputs_pair(BddFsm_ptr fsm, BddEnc_ptr enc,
                                     node_ptr path,
                                     bdd_ptr starting_state,
                                     bdd_ptr next_states,
                                     const char * comment);

#ifdef EXPLAIN_TRACE_DEBUG
static void Check_TraceList_Sanity(BddEnc_ptr enc, node_ptr path,
                                   const char * varname);
#endif

static void
mc_eu_explain_restrict_state_input_to_minterms(BddFsm_ptr fsm,
                                               BddEnc_ptr enc,
                                               node_ptr path,
                                               node_ptr initial_node);

static void
mc_explain_debug_check_not_empty_state(BddFsm_ptr fsm,
                                       BddEnc_ptr enc,
                                       bdd_ptr states,
                                       const char * message);


/*--------------------------------------------------------------------------*/
/* Definition of exported functions                                         */
/*--------------------------------------------------------------------------*/

node_ptr explain(BddFsm_ptr fsm, BddEnc_ptr enc,
                 node_ptr path, node_ptr spec_formula, node_ptr context)
{
  return explain_recur(fsm, enc, path, spec_formula, context);
}

/*--------------------------------------------------------------------------*/
/* Definition of internal functions                                         */
/*--------------------------------------------------------------------------*/

node_ptr ex_explain(BddFsm_ptr fsm, BddEnc_ptr enc, node_ptr path, bdd_ptr f)
{
  bdd_ptr acc, starting_state, image;
  DDMgr_ptr dd_manager = BddEnc_get_dd_manager(enc);
  NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));


  if (path == Nil) return(path);

  starting_state = bdd_dup((bdd_ptr) car(path));
  nusmv_assert( BddFsm_is_fair_states(fsm, starting_state) );
  image = BddFsm_get_forward_image(fsm, starting_state);

  acc = bdd_dup(f);

  if (opt_use_fair_states(opts)) {
    bdd_ptr fair_states_bdd = BddFsm_get_fair_states(fsm);
    bdd_and_accumulate(dd_manager, &acc, fair_states_bdd);
    bdd_free(dd_manager, fair_states_bdd);
  }

  bdd_and_accumulate(dd_manager, &acc, image);
  bdd_free(dd_manager, image);

  if (bdd_is_false(dd_manager, acc)) {
    /* Failure in search */
    path = Nil;
  }
  else {
    path = Extend_trace_with_state_input_pair(fsm, enc, path, starting_state,
                                              acc, "ex_explain: (1).");
  }

  bdd_free(dd_manager, starting_state);
  bdd_free(dd_manager, acc);

  return path;
}

node_ptr eu_si_explain(BddFsm_ptr fsm, BddEnc_ptr enc,
                       node_ptr path, bdd_ptr f, bdd_ptr g_si, bdd_ptr hulk)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  DDMgr_ptr dd_manager = BddEnc_get_dd_manager(enc);
  node_ptr res;
  bdd_ptr g;

  g = BddFsm_states_inputs_to_states(fsm, g_si);

  res = eu_explain(fsm, enc, path, f, g);

  /*
    If an explanation has been found, and the fairness condition is on
    states and inputs then we see if necessary to add a transition to
    the path.
  */
  if ((res != Nil) && (g != g_si)) {
    bdd_ptr state;
    bdd_ptr si;

    state = bdd_dup((bdd_ptr) car(res));
    si = bdd_and(dd_manager, state, g_si);

    /* No constraint on following transition. */
    if (state != si) {
      bdd_ptr next_states;
      bdd_ptr next_state;
      bdd_ptr inputs;
      bdd_ptr input;

      inputs = BddFsm_states_inputs_to_inputs(fsm, si);

      /* Here we pick the right input -- and at this stage we may
         not know. */
      input = BddEnc_pick_one_input(enc, inputs);
      bdd_free(dd_manager, inputs);

      next_states = BddFsm_get_constrained_forward_image(fsm, state, input);

      /* May need to restrict the image to the target states. */
      bdd_and_accumulate(dd_manager, &next_states, hulk);

      next_state = BddEnc_pick_one_state(enc, next_states);
      bdd_free(dd_manager, next_states);

      /* add next state and corresponding input to witness path */
      res = cons(nodemgr, (node_ptr) bdd_dup(next_state),
                 cons(nodemgr, (node_ptr) bdd_dup(input), res));

      bdd_free(dd_manager, input);
      bdd_free(dd_manager, next_state);
    }

    bdd_free(dd_manager, state);
    bdd_free(dd_manager, si);
  }

  bdd_free(dd_manager, g);
  return res;
} /* eu_si_explain */

node_ptr eu_explain(BddFsm_ptr fsm, BddEnc_ptr enc,
                    node_ptr path, bdd_ptr f, bdd_ptr g)
{
  DDMgr_ptr dd_manager = BddEnc_get_dd_manager(enc);
  NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));

  bdd_ptr tmp, _new, Z, acc;
  int n;
  node_ptr witness_path;

  if (path == Nil) return Nil;

  /* Frontier.At the beginning it is just the initial states provided */
  _new = bdd_dup((bdd_ptr) car(path));

  /* Set of states reached so far along a path satisfying f. */
  Z = bdd_and(dd_manager, _new, f);

  n = 0; /* Iteration counter */

  /* initialize list to the one provided from use.
     NOTE TO DEVELOPERS: the list is reversed. */
  witness_path = path;

  /* The final accepting states 'g'. If fair states are to be used,
     then 'g' is intersected with fair states */
  acc = bdd_dup(g);
  if (opt_use_fair_states(opts)) {
    tmp = BddFsm_get_fair_states(fsm);
    bdd_and_accumulate(dd_manager, &acc, tmp);
    bdd_free(dd_manager, tmp);
  }

  if (opt_verbose_level_gt(opts, 1)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_nlog(logger, wffprint, "searching (counter)example for %N\n", ErrorMgr_get_the_node(errmgr));
  }

  /* --- If initial states intersect with accepted states then no further
     computation is required, just reset the initial states to one minterm */
  tmp = bdd_and(dd_manager, _new, acc);
  if (bdd_isnot_false(dd_manager, tmp)) {
    bdd_ptr state = BddEnc_pick_one_state(enc, tmp);
    bdd_free(dd_manager, tmp);

    bdd_free(dd_manager, (bdd_ptr) car(path));
    node_bdd_setcar(path, NODE_PTR(state));
    /* 'path' will be returned */
    goto free_local_bdds_and_return;
  }
  bdd_free(dd_manager, tmp);

  /* --- accepting states were not reached => restrict frontier to f */
  bdd_free(dd_manager, _new);
  _new = bdd_dup(Z);

  /* -- if frontier is empty => return Nil */
  if (bdd_is_false(dd_manager, _new)) {
    witness_path = Nil; /* 'Nil' will be returned */
    goto free_local_bdds_and_return;
  }

  /* -- the main loop => continue working until frontier is empty or
     intersects with accepting states */
  while (true) {

    if (opt_verbose_level_gt(opts, 1)) {
      Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
      Logger_log(logger, "eu_explain: iteration %d: states = %g, " \
              "BDD nodes = %d\n", n++, BddEnc_count_states_of_bdd(enc, Z),
              bdd_size(dd_manager, Z));
    }

    /* -- generate forward image */
    tmp = _new;
    _new = BddFsm_get_forward_image(fsm, _new);
    bdd_free(dd_manager, tmp);

    /* --- If the image intersects with accepting states then it is
       time to restrict new path to minterms and return. */
    tmp = bdd_and(dd_manager, _new, acc);
    if (bdd_isnot_false(dd_manager, tmp)) {
      bdd_ptr state = BddEnc_pick_one_state(enc, tmp);
      bdd_free(dd_manager, tmp);

      witness_path =
        Extend_trace_with_states_inputs_pair(fsm, enc, witness_path,
                                             (bdd_ptr) car(witness_path),
                                             state,
                                             "eu_explain: (1).");
      bdd_free(dd_manager, state);
      mc_eu_explain_restrict_state_input_to_minterms(fsm, enc,
                                                     witness_path, path);

      goto free_local_bdds_and_return; /* 'witness_path' will be returned */
    }
    bdd_free(dd_manager, tmp);

    /* -- since accepting states were not reached => restrict the states
       to f and update all reached states and frontier */
    bdd_and_accumulate(dd_manager, &_new, f);

    tmp = bdd_not(dd_manager, Z);

    bdd_or_accumulate(dd_manager, &Z, _new);
    bdd_and_accumulate(dd_manager, &_new, tmp);

    bdd_free(dd_manager, tmp);

    /* -- if new frontier is empty => free everything and return Nil */
    if (bdd_is_false(dd_manager, _new)) {
      while (witness_path != path) {
        node_ptr tmp_node = witness_path;
        witness_path = cdr(witness_path);

        bdd_free(dd_manager, (bdd_ptr) car(tmp_node));
        free_node(nodemgr, tmp_node);
      }
      witness_path = Nil; /* 'Nil' will be returned */
      goto free_local_bdds_and_return;
    }


    /* -- otherwise add the frontier to the path and continue iterations. */
    /*
      NOTE FOR DEVELOPERS:

      Here instead of '_new' we could use 'Z'.
      Both are correct but it is unclear wich one is more efficient
    */
    witness_path =
      Extend_trace_with_states_inputs_pair(fsm, enc, witness_path,
                                           (bdd_ptr) car(witness_path),
                                           _new, "eu_explain: (1).");
  } /* while true */

free_local_bdds_and_return:
  bdd_free(dd_manager, Z);
  bdd_free(dd_manager, _new);
  bdd_free(dd_manager, acc);

  return witness_path;
} /* eu_explain */

node_ptr eg_explain(BddFsm_ptr fsm, BddEnc_ptr enc,
                    node_ptr witness_path, bdd_ptr arg_g)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  bdd_ptr g;
  bdd_ptr eg_si_g;
  bdd_ptr eg_g;
  bdd_ptr tmp_1;
  JusticeList_ptr fairness_constraints;
  DDMgr_ptr dd_manager = BddEnc_get_dd_manager(enc);

  if (witness_path == Nil) return witness_path;

  /* Duplicate arg_g */
  g = bdd_dup(arg_g);

  /* Compute eg_si_g, i.e. the set of fair state inputs wrt g. */
  eg_si_g = eg_si(fsm, g);

  /* Compute eg_g, i.e. the subset of g which contains fair cycle. */
  eg_g = eg(fsm, g);

  /*
    If first state in the path is not in eg_g (that is, not a chance
    to construct a fair cycle) then return with Nil.
  */
  if (bdd_entailed(dd_manager, (bdd_ptr) car(witness_path), eg_g) == 0) {
    bdd_free(dd_manager, eg_g);
    bdd_free(dd_manager, eg_si_g);
    bdd_free(dd_manager, g);
    return Nil;
  }

  /* The main loop: it implements search, due to the fact that we may
     be in a SCC that does not allow us to satisfy all the fairness
     constransts, and therefore we need to backtrack. */
  fairness_constraints = BddFsm_get_justice(fsm);
  while (true) {
    node_ptr old_witness_path;
    bdd_ptr starting_state = bdd_dup((bdd_ptr) car(witness_path) );

    /* Try to construct a path satisfying the fairness constraints. */
    witness_path = fairness_explain(fsm, enc, witness_path, eg_si_g,
                                    fairness_constraints);

    /* Go one step backward from the starting state. */
    tmp_1 = ex(fsm, starting_state);
    bdd_free(dd_manager, g);
    g = bdd_and(dd_manager, eg_g, tmp_1);
    bdd_free(dd_manager, tmp_1);

    /* Save previous path */
    old_witness_path = witness_path;

    /* Try to show that starting_state state can be reached again:
       - first reach its premiage (without exiting eg_g)
       - then reach starting state (in one step)
    */
    witness_path = eu_explain(fsm, enc, witness_path, eg_g, g);
    witness_path = ex_explain(fsm, enc, witness_path, starting_state);

    /* If starting_state state reached, the loop is closed. Return. */
    if (witness_path != Nil) {
      bdd_free(dd_manager, starting_state);
      break;
    }

    /*
       Otherwise, try to show that eg_g can be reached on one step.
       This must be possible, otherwise we have an error.
    */
    witness_path = ex_explain(fsm, enc, old_witness_path, eg_g);
    if (witness_path == Nil) {
      bdd_free(dd_manager, eg_g);
      bdd_free(dd_manager, g);
      bdd_free(dd_manager, starting_state);
      ErrorMgr_internal_error(errmgr, "eg_explain: witness_path == Nil");
    }
    bdd_free(dd_manager, starting_state);
  } /* loop */

  bdd_free(dd_manager, eg_g);
  bdd_free(dd_manager, eg_si_g);
  bdd_free(dd_manager, g);

  return witness_path;
}

node_ptr ebu_explain(BddFsm_ptr fsm, BddEnc_ptr enc,
                     node_ptr path, bdd_ptr f, bdd_ptr g,
                     int inf, int sup)
{
  NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));

  DDMgr_ptr dd_manager = BddEnc_get_dd_manager(enc);
  int i, n;
  bdd_ptr Y, Z, tmp_1;
  node_ptr witness_path;

  if (path == Nil) return path;

  Y = bdd_dup((bdd_ptr) car(path));
  witness_path = path;

  n = 0;
  if (opt_verbose_level_gt(opts, 1)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_nlog(logger, wffprint, "searching (counter)example for %N\n",
                ErrorMgr_get_the_node(errmgr));
  }

  /* looks for a path from the first element of "path", with length
     inf, using only transitions from states satisfying "f". The sets
     of states in the list may contain states in which "f" is false.
     This is performed later, when a complete (counter)example is
     found, to avoid the need of recovering the "old" "car(path)" */

  for (i = 0; i < inf; i++) {
    if (opt_verbose_level_gt(opts, 1)) {
      Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
      Logger_log(logger, "ebu: iteration %d: states = %g, "             \
                 "BDD nodes = %d\n", n++, BddEnc_count_states_of_bdd(enc, Y),
                 bdd_size(dd_manager, Y));
    }

    Z = bdd_dup(Y);

    tmp_1 = bdd_and(dd_manager, Y, f);
    bdd_free(dd_manager, Y);
    Y = BddFsm_get_forward_image(fsm, tmp_1);

    if (bdd_is_false(dd_manager, Y)) {
      /* there is no valid path */
      bdd_free(dd_manager, Z);
      bdd_free(dd_manager, Y);

      while (witness_path != path) {
        node_ptr m = witness_path;

        bdd_free(dd_manager, (bdd_ptr) car(witness_path));
        witness_path = cdr(witness_path);
        free_node(nodemgr, m);
      } /* (witness_path != path) */
      return Nil;
    }

    witness_path =
      Extend_trace_with_states_inputs_pair(fsm, enc, witness_path,
                                           (bdd_ptr) car(witness_path),
                                           Y, "ebu_explain: (1).");
    if (Z == Y) {
      /* fixpoint found - fill the list with Y to length inf. */
      while (++i < inf) {
        witness_path =
          Extend_trace_with_states_inputs_pair(fsm, enc, witness_path,
                                               (bdd_ptr) car(witness_path),
                                               Y, "ebu_explain: (2).");
      } /* loop (++i < inf) */

      /* No need for further garbage collections. */
      bdd_free(dd_manager, Y);
      bdd_free(dd_manager, Z);
      break;
    } /* (Z == Y) */
    bdd_free(dd_manager, Z);
  } /* for (i = 0; i< inf ; i++) */

  /* At this point, car(witness_path) is the set of states that can be
     reached in inf steps, using transitions from states where "f" is
     valid.  Now we can call eu_explain(witness_path, f, g).
     eu_explain will find a shortest extension from car(witness_path)
     to a state where "g" is valid. We then check that the length of
     this path is less than or equal to (sup-inf). */

  { /* Block 1 */
    node_ptr new_witness_path;

    new_witness_path = eu_explain(fsm, enc, witness_path, f, g);

    if (new_witness_path != Nil) {
      node_ptr m = new_witness_path;

      /* This is needed since eu_explain returns a list of singletons */
      for (i = 0; m != Nil && m != witness_path; i++) {
        m = cdr(cdr(m)); /* Skip two steps -- also input */
      }
      if (m == Nil) {
        ErrorMgr_internal_error(errmgr, "ebu_explain: cannot get back to witness_path");
      }

      /* did we reach g in time? */
      if (i <= (sup - inf)) {
        /* Yes. Instantiate the Y's, and return a list of states */
        bdd_ptr x  = BddEnc_pick_one_state(enc, (bdd_ptr) car(witness_path));

        m = witness_path;

        while (true) {
          bdd_ptr nx, is;

          /* debugging code : x should not be empty ever */
          mc_explain_debug_check_not_empty_state(fsm, enc, x, "ebu_explain");

          /* substitute Y for one state of Y in the list */
          bdd_free(dd_manager, (bdd_ptr) car(m));
          node_bdd_setcar(m, bdd_dup(x));

          if (m == path) {  /* if we reached the first state, it's over */
            bdd_free(dd_manager, x);
            return new_witness_path;
          }

          /* We extract the inputs as to restrict the BWD image */
          is = bdd_dup((bdd_ptr) car(cdr(m)));

          /* Instantiate the next Y. x is a state in car(m), such that
             there is a path from the current x to it. */
          tmp_1 = BddFsm_get_constrained_backward_image(fsm, x, is);

          bdd_free(dd_manager, is);

          /* We intersect with the next state in the path */
          bdd_and_accumulate(dd_manager, &tmp_1, (bdd_ptr) car(cdr(cdr(m))));

          /* We intersect with f since if witness_path != path, car(m)
             may include states not satisfying f */
          bdd_and_accumulate(dd_manager, &tmp_1, f);

          nx = BddEnc_pick_one_state(enc, tmp_1);
          bdd_free(dd_manager, tmp_1);

          { /* We extract a singleton input connecting x and nx */
            bdd_ptr inputs = BddFsm_states_to_states_get_inputs(fsm, nx, x);
            bdd_ptr input = BddEnc_pick_one_input(enc, inputs);

            bdd_free(dd_manager, inputs);
            /* We strore it in the path */
            bdd_free(dd_manager, (bdd_ptr)car(cdr(m)));
            node_bdd_setcar(cdr(m), bdd_dup(input));
            bdd_free(dd_manager, input);
          }

          bdd_free(dd_manager, x);
          x = nx;
          m = cdr(cdr(m));
        } /* loop */
      } /* if (i <= (sup - inf)) */

      /* path from witness_path to new_witness_path is longer than
         requested. */
      witness_path = new_witness_path;
    } /* if (new_witness_path != Nil) */

    /* Could not find an example - free all newly allocated nodes */
    while (witness_path != path) {
      node_ptr m = witness_path;
      bdd_free(dd_manager, (bdd_ptr) car(witness_path));
      witness_path = cdr(witness_path);
      free_node(nodemgr, m);
    }
  } /* Block 1 */

  return Nil;
}

node_ptr ebg_explain(BddFsm_ptr fsm, BddEnc_ptr enc,
                     node_ptr path, bdd_ptr g,
                     int inf, int sup)
{
  NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  int i, n;
  bdd_ptr Z, tmp_1;
  bdd_ptr Y;
  node_ptr witness_path;
  DDMgr_ptr dd_manager = BddEnc_get_dd_manager(enc);

  if (path == Nil) return Nil;

  Y = bdd_dup((bdd_ptr) car(path));
  witness_path = path;

#ifdef EXPLAIN_TRACE_DEBUG
  Check_TraceList_Sanity(enc, path, "witness_path");
#endif

  n = 0;
  if (opt_verbose_level_gt(opts, 1)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_nlog(logger, wffprint, "searching (counter)example for %N\n",
                ErrorMgr_get_the_node(errmgr));
  }

  /* look for a path of length inf from car(path) */
  for (i=0; i < inf; i++) {

    if (opt_verbose_level_gt(opts, 1)) {
      Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
      Logger_log(logger, "ebg: iteration %d: states = %g, "\
              "BDD nodes = %d\n", n++, BddEnc_count_states_of_bdd(enc, Y),
              bdd_size(dd_manager, Y));
    }

    Z = bdd_dup(Y);
    tmp_1 = BddFsm_get_forward_image(fsm, Y);
    bdd_free(dd_manager, Y);
    Y = tmp_1;

    if (bdd_is_false(dd_manager, Y)) {
      /* there is no valid path */
      bdd_free(dd_manager, Z);
      bdd_free(dd_manager, Y);

      while (witness_path != path) {
        node_ptr m=witness_path;

        bdd_free(dd_manager, (bdd_ptr)car(witness_path));
        witness_path = cdr(witness_path);
        free_node(nodemgr, m);
      } /* while (witness_path != path) */

      return Nil;
    } /* if (Y == zero) */

    witness_path =
      Extend_trace_with_states_inputs_pair(fsm, enc, witness_path,
                                           (bdd_ptr) car(witness_path),
                                           Y, "ebg_explain: (1).");

    if (Z == Y) {
      /* fixpoint found - fill the list with Y to length inf. */
      while (++i < inf) {
        witness_path =
          Extend_trace_with_states_inputs_pair(fsm, enc, witness_path,
                                               (bdd_ptr) car(witness_path),
                                               Y, "ebg_explain: (2).");
      } /* while (++i < inf) */

      bdd_free(dd_manager, Y);
      bdd_free(dd_manager, Z);
      break;
    } /* if (Z == Y) */

    bdd_free(dd_manager, Z);
  } /* for (i=0; i < inf; i++) */
  bdd_free(dd_manager, Y);

  /*
     At this point, car(witness_path) is the set of states that can be
     reached in inf steps.  Look for a continuation path of length
     sup-inf using transitions from states that satisfy g
  */

  { /* Block 2 */
    node_ptr old_witness_path;
    bdd_ptr fair_g = bdd_dup(g);

    if (opt_use_fair_states(opts)) {
      bdd_ptr fair_states_bdd = BddFsm_get_fair_states(fsm);
      bdd_and_accumulate(dd_manager, &fair_g, fair_states_bdd);
      bdd_free(dd_manager, fair_states_bdd);
    }

    old_witness_path = witness_path;

    Y = (bdd_ptr) car(witness_path);
    for (i = inf; i < sup; i++) {
      if (opt_verbose_level_gt(opts, 1))
        StreamMgr_print_error(streams,  "ebg: iteration %d: states = %g, "\
                "BDD nodes = %d\n", n++, BddEnc_count_states_of_bdd(enc, Y),
                bdd_size(dd_manager, Y));

      Z = bdd_dup(Y);
      tmp_1 = bdd_and(dd_manager, fair_g, Y);

      Y = BddFsm_get_forward_image(fsm, tmp_1);

      /*
         Y must satisfy fair_g otherwise the last element can be such
         that it does not satisfy fair_g
      */
      bdd_and_accumulate(dd_manager, &Y, fair_g);
      bdd_free(dd_manager, tmp_1);

      if (bdd_is_false(dd_manager, Y)) {
        /* there is no valid path */
        bdd_free(dd_manager, Z);
        bdd_free(dd_manager, Y);
        while (witness_path != path) {
          node_ptr m=witness_path;

          bdd_free(dd_manager, (bdd_ptr)car(witness_path));
          witness_path = cdr(witness_path);
          free_node(nodemgr, m);
        } /* while (witness_path != path) */
        return Nil;
      } /* if (Y == zero) */

      witness_path =
        Extend_trace_with_states_inputs_pair(fsm, enc, witness_path,
                                             (bdd_ptr) car(witness_path),
                                             Y, "ebg_explain: (3).");
      if (Y == Z) {
        /* fixpoint found - fill the list with Y to length sup. */
        while (++i < sup) {
          witness_path =
            Extend_trace_with_states_inputs_pair(fsm, enc, witness_path,
                                                 (bdd_ptr) car(witness_path),
                                                 Y, "ebg_explain: (4).");
        } /* while (++i < sup) */

        /* No need for further garbage collections. */
        bdd_free(dd_manager, Y);
        bdd_free(dd_manager, Z);
        break;
      } /* if (Y == Z) */
      bdd_free(dd_manager, Z);
    } /* for (i = inf; i < sup; i++) */
    bdd_free(dd_manager, Y);

#ifdef EXPLAIN_TRACE_DEBUG
    Check_TraceList_Sanity(enc, witness_path, "ebg_explain: (5)");
    StreamMgr_print_output(streams,  "States in fair_g:\n");
    BddEnc_print_set_of_states(enc, fair_g, false, (VPFNNF) NULL,
                               outstream);
#endif

    /*
      transform witness_path from a list of sets of states into a list
      of states.
    */

    { /* Block 3 */
      node_ptr m = witness_path;
      bdd_ptr tmp_1 = bdd_and(dd_manager, fair_g, (bdd_ptr)car(m));
      bdd_ptr x = BddEnc_pick_one_state(enc, tmp_1);

      /* no longer needed */
      bdd_free(dd_manager, tmp_1);

      /* g should hold in all states up to car(old_witness_path), inclusive */
      while (true) {
        bdd_ptr nx, is;

        /* We intersect with fair_g since if witness_path != path,
           car(m) may include states not satisfying fair_g */
        /* substitute Y for one state of Y in the list */
        bdd_free(dd_manager, (bdd_ptr)car(m));
        node_bdd_setcar(m, bdd_dup(x));

#ifdef EXPLAIN_TRACE_DEBUG
        Check_TraceList_Sanity(enc, witness_path,
                               "ebg_explain: witness_path (after setcar)");
#endif

        if (m == old_witness_path) break;

        /* We extract the inputs */
        is = bdd_dup((bdd_ptr) car(cdr(m)));

        /* instantiate the next Y */
        tmp_1 = BddFsm_get_constrained_backward_image(fsm, x, is);

        bdd_free(dd_manager, is);

        /* We intersect with fair_g since if witness_path != path,
           car(m) may include states not satisfying fair_g */
        bdd_and_accumulate(dd_manager, &tmp_1, fair_g);

        /* We intersect with the next state in the path */
        bdd_and_accumulate(dd_manager, &tmp_1, (bdd_ptr)car(cdr(cdr(m))));

        nx = BddEnc_pick_one_state(enc, tmp_1);
        bdd_free(dd_manager, tmp_1);

        {
          /* We extract a singleton input connecting x and nx */
          bdd_ptr inputs = BddFsm_states_to_states_get_inputs(fsm, nx, x);
          bdd_ptr input = BddEnc_pick_one_input(enc, inputs);

          bdd_free(dd_manager, inputs);
          /* We strore it in the path */
          bdd_free(dd_manager, (bdd_ptr)car(cdr(m)));
          node_bdd_setcar(cdr(m), bdd_dup(input));
          bdd_free(dd_manager, input);
        }

        bdd_free(dd_manager, x);
        x = nx;
        /* go to next state */
        m = cdr(cdr(m));

      } /* while (true) */

      bdd_free(dd_manager, fair_g);

      /* Continue instantiating up to car(path), inclusive */
      while (true) {
        bdd_ptr nx, is;

        /* substitute Y for one state of Y in the list */
        bdd_free(dd_manager, (bdd_ptr)car(m));
        node_bdd_setcar(m, bdd_dup(x));
        if (m == path) break;

        is = bdd_dup((bdd_ptr) car(cdr(m)));

        /* instantiate the next Y */
        tmp_1 = BddFsm_get_constrained_backward_image(fsm, x, is);

        /* We intersect with the next state in the path */
        bdd_and_accumulate(dd_manager, &tmp_1, (bdd_ptr)car(cdr(cdr(m))));

        nx = BddEnc_pick_one_state(enc, tmp_1);
        bdd_free(dd_manager, tmp_1);

        {
          /* We extract a singleton input connecting x and nx */
          bdd_ptr inputs = BddFsm_states_to_states_get_inputs(fsm, nx, x);
          bdd_ptr input = BddEnc_pick_one_input(enc, inputs);

          bdd_free(dd_manager, inputs);
          /* We strore it in the path */
          bdd_free(dd_manager, (bdd_ptr)car(cdr(m)));
          node_bdd_setcar(cdr(m), bdd_dup(input));
          bdd_free(dd_manager, input);
        }

        bdd_free(dd_manager, x);
        x = nx;
        /* got to next state */
        m = cdr(cdr(m));
      } /* while (true) */

      bdd_free(dd_manager, x);

#ifdef EXPLAIN_TRACE_DEBUG
      Check_TraceList_Sanity(enc, witness_path,
                             "witness_path: witness_path (returned)");
#endif
      return witness_path;
    } /* Block 3 */
  } /* Block 2 */
} /* ebg_explain */


/*--------------------------------------------------------------------------*/
/* Definition of static functions                                           */
/*--------------------------------------------------------------------------*/

/*!
  \brief Recursively traverse the formula CTL and rewrite
   it in order to use the base witnesses generator functions.

  Recursively traverse the formula CTL and rewrite
   it in order to use the base witnesses generator functions.<br>
   The rewritings performed use the equivalence between CTL formulas,
   i.e. <i>A\[f U g\]</i> is equivalent to
   <i>!(E\[!g U (!g & !f)\] | EG !g)</i>.

  \sa explain
*/
static node_ptr explain_recur(BddFsm_ptr fsm, BddEnc_ptr enc, node_ptr path,
                              node_ptr formula_expr, node_ptr context)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));

  DDMgr_ptr dd_manager = BddEnc_get_dd_manager(enc);
  bdd_ptr a1, a2;
  node_ptr new_path;

  if (formula_expr == Nil) return Nil;

  nusmv_yylineno = node_get_lineno(formula_expr);
  switch (node_get_type(formula_expr)) {
  case CONTEXT:
    return explain_recur(fsm, enc, path,
                         cdr(formula_expr), car(formula_expr));

  case AND:
    return explain_and(fsm, enc, path, formula_expr, context);
  case OR:
  case NOT:
  case IMPLIES:
  case IFF:
    new_path = explain_recur(fsm, enc, path, car(formula_expr), context);
    if (new_path != Nil) return new_path;
    else return explain_recur(fsm, enc, path, cdr(formula_expr), context);

  case EX:
    a1 = eval_ctl_spec(fsm, enc, car(formula_expr), context);
    ErrorMgr_set_the_node(errmgr, formula_expr);
    new_path = ex_explain(fsm, enc, path, a1);
    bdd_free(dd_manager, a1);
    if (new_path != Nil) {
      node_ptr q = explain_recur(fsm, enc, new_path, car(formula_expr),
                                 context);
      if (q != Nil)  return q;
    }
    return new_path;

  case AX:
    return explain_recur(fsm, enc, path,
                         find_node(nodemgr, NOT,
                           find_node(nodemgr, EX,
                                     find_node(nodemgr, NOT, car(formula_expr), Nil),
                                     Nil),
                                   Nil),
                         context);

  case EF:
    return explain_recur(fsm, enc, path,
                         find_node(nodemgr, EU, ExprMgr_true(exprs), car(formula_expr)),
                         context);

  case AG:
    return explain_recur(fsm, enc, path,
                         find_node(nodemgr, NOT, find_node(nodemgr, EU, ExprMgr_true(exprs),
                                         find_node(nodemgr, NOT, car(formula_expr),
                                                   Nil)),
                                   Nil),
                         context);

  case EG:
    a1 = eval_ctl_spec(fsm, enc, car(formula_expr), context);
    ErrorMgr_set_the_node(errmgr, formula_expr);
    new_path = eg_explain(fsm, enc, path, a1);
    bdd_free(dd_manager, a1);
    return new_path;

  case AF:
    /* AF g and !EG !g are equivalent. */
    return explain_recur(fsm, enc, path,
                         find_node(nodemgr, NOT, find_node(nodemgr, EG,
                                         find_node(nodemgr, NOT, car(formula_expr),
                                                   Nil),
                                                  Nil),
                                   Nil), context);

  case EU:
    a1 = eval_ctl_spec(fsm, enc, car(formula_expr), context);
    a2 = eval_ctl_spec(fsm, enc, cdr(formula_expr), context);
    ErrorMgr_set_the_node(errmgr, formula_expr);
    new_path = eu_explain(fsm, enc, path, a1, a2);
    bdd_free(dd_manager, a2);
    bdd_free(dd_manager, a1);
    if (new_path != Nil) {
      node_ptr q = explain_recur(fsm, enc, new_path, cdr(formula_expr),
                                 context);

      if (q != Nil) return q;
    }
    return new_path;

  case AU:
    /* A[f U g] and !(E[!g U (!g & !f)] | EG !g) are equivalent. */
    return explain_recur(fsm, enc, path, find_node
                         (nodemgr, NOT, find_node
                          (nodemgr, OR, find_node
                           (nodemgr, EU, find_node
                            (nodemgr, NOT, cdr(formula_expr), Nil), find_node
                            (nodemgr, AND, find_node
                             (nodemgr, NOT, car(formula_expr), Nil), find_node
                             (nodemgr, NOT, cdr(formula_expr), Nil))), find_node
                           (nodemgr, EG, find_node
                            (nodemgr, NOT, cdr(formula_expr), Nil), Nil)), Nil),
                         context);

  case EBU:
    a1 = eval_ctl_spec(fsm, enc, car(car(formula_expr)), context);
    a2 = eval_ctl_spec(fsm, enc, cdr(car(formula_expr)), context);
    {
      int inf = BddEnc_eval_num(enc, car(cdr(formula_expr)), context);
      int sup = BddEnc_eval_num(enc, cdr(cdr(formula_expr)), context);

      ErrorMgr_set_the_node(errmgr, formula_expr);
      new_path = ebu_explain(fsm, enc, path, a1, a2, inf, sup);
    }
    bdd_free(dd_manager, a2);
    bdd_free(dd_manager, a1);

    if (new_path != Nil) {
      node_ptr q = explain_recur(fsm, enc, new_path, cdr(car(formula_expr)),
                                 context);

      if (q != Nil) return q;
    }
    return new_path;

  case ABU:
    /*
      A[f BU l..h g] is equivalent to
      ! ((EBF 0..(l - 1) !f)
      | EBG l..l ((EBG 0..(h - l) !g)
      | E[!g BU 0..(h - l) (!g & !f)]))

      f:car(car(formula_expr)) g:cdr(car(formula_expr))
         l:car(cdr(l)) h:cdr(car(formula_expr))
    */
    return (explain_recur(fsm, enc, path, find_node
               (nodemgr, NOT, find_node
                (nodemgr, OR, find_node
                 (nodemgr, EBF, find_node
                  (nodemgr, NOT, car(car(formula_expr)), Nil), find_node
                  (nodemgr, TWODOTS,
                   ExprMgr_number(exprs, 0), find_node
                   (nodemgr, MINUS, car(cdr(formula_expr)), ExprMgr_number(exprs, 1)))), find_node
                 (nodemgr, EBG, find_node
                  (nodemgr, OR, find_node
                   (nodemgr, EBG, find_node
                    (nodemgr, NOT, cdr(car(formula_expr)), Nil), find_node
                    (nodemgr, TWODOTS,
                     ExprMgr_number(exprs, 0), find_node
                     (nodemgr, MINUS,
                      cdr(cdr(formula_expr)),
                      car(cdr(formula_expr))))), find_node
                   (nodemgr, EBU, find_node
                    (nodemgr, EU, find_node
                     (nodemgr, NOT, cdr(car(formula_expr)), Nil), find_node
                     (nodemgr, AND, find_node
                      (nodemgr, NOT, car(car(formula_expr)), Nil), find_node
                      (nodemgr, NOT, cdr(car(formula_expr)), Nil))), find_node
                    (nodemgr, TWODOTS,
                     ExprMgr_number(exprs, 0), find_node
                     (nodemgr, MINUS,
                      cdr(cdr(formula_expr)),
                      car(cdr(formula_expr)))))), find_node
                  (nodemgr, TWODOTS,
                   car(cdr(formula_expr)),
                   car(cdr(formula_expr))))), Nil),
                          context));

  case EBF:
    /* EBF range g and E[TRUE BU range g] are equivalent.  */
    return (explain_recur(fsm, enc, path, find_node
                          (nodemgr, EBU, find_node
                           (nodemgr, EU, ExprMgr_true(exprs), car(formula_expr)),
                           cdr(formula_expr)),
                          context));

  case ABG:
    /* ABG range g and !EBF range !g are equivalent. */
    return (explain_recur(fsm, enc, path, find_node
                          (nodemgr, NOT, find_node
                           (nodemgr, EBF, find_node
                            (nodemgr, NOT, car(formula_expr), Nil),
                            cdr(formula_expr)), Nil),
                          context));

  case EBG:
    a1 = eval_ctl_spec(fsm, enc, car(formula_expr), context);
    {
      int inf = BddEnc_eval_num(enc, car(cdr(formula_expr)), context);
      int sup = BddEnc_eval_num(enc, cdr(cdr(formula_expr)), context);

      ErrorMgr_set_the_node(errmgr, formula_expr);
      new_path = ebg_explain(fsm, enc, path, a1, inf, sup);
    }
    bdd_free(dd_manager, a1);
    return new_path;

  case ABF:
    /* ABF range g and !EBG range !g are equivalent. */
    return (explain_recur(fsm, enc, path, find_node
                          (nodemgr, NOT, find_node
                           (nodemgr, EBG, find_node
                            (nodemgr, NOT, car(formula_expr), Nil),
                            cdr(formula_expr)), Nil),
                          context));

  case ARRAY:
  case DOT:
  case ATOM:
    {
      ResolveSymbol_ptr rs;
      SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(enc));
      node_ptr resName;

      rs = SymbTable_resolve_symbol(st, formula_expr, context);
      resName = ResolveSymbol_get_resolved_name(rs);

      if (ResolveSymbol_is_ambiguous(rs)) {
        char* err = ResolveSymbol_get_error_message(rs, wffprint);
        ErrorMgr_rpterr(errmgr, "%s", err);
        FREE(err);
      }

      if (ResolveSymbol_is_parameter(rs)) {
        node_ptr par = SymbTable_get_flatten_actual_parameter(st, resName);
        return explain_recur(fsm, enc, path, par, context);
      }
      else if (ResolveSymbol_is_define(rs)) {
        return explain_recur(fsm, enc, path,
                             SymbTable_get_define_body(st, resName),
                             SymbTable_get_define_context(st, resName));
      }
      else if (ResolveSymbol_is_constant(rs)) { return Nil; }
      else if (ResolveSymbol_is_undefined(rs)) {
        if (ARRAY == node_get_type(formula_expr)) {
          /* Array may be an identifier-with-brackets or an expression.
             Here an array-expression is detected => expression is to be
             flattened at first to resolve identifiers-with-brackets (see
             description of flattener_core_flatten for details) */
          node_ptr tmp = Compile_FlattenSexp(st, formula_expr, context);
          /* loop in recursion is impossible */
          nusmv_assert(tmp != formula_expr);
          return explain_recur(fsm, enc, path, tmp, Nil);
        }
        else {
          ErrorMgr_error_undefined(errmgr, resName);
        }
      }

      return Nil;  /* defined, but not a define or a parameter or a
                      constant */
    }

  default:
    return Nil;
  }
}

/*!
  \brief Auxiliary function to the computation of a
   witness of the formula <i>EG f</i>.

  In the computation of the witness for the
   formula <i>EG f</i>, at each step we must ensure that the current
   prefix can be extended to a fair path along which each state
   satisfies <i>f</i>. This function performs the inner fixpoint
   computation for each fairness constraints in the fix point
   computation of the formula <i>EG(f)<i>. For every constraints
   <i>h</i>, we obtain an increasing sequence of approximations Q_0^h,
   Q_1^h, ..., where each Q_i^h is the set of states from which a state
   in the accumulated set can be reached in <i>i</i> or fewer steps,
   while satisfying <i>f</i>.

  \sa explain, eg_explain, fair_iter, eg
*/
static node_ptr fairness_explain(BddFsm_ptr fsm, BddEnc_ptr enc,
                                 node_ptr witness_path, bdd_ptr hulk_si,
                                 JusticeList_ptr fairness_constrainst_list)
{
  /*
    NOTE TO DEVELOPERS:
    The algorithm here does not take into account whether some
    fairness constraints have already been covered by (the paths to)
    previous ones (other than by chance being fulfilled in the
    currently last state of the path). In particular, each fairness
    constraint that talks about input variables enforces at least one
    transition. One might collect all states-inputs pairs on the way
    and, before calling eu_si_explain, check whether the current
    fairness constraint has already been covered.
  */
  DDMgr_ptr dd_manager = BddEnc_get_dd_manager(enc);
  FairnessListIterator_ptr iter;
  node_ptr res;
  bdd_ptr hulk;

  /* initialization of local variables */
  res = witness_path;
  hulk = BddFsm_states_inputs_to_states(fsm, hulk_si);

  iter = FairnessList_begin( FAIRNESS_LIST(fairness_constrainst_list) );
  while( ! FairnessListIterator_is_end(iter) ) {
    BddStates fc_si;
    BddStatesInputs hulk_fc_si;
    BddStatesInputs fair_si;

    /* Select fairness constraint */
    fc_si = JusticeList_get_p(fairness_constrainst_list, iter);

    /* Conjunct with fair transitions */
    fair_si = BddFsm_get_fair_states_inputs(fsm);
    bdd_and_accumulate(dd_manager, &fc_si, fair_si);

    /* Conjunct it with the current set */
    hulk_fc_si = bdd_and(dd_manager, hulk_si, fc_si);

    /*
       Construct a path to the fairness constraint without leaving the hulk
    */
    res = eu_si_explain(fsm, enc, res, hulk, hulk_fc_si, hulk);

    bdd_free(dd_manager, fc_si);
    bdd_free(dd_manager, fair_si);
    bdd_free(dd_manager, hulk_fc_si);

    iter = FairnessListIterator_next(iter);
  } /* loop */

  bdd_free(dd_manager, hulk);
  return res;
}


/*!
  \brief Generates a witness path for car(formula) AND cdr(formula)

  Generates a witness path for car(formula) AND cdr(formula)

  \se None

  \sa explain_recur
*/

node_ptr explain_and(BddFsm_ptr fsm, BddEnc_ptr enc,
                     node_ptr path, node_ptr formula_expr,
                     node_ptr context)
{
  NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));

  DDMgr_ptr dd;
  node_ptr res_l, res_r;
  node_ptr path_l, path_r;
  bdd_ptr state;

  dd = BddEnc_get_dd_manager(enc);

  /* Ensure that a solution exists */
  path = explain_eval(fsm, enc, path, formula_expr, context);

  if (Nil == path) return Nil;

  /* Construct a witness path based on the solution */
  state = (bdd_ptr) car(path);

  /* temporary path for finding a witness for the left part */
  path_l = cons(nodemgr, (node_ptr)bdd_dup(state), cdr(path));
  /* computing the witness for the left conjunct */
  res_l = explain(fsm, enc, path_l, car(formula_expr), context);

  /* temporary path for finding a witness for the right part */
  path_r = cons(nodemgr, (node_ptr)bdd_dup(state), cdr(path));
  /* computing the witness for the right conjunct */
  res_r = explain(fsm, enc, path_r, cdr(formula_expr), context);

  /* Ensure at most one branch extends the list of counter-examples.
   * Otherwise, since we don't support printing both paths
   * for witness generation, output a warning message
   */
  if ((res_l == path_l) && (res_r == path_r)) {
    /* The counter examples did not extend */
    bdd_free(dd, (bdd_ptr)car(path_l));
    free_node(nodemgr, path_l);
    bdd_free(dd, (bdd_ptr)car(path_r));
    free_node(nodemgr, path_r);
    return path;
  }
  else if ((Nil != res_r) && (res_l == path_l)) {
    /* left branch did not extend the counter example, use right
       branch result */
    bdd_free(dd, (bdd_ptr)car(path_l));
    free_node(nodemgr, path_l);
    bdd_free(dd, (bdd_ptr)car(path));
    return res_r;
  }
  else if ((Nil != res_l) && (res_r == path_r)) {
    /* right branch did not extend the counter example, use left
       branch result */
    bdd_free(dd, (bdd_ptr)car(path_r));
    free_node(nodemgr, path_r);
    bdd_free(dd, (bdd_ptr)car(path));
    return res_l;
  }
  else {
    /* Both left and right witness extend the counter example. We stop
       generation and we return the path so far */
    if (opt_verbose_level_gt(opts, 2)) {
      Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
      Logger_nlog(logger, wffprint, 
                  "Warning: Witness generation terminated at branch point.\n"
                  "Warning: Branch due to a conjunction.\n"
                  "Warning: %N\n"
                  "(Considering only extension of the right conjunct)\n",
                  formula_expr);
    }

    bdd_free(dd, (bdd_ptr)car(path_l));
    free_node(nodemgr, path_l);
    bdd_free(dd, (bdd_ptr)car(path));
    return res_r;
  }
}


/*!
  \brief required

  optional

  \se required

  \sa optional
*/

node_ptr explain_eval(BddFsm_ptr fsm, BddEnc_ptr enc,
                      node_ptr path, node_ptr formula_expr,
                      node_ptr context)
{
  node_ptr result;

  bdd_ptr val, start;
  DDMgr_ptr dd;

  if (Nil == path) return Nil;

  dd = BddEnc_get_dd_manager(enc);

  start = bdd_dup((bdd_ptr)car(path));

  val = eval_ctl_spec(fsm, enc, formula_expr, context);

  bdd_and_accumulate(dd, &start, val);

  bdd_free(dd, val);

  if (bdd_is_false(dd, start)) {
    result = Nil;
  }
  else {
    bdd_free(dd, (bdd_ptr)car(path));
    node_bdd_setcar(path, (bdd_ptr)bdd_dup(start));
    result = path;
  }
  bdd_free(dd, start);
  return result;
}

/*!
  \brief 

  
*/
static node_ptr Extend_trace_with_state_input_pair(BddFsm_ptr fsm,
                                                   BddEnc_ptr enc,
                                                   node_ptr path,
                                                   bdd_ptr starting_state,
                                                   bdd_ptr next_states,
                                                   const char * comment)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  DDMgr_ptr dd_manager = BddEnc_get_dd_manager(enc);
  node_ptr res;
  bdd_ptr next_state, inputs, input;

#ifdef EXPLAIN_TRACE_DEBUG
  size_t size = strlen(comment) + 10;
  char* com = ALLOC(char, size);

  snprintf(com, size, "%s: (%d)", comment, 1);

  Check_TraceList_Sanity(enc, path, com);
#endif

  next_state = BddEnc_pick_one_state(enc, next_states);
  inputs = BddFsm_states_to_states_get_inputs(fsm,
                                              starting_state,
                                              next_state);
  input = BddEnc_pick_one_input(enc, inputs);

  res = cons(nodemgr, (node_ptr) bdd_dup(next_state),
             cons(nodemgr, (node_ptr) bdd_dup(input), path));

#ifdef EXPLAIN_TRACE_DEBUG
  snprintf(com, size, "%s: (%d)", comment, 2);

  Check_TraceList_Sanity(enc, res, com);

  FREE(com);
#endif

  bdd_free(dd_manager, input);
  bdd_free(dd_manager, inputs);
  bdd_free(dd_manager, next_state);

  return res;
}

/*!
  \brief 

  
*/
static node_ptr Extend_trace_with_states_inputs_pair(BddFsm_ptr fsm,
                                                     BddEnc_ptr enc,
                                                     node_ptr path,
                                                     bdd_ptr starting_states,
                                                     bdd_ptr next_states,
                                                     const char * comment)
{
  node_ptr res;
  bdd_ptr inputs;
  DDMgr_ptr dd_manager = BddEnc_get_dd_manager(enc);
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

#ifdef EXPLAIN_TRACE_DEBUG
  size_t size = strlen(comment) + 10;
  char* com = ALLOC(char, size);

  snprintf(com, size, "%s: (%d)", comment, 1);

  Check_TraceList_Sanity(enc, path, com);
#endif

  inputs = BddFsm_states_to_states_get_inputs(fsm, starting_states,
                                              next_states);

  res = cons(nodemgr, (node_ptr) bdd_dup(next_states),
             cons(nodemgr, (node_ptr) bdd_dup(inputs), path));

#ifdef EXPLAIN_TRACE_DEBUG
  snprintf(com, size, "%s: (%d)", comment, 2);

  Check_TraceList_Sanity(enc, res, com);

  FREE(com);
#endif

  bdd_free(dd_manager, inputs);

  return res;
}


#ifdef EXPLAIN_TRACE_DEBUG

/*!
  \brief 

  
*/
static void Check_TraceList_Sanity(BddEnc_ptr enc, node_ptr path,
                                   const char * varname)
{
  int i, l;
  node_ptr scan;
  DDMgr_ptr dd = BddEnc_get_dd_manager(enc);
  boolean must_abort = 0;

  l = llength(path);
  scan = path;

  StreamMgr_print_error(streams,  "Checking TraceList Sanity: %s\n", varname);
  StreamMgr_print_error(streams,  "Length of list: %d\n", l);

  for (i = 0; i < l; i++) {
    bdd_ptr elem = (bdd_ptr)car(scan);

    if ( (i % 2) == 0 ) {
      StreamMgr_print_error(streams,   "STATE(%d):\n", i);
    }
    else {
      StreamMgr_print_error(streams,   "INPUT(%d):\n", i);
    }
    nusmv_assert( elem != (bdd_ptr) NULL );
    must_abort = (must_abort || bdd_is_false(dd, elem));
    if ( must_abort ) {
      StreamMgr_print_error(streams,   "**** DOOMED TO ABORT, STEP %d\n", i);
    }
    dd_printminterm(dd, elem);

    scan = cdr(scan);
  }
  if ( must_abort ) error_unreachable_code();
}
#endif

/*!
  \brief Given a path the function restrict its states
   and inputs to minterms

  'path' is a list of ((state, input)+, state).
   States and inputs are BDDs.
   The list has to be reversed i.e. state car(path) can be reached
   from state car(cdr(cdr(path))) through input car(cdr(path)).

   The first element of the path, i.e. car(path) has to be already
   minterm.

   The function restricts every state and input to single state/input,
   resp, i.e. to minterm.

   initial_node is the last state node of path to be restricted.
*/
static void
mc_eu_explain_restrict_state_input_to_minterms(BddFsm_ptr fsm,
                                               BddEnc_ptr enc,
                                               node_ptr path,
                                               node_ptr initial_node)
{
  node_ptr iter;
  DDMgr_ptr dd_manager = BddEnc_get_dd_manager(enc);

  {
    /* We ensure the initial state is a minterm */
    bdd_ptr state = BddEnc_pick_one_state(enc, (bdd_ptr)car(path));

    bdd_free(dd_manager, (bdd_ptr)car(path));
    node_bdd_setcar(path, state);
  }

  for (iter = path; iter != initial_node; iter = cdr(cdr(iter))) {
     /* NOTE FOR DESCRIPTION: Here current_state must be minterm, and
        by construction it is such. */
    bdd_ptr current_state = (bdd_ptr) car(iter);
    bdd_ptr one_prev_state;
    bdd_ptr one_input;

    /* there must exist current state, input and previous state */
    nusmv_assert(iter != Nil && cdr(iter) != Nil && cdr(cdr(iter)) != Nil);


    /* debugging code : current_state should not be empty ever */
    mc_explain_debug_check_not_empty_state(fsm, enc, current_state,
                                           "eu_explain");

    { /* get one previous state, i.e. compute preimage for the given
         current state and intersect with provided previous states.

         NOTE FOR DEVELOPERS:
         Here we avoid to use bdd_dup since the BDDs are only used as
         argument of other functions, and by construction they already
         have a non-zero reference count.
      */
      bdd_ptr inputs = (bdd_ptr)car(cdr(iter));
      bdd_ptr prev_states = (bdd_ptr)car(cdr(cdr(iter)));
      bdd_ptr image = BddFsm_get_constrained_backward_image(fsm,
                                                            current_state,
                                                            inputs);
      bdd_and_accumulate(dd_manager, &image, prev_states);
      one_prev_state = BddEnc_pick_one_state(enc, image);
      bdd_free(dd_manager, image);
    }


    { /* We extract a singleton input connecting current and previous state */
      bdd_ptr all_inputs = BddFsm_states_to_states_get_inputs(fsm,
                                                              one_prev_state,
                                                              current_state);
      one_input = BddEnc_pick_one_input(enc, all_inputs);
      bdd_free(dd_manager, all_inputs);
    }

    /* Store the computed state and input in the path */
    bdd_free(dd_manager, (bdd_ptr)car(cdr(iter)));
    node_bdd_setcar(cdr(iter), one_input);

    bdd_free(dd_manager, (bdd_ptr)car(cdr(cdr(iter))));
    node_bdd_setcar(cdr(cdr(iter)), one_prev_state);
  } /* while iter != initial_node */
}

/*!
  \brief Debugging code for eu_explain

  Just check that the set of state is not empty.
  If reachable states are enabled then the provided set of states
  has to be reachable as well

  \se None

  \sa eu_explain
*/


/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
static void
mc_explain_debug_check_not_empty_state(BddFsm_ptr fsm, BddEnc_ptr enc,
                                       bdd_ptr states, const char * message)
{
  NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  OStream_ptr errstream = StreamMgr_get_error_ostream(streams);

  bdd_ptr tmp;
  DDMgr_ptr dd_manager = BddEnc_get_dd_manager(enc);

  if (opt_use_reachable_states(opts)) {
    tmp = BddFsm_get_reachable_states(fsm);
    bdd_and_accumulate(dd_manager, &tmp, states);
  }
  else {
    tmp = bdd_dup(states);
  }

  if (bdd_is_false(dd_manager, tmp)) {
    SymbTableIter iter;
    NodeList_ptr vars;
    SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(enc));

    SymbTable_gen_iter(st, &iter, STT_VAR);
    vars = SymbTable_iter_to_list(st, iter);
    StreamMgr_print_error(streams,  "Error: The following state is not reachable:\n");
    BddEnc_print_bdd_begin(enc, vars, false);
    BddEnc_print_bdd(enc, tmp, (VPFBEFNNV) NULL, errstream, NULL);
    BddEnc_print_bdd_end(enc);
    NodeList_destroy(vars);
    /* Here the free of all the variables has to be performed */
    bdd_free(dd_manager, tmp);
    ErrorMgr_internal_error(errmgr, "%s: state not reachable", message);
  }
  else {
    bdd_free(dd_manager, tmp);
  }
}
