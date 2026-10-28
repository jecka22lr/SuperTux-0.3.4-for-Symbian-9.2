//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <sstream>
#include <vector>
#include <string.h>
#include <stdexcept>
#include <tinygettext/tinygettext.hpp>
#include <physfs.h>

#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "util/obstackpp.hpp"
#include "physfs/ifile_stream.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "supertux/globals.hpp"

#include "supertux/gameconfig.hpp"

namespace lisp {

Parser::Parser(bool translate) :
  lexer(0), 
  filename(),
  dictionary_manager(0), 
  dictionary(0),
  token(),
  obst()
{
  if(translate) {
    dictionary_manager = new tinygettext::DictionaryManager();
    dictionary_manager->set_charset("UTF-8");
    if (g_config && (g_config->locale != "")) 
      dictionary_manager->set_language(tinygettext::Language::from_name(g_config->locale));
  }

  obstack_init(&obst);
}

Parser::~Parser()
{
  obstack_free(&obst, NULL);
  delete lexer;
  delete dictionary_manager;
}

static std::string dirname(const std::string& filename)
{
  std::string::size_type p = filename.find_last_of('/');
  if(p == std::string::npos)
    return "";

  return filename.substr(0, p+1);
}

const Lisp*
Parser::parse(const std::string& filename)
{
  IFileStreambuf ins(filename);
  std::istream in(&ins);

  if(!in.good()) {
    std::stringstream msg;
    msg << "Parser problem: Couldn't open file '" << filename << "'.";
    throw std::runtime_error(msg.str());
  }

  if(dictionary_manager) {
    std::string rel_dir = dirname (filename);
    char **searchpath = PHYSFS_getSearchPath();
    for(char** i = searchpath; *i != NULL; i++)
    {
      std::string abs_dir = std::string (*i) + PHYSFS_getDirSeparator () + rel_dir;
      dictionary_manager->add_directory (abs_dir);
    }
    dictionary = & (dictionary_manager->get_dictionary());
  }

  return parse(in, filename);
}

const Lisp*
Parser::parse(std::istream& stream, const std::string& sourcename)
{
  delete lexer;
  lexer = new Lexer(stream);

  this->filename = sourcename;
  token = lexer->getNextToken();

  Lisp* result = new(obst) Lisp(Lisp::TYPE_CONS);
  result->v.cons.car = read();
  result->v.cons.cdr = 0;

  delete lexer;
  lexer = 0;

  return result;
}

void
Parser::parse_error(const char* msg) const
{
  std::stringstream emsg;
  emsg << "Parse Error at '" << filename << "' line " << lexer->getLineNumber()
       << ": " << msg;
  throw std::runtime_error(emsg.str());
}

const Lisp*
Parser::read()
{
  Lisp* result;
  switch(token) {
    case Lexer::TOKEN_EOF: {
      parse_error("Unexpected EOF.");
    }
    case Lexer::TOKEN_CLOSE_PAREN: {
      parse_error("Unexpected ')'.");
    }
    case Lexer::TOKEN_OPEN_PAREN: {
      result = new(obst) Lisp(Lisp::TYPE_CONS);

      token = lexer->getNextToken();
      if(token == Lexer::TOKEN_CLOSE_PAREN) {
        result->v.cons.car = 0;
        result->v.cons.cdr = 0;
        break;
      }

      if(token == Lexer::TOKEN_SYMBOL &&
         strcmp(lexer->getString(), "_") == 0) {
        // evaluate translation function (_ str) in place here
        token = lexer->getNextToken();
        if(token != Lexer::TOKEN_STRING)
          parse_error("Expected string after '(_'");

        result = new(obst) Lisp(Lisp::TYPE_STRING);
        if(dictionary) {
          std::string translation = dictionary->translate(lexer->getString());
          result->v.string = new(obst) char[translation.size()+1];
          memcpy(result->v.string, translation.c_str(), translation.size()+1);
        } else {
          size_t len = strlen(lexer->getString()) + 1;
          result->v.string = new(obst) char[len];
          memcpy(result->v.string, lexer->getString(), len);
        }
        token = lexer->getNextToken();
        if(token != Lexer::TOKEN_CLOSE_PAREN)
          parse_error("Expected ')' after '(_ string'");
        break;
      }

      // (tiles 0 0 12 ...): a tilemap holds up to hundreds of thousands of
      // numbers. As ordinary cells each one costs a cons cell plus an integer
      // cell (24 bytes); packed into one array it costs 4. Big community levels
      // need 20 MB or more the ordinary way, which a phone does not have.
      if(token == Lexer::TOKEN_SYMBOL && strcmp(lexer->getString(), "tiles") == 0) {
        result->v.cons.car = read();  // the symbol; moves on to the next token
        result->v.cons.cdr = 0;
        if(token == Lexer::TOKEN_CLOSE_PAREN)
          break;
        if(token != Lexer::TOKEN_INTEGER) {
          read_rest(result);  // not a list of numbers: read it the usual way
          break;
        }
        std::vector<int> numbers;
        numbers.reserve(4096);
        while(token == Lexer::TOKEN_INTEGER) {
          numbers.push_back(atoi(lexer->getString()));
          token = lexer->getNextToken();
        }
        Lisp* cur = result;
        if(token == Lexer::TOKEN_CLOSE_PAREN) {
          Lisp* array = new(obst) Lisp(Lisp::TYPE_INTARRAY);
          int* data = new(obst) int[numbers.size()];
          memcpy(data, &numbers[0], numbers.size() * sizeof(int));
          array->v.ints.data = data;
          array->v.ints.count = (int) numbers.size();
          Lisp* cell = new(obst) Lisp(Lisp::TYPE_CONS);
          cell->v.cons.car = array;
          cell->v.cons.cdr = 0;
          result->v.cons.cdr = cell;
          break;
        }
        // numbers followed by something else: keep the numbers as ordinary cells
        for(size_t n = 0; n < numbers.size(); n++) {
          Lisp* number = new(obst) Lisp(Lisp::TYPE_INTEGER);
          number->v.integer = numbers[n];
          Lisp* cell = new(obst) Lisp(Lisp::TYPE_CONS);
          cell->v.cons.car = number;
          cell->v.cons.cdr = 0;
          cur->v.cons.cdr = cell;
          cur = cell;
        }
        read_rest(cur);
        break;
      }

      Lisp* cur = result;
      do {
        cur->v.cons.car = read();
        if(token == Lexer::TOKEN_CLOSE_PAREN) {
          cur->v.cons.cdr = 0;
          break;
        }
        Lisp *newcur = new(obst) Lisp(Lisp::TYPE_CONS);
        cur->v.cons.cdr = newcur;
        cur = newcur;
      } while(1);

      break;
    }
    case Lexer::TOKEN_SYMBOL: {
      result = new(obst) Lisp(Lisp::TYPE_SYMBOL);
      size_t len = strlen(lexer->getString()) + 1;
      result->v.string = new(obst) char[len];
      memcpy(result->v.string, lexer->getString(), len);
      break;
    }
    case Lexer::TOKEN_STRING: {
      result = new(obst) Lisp(Lisp::TYPE_STRING);
      size_t len = strlen(lexer->getString()) + 1;
      result->v.string = new(obst) char[len];
      memcpy(result->v.string, lexer->getString(), len);
      break;
    }
    case Lexer::TOKEN_INTEGER:
      result = new(obst) Lisp(Lisp::TYPE_INTEGER);
      result->v.integer = atoi(lexer->getString());
      break;
    case Lexer::TOKEN_REAL:
      result = new(obst) Lisp(Lisp::TYPE_REAL);
      result->v.real = strtof(lexer->getString(), NULL);
      break;
    case Lexer::TOKEN_TRUE:
      result = new(obst) Lisp(Lisp::TYPE_BOOLEAN);
      result->v.boolean = true;
      break;
    case Lexer::TOKEN_FALSE:
      result = new(obst) Lisp(Lisp::TYPE_BOOLEAN);
      result->v.boolean = false;
      break;

    default:
      // this should never happen
      result = NULL;
      assert(false);
  }

  token = lexer->getNextToken();
  return result;
}

// appends the remaining elements of the list being read after the cell 'cur'
// (whose car is already filled in), up to and not past the closing ')'
void
Parser::read_rest(Lisp* cur)
{
  while(token != Lexer::TOKEN_CLOSE_PAREN) {
    Lisp* newcur = new(obst) Lisp(Lisp::TYPE_CONS);
    newcur->v.cons.cdr = 0;
    cur->v.cons.cdr = newcur;
    cur = newcur;
    cur->v.cons.car = read();
  }
  cur->v.cons.cdr = 0;
}

} // end of namespace lisp

/* EOF */
