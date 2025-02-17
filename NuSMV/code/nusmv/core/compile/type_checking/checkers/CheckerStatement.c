/* ---------------------------------------------------------------------------

 This file is part of the
  ``compile.type_checking.checkers'' package of NuSMV version 2.
  Copyright (C) 2006 by FBK-irst.

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
  \brief Implementaion of class 'CheckerStatement'

  \todo: Missing description

*/


#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/StreamMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/node/NodeMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/ErrorMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/type_checking/checkers/CheckerStatement.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/type_checking/checkers/CheckerStatement_private.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/type_checking/checkers/checkersInt.h"

#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/compile.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/type_checking/TypeChecker_private.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/symb_table/symb_table.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/WordNumberMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/parser/symbols.h"
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
/* See 'CheckerStatement_private.h' for class 'CheckerStatement' definition. */

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

static void checker_statement_finalize(Object_ptr object, void* dummy);

static SymbType_ptr
checker_statement_check_expr(CheckerBase_ptr self,
                             node_ptr expression, node_ptr context);

static boolean
checker_statement_viol_handler(CheckerBase_ptr checker,
                               TypeSystemViolation violation,
                               node_ptr expression);

static void
print_operator(FILE* output_stream, node_ptr expr);



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

CheckerStatement_ptr CheckerStatement_create(const NuSMVEnv_ptr env)
{
  CheckerStatement_ptr self = ALLOC(CheckerStatement, 1);
  CHECKER_STATEMENT_CHECK_INSTANCE(self);

  checker_statement_init(self, env);
  return self;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void checker_statement_init(CheckerStatement_ptr self, const NuSMVEnv_ptr env)
{
  /* base class initialization */
  checker_core_init(CHECKER_CORE(self), env,
                    "Statements SMV Type Checker",
                    NUSMV_STATEMENTS_SYMBOL_FIRST,
                    (NUSMV_STATEMENTS_SYMBOL_LAST -
                     NUSMV_STATEMENTS_SYMBOL_FIRST));

  /* members initialization */
  self->inside_attime = false;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = checker_statement_finalize;
  OVERRIDE(CheckerBase, check_expr) = checker_statement_check_expr;
  OVERRIDE(CheckerBase, viol_handler) = checker_statement_viol_handler;
}

void checker_statement_deinit(CheckerStatement_ptr self)
{
  /* members deinitialization */


  /* base class initialization */
  checker_core_deinit(CHECKER_CORE(self));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The CheckerStatement class virtual finalizer

  Called by the class destructor
*/
static void checker_statement_finalize(Object_ptr object, void* dummy)
{
  CheckerStatement_ptr self = CHECKER_STATEMENT(object);

  checker_statement_deinit(self);
  FREE(self);
}

/*!
  \brief 

  
*/
static SymbType_ptr
checker_statement_check_expr(CheckerBase_ptr self,
                             node_ptr expr, node_ptr context)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  /* wrap expr into the context. This is required by
     the facilities which remembers the type of expressions
     and by the violation handler.
  */
  node_ptr ctx_expr;
  if (context != Nil) ctx_expr = find_node(nodemgr, CONTEXT, context, expr);
  else ctx_expr = expr;

  { /* checks memoizing */
    SymbType_ptr tmp = _GET_TYPE(ctx_expr);
    if (nullType != tmp) return tmp;
  }

  switch (node_get_type(expr)) {

  case TRANS:
  case INIT:
  case INVAR:
  case FAIRNESS:
  case JUSTICE:
  case COMPASSION:
  case SPEC:
  case LTLSPEC:
  case PSLSPEC:
  case INVARSPEC:
  case ISA:
  case CONSTRAINT:
  case MODULE:
  case PROCESS:
  case MODTYPE:
  case LAMBDA:
  {
    /* get the operand's type */
    SymbType_ptr type = _THROW(car(expr), Nil);

    if (SymbType_is_error(type)) {
      /* earlier error */
      return _SET_TYPE(ctx_expr, SymbTablePkg_error_type(env));
    }

    /* The operand must be boolean or statement-type (statements can
       be produced by predicate-normalisation in symbolic FSM (when
       assignments are pushed into expressions). */
    if (SymbType_is_boolean(type) || SymbType_is_statement(type)) {
      return _SET_TYPE(ctx_expr, type);
    }

    /* there is violation */
    if (_VIOLATION(SymbType_is_back_comp(type) ?
                   TC_VIOLATION_TYPE_BACK_COMP : TC_VIOLATION_TYPE_MANDATORY,
                   expr)) {
      return _SET_TYPE(ctx_expr, SymbTablePkg_error_type(env));
    }

    /* this is not an error after all -> keeps the current type */
    return _SET_TYPE(ctx_expr, type);
  }

  case ATTIME:
    {
      /* boolean expression and a constant integer */
      SymbType_ptr type;

      /* not nested */
      if ((CHECKER_STATEMENT(self)->inside_attime) &&
          _VIOLATION(TC_VIOLATION_ATTIME_NESTED, expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type(env));
      }

      /* time is a number */
      if ((node_get_type(cdr(expr)) != NUMBER) &&
          _VIOLATION(TC_VIOLATION_ATTIME_NUM_REQ, expr)) {
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type(env));
      }
      CHECKER_STATEMENT(self)->inside_attime = true;
      type = _THROW(car(expr), context);
      CHECKER_STATEMENT(self)->inside_attime = false;
      if (SymbType_is_error(type)) {
        /* earlier error */
        return _SET_TYPE(ctx_expr, SymbTablePkg_error_type(env));
      }

      /* this is not an error after all -> keeps the current type */
      return _SET_TYPE(ctx_expr, type);
    }

  case DEFINE:
  { /* DEFINE is artificial contract to check DEFINES body */
    SymbType_ptr type = _THROW(car(expr), Nil);

    /* earlier error */
    if (SymbType_is_error(type)) return _SET_TYPE(ctx_expr, type);

    /* DEFINEs can have any type => do not check constrains on the type */
    return _SET_TYPE(ctx_expr, type);
  }

  case ASSIGN:
  { /* type check the assignment: EQDEF */
    SymbType_ptr type;

    /* the car is an EQDEF or it is a CONTEXT node that has an EQDEF
       as right child.

       Checking for CONTEXT enable to type check not-flattened ASSIGN.
    */
    nusmv_assert(
                 EQDEF == node_get_type(car(expr)) ||
                 (CONTEXT == node_get_type(car(expr)) &&
                  EQDEF == node_get_type(cdr(car(expr))))
                 );


    type = _THROW(car(expr), Nil);

    /* mark the error situation */
    if (SymbType_is_error(type)) return _SET_TYPE(ctx_expr, type);
    return _SET_TYPE(ctx_expr, type);
  }

  case COMPUTE:
  {
    SymbType_ptr type;

    /* the expression is also wrapped in context */
    nusmv_assert(CONTEXT == node_get_type(car(expr)));

    /* now the expression should be MAX or MIN */
    nusmv_assert(MINU == node_get_type(cdr(car(expr))) ||
                 MAXU == node_get_type(cdr(car(expr))));

    /* get the operands' type */
    type = _THROW(car(expr), context);

    if (SymbType_is_error(type)) {
      return _SET_TYPE(expr, SymbTablePkg_error_type(env)); /*earlier error*/
    }

    /* the expressions must be boolean */
    if (SymbType_is_boolean(type)) {
      return _SET_TYPE(expr, type);
    }

    if(_VIOLATION(SymbType_is_back_comp(type) ?
                  TC_VIOLATION_TYPE_BACK_COMP : TC_VIOLATION_TYPE_MANDATORY,
                  expr)) {
      _SET_TYPE(expr, SymbTablePkg_error_type(env));
    }

    /* this is not an error after all -> keeps the current type */
    return _SET_TYPE(ctx_expr, type);
  } /* COMPUTE */


  default:
    error_unreachable_code();


  } /* switch */

  return nullType;
}




/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The type core violation handler.

  The violation handler is implemented as
  a virtual method, which is invoked by the checker when an expression
  being checked violates the type system.
  See the violation handler TypeCheckingViolationHandler_ptr
  for more explanations.

  The below function is the default violation handler, and a
  user can potentially define its own violation handler, by deriving
  a new class from this class and by overriding this virtual method.

  This violation handler outputs an error and warning message to
  errstream. A warning is output if the detected violation is
  TC_VIOLATION_TYPE_BACK_COMP or TC_VIOLATION_DUPLICATE_CONSTANTS and
  the system variable "type_checking_backward_compatibility" is
  true. Also the TC_VIOLATION_TYPE_WARNING violation outputs a
  warning. Only in this case the false value is returned, indicating
  that this is NOT an error. Otherwise the true value is returned,
  indicating that this is an error.

  Also, if the system variable "type_check_warning_on" is false,
  warning messages are not output.

  NB: if the expression is checked in some context (context is not null) then
  before providing the expression to this function the expression should be
  wrapped into context, i.e. with find_node(nodemgr, CONEXT, context, expr)

  \sa TypeSystemViolation
*/
static boolean
checker_statement_viol_handler(CheckerBase_ptr self,
                               TypeSystemViolation violation,
                               node_ptr expression)
{
  /* In the output message, the information about the expression
     location are output. So, make sure that the input file name and
     line number are correctly set!
  */
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  FILE* errstream = StreamMgr_get_error_stream(streams);

  boolean isError = true; /* is this error or warning */

  /* get rid of the context the expression may be wrapped in */
  node_ptr context = Nil;
  node_ptr expr = expression;

  if (node_get_type(expression) == CONTEXT) {
    context = car(expression);
    expr = cdr(expression);
  }

  /* checks the given violation */
  nusmv_assert(TypeSystemViolation_is_valid(violation));

  /* only violation TC_VIOLATION_TYPE_BACK_COMP or
     TC_VIOLATION_DUPLICATE_CONSTANTS and the variable
     type_checking_backward_compatibility being true, may
     make a warning from an error.
     TC_VIOLATION_TYPE_WARNING always forces a warning
  */
  if ( TC_VIOLATION_TYPE_WARNING == violation
       || ((TC_VIOLATION_TYPE_BACK_COMP == violation ||
            TC_VIOLATION_DUPLICATE_CONSTANTS == violation)
        && opt_backward_comp(opts))) {
    isError = false;
  }

  if (!isError && !opt_type_checking_warning_on(opts)) {
    /* this is a warning and warning messages are not allowed.
     So, do nothing, just return false (this is not an error)
    */
    return false;
  }

  _PRINT_ERROR_MSG(expr, isError);

  switch (violation) {
  case TC_VIOLATION_ATTIME_NESTED:
    StreamMgr_print_error(streams,  "Nested ATTIME are not allowed\n");
    break;

  case TC_VIOLATION_ATTIME_NUM_REQ:
    StreamMgr_print_error(streams,  "ATTIME requires time is a constant integer number\n");
    break;

  case TC_VIOLATION_TYPE_MANDATORY:
  case TC_VIOLATION_TYPE_BACK_COMP:
  case TC_VIOLATION_TYPE_WARNING:
    if (isError) StreamMgr_print_error(streams,  "illegal ");
    else         StreamMgr_print_error(streams,  "potentially incorrect ");

    switch (node_get_type(expr)) {
    case DEFINE:
    case ISA:
    case MODULE:
    case MODTYPE:
    case LAMBDA:
      {
        const MasterPrinter_ptr sexpprint =
          MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_SEXP_PRINTER));

        StreamMgr_nprint_error(streams, sexpprint, "%N", expr);
        error_unreachable_code(); /* this is impossible */
      }

    /* high level unary operator */
    case TRANS:
    case INIT:
    case INVAR:
    case FAIRNESS:
    case JUSTICE:
    case COMPASSION:
    case SPEC:
    case LTLSPEC:
    case PSLSPEC:
    case INVARSPEC:
    case CONSTRAINT:
    case COMPUTE:
    case PROCESS: /*PROCESS is artificial contract to check 'running' symbols*/
      StreamMgr_print_error(streams, "type of ");
      print_operator(errstream, expr);
      StreamMgr_print_error(streams, " expression : ");
      checker_base_print_type(self, errstream, car(expr), context);
      break;

    default: /* unknown kind of an expression */
      {
        const MasterPrinter_ptr sexpprint =
          MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_SEXP_PRINTER));

        StreamMgr_print_error(streams,  "\nchecker_statement_viol_handler: expression ");
        StreamMgr_nprint_error(streams, sexpprint, "%N", expr);
        ErrorMgr_internal_error(errmgr, "\nUnknown kind of expression");
      }
    } /* switch (node_get_type(expr)) */

    StreamMgr_print_error(streams, "\n");
    break;

  default:
    error_unreachable_code(); /* unknown kind of an error */
  } /* switch (errorKind) */

  return isError;
}



/**Static function*************************************************************

  Synopsis           [Just prints an expression's operator to output_stream]

  Description        [This function is the almost the same as
  print_sexp, except this function does not print the children of the node.
  The expr must be a correct expression.
  The function is used in printing of an error messages only.]

  SideEffects        []

  SeeAlso            [print_sexp]
******************************************************************************/

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
static void print_operator(FILE* output_stream, node_ptr expr)
{
  nusmv_assert((node_ptr) Nil != expr);
  switch(node_get_type(expr)){

  case TRANS:     fprintf(output_stream,"TRANS"); return;
  case INIT:      fprintf(output_stream,"INIT"); return;
  case INVAR:     fprintf(output_stream,"INVAR"); return;
  case ASSIGN:    fprintf(output_stream,"ASSIGN"); return;
  case FAIRNESS:  fprintf(output_stream,"FAIRNESS"); return;
  case JUSTICE:      fprintf(output_stream,"JUSTICE"); return;
  case COMPASSION:   fprintf(output_stream,"COMPASSION"); return;
  case SPEC:      fprintf(output_stream,"SPEC"); return;
  case LTLSPEC:   fprintf(output_stream,"LTLSPEC"); return;
  case PSLSPEC:   fprintf(output_stream,"PSLSPEC"); return;
  case INVARSPEC: fprintf(output_stream,"INVARSPEC"); return;
  case COMPUTE:   fprintf(output_stream,"COMPUTE"); return;
  case DEFINE:    fprintf(output_stream,"\n(DEFINE "); return;
  case ISA:    fprintf(output_stream,"\n(ISA "); return;
  case CONSTRAINT:    fprintf(output_stream,"\nCONSTRAINT "); return;
  case MODULE:    fprintf(output_stream,"\n(MODULE "); return;
  case ATTIME:    fprintf(output_stream,"\nATTIME "); return;

  /* PROCESS is artificial contract to check 'running' symbols */
  case PROCESS: fprintf(output_stream,"'running'"); return;

  case MODTYPE:   fprintf(output_stream,"\n(MODTYPE "); return;
  case LAMBDA:    fprintf(output_stream,"\n(LAMBDA "); return;


  default:
    error_unreachable_code_msg("\n%d\n", node_get_type(expr));
  }

}


/**AutomaticEnd***************************************************************/

