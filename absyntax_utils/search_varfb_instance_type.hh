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

class search_varfb_instance_type_c: public search_base_type_c {

  private:
    search_var_instance_decl_c search_var_instance_decl;
    decompose_var_instance_name_c *decompose_var_instance_name;
    symbol_c *current_structelement_name;
    symbol_c *current_rawtype;
    bool is_complex;

  public:
    search_varfb_instance_type_c(symbol_c *search_scope);
    symbol_c *get_type(symbol_c *variable_name);
    symbol_c *get_rawtype(symbol_c *variable_name);

    unsigned int get_vartype(symbol_c *variable_name);
    bool type_is_complex(void);

  private:
    /* a helper function... */
    void *visit_list(list_c *list);

    /* a helper function... */
    void *base_type(symbol_c *symbol);


  private:
    /* We override the base class' visitor to identifier_c.
     * This is so because the base class does not consider a function block
     * to be a type, unlike this class that allows a variable instance
     * of a function block type...
     */
    void *visit(identifier_c *type_name);

    /********************************/
    /* B 1.3.3 - Derived data types */
    /********************************/
    
    /*  identifier ':' array_spec_init */
    void *visit(array_type_declaration_c *symbol);
    
    /* array_specification [ASSIGN array_initialization} */
    /* array_initialization may be NULL ! */
    void *visit(array_spec_init_c *symbol);
    
    /* ARRAY '[' array_subrange_list ']' OF non_generic_type_name */
    void *visit(array_specification_c *symbol);

    /*  structure_type_name ':' structure_specification */
    void *visit(structure_type_declaration_c *symbol);

    /* structure_type_name ASSIGN structure_initialization */
    /* structure_initialization may be NULL ! */
    // SYM_REF2(initialized_structure_c, structure_type_name, structure_initialization)
    void *visit(initialized_structure_c *symbol);

    /* helper symbol for structure_declaration */
    /* structure_declaration:  STRUCT structure_element_declaration_list END_STRUCT */
    /* structure_element_declaration_list structure_element_declaration ';' */
    void *visit(structure_element_declaration_list_c *symbol);

    /*  structure_element_name ':' spec_init */
    void *visit(structure_element_declaration_c *symbol);

    /* helper symbol for structure_initialization */
    /* structure_initialization: '(' structure_element_initialization_list ')' */
    /* structure_element_initialization_list ',' structure_element_initialization */
    void *visit(structure_element_initialization_list_c *symbol); /* should never get called... */
    /*  structure_element_name ASSIGN value */
    void *visit(structure_element_initialization_c *symbol); /* should never get called... */



    /**************************************/
    /* B.1.5 - Program organization units */
    /**************************************/
    /*****************************/
    /* B 1.5.2 - Function Blocks */
    /*****************************/
    /*  FUNCTION_BLOCK derived_function_block_name io_OR_other_var_declarations function_block_body END_FUNCTION_BLOCK */
    // SYM_REF4(function_block_declaration_c, fblock_name, var_declarations, fblock_body, unused)
    void *visit(function_block_declaration_c *symbol);

}; // search_varfb_instance_type_c






