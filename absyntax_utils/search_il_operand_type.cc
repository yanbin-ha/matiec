/*
 * (c) 2003 Mario de Sousa
 *
 * Offered to the public under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * This code is made available on the understanding that it will not be
 * used in safety-critical situations without a full and competent review.
 */

/*
 * An IEC 61131-3 IL and ST compiler.
 *
 * Based on the
 * FINAL DRAFT - IEC 61131-3, 2nd Ed. (2001-12-10)
 *
 */




/* Returns the data type of an il_operand.
 *
 * Note that the il_operand may be a variable, in which case
 * we return the type of the variable instance.
 * The il_operand may also be a constant, in which case
 * we return the data type of that constant.
 *
 * The variable instance may be a member of a structured variable,
 * or an element in an array, or any combination of the two.
 *
 * The class constructor must be given the search scope
 * (function, function block or program within which
 * the possible il_operand variable instance was declared).
 */

#include "absyntax_utils.hh"

symbol_c *search_il_operand_type_c::get_type(symbol_c *il_operand) {
  symbol_c *res;

  /* We first assume that it is a constant... */
  res = search_constant_type.get_type(il_operand);
  if (res != NULL) return res;

  /* Nope, now we assume it is a variable, and determine its type... */
  res = search_varfb_instance_type.get_type(il_operand);
  if (NULL != res) return res;

  /* not found */
  return NULL;
}
