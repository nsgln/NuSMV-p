/* ---------------------------------------------------------------------------


  This file is part of the ``compile.type_checking'' package of NuSMV version 2.
  Copyright (C) 2000-2005 by  FBK-irst.

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
  \author Andrei Tchaltsev
  \brief The private interface of the type-checking package.

  This package contains the functions required to
  the type checking package internally

*/

#ifndef __NUSMV_CORE_COMPILE_TYPE_CHECKING_TYPE_CHECKING_INT_H__
#define __NUSMV_CORE_COMPILE_TYPE_CHECKING_TYPE_CHECKING_INT_H__

#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/type_checking/TypeChecker.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/type_checking/checkers/checkersInt.h"

#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/compile/compile.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/opt/opt.h" /* for options type_checking_... */
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/node/node.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
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


extern int nusmv_yylineno;


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

#endif /* __NUSMV_CORE_COMPILE_TYPE_CHECKING_TYPE_CHECKING_INT_H__ */
