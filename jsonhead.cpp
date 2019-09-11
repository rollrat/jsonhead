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

jsonhead::json_lexer::json_lexer(std::string file_path, long long buffer_size) 
  : ifs(file_path), curtok(json_token::none), buffer_size(buffer_size) {
  ifs.seekg(0, std::ios::end);
  file_size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  buffer = new wchar_t[buffer_size];
  memset(buffer, 0, buffer_size);

  if (!ifs)
    throw std::runtime_error("file not found!");
}

jsonhead::json_lexer::~json_lexer() {
  delete[] buffer;
  ifs.close();
}

bool jsonhead::json_lexer::next() {
  this->curstr = WString();
  while (true) {
    auto cur = next_ch();
    if (cur == 0) {
      curtok = json_token::eof;
      return true;
    }

    switch (cur)
    {
    case L' ':
    case L'\r':
    case L'\n':
      continue;

    case L',':
      this->curtok = json_token::v_comma;
      this->curstr = WString(cur, 1);
      break;

    case L':':
      this->curtok = json_token::v_pair;
      this->curstr = WString(cur, 1);
      break;

    case L'{':
      this->curtok = json_token::object_starts;
      this->curstr = WString(cur, 1);
      break;

    case L'}':
      this->curtok = json_token::object_ends;
      this->curstr = WString(cur, 1);
      break;

    case L'[':
      this->curtok = json_token::array_starts;
      this->curstr = WString(cur, 1);
      break;

    case L']':
      this->curtok = json_token::array_ends;
      this->curstr = WString(cur, 1);
      break;

    case L't':
    case L'f':
    case L'n':
      {
        WStringBuilder ss(10);
        while (cur && isalpha(cur))
        {
          ss.Append(cur);
          cur = next_ch();
        }

        auto s = ss.ToString();
        if (s == L"true")
          this->curtok = json_token::v_true;
        else if (s == L"false")
          this->curtok = json_token::v_false;
        else if (s == L"null")
          this->curtok = json_token::v_null;
        else
          return false;

        this->curstr = s;
      }
      break;

    case L'"':
      {
        WStringBuilder ss;

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
        WStringBuilder ss;

        if (cur == L'-') {
          ss.Append(cur);
          cur = next_ch();
        }

        // [0-9]+
        while (cur && isdigit(cur)) {
          ss.Append(cur);
          cur = next_ch();
        }
        
        // [0-9]+.[0-9]+
        if (cur && cur == L'.') {
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
        if (cur && (cur == L'E' || cur == L'e')) {
          cur = next_ch();
          
          if (!cur || !(cur == L'+' || cur == L'-' || isdigit(cur)))
            return false;
          
          if (cur == L'+' || cur == L'-') {
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

jsonhead::WString jsonhead::json_lexer::str() {
  return std::move(this->curstr);
}

const wchar_t *jsonhead::json_lexer::gbuffer() const {
  return pointer;
}

inline void jsonhead::json_lexer::buffer_refresh() {
  current_block_size = ifs.read(buffer, buffer_size).gcount();
  read_size += current_block_size;
  pointer = buffer;
}

inline bool jsonhead::json_lexer::require_refresh() {
  return pointer == nullptr || buffer + current_block_size == pointer;
}

wchar_t jsonhead::json_lexer::next_ch() {
  if (require_refresh()) {
    if (ifs.eof())
      return (char)0;
    buffer_refresh();
  }
  return *pointer++;
}

///===-----------------------------------------------------------------------===
///
///               Json Model
///
///===-----------------------------------------------------------------------===

std::wostream& jsonhead::json_object::operator<<(std::wostream& os) const {
  os << '{';
  for (auto it = keyvalue.begin(); it != keyvalue.end(); ++it) {
    os << it->first << ':' << it->second;
    if (std::next(it) != keyvalue.end())
      os << ',';
  }
  os << '}';
  return os;
}

std::wostream& jsonhead::json_array::operator<<(std::wostream& os) const {
  os << '[';
  for (auto it = array.begin(); it != array.end(); ++it) {
    os << *it;
    if (std::next(it) != array.end())
      os << ',';
  }
  os << ']';
  return os;
}

std::wostream& jsonhead::json_numeric::operator<<(std::wostream& os) const {
  os << numstr;
  return os;
}

std::wostream& jsonhead::json_string::operator<<(std::wostream& os) const {
  os << str;
  return os;
}

std::wostream& jsonhead::json_state::operator<<(std::wostream& os) const {
  //switch (type) {

  //}
  return os;
}
///===-----------------------------------------------------------------------===
///
///               Json Parser
///
///===-----------------------------------------------------------------------===

static int goto_table[][20] = 
{
  {   0,   1,   3,   2,   0,   0,   0,   0,   4,   0,   0,   0,   5,   0,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  28 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  -1 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  -2 },
  {   0,   0,   0,   0,   7,   8,   0,   0,   0,   6,   0,   0,   0,   0,   0,   0,   0,   9,   0,   0 },
  {   0,   0,  16,  15,   0,   0,  11,  12,   4,   0,   0,   0,   5,  10,  17,  18,  19,  13,  14,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -5,  -5,   0,   0,  -5,   0,   0,   0,   0,   0,  -5 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,  20,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -7,  21,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  22,   0,   0,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -3,  -3,   0,   0,  -3,   0,   0,   0,   0,   0,  -3 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  23,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  24,   0,   0, -10,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0, -12, -12,   0,   0, -12,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0, -13, -13,   0,   0, -13,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0, -14, -14,   0,   0, -14,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0, -15, -15,   0,   0, -15,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0, -16, -16,   0,   0, -16,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0, -17, -17,   0,   0, -17,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0, -18, -18,   0,   0, -18,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -6,  -6,   0,   0,  -6,   0,   0,   0,   0,   0,  -6 },
  {   0,   0,   0,   0,  25,   8,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   9,   0,   0 },
  {   0,   0,  16,  15,   0,   0,   0,  26,   4,   0,   0,   0,   5,   0,  17,  18,  19,  13,  14,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -4,  -4,   0,   0,  -4,   0,   0,   0,   0,   0,  -4 },
  {   0,   0,  16,  15,   0,   0,  27,  12,   4,   0,   0,   0,   5,   0,  17,  18,  19,  13,  14,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -8,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,  -9,  -9,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, -11,   0,   0,   0,   0,   0,   0 },
};

static int production[] = {
   1,   1,   1,   2,   3,   2,   3,   1,   3,   3,   1,   3,   1,   1,   1,   1,   1,   1,   1
};

static int production_rules[][3] = 
{
  {   1 },
  {   3 },
  {   2 },
  {  12,  13 },
  {  12,   6,  13 },
  {   8,   9 },
  {   8,   4,   9 },
  {   5 },
  {   5,  10,   4 },
  {  17,  11,   7 },
  {   7 },
  {   7,  10,   6 },
  {  17 },
  {  18 },
  {   3 },
  {   2 },
  {  14 },
  {  15 },
  {  16 },
};

static int group_table[] ={
   0,   1,   1,   2,   2,   3,   3,   4,   4,   5,   6,   6,   7,   7,   7,   7,   7,   7,   7
};

static jsonhead::json_token symbol_index[] = 
{
              jsonhead::json_token::none,
      jsonhead::json_token::json_nt_json,
     jsonhead::json_token::json_nt_array,
    jsonhead::json_token::json_nt_object,
   jsonhead::json_token::json_nt_members,
      jsonhead::json_token::json_nt_pair,
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

  if (code == ACCEPT_INDEX)
  {
    // End of json format
    return false;
  }
  else if (code > 0)
  {
    // Shift
    stack.push(code);
    contents.push(lex.str());
  }
  else if (code < 0)
  {
    // Reduce
    reduce(code);
    goto REDUCE;
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

  //   0:         S' -> JSON
  //   1:       JSON -> OBJECT
  //   2:       JSON -> ARRAY
  //   3:      ARRAY -> [ ]
  //   4:      ARRAY -> [ ELEMENTS ]
  //   5:     OBJECT -> { }
  //   6:     OBJECT -> { MEMBERS }
  //   7:    MEMBERS -> PAIR
  //   8:    MEMBERS -> PAIR , MEMBERS
  //   9:       PAIR -> v_string : VALUE
  //  10:   ELEMENTS -> VALUE
  //  11:   ELEMENTS -> VALUE , ELEMENTS
  //  12:      VALUE -> v_string
  //  13:      VALUE -> v_number
  //  14:      VALUE -> OBJECT
  //  15:      VALUE -> ARRAY
  //  16:      VALUE -> true
  //  17:      VALUE -> false
  //  18:      VALUE -> null

  switch (reduce_production)
  {
  case 0:
    _entry = values.top();
    values.pop();
    break;

  //case 1:
  //case 2:

  case 3:
    contents.pop();
    contents.pop();
    values.push(jarray(new json_array));
    break;

  case 4:
    contents.pop();
    contents.pop();
    break;

  case 5:
    contents.pop();
    contents.pop();
    values.push(jobject(new json_object()));
    break;

  case 6:
    contents.pop();
    contents.pop();
    break;

  case 7:
    {
      auto jo = jobject(new json_object());
      if (!(_skip_literal && values.top()->is_string()))
        jo->keyvalue[contents.top()] = std::move(values.top());
      else
        jo->keyvalue[contents.top()] = std::shared_ptr<json_string>(new json_string(std::move(WString())));
      values.pop();
      values.push(jo);
      contents.pop();
    }
    break;

  case 8:
    {
      contents.pop();
      auto jo = values.top(); values.pop();
      if (!(_skip_literal && values.top()->is_string()))
        ((json_object*)&*jo)->keyvalue[contents.top()] = std::move(values.top());
      else
        ((json_object*)&*jo)->keyvalue[contents.top()] = std::shared_ptr<json_string>(new json_string(std::move(WString())));
      values.pop();
      values.push(jo);
      contents.pop();
    }
    break;

  case 9:
    contents.pop();
    break;

  case 10:
    {
      auto ja = jarray(new json_array());
      ja->array.push_back(values.top());
      values.pop();
      values.push(ja);
    }
    break;

  case 11:
    {
      auto ja = values.top(); values.pop();
      ((json_array*)&*ja)->array.push_back(values.top());
      values.pop();
      values.push(ja);
      contents.pop();
    }
    break;

  case 12:
    values.push(std::shared_ptr<json_string>(new json_string(contents.top())));
    contents.pop();
    break;

  case 13:
    values.push(std::shared_ptr<json_numeric>(new json_numeric(contents.top())));
    contents.pop();
    break;

  //case 14:
  //case 15:

  case 16:
    values.push(std::shared_ptr<json_state>(new json_state(jsonhead::json_token::v_true)));
    contents.pop();
    break;

  case 17:
    values.push(std::shared_ptr<json_state>(new json_state(jsonhead::json_token::v_false)));
    contents.pop();
    break;

  case 18:
    values.push(std::shared_ptr<json_state>(new json_state(jsonhead::json_token::v_null)));
    contents.pop();
    break;
  }
}