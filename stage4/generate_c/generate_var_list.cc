/*
 * (c) 2007 Mario de Sousa and Laurent Bessard
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

/*
 * This is one of the versions available for the 4th stage.
 *
 * This 4th stage generates a c++ source program equivalent
 * to the IL and ST code.
 */

//#include <stdio.h>  /* required for NULL */
//#include <string>
//#include <iostream>

//#include "../../util/symtable.hh"




typedef struct
{
  symbol_c *symbol;
} SYMBOL;




/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

class generate_var_list_c: protected generate_c_typedecl_c {
  
  public:
    typedef enum {
      none_dt,
      programs_dt,
      variables_dt
    } declarationtype_t;

    declarationtype_t current_declarationtype;
    
    typedef enum {
      none_vtc,
      variable_vtc,
      pointer_vtc,
      array_vtc,
      structure_vtc,
      function_block_vtc
    } vartypecategory_t;
    
    vartypecategory_t current_var_type_category;
    
  private:
    symbol_c *current_var_type_symbol;
    symbol_c *current_var_type_name;
    unsigned int current_var_number;
    unsigned int step_number;
    unsigned int transition_number;
    unsigned int action_number;
    bool configuration_defined;
    std::list<SYMBOL> current_symbol_list;
    search_base_type_c search_base_type;
    search_fb_typedecl_c *search_fb_typedecl;
    
  public:
    generate_var_list_c(stage4out_c *s4o_ptr, symbol_c *scope)
    : generate_c_typedecl_c(s4o_ptr) {
      search_fb_typedecl = new search_fb_typedecl_c(scope);
      current_var_number = 0;
      current_var_type_symbol = current_var_type_name = NULL;
      current_declarationtype = none_dt;
      current_var_type_category = none_vtc;
    }
    
    ~generate_var_list_c(void) {
      delete search_fb_typedecl;
    }
    
    void update_var_type_symbol(symbol_c *symbol) {
      
      this->current_var_type_name = spec_init_sperator_c::get_spec(symbol);
      if (this->current_var_type_name == NULL) {
        std::list<SYMBOL>::iterator pt;
        for(pt = current_symbol_list.begin(); pt != current_symbol_list.end(); pt++) {
          fprintf(stderr, "%s.", ((identifier_c*)(pt->symbol))->value);
        }
        ERROR;
      }
      
      this->current_var_type_symbol = search_fb_typedecl->get_decl(this->current_var_type_name);
      if (this->current_var_type_symbol != NULL)
        this->current_var_type_category = function_block_vtc;
      else {
        this->current_var_type_symbol = (symbol_c *)(this->current_var_type_name->accept(search_base_type));
        
        structure_element_declaration_list_c *structure_symbol = dynamic_cast<structure_element_declaration_list_c *>(this->current_var_type_symbol);
        if (structure_symbol != NULL)
          this->current_var_type_category = structure_vtc;
        else
          this->current_var_type_category = variable_vtc;
      }
      
      if (this->current_var_type_symbol == NULL)
        ERROR;
    }

    void reset_var_type_symbol(void) {
      this->current_var_type_symbol = NULL;
    }
    
    void generate_programs(symbol_c *symbol) {
      s4o.print("// Programs\n");
      current_var_number = 0;
      configuration_defined = false;
      current_declarationtype = programs_dt;
      symbol->accept(*this);
      current_declarationtype = none_dt;
      s4o.print("\n");
    }
    
    void generate_variables(symbol_c *symbol) {
      s4o.print("// Variables\n");
      current_var_number = 0;
      configuration_defined = false;
      current_declarationtype = variables_dt;
      symbol->accept(*this);
      current_declarationtype = none_dt;
      s4o.print("\n");
    }
    
    void declare_variables(symbol_c *symbol) {
      list_c *list = dynamic_cast<list_c *>(symbol);
      /* should NEVER EVER occur!! */
      if (list == NULL) ERROR;

      for(int i = 0; i < list->n; i++) {
        declare_variable(list->elements[i]);
      }
    }
    
    void declare_variable(symbol_c *symbol) {
      print_var_number();
      s4o.print(";");
      switch (this->current_var_type_category) {
        case pointer_vtc:
          s4o.print("PT");
          break;
        case array_vtc:
          s4o.print("ARRAY");
          break;
        case structure_vtc:
          s4o.print("STRUCT");
          break;
        case function_block_vtc:
          s4o.print("FB");
          break;
        default:
          s4o.print("VAR");
          break;
      }
      s4o.print(";");
      print_symbol_list();
      symbol->accept(*this);
      s4o.print(";");
      print_symbol_list();
      symbol->accept(*this);
      s4o.print(";");
      switch (this->current_var_type_category) {
        case structure_vtc:
        case function_block_vtc:
          this->current_var_type_name->accept(*this);
          s4o.print(";\n");
          SYMBOL *current_name;
          current_name = new SYMBOL;
          current_name->symbol = symbol;
          current_symbol_list.push_back(*current_name);
          this->current_var_type_symbol->accept(*this);
          current_symbol_list.pop_back();
          break;
        case array_vtc:
          this->current_var_type_name->accept(*this);
          s4o.print(";\n");
          break;
        default:
          this->current_var_type_symbol->accept(*this);
          s4o.print(";\n");
          break;
      }
    }
    
    void print_var_number(void) {
      char str[10];
      sprintf(str, "%d", current_var_number);
      s4o.print(str);
      current_var_number++;
    }
        
    void print_step_number(void) {
      char str[10];
      sprintf(str, "%d", step_number);
      s4o.print(str);
    }
    
    void print_transition_number(void) {
      char str[10];
      sprintf(str, "%d", transition_number);
      s4o.print(str);
    }
    
    void print_action_number(void) {
      char str[10];
      sprintf(str, "%d", action_number);
      s4o.print(str);
    }
    
    void print_symbol_list(void) {
      std::list<SYMBOL>::iterator pt;
      for(pt = current_symbol_list.begin(); pt != current_symbol_list.end(); pt++) {
        pt->symbol->accept(*this);
        s4o.print(".");
      }
    }

/********************************************/
/* B.1.4.3 - Declaration and initialization */
/********************************************/
    
    /*  [variable_name] location ':' located_var_spec_init */
    /* variable_name -> may be NULL ! */
    //SYM_REF4(located_var_decl_c, variable_name, location, located_var_spec_init, unused)
    void *visit(located_var_decl_c *symbol) {
        /* Start off by setting the current_var_type_symbol and
         * current_var_init_symbol private variables...
         */
        update_var_type_symbol(symbol->located_var_spec_init);
        
        if (symbol->variable_name != NULL) {
          this->current_var_type_category = pointer_vtc;
          declare_variable(symbol->variable_name);
        }
        
        current_var_type_symbol = NULL;
        return NULL;
    }
    
    /*  var1_list ':' array_spec_init */
    // SYM_REF2(array_var_init_decl_c, var1_list, array_spec_init)
    void *visit(array_var_init_decl_c *symbol) {
      TRACE("array_var_init_decl_c");
      /* Start off by setting the current_var_type_symbol and
       * current_var_init_symbol private variables...
       */
      update_var_type_symbol(symbol->array_spec_init);
      
      this->current_var_type_category = array_vtc;
      //declare_variables(symbol->var1_list);
    
      /* Values no longer in scope, and therefore no longer used.
       * Make an effort to keep them set to NULL when not in use
       * in order to catch bugs as soon as possible...
       */
      reset_var_type_symbol();
    
      return NULL;
    }

    /*  var1_list ':' initialized_structure */
    // SYM_REF2(structured_var_init_decl_c, var1_list, initialized_structure)
    void *visit(structured_var_init_decl_c *symbol) {
      TRACE("structured_var_init_decl_c");
      /* Please read the comments inside the var1_init_decl_c
       * visitor, as they apply here too.
       */
    
      /* Start off by setting the current_var_type_symbol and
       * current_var_init_symbol private variables...
       */
      update_var_type_symbol(symbol->initialized_structure);
    
      /* now to produce the c equivalent... */
      //declare_variables(symbol->var1_list);
    
      /* Values no longer in scope, and therefore no longer used.
       * Make an effort to keep them set to NULL when not in use
       * in order to catch bugs as soon as possible...
       */
      reset_var_type_symbol();
    
      return NULL;
    }
    
    /* fb_name_list ':' function_block_type_name ASSIGN structure_initialization */
    /* structure_initialization -> may be NULL ! */
    void *visit(fb_name_decl_c *symbol) {
      TRACE("fb_name_decl_c");
      /* Please read the comments inside the var1_init_decl_c
       * visitor, as they apply here too.
       */
    
      /* Start off by setting the current_var_type_symbol and
       * current_var_init_symbol private variables...
       */
      update_var_type_symbol(symbol);
    
      /* now to produce the c equivalent... */
      declare_variables(symbol->fb_name_list);
    
      /* Values no longer in scope, and therefore no longer used.
       * Make an effort to keep them set to NULL when not in use
       * in order to catch bugs as soon as possible...
       */
      reset_var_type_symbol();
    
      return NULL;
    }

    /*  global_var_name ':' (simple_specification|subrange_specification|enumerated_specification|array_specification|prev_declared_structure_type_name|function_block_type_name */
    //SYM_REF2(external_declaration_c, global_var_name, specification)
    void *visit(external_declaration_c *symbol) {
      TRACE("external_declaration_c");
      /* Please read the comments inside the var1_init_decl_c
       * visitor, as they apply here too.
       */
    
      /* Start off by setting the current_var_type_symbol and
       * current_var_init_symbol private variables...
       */
      update_var_type_symbol(symbol->specification);
      
      /* now to produce the c equivalent... */
      if (this->current_var_type_category == variable_vtc)
        this->current_var_type_category = pointer_vtc;
      declare_variable(symbol->global_var_name);
      
      /* Values no longer in scope, and therefore no longer used.
       * Make an effort to keep them set to NULL when not in use
       * in order to catch bugs as soon as possible...
       */
      reset_var_type_symbol();
    
      return NULL;
    }

    /*| global_var_spec ':' [located_var_spec_init|function_block_type_name] */
    /* type_specification ->may be NULL ! */
    // SYM_REF2(global_var_decl_c, global_var_spec, type_specification)
    void *visit(global_var_decl_c *symbol) {
      TRACE("global_var_decl_c");
      /* Please read the comments inside the var1_init_decl_c
       * visitor, as they apply here too.
       */
    
      /* Start off by setting the current_var_type_symbol and
       * current_var_init_symbol private variables...
       */
      update_var_type_symbol(symbol->type_specification);
    
      /* now to produce the c equivalent... */
      symbol->global_var_spec->accept(*this);
    
      /* Values no longer in scope, and therefore no longer used.
       * Make an effort to keep them set to NULL when not in use
       * in order to catch bugs as soon as possible...
       */
      reset_var_type_symbol();
    
      return NULL;
    }
    
    void *visit(global_var_list_c *symbol) {
      declare_variables(symbol);
      return NULL;
    }
    
    /*| global_var_name location */
    // SYM_REF2(global_var_spec_c, global_var_name, location)
    void *visit(global_var_spec_c *symbol) {
      if (symbol->global_var_name != NULL) {
        this->current_var_type_category = pointer_vtc;
        declare_variable(symbol->global_var_name);
      }
      return NULL;
    }
    
    void *visit(var1_init_decl_c *symbol) {
      TRACE("var1_init_decl_c");
    
      /* Start off by setting the current_var_type_symbol and
       * current_var_init_symbol private variables...
       */
      update_var_type_symbol(symbol->spec_init);
    
      /* now to produce the c equivalent... */
      declare_variables(symbol->var1_list);
    
      /* Values no longer in scope, and therefore no longer used.
       * Make an effort to keep them set to NULL when not in use
       * in order to catch bugs as soon as possible...
       */
      reset_var_type_symbol();
    
      return NULL;
    }

    void *visit(en_param_declaration_c *symbol) {
      TRACE("en_param_declaration_c");

      /* Start off by setting the current_var_type_symbol and
       * current_var_init_symbol private variables...
       */
      this->current_var_type_symbol = symbol->type;

      /* now to produce the c equivalent... */
      declare_variable(symbol->name);

      /* Values no longer in scope, and therefore no longer used.
       * Make an effort to keep them set to NULL when not in use
       * in order to catch bugs as soon as possible...
       */
      reset_var_type_symbol();

      return NULL;
    }

    void *visit(eno_param_declaration_c *symbol) {
      TRACE("eno_param_declaration_c");

      /* Start off by setting the current_var_type_symbol and
       * current_var_init_symbol private variables...
       */
      this->current_var_type_symbol = symbol->type;

      /* now to produce the c equivalent... */
      declare_variable(symbol->name);

      /* Values no longer in scope, and therefore no longer used.
       * Make an effort to keep them set to NULL when not in use
       * in order to catch bugs as soon as possible...
       */
      reset_var_type_symbol();

      return NULL;
    }

/********************************/
/* B 1.3.3 - Derived data types */
/********************************/
    void *visit(data_type_declaration_c *symbol) {
      return NULL;
    }

    void *visit(structure_element_declaration_list_c *symbol) {
      for(int i = 0; i < symbol->n; i++) {
        symbol->elements[i]->accept(*this);
      }
      return NULL;
    }

    void *visit(structure_element_declaration_c *symbol) {
      /* Start off by setting the current_var_type_symbol and
       * current_var_init_symbol private variables...
       */
      update_var_type_symbol(symbol->spec_init);
      
      /* now to produce the c equivalent... */
      declare_variable(symbol->structure_element_name);
      
      /* Values no longer in scope, and therefore no longer used.
       * Make an effort to keep them set to NULL when not in use
       * in order to catch bugs as soon as possible...
       */
      reset_var_type_symbol();
      
      return NULL;
    }

/**************************************/
/* B.1.5 - Program organization units */
/**************************************/

/***********************/
/* B 1.5.1 - Functions */
/***********************/
    void *visit(function_declaration_c *symbol) {
      return NULL;
    }

/*****************************/
/* B 1.5.2 - Function Blocks */
/*****************************/
    void *visit(function_block_declaration_c *symbol) {
      if (current_declarationtype == variables_dt && configuration_defined) {
        symbol->var_declarations->accept(*this);
        symbol->fblock_body->accept(*this);
      }
      return NULL;
    }

/**********************/
/* B 1.5.3 - Programs */
/**********************/
    void *visit(program_declaration_c *symbol) {
      if (current_declarationtype == variables_dt && configuration_defined) {
        symbol->var_declarations->accept(*this);
        symbol->function_block_body->accept(*this);
      }
      return NULL;
    }

/**********************************************/
/* B 1.6 - Sequential function chart elements */
/**********************************************/
    
    /* | sequential_function_chart sfc_network */
    //SYM_LIST(sequential_function_chart_c)
    void *visit(sequential_function_chart_c *symbol) {
      step_number = 0;
      transition_number = 0;
      action_number = 0;
      for(int i = 0; i < symbol->n; i++) {
        symbol->elements[i]->accept(*this);
      }
      return NULL;
    }
    
    /* INITIAL_STEP step_name ':' action_association_list END_STEP */
    //SYM_REF2(initial_step_c, step_name, action_association_list)
    void *visit(initial_step_c *symbol) {
      print_var_number();
      s4o.print(";VAR;");
      print_symbol_list();
      symbol->step_name->accept(*this);
      s4o.print(".X;");
      print_symbol_list();
      s4o.print("__step_list[");
      print_step_number();
      s4o.print("].state;BOOL;\n");
      step_number++;
      return NULL;
    }
    
    /* STEP step_name ':' action_association_list END_STEP */
    //SYM_REF2(step_c, step_name, action_association_list)
    void *visit(step_c *symbol) {
      print_var_number();
      s4o.print(";VAR;");
      print_symbol_list();
      symbol->step_name->accept(*this);
      s4o.print(".X;");
      print_symbol_list();
      s4o.print("__step_list[");
      print_step_number();
      s4o.print("].state;BOOL;\n");
      step_number++;
      return NULL;
    }
    
    /* TRANSITION [transition_name] ['(' PRIORITY ASSIGN integer ')'] 
     *   FROM steps TO steps 
     *   transition_condition 
     * END_TRANSITION
     */
    /* transition_name -> may be NULL ! */
    /* integer -> may be NULL ! */
    //SYM_REF5(transition_c, transition_name, integer, from_steps, to_steps, transition_condition)
    void *visit(transition_c *symbol) {
      print_var_number();
      s4o.print(";VAR;");
      print_symbol_list();
      symbol->from_steps->accept(*this);
      s4o.print("->");
      symbol->to_steps->accept(*this);
      s4o.print(";");
      print_symbol_list();
      s4o.print("__debug_transition_list[");
      print_transition_number();
      s4o.print("];BOOL;\n");
      transition_number++;
      return NULL;
    }
    
    /* step_name | '(' step_name_list ')' */
    /* step_name      -> may be NULL ! */
    /* step_name_list -> may be NULL ! */
    //SYM_REF2(steps_c, step_name, step_name_list)
    void *visit(steps_c *symbol) {
      if (symbol->step_name != NULL)
        symbol->step_name->accept(*this);
      if (symbol->step_name_list != NULL)
        symbol->step_name_list->accept(*this);
      return NULL;
    }
    
    /* | step_name_list ',' step_name */
    //SYM_LIST(step_name_list_c)
    void *visit(step_name_list_c *symbol) {
      for(int i = 0; i < symbol->n; i++) {
        symbol->elements[i]->accept(*this);
        if (i < symbol->n - 1)
          s4o.print(",");
      }
      return NULL;
    }

    /* ACTION action_name ':' function_block_body END_ACTION */
    //SYM_REF2(action_c, action_name, function_block_body)
    void *visit(action_c *symbol) {
      print_var_number();
      s4o.print(";VAR;");
      print_symbol_list();
      symbol->action_name->accept(*this);
      s4o.print(".Q;");
      print_symbol_list();
      s4o.print("__action_list[");
      print_action_number();
      s4o.print("].state;BOOL;\n");
      action_number++;
      return NULL;
    }


/**************************************/
/* B.1.7 - Configuration elements     */
/**************************************/

    /*  PROGRAM [RETAIN | NON_RETAIN] program_name [WITH task_name] ':' program_type_name ['(' prog_conf_elements ')'] */
    //SYM_REF6(program_configuration_c, retain_option, program_name, task_name, program_type_name, prog_conf_elements, unused)
    void *visit(program_configuration_c *symbol) {
      
      switch (current_declarationtype) {
        case programs_dt:
          print_var_number();
          s4o.print(";");
          print_symbol_list();
          symbol->program_name->accept(*this);
          s4o.print(";");
          symbol->program_type_name->accept(*this);
          s4o.print(";\n");
          break;
        case variables_dt:
          /* Start off by setting the current_var_type_symbol and
           * current_var_init_symbol private variables...
           */
          update_var_type_symbol(symbol->program_type_name);
          
          declare_variable(symbol->program_name);
          
          /* Values no longer in scope, and therefore no longer used.
           * Make an effort to keep them set to NULL when not in use
           * in order to catch bugs as soon as possible...
           */
          reset_var_type_symbol();
          
          break;
        default:
          break;
      }
      
      return NULL;
    }

    /* CONFIGURATION configuration_name
     *   optional_global_var_declarations
     *   (resource_declaration_list | single_resource_declaration)
     *   optional_access_declarations
     *   optional_instance_specific_initializations
     * END_CONFIGURATION
     */
    //SYM_REF5(configuration_declaration_c, configuration_name, global_var_declarations, resource_declarations, access_declarations, instance_specific_initializations)
    void *visit(configuration_declaration_c *symbol) {
      SYMBOL *current_name;
      current_name = new SYMBOL;
      current_name->symbol = symbol->configuration_name;
      current_symbol_list.push_back(*current_name);
      configuration_defined = true;
      
      switch (current_declarationtype) {
        case variables_dt:
          if (symbol->global_var_declarations != NULL)
            symbol->global_var_declarations->accept(*this);
          break;
        default:
          break;
      }

      symbol->resource_declarations->accept(*this);
      current_symbol_list.pop_back();
      configuration_defined = false;
      return NULL;
    }
    
    /* RESOURCE resource_name ON resource_type_name
     *   optional_global_var_declarations
     *   single_resource_declaration
     * END_RESOURCE
     */
    //SYM_REF4(resource_declaration_c, resource_name, resource_type_name, global_var_declarations, resource_declaration)
    void *visit(resource_declaration_c *symbol) {
      SYMBOL *current_name;
      current_name = new SYMBOL;
      current_name->symbol = symbol->resource_name;
      current_symbol_list.push_back(*current_name);

      switch (current_declarationtype) {
        case variables_dt:
          if (symbol->global_var_declarations != NULL)
            symbol->global_var_declarations->accept(*this);
          break;
        default:
          break;
      }
      
      
      symbol->resource_declaration->accept(*this);
      
      current_symbol_list.pop_back();
      return NULL;
    }

    /* task_configuration_list program_configuration_list */
    //SYM_REF2(single_resource_declaration_c, task_configuration_list, program_configuration_list)
    void *visit(single_resource_declaration_c *symbol) {
      symbol->program_configuration_list->accept(*this);
      return NULL;
    }
    
    
/*************************************/
/* B 2.1 - Instructions and operands */
/*************************************/
    void *visit(instruction_list_c *symbol) {
      return NULL;
    }
    
/************************/
/* B 3.2 - Statements   */
/************************/
    void *visit(statement_list_c *symbol) {
      return NULL;
    }
    
};
