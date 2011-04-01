/*
 *  matiec - a compiler for the programming languages defined in IEC 61131-3
 *
 *  Copyright (C) 2003-2011  Mario de Sousa (msousa@fe.up.pt)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This code is made available on the understanding that it will not be
 * used in safety-critical situations without a full and competent review.
 */

/*
 * An IEC 61131-3 compiler.
 *
 * Based on the
 * FINAL DRAFT - IEC 61131-3, 2nd Ed. (2001-12-10)
 *
 */

/* Determine the data type of a variable.
 * The variable may be a simple variable, a function block instance, a
 * struture element within a data structured type (a struct or a fb), or
 * an array element.
 * A mixture of array element of a structure element of a structure element
 * of a .... is also suported!
 *
 * A reference to the relevant base type __definition__ is returned.
 * This means that if we find that the variable is of type MY_INT,
 * which was previously declared to be
 * TYPE MY_INT: INT := 9;
 * this class wil return INT, and __not__ MY_INT !!
 *
 *
 *  example:
 *    window.points[1].coordinate.x
 *    window.points[1].colour
 *    etc... ARE ALLOWED!
 *
 * This class must be passed the scope within which the
 * variable was declared, and the variable name...
 */


/*
 * TODO: this code has a memory leak...
 *       We call 'new' in several locations, but bever get to 'delete' the object instances...
 */
#include "absyntax_utils.hh"


search_varfb_instance_type_c::search_varfb_instance_type_c(symbol_c *search_scope): search_var_instance_decl(search_scope) {
  this->decompose_var_instance_name = NULL;
  this->current_structelement_name = NULL;
  this->current_rawtype = NULL;
}

symbol_c *search_varfb_instance_type_c::get_type(symbol_c *variable_name) {
  this->current_structelement_name = NULL;
  this->current_rawtype = NULL;
  this->decompose_var_instance_name = new decompose_var_instance_name_c(variable_name);
  if (NULL == decompose_var_instance_name) ERROR;

  /* find the part of the variable name that will appear in the
   * variable declaration, for e.g., in window.point.x, this would be
   * window!
   */
  symbol_c *var_name_part = decompose_var_instance_name->next_part();
  if (NULL == var_name_part) ERROR;

  /* Now we try to find the variable instance declaration, to determine its type... */
  symbol_c *var_decl = search_var_instance_decl.get_decl(var_name_part);
  if (NULL == var_decl) {
    /* variable instance declaration not found! */
      ERROR;
  }

  /* if it is a struct or function block, we must search the type
   * of the struct or function block member.
   * This is done by this class visiting the var_decl.
   * This class, while visiting, will recursively call
   * decompose_var_instance_name->get_next() when and if required...
   */
  symbol_c *res = (symbol_c *)var_decl->accept(*this);
  if (NULL == res) ERROR;

  /* make sure that we have decomposed all structure elements of the variable name */
  symbol_c *var_name = decompose_var_instance_name->next_part();
  if (NULL != var_name) ERROR;

  return res;
}

unsigned int search_varfb_instance_type_c::get_vartype(symbol_c *variable_name) {
  this->current_structelement_name = NULL;
  this->current_rawtype = NULL;
  this->is_complex = false;
  this->decompose_var_instance_name = new decompose_var_instance_name_c(variable_name);
  if (NULL == decompose_var_instance_name) ERROR;

  /* find the part of the variable name that will appear in the
   * variable declaration, for e.g., in window.point.x, this would be
   * window!
   */
  symbol_c *var_name_part = decompose_var_instance_name->next_part();
  if (NULL == var_name_part) ERROR;

  /* Now we try to find the variable instance declaration, to determine its type... */
  symbol_c *var_decl = search_var_instance_decl.get_decl(var_name_part);
  if (NULL == var_decl) {
    /* variable instance declaration not found! */
    return 0;
  }

  /* if it is a struct or function block, we must search the type
   * of the struct or function block member.
   * This is done by this class visiting the var_decl.
   * This class, while visiting, will recursively call
   * decompose_var_instance_name->get_next() when and if required...
   */
  var_decl->accept(*this);
  unsigned int res = search_var_instance_decl.get_vartype();
  
  /* make sure that we have decomposed all structure elements of the variable name */
  symbol_c *var_name = decompose_var_instance_name->next_part();
  if (NULL != var_name) ERROR;

  return res;
}

symbol_c *search_varfb_instance_type_c::get_rawtype(symbol_c *variable_name) {
  symbol_c *rawtype = this->get_type(variable_name);
  if (this->current_rawtype != NULL)
    return this->current_rawtype;
  else
	return rawtype;
}

bool search_varfb_instance_type_c::type_is_complex(void) {
  return this->is_complex;
}

/* a helper function... */
void *search_varfb_instance_type_c::visit_list(list_c *list)	{
  if (NULL == current_structelement_name) ERROR;

  for(int i = 0; i < list->n; i++) {
    void *res = list->elements[i]->accept(*this);
    if (res != NULL)
      return res;
  }
  /* not found! */
  return NULL;
}

/* a helper function... */
void *search_varfb_instance_type_c::base_type(symbol_c *symbol)	{
    search_base_type_c search_base_type;
    return symbol->accept(search_base_type);
}

/* We override the base class' visitor to identifier_c.
 * This is so because the base class does not consider a function block
 * to be a type, unlike this class that allows a variable instance
 * of a function block type...
 */
void *search_varfb_instance_type_c::visit(identifier_c *type_name) {
  /* look up the type declaration... */
  symbol_c *fb_decl = function_block_type_symtable.find_value(type_name);
  if (fb_decl != function_block_type_symtable.end_value())
    /* Type declaration found!! */
    return fb_decl->accept(*this);

  this->current_rawtype = type_name;

  /* No. It is not a function block, so we let
   * the base class take care of it...
   */
  if (NULL == decompose_var_instance_name->next_part(false)) {
    return base_type(type_name);
  }
  else {
	return search_base_type_c::visit(type_name);
  }
}

/********************************/
/* B 1.3.3 - Derived data types */
/********************************/

/*  identifier ':' array_spec_init */
void *search_varfb_instance_type_c::visit(array_type_declaration_c *symbol) {
  this->is_complex = true;
  return symbol->array_spec_init->accept(*this);
}
    
/* array_specification [ASSIGN array_initialization] */
/* array_initialization may be NULL ! */
void *search_varfb_instance_type_c::visit(array_spec_init_c *symbol) {
  this->is_complex = true;
  return symbol->array_specification->accept(*this);
}

/* ARRAY '[' array_subrange_list ']' OF non_generic_type_name */
void *search_varfb_instance_type_c::visit(array_specification_c *symbol) {
  this->is_complex = true;
  return symbol->non_generic_type_name->accept(*this);
}

/*  structure_type_name ':' structure_specification */
void *search_varfb_instance_type_c::visit(structure_type_declaration_c *symbol) {
  this->is_complex = true;
  return symbol->structure_specification->accept(*this);
  /* NOTE: structure_specification will point to either a
   *       initialized_structure_c
   *       OR A
   *       structure_element_declaration_list_c
   */
}

/* structure_type_name ASSIGN structure_initialization */
/* structure_initialization may be NULL ! */
// SYM_REF2(initialized_structure_c, structure_type_name, structure_initialization)
void *search_varfb_instance_type_c::visit(initialized_structure_c *symbol)	{
  this->is_complex = true;
  /* recursively find out the data type of var_name... */
  return symbol->structure_type_name->accept(*this);
}

/* helper symbol for structure_declaration */
/* structure_declaration:  STRUCT structure_element_declaration_list END_STRUCT */
/* structure_element_declaration_list structure_element_declaration ';' */
void *search_varfb_instance_type_c::visit(structure_element_declaration_list_c *symbol)	{
  /* make sure that we have decomposed all structure elements of the variable name */
  current_structelement_name = decompose_var_instance_name->next_part();
  /* now search the structure declaration */
  return visit_list(symbol);
}

/*  structure_element_name ':' spec_init */
void *search_varfb_instance_type_c::visit(structure_element_declaration_c *symbol) {
  if (NULL == current_structelement_name) ERROR;

  if (compare_identifiers(symbol->structure_element_name, current_structelement_name) == 0)
    return symbol->spec_init->accept(*this);

  return NULL;
}

/* helper symbol for structure_initialization */
/* structure_initialization: '(' structure_element_initialization_list ')' */
/* structure_element_initialization_list ',' structure_element_initialization */
void *search_varfb_instance_type_c::visit(structure_element_initialization_list_c *symbol) {ERROR; return NULL;} /* should never get called... */
/*  structure_element_name ASSIGN value */
void *search_varfb_instance_type_c::visit(structure_element_initialization_c *symbol) {ERROR; return NULL;} /* should never get called... */



/**************************************/
/* B.1.5 - Program organization units */
/**************************************/
/*****************************/
/* B 1.5.2 - Function Blocks */
/*****************************/
/*  FUNCTION_BLOCK derived_function_block_name io_OR_other_var_declarations function_block_body END_FUNCTION_BLOCK */
// SYM_REF4(function_block_declaration_c, fblock_name, var_declarations, fblock_body, unused)
void *search_varfb_instance_type_c::visit(function_block_declaration_c *symbol) {
  /* make sure that we have decomposed all strcuture elements of the variable name */

  symbol_c *var_name = decompose_var_instance_name->next_part();
  if (NULL == var_name) {
    /* this is it... !
     * No need to look any further...
     * Note also that, unlike for the struct types, a function block may
     * not be defined based on another (i.e. no inheritance is allowed),
     * so this function block is already the most base type.
     * We simply return it.
     */
    return (void *)symbol;
   }

   /* now search the function block declaration for the variable... */
   search_var_instance_decl_c search_decl(symbol);
   symbol_c *var_decl = search_decl.get_decl(var_name);
   if (NULL == var_decl) {
     /* variable instance declaration not found! */
     return NULL;
   }

   /* We have found the declaration.
    * Should we look any further?
    */
   var_name = decompose_var_instance_name->next_part();
   if (NULL == var_name) {
     /* this is it... ! */
     return base_type(var_decl);
   }

  current_structelement_name = var_name;
  /* recursively find out the data type of var_name... */
  return symbol->var_declarations->accept(*this);
}
