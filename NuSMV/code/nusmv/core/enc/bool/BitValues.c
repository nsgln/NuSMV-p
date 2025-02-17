
/* ---------------------------------------------------------------------------


  This file is part of the ``enc.bool'' package of NuSMV version 2.
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
  \brief Implementation of class 'BitValues'

  \todo: Missing description

*/


#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/enc/bool/BitValues.h"

#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/enc/bool/BoolEnc.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/parser/symbols.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/wff/ExprMgr.h"
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

typedef struct BitValues_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  BoolEnc_ptr enc;
  node_ptr scalar_var;
  BitValue* values;
  NodeList_ptr bits; /* name of bits */
} BitValues;



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

static void bit_values_array_init(BitValues_ptr self,
                                  BoolEnc_ptr enc, node_ptr var);

static void bit_values_array_deinit(BitValues_ptr self);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

BitValues_ptr BitValues_create(BoolEnc_ptr enc, node_ptr var)
{
  BitValues_ptr self = ALLOC(BitValues, 1);
  BIT_VALUES_CHECK_INSTANCE(self);

  bit_values_array_init(self, enc, var);
  return self;
}

void BitValues_destroy(BitValues_ptr self)
{
  BIT_VALUES_CHECK_INSTANCE(self);

  bit_values_array_deinit(self);
  FREE(self);
}

size_t BitValues_get_size(const BitValues_ptr self)
{
  BIT_VALUES_CHECK_INSTANCE(self);
  return NodeList_get_length(self->bits);
}

node_ptr BitValues_get_scalar_var(const BitValues_ptr self)
{
  BIT_VALUES_CHECK_INSTANCE(self);
  return self->scalar_var;
}

NodeList_ptr BitValues_get_bits(const BitValues_ptr self)
{
  BIT_VALUES_CHECK_INSTANCE(self);
  return self->bits;
}

void BitValues_reset(BitValues_ptr self)
{
  unsigned int i;
  size_t size = BitValues_get_size(self);
  BIT_VALUES_CHECK_INSTANCE(self);
  for (i=0; i<size; ++i) self->values[i] = BIT_VALUE_DONTCARE;
}

void BitValues_set(BitValues_ptr self, size_t index, BitValue val)
{
  BIT_VALUES_CHECK_INSTANCE(self);
  nusmv_assert(index < BitValues_get_size(self));
  self->values[index] = val;
}

void BitValues_set_from_expr(BitValues_ptr self, size_t index, node_ptr expr)
{
  BIT_VALUES_CHECK_INSTANCE(self);
  BitValues_set(self, index, BitValues_get_value_from_expr(self, expr));
}

BitValue BitValues_get(const BitValues_ptr self, size_t index)
{
  BIT_VALUES_CHECK_INSTANCE(self);
  nusmv_assert(index < BitValues_get_size(self));
  return self->values[index];
}

void BitValues_set_from_values_list(BitValues_ptr self,
                                    const BoolEnc_ptr enc,
                                    node_ptr vals)
{
  SymbTable_ptr st;
  node_ptr iter;

  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));

  BIT_VALUES_CHECK_INSTANCE(self);

  st = BaseEnc_get_symb_table(BASE_ENC(enc));
  BitValues_reset(self); /* to set don't care for all unspecified bits */

  for (iter=vals; iter != Nil; iter=cdr(iter)) {
    node_ptr assgn;
    node_ptr bit, val;
    nusmv_assert(node_get_type(iter) == CONS);
    assgn = car(iter);
    nusmv_assert(node_get_type(assgn) == IFF || node_get_type(assgn) == EQUAL);

    if (BoolEnc_is_var_bit(enc, car(assgn))) {
      bit = car(assgn); val = cdr(assgn);
    }
    else if (BoolEnc_is_var_bit(enc, cdr(assgn))) {
      bit = cdr(assgn); val = car(assgn);
    }
    else error_unreachable_code(); /* there is no assignment */

    /* simplifies value */
    val = ExprMgr_simplify(exprs, st, val);

    BitValues_set(self, BoolEnc_get_index_from_bit(enc, bit),
                  BitValues_get_value_from_expr(self, val));
  }
}

BitValue BitValues_get_value_from_expr(const BitValues_ptr self, node_ptr expr)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self->enc));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));

  if (ExprMgr_is_true(exprs, expr)) return BIT_VALUE_TRUE;
  if (ExprMgr_is_false(exprs, expr)) return BIT_VALUE_FALSE;
  error_unreachable_code(); /* no other possible values! */
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The BitValues class private initializer

  The BitValues class private initializer

  \sa BitValues_create
*/
static void bit_values_array_init(BitValues_ptr self,
                                  BoolEnc_ptr enc, node_ptr var)
{ /* members initialization */
  self->enc = enc;
  self->bits = BoolEnc_get_var_bits(enc, var);
  self->scalar_var = var;
  self->values = ALLOC(BitValue, BitValues_get_size(self));
  nusmv_assert(self->values != (BitValue*) NULL);

  BitValues_reset(self);
}

/*!
  \brief The BitValues class private deinitializer

  The BitValues class private deinitializer

  \sa BitValues_destroy
*/
static void bit_values_array_deinit(BitValues_ptr self)
{
  /* members deinitialization */
  FREE(self->values);
  self->values = (BitValue*) NULL;
  NodeList_destroy(self->bits);
}



/**AutomaticEnd***************************************************************/

