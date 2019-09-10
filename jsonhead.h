//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#ifndef _JSONHEAD_
#define _JSONHEAD_

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <memory>
#include <stack>
#include "String.h"
#include "StringBuilder.h"

namespace jsonhead {

// https://github.com/cierelabs/yaml_spirit/blob/master/doc/specs/json-ebnf.txt
typedef enum class _json_token
{
  none,
  v_number,          // \-?(0|[1-9]\d*)(\.\d+)?([Ee][+-]?\d+)?
  v_string,          // "(\\(["/bfnrt]|u{Hex}{Hex}{Hex}{Hex}))*"
  v_comma=',',       // ,
  v_pair=':',        // :
  v_true,            // true
  v_false,           // false
  v_null,            // null
  object_starts='{', // {
  object_ends='}',   // }
  array_starts='[',  // [
  array_ends=']',    // ]
  eof,
} json_token;

class json_lexer
{
  json_token curtok;
  ofw::String curstr;
  
  size_t buffer_size;
  char* buffer;
  char* pointer = nullptr;
  std::streamsize current_block_size = 0;
  
  std::ifstream ifs;

  bool appendable = true;

public:

  json_lexer(std::string file_path, size_t buffer_size = 1024 * 1024 * 32);
  ~json_lexer();

  bool next();

  json_token type() const;
  ofw::String str() const;

  const char *gbuffer() const;

private:
  void buffer_refresh();
  bool require_refresh();
  char next_ch();
};

class json_value
{
  int type;
public:
  json_value(int type) : type(type) {}

  bool is_object() const { return type == 0; }
  bool is_array() const { return type == 1; }
  bool is_numberic() const { return type == 2; }
  bool is_value() const { return type == 3; }
  bool is_keyword() const { return type == 4; }
};

using jvalue = std::shared_ptr<json_value>;

class json_object : public json_value
{
public:
  json_object() : json_value(0) { }
  std::map<ofw::String, jvalue> keyvalue;
};

class json_array : public json_value
{
public:
  json_array() : json_value(1) { }
  std::vector<jvalue> array;
};

using jobject = std::shared_ptr<json_object>;
using jarray = std::shared_ptr<json_array>;

class json_numeric : public json_value
{
public:
  json_numeric(ofw::String num) : json_value(2), numstr(num) { }
  ofw::String numstr;
};

class json_string : public json_value
{
public:
  json_string(ofw::String str) : json_value(3), str(str) { }
  ofw::String str;
};

class json_state : public json_value
{
public:
  json_state(json_token token) : json_value(4), type(token) { }
  json_token type;
};

class json_parser
{
  json_lexer lex;
  jvalue entry;
  bool skip_literal;
  bool error;
  bool latest_key_input = false;

public:
  json_parser(std::string file_path);

  bool step();
  bool& skip_literal() { return skip_literal; }
  bool error() const { return error; }

private:
#if 0
  std::stack<ofw::String> keys;
  std::stack<jvalue> stack;
  std::stack<json_token> token;
#endif

  std::stack<int> stack;
  void reduce(int code);
};

}

#endif