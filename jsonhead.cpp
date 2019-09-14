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
#include <set>

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
  this->curstr = String();
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
    case '\t':
      continue;

    case ',':
      this->curtok = json_token::v_comma;
      this->curstr = String(cur, 1);
      break;

    case ':':
      this->curtok = json_token::v_pair;
      this->curstr = String(cur, 1);
      break;

    case '{':
      this->curtok = json_token::object_starts;
      this->curstr = String(cur, 1);
      break;

    case '}':
      this->curtok = json_token::object_ends;
      this->curstr = String(cur, 1);
      break;

    case '[':
      this->curtok = json_token::array_starts;
      this->curstr = String(cur, 1);
      break;

    case ']':
      this->curtok = json_token::array_ends;
      this->curstr = String(cur, 1);
      break;

    case 't':
    case 'f':
    case 'n':
      {
        StringBuilder ss(10);
        while (cur && isalpha(cur))
        {
          ss.Append(cur);
          cur = next_ch();
        }
        prev();

        auto s = ss.ToString();
        if (s == "true")
          this->curtok = json_token::v_true;
        else if (s == "false")
          this->curtok = json_token::v_false;
        else if (s == "nul")
          this->curtok = json_token::v_null;
        else
          return false;

        this->curstr = s;
      }
      break;

    case '"':
      {
        StringBuilder ss;

        // donot parse \uXXXX unicode style character
        while (cur = next_ch())
        {
          if (cur == '"') break;
#if REAL_JSON
          // escapes
          if (1 <= cur && cur <= 19) return false;
#endif
#if !CONFIG_DISABLE_UTF8
          // We will be support wide characters.
          if (cur < 0) {
            unsigned char cc = cur;
            int len = 0;
            if ((cc & 0xfc) == 0xfc) {
              len = 6;
            }
            else if ((cc & 0xf8) == 0xf8) {
              len = 5;
            }
            else if ((cc & 0xf0) == 0xf0) {
              len = 4;
            }
            else if ((cc & 0xe0) == 0xe0) {
              len = 3;
            }
            else if ((cc & 0xc0) == 0xc0) {
              len = 2;
            }
            for (; --len; cur = next_ch())
              ss.Append(cur);
            ss.Append(cur);
            continue;
          }
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
        StringBuilder ss;

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
        prev();

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

jsonhead::String jsonhead::json_lexer::str() {
  return std::move(this->curstr);
}

const char *jsonhead::json_lexer::gbuffer() const {
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

char jsonhead::json_lexer::next_ch() {
  if (require_refresh()) {
    if (ifs.eof())
      return (char)0;
    buffer_refresh();
  }
  return *pointer++;
}

void jsonhead::json_lexer::prev() {
  pointer--;
}

///===-----------------------------------------------------------------------===
///
///               Json Model
///
///===-----------------------------------------------------------------------===

std::ostream& jsonhead::json_object::print(std::ostream& os, bool format, std::string indent) const {
  if (!format)
    os << '{';
  else
    os << "{\n";
  for (auto it = keyvalue.rbegin(); it != keyvalue.rend(); ++it) {
    if (!format) {
      os << '\"' << it->first << "\":";
      it->second->print(os);
    }
    else {
      os << indent << "  \"" << it->first << "\": ";
      it->second->print(os, true, indent + "  ");
    }
    if (std::next(it) != keyvalue.rend()) {
      if (!format)
        os << ',';
      else
        os << ",\n";
    }
  }
  if (!format)
    os << '}';
  else
    os << '\n' << indent << "}";
  return os;
}

std::ostream& jsonhead::json_array::print(std::ostream& os, bool format, std::string indent) const {
  if (array.size() > 0) {
    if (!format)
      os << '[';
    else
      os << "[\n";
    for (auto it = array.rbegin(); it != array.rend(); ++it) {
      if (!format)
        (*it)->print(os);
      else {
        os << indent << "  ";
        (*it)->print(os, true, indent + "  ");
      }
      if (std::next(it) != array.rend()) {
        if (!format)
          os << ',';
        else
          os << ",\n";
      }
    }
    if (!format)
      os << ']';
    else
      os << '\n' << indent << "]";
  }
  else {
    os << "[]";
  }
  return os;
}

std::ostream& jsonhead::json_numeric::print(std::ostream& os, bool format, std::string indent) const {
  os << numstr;
  return os;
}

std::ostream& jsonhead::json_string::print(std::ostream& os, bool format, std::string indent) const {
  os << '"' << str << '"';
  return os;
}

std::ostream& jsonhead::json_state::print(std::ostream& os, bool format, std::string indent) const {
  switch (type) 
  {
  case json_token::v_false:
    os << "false";
    break;

  case json_token::v_true:
    os << "true";
    break;

  case json_token::v_null:
    os << "null";
    break;
  }
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

jsonhead::json_parser::json_parser(std::string file_path, size_t pool_capacity)
  : lex(file_path)
#ifdef CONFIG_ALLOCATOR
  , jarray_pool(pool_capacity), jobject_pool(pool_capacity), jstring_pool(pool_capacity),
    jnumeric_pool(pool_capacity), jstate_pool(pool_capacity)
#endif
{
}

bool jsonhead::json_parser::step() {
  if (!_reduce && !lex.next()) return false;
   
  _reduce = false;

  if (stack.empty())
    stack.push(0);

  bool require_reduce = false;
  int code = goto_table[stack.top()][(int)lex.type()];

  if (code == ACCEPT_INDEX)
  {
    // End of json format
    _entry = values.top();
    values.pop();
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
    _reduce = true;
  }
  else
  {
    // Panic mode
    this->_error = true;
    return false;
  }

  return true;
}

void jsonhead::json_parser::reduce(int code) {
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
  //case 0:
  //case 1:
  //case 2:

  case 3:
    contents.pop();
    contents.pop();
#ifndef CONFIG_ALLOCATOR
    values.push(jarray(new json_array));
#else
    values.push(jarray(jarray_pool.allocate()));
#endif
    break;

  case 4:
    contents.pop();
    contents.pop();
    break;

  case 5:
    contents.pop();
    contents.pop();
#ifndef CONFIG_ALLOCATOR
    values.push(jobject(new json_object()));
#else
    values.push(jobject(jobject_pool.allocate()));
#endif
    break;

  case 6:
    contents.pop();
    contents.pop();
    break;

  case 7:
    {
#ifndef CONFIG_ALLOCATOR
      auto jo = jobject(new json_object());
#else
      auto jo = jobject(jobject_pool.allocate());
#endif
      if (!(_skip_literal && values.top()->is_string()))
#ifndef CONFIG_STABLE
        jo->keyvalue[contents.top()] = std::move(values.top());
#else
        jo->keyvalue.push_back({contents.top(), std::move(values.top())});
#endif
#ifndef CONFIG_ALLOCATOR
      else
        
#ifndef CONFIG_STABLE
        jo->keyvalue[contents.top()] = std::shared_ptr<json_string>(new json_string(std::move(String())));
#else
        jo->keyvalue.push_back({contents.top(), std::shared_ptr<json_string>(new json_string(std::move(String())))});
#endif
#else
      else
#ifndef CONFIG_STABLE
        jo->keyvalue[contents.top()] = jstring_pool.allocate(std::move(String()));
#else
        jo->keyvalue.push_back({contents.top(), jstring_pool.allocate(std::move(String()))});
#endif
#endif
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
#ifndef CONFIG_STABLE
        ((json_object*)&*jo)->keyvalue[contents.top()] = std::move(values.top());
#else
        ((json_object*)&*jo)->keyvalue.push_back({contents.top(), std::move(values.top())});
#endif
#ifndef CONFIG_ALLOCATOR
      else
#ifndef CONFIG_STABLE
        ((json_object*)&*jo)->keyvalue[contents.top()] = std::shared_ptr<json_string>(new json_string(std::move(String())));
#else
        ((json_object*)&*jo)->keyvalue.push_back({contents.top(), std::shared_ptr<json_string>(new json_string(std::move(String())))});
#endif
#else
      else
#ifndef CONFIG_STABLE
        ((json_object*)&*jo)->keyvalue[contents.top()] = jstring_pool.allocate(std::move(String()));
#else
        ((json_object*)&*jo)->keyvalue.push_back({contents.top(), jstring_pool.allocate(std::move(String()))});
#endif
#endif
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
#ifndef CONFIG_ALLOCATOR
      auto ja = jarray(new json_array());
#else
      auto ja = jarray(jarray_pool.allocate());
#endif
      if (!(_skip_literal && values.top()->is_string()))
        ja->array.push_back(values.top());
      values.pop();
      values.push(ja);
    }
    break;

  case 11:
    {
      auto ja = values.top(); values.pop();
      if (!(_skip_literal && values.top()->is_string()))
        ((json_array*)&*ja)->array.push_back(values.top());
      values.pop();
      values.push(ja);
      contents.pop();
    }
    break;

  case 12:
#ifndef CONFIG_ALLOCATOR
    values.push(std::shared_ptr<json_string>(new json_string(contents.top())));
#else
    values.push(jstring_pool.allocate(contents.top()));
#endif
    contents.pop();
    break;

  case 13:
#ifndef CONFIG_ALLOCATOR
    values.push(std::shared_ptr<json_numeric>(new json_numeric(contents.top())));
#else
    values.push(jnumeric_pool.allocate(contents.top()));
#endif
    contents.pop();
    break;

  //case 14:
  //case 15:

  case 16:
#ifndef CONFIG_ALLOCATOR
    values.push(std::shared_ptr<json_state>(new json_state(jsonhead::json_token::v_true)));
#else
    values.push(jstate_pool.allocate(jsonhead::json_token::v_true));
#endif
    contents.pop();
    break;

  case 17:
#ifndef CONFIG_ALLOCATOR
    values.push(std::shared_ptr<json_state>(new json_state(jsonhead::json_token::v_false)));
#else
    values.push(jstate_pool.allocate(jsonhead::json_token::v_false));
#endif
    contents.pop();
    break;

  case 18:
#ifndef CONFIG_ALLOCATOR
    values.push(std::shared_ptr<json_state>(new json_state(jsonhead::json_token::v_null)));
#else
    values.push(jstate_pool.allocate(jsonhead::json_token::v_null));
#endif
    contents.pop();
    break;
  }
}

///===-----------------------------------------------------------------------===
///
///               Json Tree
///
///===-----------------------------------------------------------------------===

std::ostream& jsonhead::json_tree_node::print(std::ostream& os, std::string indent) const {
  if (this->type == json_tree_type::boolean)
    os << "boolean";
  else if (this->type == json_tree_type::string)
    os << "string";
  else if (this->type == json_tree_type::numeric)
    os << "numeric";
  else if (this->type == json_tree_type::none)
    os << "null";
  return os;
}

jsonhead::json_tree_array::json_tree_array()
  : json_tree_node(json_tree_type::array) {
}

bool jsonhead::json_tree_array::operator==(const json_tree_node& node) {
  if (type != node.type)
    return false;
  auto jta = (json_tree_array *)(&node);
  if (array.size() != jta->array.size())
    return false;
  for (int i = 0; i < array.size(); i++)
    if (*array[i] != *jta->array[i])
      return false;
  return true;
}

bool jsonhead::json_tree_array::check_consistency() {
  if (array.size() == 0)
    return true;
  auto f = array[0];
  for (auto it = array.begin() + 1; it != array.end(); it++)
    if (*f != **it)
      return false;
  return true;
}

static bool check_safe_array(jsonhead::jtree_value dest, jsonhead::jtree_value src) {
  auto t1 = (jsonhead::json_tree_safe_array*)(&*dest);
  auto t2 = (jsonhead::json_tree_safe_array*)(&*src);

  if (t1->element_size == 0 || t1->element_type->type == jsonhead::json_tree_type::none)
    return true;
  if (t1->element_type->type == jsonhead::json_tree_type::safe_array)
    return check_safe_array(t1->element_type, t2->element_type);
  return false;
}

jsonhead::jtree_safe_array jsonhead::json_tree_array::to_safe_array() {
  if (array.size() == 0) {
    return jtree_safe_array(new json_tree_safe_array(jtree_value(new json_tree_node(json_tree_type::none)), array.size()));
  }
#ifndef CONFIG_STRICT
  if (array[0]->type == json_tree_type::object) {
    json_tree_object* obj = new json_tree_object();
    std::map<String, int> keypair;

    for (auto& it : array) {
      auto tob = ((json_tree_object*)(&*it));
      for (auto& kp : tob->keyvalue)
        if (keypair.find(kp.first) == keypair.end()) {
          obj->keyvalue.push_back({kp.first, kp.second});
          keypair[kp.first] = keypair.size();
        }
        else if (obj->keyvalue[keypair[kp.first]].second->type == json_tree_type::none)
          obj->keyvalue[keypair[kp.first]] = {kp.first, kp.second};
        else if (obj->keyvalue[keypair[kp.first]].second->type == json_tree_type::safe_array)
        {
          if (check_safe_array(obj->keyvalue[keypair[kp.first]].second, kp.second))
            obj->keyvalue[keypair[kp.first]] = {kp.first, kp.second};
        }
    }

    return jtree_safe_array(new json_tree_safe_array(jtree_object(obj), array.size()));
  }
#endif
  return jtree_safe_array(new json_tree_safe_array(array[0], array.size()));
}

std::ostream& jsonhead::json_tree_array::print(std::ostream& os, std::string indent) const {
  if (array.size() > 0) {
    os << "[\n";
    for (auto it = array.rbegin(); it != array.rend(); ++it) {
      os << indent << "  ";
      (*it)->print(os, indent + "  ");
      if (std::next(it) != array.rend()) {
        os << ",\n";
      }
    }
    os << '\n' << indent << "]";
  }
  else {
    os << "[]";
  }
  return os;
}

jsonhead::json_tree_safe_array::json_tree_safe_array(jtree_value elem_type, size_t elem_size) : 
    json_tree_node(json_tree_type::safe_array), element_size(elem_size), element_type(elem_type) {
}

bool jsonhead::json_tree_safe_array::operator==(const json_tree_node& node) {
  if (type != node.type)
    return false;
  auto jta = (json_tree_safe_array *)(&node);
#ifdef CONFIG_IGNORE_ELEMENT_SIZE
  return *element_type == *jta->element_type;
#else
  return element_size == jta->element_size && *element_type == *jta->element_type;
#endif
}

std::ostream& jsonhead::json_tree_safe_array::print(std::ostream& os, std::string indent) const {
  if (element_type->type == json_tree_type::boolean ||
      element_type->type == json_tree_type::none    ||
      element_type->type == json_tree_type::numeric ||
      element_type->type == json_tree_type::string) {
    os << "[ ";
    element_type->print(os);
    os << " ]";
  }
  else {
    os << "[\n";
    os << indent << "  ";
    element_type->print(os, indent + "  ");
#ifndef CONFIG_IGNORE_ELEMENT_SIZE
    os << " x " << element_size;
#endif
    os << '\n' << indent << "]";
  }
  return os;
}

jsonhead::json_tree_object::json_tree_object()
  : json_tree_node(json_tree_type::object) {
}

bool jsonhead::json_tree_object::operator==(const json_tree_node& node) {
  if (type != node.type)
    return false;
  auto jto = (json_tree_object *)(&node);
#ifdef CONFIG_STRICT
  if (keyvalue.size() != jto->keyvalue.size())
    return false;
  for (int i = 0; i < keyvalue.size(); i++)
    if (keyvalue[i].first != jto->keyvalue[i].first ||
        *keyvalue[i].second != *jto->keyvalue[i].second)
      return false;
#else
  // Assume all objects of the same array are the same type
#endif
  return true;
}

std::ostream& jsonhead::json_tree_object::print(std::ostream& os, std::string indent) const
{
  os << "{\n";
  for (auto it = keyvalue.rbegin(); it != keyvalue.rend(); ++it) {
    os << indent << "  \"" << it->first << "\": ";
    it->second->print(os, indent + "  ");
    if (std::next(it) != keyvalue.rend()) {
      os << ",\n";
    }
  }
  os << '\n' << indent << "}";
  return os;
}

jsonhead::json_tree::json_tree(jvalue entry) {
  if (entry->is_array() || entry->is_object()) {
    _tree_entry = to_jtree_node(entry);
  }
  else
    throw std::runtime_error("entry must be array or object type!");
}

jsonhead::jtree_value jsonhead::json_tree::to_jtree_node(jvalue value) {
  if (value->is_array()) {
    return to_jtree_array(std::static_pointer_cast<json_array>(value));
  }
  else if (value->is_object()) {
    return to_jtree_object(std::static_pointer_cast<json_object>(value));
  }
  else if (value->is_string()) {
    return jtree_value(new json_tree_node(json_tree_type::string));
  }
  else if (value->is_numeric()) {
    return jtree_value(new json_tree_node(json_tree_type::numeric));
  }
  else if (value->is_keyword()) {
    auto type = ((json_state*)&*value);
    if (type->type == json_token::v_true || type->type == json_token::v_false)
      return jtree_value(new json_tree_node(json_tree_type::boolean));
    else if (type->type == json_token::v_null)
      return jtree_value(new json_tree_node(json_tree_type::none));
  }
  throw std::runtime_error("internal error!");
}

jsonhead::jtree_value jsonhead::json_tree::to_jtree_array(jarray array) {
  json_tree_array *arr = new json_tree_array();
  for (auto it = array->array.begin(); it != array->array.end(); it++) {
    arr->array.push_back(to_jtree_node(*it));
  }
  if (arr->check_consistency()) {
    auto sa = arr->to_safe_array();
    delete arr;
    return sa;
  }
  return jtree_value(arr);
}

jsonhead::jtree_object jsonhead::json_tree::to_jtree_object(jobject object) {
  json_tree_object* obj = new json_tree_object();
  for (auto it = object->keyvalue.begin(); it != object->keyvalue.end(); it++)
    obj->keyvalue.push_back({it->first, to_jtree_node(it->second)});
  return jtree_object(obj);
}
