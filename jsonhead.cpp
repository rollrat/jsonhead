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
        bool force = false;

        // donot parse \uXXXX unicode style character
        while (cur = next_ch())
        {
          if (!force && cur == '"') break;
          // escapes
          if (1 <= cur && cur <= 19) return false;
          ss.Append(cur);
          if (!force && cur == '\\') {
            force = true;
            continue;
          }
          force = false;
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

jsonhead::json_parser::json_parser(std::string file_path)
  : lex(file_path) {
}

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
