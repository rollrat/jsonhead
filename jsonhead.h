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

#include "WString.h"
#include "WStringBuilder.h"
#include <algorithm>
#include <fstream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <tuple>
#include <vector>

namespace jsonhead {

typedef enum class _json_token {
  none = 0,
  json_nt_json,
  json_nt_array,
  json_nt_object,
  json_nt_members,
  json_nt_pair,
  json_nt_elements,
  json_nt_value,
  object_starts, // {
  object_ends,   // }
  v_comma,       // ,
  v_pair,        // :
  array_starts,  // [
  array_ends,    // ]
  v_true,        // true
  v_false,       // false
  v_null,        // null
  v_string,      // "(\\(["/bfnrt]|u{Hex}{Hex}{Hex}{Hex}))*"
  v_number,      // \-?(0|[1-9]\d*)(\.\d+)?([Ee][+-]?\d+)?
  eof,
  error,
} json_token;

class json_lexer {
  json_token curtok;
  WString curstr;
  
  long long file_size;
  long long read_size = 0;
  long long buffer_size;
  long long current_block_size = 0;
  wchar_t *buffer;
  wchar_t *pointer = nullptr;

  std::wifstream ifs;

  bool appendable = true;

public:
  json_lexer(std::string file_path, long long buffer_size = 1024 * 1024 * 32);
  ~json_lexer();

  bool next();

  inline json_token type() const;
  inline WString str();

  const wchar_t *gbuffer() const;

  std::wifstream &stream() { return ifs; }

  long long filesize() const { return file_size; }
  long long readsize() const { return read_size; }

private:
  void buffer_refresh();
  bool require_refresh();
  wchar_t next_ch();
};

class json_value {
  int type;

public:
  json_value(int type) : type(type) {}

  bool is_object() const { return type == 0; }
  bool is_array() const { return type == 1; }
  bool is_numberic() const { return type == 2; }
  bool is_string() const { return type == 3; }
  bool is_keyword() const { return type == 4; }
};

using jvalue = std::shared_ptr<json_value>;

class json_object : public json_value {
public:
  json_object() : json_value(0) {}
  std::map<WString, jvalue> keyvalue;
};

class json_array : public json_value {
public:
  json_array() : json_value(1) {}
  std::vector<jvalue> array;
};

using jobject = std::shared_ptr<json_object>;
using jarray = std::shared_ptr<json_array>;

class json_numeric : public json_value {
public:
  json_numeric(WString num) : json_value(2), numstr(std::move(num)) {}
  WString numstr;
};

class json_string : public json_value {
public:
  json_string(WString str) : json_value(3), str(std::move(str)) {}
  WString str;
};

class json_state : public json_value {
public:
  json_state(json_token token) : json_value(4), type(token) {}
  json_token type;
};

class json_parser {
  json_lexer lex;
  jvalue entry;
  bool _skip_literal = false;
  bool _error = false;

public:
  json_parser(std::string file_path);

  bool step();
  bool &skip_literal() { return _skip_literal; }
  bool error() const { return _error; }
  
  long long filesize() const { return lex.filesize(); }
  long long readsize() const { return lex.readsize(); }

private:
#if 0
  bool latest_key_input = false;
  std::stack<WString> keys;
  std::stack<jvalue> stack;
  std::stack<json_token> token;
#endif

  std::stack<WString> contents;
  std::stack<int> stack;
  std::stack<jvalue> values;
  void reduce(int code);
};

} // namespace jsonhead

#endif