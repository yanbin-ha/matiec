/*
 *  Copyright (C) 2003-2011  Mario de Sousa (msousa@fe.up.pt)
 *  Copyright (C) 2007-2011  Laurent Bessard and Edouard Tisserant
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


/*
 * This file contains the code that stores the output generated
 * by each specific version of the 4th stage.
 */






// #include <stdio.h>  /* required for NULL */
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "stage4.hh"






stage4out_c::stage4out_c(std::string indent_level):
	m_file(NULL) {
  out = &std::cout;
  this->indent_level = indent_level;
  this->indent_spaces = "";
}

stage4out_c::stage4out_c(const char *dir, const char *radix, const char *extension, std::string indent_level) {	
  std::string filename(radix);
  filename += ".";
  filename += extension;
  std::string filepath("");
  if (dir != NULL) {
    filepath += dir;
    filepath += "/";
  }
  filepath += filename;
  std::fstream *file = new std::fstream(filepath.c_str(), std::fstream::out);
  if(file->fail()){
    std::cerr << "Cannot open " << filename << " for write access \n";
    exit(EXIT_FAILURE);
  }else{
    std::cout << filename << "\n";
  }
  out = file;
  m_file = file;
  this->indent_level = indent_level;
  this->indent_spaces = "";
  allow_output = true;
}

stage4out_c::~stage4out_c(void) {
  if(m_file)
  {
    m_file->close();
    delete m_file;
  }
}

void stage4out_c::flush(void) {
  out->flush();
}

void stage4out_c::enable_output(void) {
  allow_output = true;
}
    
void stage4out_c::disable_output(void) {
  allow_output = false;
}

void stage4out_c::indent_right(void) {
  indent_spaces+=indent_level;
}

void stage4out_c::indent_left(void) {
  if (indent_spaces.length() >= indent_level.length())
    indent_spaces.erase(indent_spaces.length() - indent_level.length(), indent_level.length());
  else
    indent_spaces.erase();
}


void *stage4out_c::print(const char *str) {
  if (!allow_output) return NULL;
  *out << str;
  return NULL;
}

void *stage4out_c::print_integer(int integer) {
  if (!allow_output) return NULL;
  *out << integer;
  return NULL;
}

void *stage4out_c::print_long_integer(unsigned long l_integer) {
  if (!allow_output) return NULL;
  *out << l_integer << "UL";
  return NULL;
}

void *stage4out_c::print_long_long_integer(unsigned long long ll_integer) {
  if (!allow_output) return NULL;
  *out << ll_integer << "ULL";
  return NULL;
}

void *stage4out_c::printupper(const char *str) {
  if (!allow_output) return NULL;
  for (int i = 0; str[i] != '\0'; i++)
    *out << (unsigned char)toupper(str[i]);
  return NULL;
}

void *stage4out_c::printlocation(const char *str) {
  if (!allow_output) return NULL;
  *out << "__";
  for (int i = 0; str[i] != '\0'; i++)
    if(str[i] == '.')
      *out << '_';
    else
      *out << (unsigned char)toupper(str[i]);
  return NULL;
}

void *stage4out_c::printlocation_comasep(const char *str) {
  if (!allow_output) return NULL;
  *out << (unsigned char)toupper(str[0]);
  *out << ',';
  *out << (unsigned char)toupper(str[1]);
  *out << ',';
  for (int i = 2; str[i] != '\0'; i++)
    if(str[i] == '.')
      *out << ',';
    else
      *out << (unsigned char)toupper(str[i]);
  return NULL;
}


void *stage4out_c::print(std::string str) {
  if (!allow_output) return NULL;
  *out << str;
  return NULL;
}


void *stage4out_c::printupper(std::string str) {
  if (!allow_output) return NULL;
  /* The string standard class does not have a converter member function to upper case.
   * We have to do it ourselves, a character at a time...
   */
#if 0
  /* The C++ way of doint things... */
  for (string::const_iterator p = str.begin(); p != str.end(); ++p)
    *out << (unsigned char)toupper(*p);
#else
  /* Or more simply... */
    printupper(str.c_str());
#endif
  return NULL;
}


void *stage4out_c::printlocation(std::string str) {
  if (!allow_output) return NULL;
  return printlocation(str.c_str());
}


/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/



/* forward declarations... */
/* These functions will be implemented in generate_XXX.cc */
visitor_c *new_code_generator(stage4out_c *s4o, const char *builddir);
void delete_code_generator(visitor_c *code_generator);


int stage4(symbol_c *tree_root, const char *builddir) {
  stage4out_c s4o;
  visitor_c *generate_code = new_code_generator(&s4o, builddir);

  if (NULL == generate_code)
    return -1;

  tree_root->accept(*generate_code);

  delete_code_generator(generate_code);

  return 0;
}

