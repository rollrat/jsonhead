//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#include "jsonhead.h"
#include <sstream>

///===-----------------------------------------------------------------------===
///
///               Json Lexer
///
///===-----------------------------------------------------------------------===

jsonhead::json_lexer::json_lexer(std::string file_path, size_t buffer_size) 
  : ifs(file_path), curtok(json_token::none), buffer_size(buffer_size) {
  buffer = new char[buffer_size];
  memset(buffer, 0, buffer_size);

  if (!ifs)
    throw std::runtime_error("file not found!");
}

jsonhead::json_lexer::~json_lexer() {
  delete[] buffer;
  ifs.close();
}

bool jsonhead::json_lexer::next() {
  while (true) {
    auto cur = next_ch();
    if (cur == 0) {
      curtok = json_token::eof;
      return true;
    }

    switch (cur)
    {
    case ' ':
    case '\r':
    case '\n':
      continue;

    case ',':
    case ':':
    case '{':
    case '}':
    case '[':
    case ']':
      this->curtok = (json_token)cur;
      this->curstr = ofw::String(cur, 1);
      break;

    case 't':
    case 'f':
    case 'n':
      {
        ofw::StringBuilder ss(10);
        while (cur && isalpha(cur))
        {
          ss.Append(cur);
          cur = next_ch();
        }

        auto s = ss.ToString();
        if (s == "true")
          this->curtok = json_token::v_true;
        else if (s == "false")
          this->curtok = json_token::v_false;
        else if (s == "null")
          this->curtok = json_token::v_null;
        else
          return false;

        this->curstr = s;
      }
      break;

    case '"':
      {
        ofw::StringBuilder ss;

        // donot parse \uXXXX unicode style character
        while (cur = next_ch())
        {
          if (cur == '"') break;
#if REAL_JSON
          // escapes
          if (1 <= cur && cur <= 19) return false;
#endif
          ss.Append(cur);
          if (cur == '\\') {
            if (!(cur = next_ch()))
              return false;
            ss.Append(cur);
          }
        }

        curtok = json_token::v_string;
        curstr = ss.ToString();
      }

      break;

    default:
      {
        ofw::StringBuilder ss;

        if (cur == '-') {
          ss.Append(cur);
          cur = next_ch();
        }

        // [0-9]+
        while (cur && isdigit(cur)) {
          ss.Append(cur);
          cur = next_ch();
        }
        
        // [0-9]+.[0-9]+
        if (cur && cur == '.') {
          cur = next_ch();
          if (!cur || !isdigit(cur))
            return false;
          
          while (cur && isdigit(cur)) {
            ss.Append(cur);
            cur = next_ch();
          }
        }
        
        // [0-9]+[Ee][+-]?[0-9]+
        // [0-9]+.[0-9]+[Ee][+-]?[0-9]+
        if (cur && (cur == 'E' || cur == 'e')) {
          cur = next_ch();
          
          if (!cur || !(cur == '+' || cur == '-' || isdigit(cur)))
            return false;
          
          if (cur == '+' || cur == '-') {
            ss.Append(cur);
            cur = next_ch();
          }
          
          if (!cur || !isdigit(cur))
            return false;
          
          while (cur && isdigit(cur)) {
            ss.Append(cur);
            cur = next_ch();
          }
        }

        curtok = json_token::v_number;
        curstr = ss.ToString();
      }
    }

    return true;
  }
}

jsonhead::json_token jsonhead::json_lexer::type() const {
  return this->curtok;
}

ofw::String jsonhead::json_lexer::str() const {
  return this->curstr;
}

const char *jsonhead::json_lexer::gbuffer() const {
  return pointer;
}

void jsonhead::json_lexer::buffer_refresh() {
  current_block_size = ifs.read(buffer, buffer_size).gcount();
  pointer = buffer;
}

bool jsonhead::json_lexer::require_refresh() {
  return pointer == nullptr || buffer + current_block_size == pointer;
}

char jsonhead::json_lexer::next_ch() {
  if (require_refresh()) {
    if (ifs.eof())
      return (char)0;
    buffer_refresh();
  }
  return *pointer++;
}

///===-----------------------------------------------------------------------===
///
///               Json Parser
///
///===-----------------------------------------------------------------------===

static int goto_table[][20] = {
   {   0,   1,   0,   2,   0,   0,   0,   0,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  28 },
   {   0,   0,   4,   0,   0,   0,   0,   0,   0,   0,   0,   0,   5,   0,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   7,   8,   0,   0,   0,   6,   0,   0,   0,   0,   0,   0,   0,   9,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  -1 },
   {   0,   0,  16,  15,   0,   0,  11,  12,   3,   0,   0,   0,   5,  10,  17,  18,  19,  13,  14,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -4,  -4,   0,  -4,  -4,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,  20,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -6,  21,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  22,   0,   0,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -2,  -2,   0,   0,  -2,   0,   0,   0,   0,   0,  -2 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  23,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  24,   0,   0,  -9,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0, -11, -11,   0,   0, -11,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0, -12, -12,   0,   0, -12,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0, -13, -13,   0,   0, -13,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0, -14, -14,   0,   0, -14,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0, -15, -15,   0,   0, -15,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0, -16, -16,   0,   0, -16,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0, -17, -17,   0,   0, -17,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -5,  -5,   0,  -5,  -5,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,  25,   8,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   9,   0,   0 },
   {   0,   0,  16,  15,   0,   0,   0,  26,   3,   0,   0,   0,   5,   0,  17,  18,  19,  13,  14,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -3,  -3,   0,   0,  -3,   0,   0,   0,   0,   0,  -3 },
   {   0,   0,  16,  15,   0,   0,  27,  12,   3,   0,   0,   0,   5,   0,  17,  18,  19,  13,  14,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -7,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -8,  -8,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
   {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, -10,   0,   0,   0,   0,   0,   0 },
};

static int production[] = {
   1,   2,   2,   3,   2,   3,   1,   3,   3,   1,   3,   1,   1,   1,   1,   1,   1,   1
};

static int group_table[] ={
   0,   1,   2,   2,   3,   3,   4,   4,   5,   6,   6,   7,   7,   7,   7,   7,   7,   7
};

static jsonhead::json_token symbol_index[] = {
              jsonhead::json_token::none,
      jsonhead::json_token::json_nt_json,
    jsonhead::json_token::json_nt_object,
   jsonhead::json_token::json_nt_members,
      jsonhead::json_token::json_nt_pair,
     jsonhead::json_token::json_nt_array,
  jsonhead::json_token::json_nt_elements,
     jsonhead::json_token::json_nt_value, 

     jsonhead::json_token::object_starts,
       jsonhead::json_token::object_ends,
           jsonhead::json_token::v_comma,
            jsonhead::json_token::v_pair,
      jsonhead::json_token::array_starts,
        jsonhead::json_token::array_ends,
            jsonhead::json_token::v_true,
           jsonhead::json_token::v_false,
            jsonhead::json_token::v_null,
          jsonhead::json_token::v_string,
          jsonhead::json_token::v_number,
               jsonhead::json_token::eof,
};

#define ACCEPT_INDEX 28

jsonhead::json_parser::json_parser(std::string file_path)
  : lex(file_path) {
}

#if 0
bool jsonhead::json_parser::step() {
  if (!lex.next()) return false;

  if (stack.empty()) {
    if (lex.type == json_token::array_starts) {
      token.push(lex.type);
      stack.push(jarray());
      entry = stack.top();
      return true;
    }
    if (lex.type == json_token::object_starts) {
      token.push(lex.type);
      stack.push(jobject());
      entry = stack.top();
      return true;
    }
    this->error = true;
    return false;
  }

  switch (lex.type) {
  case json_token::v_number:
    if (token.top() == json_token::array_starts) {
      ((json_array *)&*stack.top())->array.push_back(std::shared_ptr<json_numeric>(new json_numeric(lex.str())));
    }
    break;

  case json_token::v_string:
    if (token.top() == json_token::array_starts) {
      ((json_array *)&*stack.top())->array.push_back(std::shared_ptr<json_string>(new json_string(lex.str())));
    }
    else if (token.top() == json_token::object_starts) {
    }
    break;

  case json_token::v_comma:
    if (token.top() == json_token::object_starts) {
      
    }
    break;

  case json_token::v_pair:
    // skip
    break;

  case json_token::v_true:
  case json_token::v_false:
  case json_token::v_null:
    if (token.top() == json_token::array_starts) {
      ((json_array *)&*stack.top())->array.push_back(std::shared_ptr<json_state>(new json_state(lex.type)));
    }
    break;

  case json_token::object_starts:
  case json_token::array_starts:

  case json_token::object_ends:
  case json_token::array_ends:
    token.pop(); // [ or {
    stack.pop(); // release jobject or jarray
    break;

  case json_token::eof:
    return false;
  }

  return true;
}
#endif

bool jsonhead::json_parser::step() {
  if (!lex.next()) return false;

REDUCE:

  if (stack.empty())
    stack.push(0);

  bool require_reduce = false;
  int code = goto_table[stack.top()][(int)lex.type()];

  if (code > 0)
  {
    // Shift
    stack.push(code);
  }
  else if (code < 0)
  {
    // Reduce
    reduce(code);
    goto REDUCE;
  }
  else if (code == ACCEPT_INDEX)
  {
    // End of json format
    return false;
  }
  else
  {
    // Panic mode
    this->_error = true;
    return false;
  }

  return true;
}

void jsonhead::json_parser::reduce(int code)
{
  int reduce_production = -code;

  // Reduce Stack
  for (int i = 0; i < production[reduce_production]; i++) {
    stack.pop();
  }

  stack.push(goto_table[stack.top()][group_table[reduce_production]]);
}