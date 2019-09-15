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

#include "String.h"
#include "StringBuilder.h"
#include <algorithm>
#include <fstream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <tuple>
#include <vector>
#include <ostream>

#define CONFIG_STABLE
#define CONFIG_IGNORE_ELEMENT_SIZE
//#define CONFIG_STRICT
#define CONFIG_COMPRESS
//#define CONFIG_DISABLE_TOP_LEVEL_COMPRESS
#define CONFIG_LAZY_CHECK

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

#ifdef CONFIG_ALLOCATOR
///===-----------------------------------------------------------------------===
///
///               Json Allocator
///
///===-----------------------------------------------------------------------===

template<typename type>
class json_allocator {
  size_t capacity;
  
  using pointer = type *;
  using value_size = /*alignas(alignof(type))*/ unsigned char[sizeof(type)];

  class allocator_node {
  public:
    typename value_size value;
    constexpr pointer rep() { return reinterpret_cast<pointer>(value); }
  };

  std::vector<std::unique_ptr<allocator_node[]>> alloc;
  int count;

public:

  json_allocator(size_t capacity) : capacity(capacity), count(0) {
    alloc.push_back(std::unique_ptr<allocator_node[]>(std::move(new allocator_node[capacity])));
    count = 0;
  }

  ~json_allocator() {
    for (int i = 0; i < alloc.size() - 1; i++)
      for (int j = 0; j < capacity; j++)
        alloc[i][j].rep()->type::~type();
    for (int j = 0; j < count; j++)
        alloc.back()[j].rep()->type::~type();
  }

  template <typename... Args>
  pointer allocate(Args&& ... args) {
    if (count == capacity) {
      alloc.push_back(std::unique_ptr<allocator_node[]>(std::move(new allocator_node[capacity])));
      count = 0;
    }
    pointer ptr = alloc.back()[count++].rep();
    new (ptr) type(std::forward<Args>(args)...);
    return ptr;
  }
};
#endif

///===-----------------------------------------------------------------------===
///
///               Json Lexer
///
///===-----------------------------------------------------------------------===

class json_lexer {
  json_token curtok;
  String curstr;
  
  long long file_size;
  long long read_size = 0;
  long long buffer_size;
  long long current_block_size = 0;
  char *buffer;
  char *pointer = nullptr;

  std::ifstream ifs;

  bool appendable = true;

public:
  json_lexer(std::string file_path, long long buffer_size = 1024 * 1024 * 32);
  ~json_lexer();

  bool next();

  inline json_token type() const;
  inline String str();

  const char *gbuffer() const;

  std::ifstream &stream() { return ifs; }

  long long filesize() const { return file_size; }
  long long readsize() const { return read_size; }

  long long position() const { return read_size - current_block_size + (pointer - buffer); }

private:
  void buffer_refresh();
  bool require_refresh();
  char next_ch();
  void prev();
};

///===-----------------------------------------------------------------------===
///
///               Json Parser
///
///===-----------------------------------------------------------------------===

class json_value {
  int type;

public:
  json_value(int type) : type(type) {}

  bool is_object() const { return type == 0; }
  bool is_array() const { return type == 1; }
  bool is_numeric() const { return type == 2; }
  bool is_string() const { return type == 3; }
  bool is_keyword() const { return type == 4; }

  virtual std::ostream& print(std::ostream& os, bool format = false, std::string indent = "") const = 0;
};

#ifndef CONFIG_ALLOCATOR
using jvalue = std::shared_ptr<json_value>;
#else
using jvalue = json_value*;
#endif

class json_object : public json_value {
public:
  json_object() : json_value(0) {}
#ifndef CONFIG_STABLE
  std::map<String, jvalue> keyvalue;
#else
  std::vector<std::pair<String, jvalue>> keyvalue;
#endif

  virtual std::ostream& print(std::ostream& os, bool format = false, std::string indent = "") const;
};

class json_array : public json_value {
public:
  json_array() : json_value(1) {}
  std::vector<jvalue> array;
  
  virtual std::ostream& print(std::ostream& os, bool format = false, std::string indent = "") const;
};

#ifndef CONFIG_ALLOCATOR
using jobject = std::shared_ptr<json_object>;
using jarray = std::shared_ptr<json_array>;
#else
using jobject = json_object*;
using jarray = json_array*;
#endif

class json_numeric : public json_value {
public:
  json_numeric(String num) : json_value(2), numstr(std::move(num)) {}
  String numstr;

  virtual std::ostream& print(std::ostream& os, bool format = false, std::string indent = "") const;
};

class json_string : public json_value {
public:
  json_string(String str) : json_value(3), str(std::move(str)) {}
  String str;

  virtual std::ostream& print(std::ostream& os, bool format = false, std::string indent = "") const;
};

class json_state : public json_value {
public:
  json_state(json_token token) : json_value(4), type(token) {}
  json_token type;

  virtual std::ostream& print(std::ostream& os, bool format = false, std::string indent = "") const;
};

class json_parser {
  json_lexer lex;
  jvalue _entry;
  bool _skip_literal = false;
  bool _error = false;
  bool _reduce = false;
  
#ifdef CONFIG_ALLOCATOR
  json_allocator<json_array> jarray_pool;
  json_allocator<json_object> jobject_pool;
  json_allocator<json_string> jstring_pool;
  json_allocator<json_numeric> jnumeric_pool;
  json_allocator<json_state> jstate_pool;
#endif

public:
  json_parser(std::string file_path, size_t pool_capacity = 1024 * 256);

  bool step();
  bool &skip_literal() { return _skip_literal; }
  bool error() const { return _error; }
  
  long long filesize() const { return lex.filesize(); }
  long long readsize() const { return lex.readsize(); }
  long long position() const { return lex.position(); }

  jvalue entry() { return _entry; }

  bool reduce_before() { return _reduce; }
  jvalue latest_reduce() { return values.top(); }

private:
#if 0
  bool latest_key_input = false;
  std::stack<WString> keys;
  std::stack<jvalue> stack;
  std::stack<json_token> token;
#endif

  std::stack<String> contents;
  std::stack<int> stack;
  std::stack<jvalue> values;
  void reduce(int code);
};

///===-----------------------------------------------------------------------===
///
///               Json Tree
///
///===-----------------------------------------------------------------------===

typedef enum class _json_tree_type {
  array,
  safe_array,
  object,
  string,
  boolean,
  numeric,
  none,
} json_tree_type;

class json_tree_node {
public:
  json_tree_type type;
  json_tree_node(json_tree_type type) : type(type) { }
  virtual bool operator==(const json_tree_node& node) {
    return type == node.type || type == json_tree_type::none 
      || node.type == json_tree_type::none;
  }
  bool operator!=(const json_tree_node& node) { return !(*this == node); }
  virtual std::ostream& print(std::ostream& os, std::string indent = "") const;
};

using jtree_value = std::shared_ptr<json_tree_node>;

class json_tree_safe_array;
using jtree_safe_array = std::shared_ptr <json_tree_safe_array>;

class json_tree_array final : public json_tree_node {
public:
  json_tree_array();
  std::vector<jtree_value> array;
  bool operator==(const json_tree_node& node);
  bool check_consistency();
  jtree_safe_array to_safe_array();
  std::ostream& print(std::ostream& os, std::string indent = "") const;
};

class json_tree_safe_array final : public json_tree_node {
public:
  json_tree_safe_array(jtree_value elem_type, size_t elem_size);
  jtree_value element_type;
  size_t element_size;
  bool operator==(const json_tree_node& node);
  std::ostream& print(std::ostream& os, std::string indent = "") const;
};

class json_tree_object final : public json_tree_node {
public:
  json_tree_object();
  std::vector<std::pair<String, jtree_value>> keyvalue;
  bool operator==(const json_tree_node& node);
  std::ostream& print(std::ostream& os, std::string indent = "") const;
  bool print_reverse = false;
};

using jtree_array = std::shared_ptr<json_tree_array>;
using jtree_object = std::shared_ptr<json_tree_object>;

class json_tree {
  jtree_value _tree_entry;

public:
  json_tree(jvalue entry);

  jtree_value tree_entry() { return _tree_entry; }

private:
  jtree_value to_jtree_node(jvalue value);
  jtree_value to_jtree_array(jarray array);
  jtree_object to_jtree_object(jobject object);
};

class json_tree_exporter {
  jtree_value _tree_entry;
  StringBuilder builder;
  String indent = "";
  String base_class_name;
  bool freeze = false;
  int no_name_class = 0;

public:
  json_tree_exporter(jtree_value tree_entry);

  String export_cs_newtonsoftjson_style(String class_name = "MyJsonModel");
  String export_cpp_nlohmann_style(String class_name = "MyJsonModel");
  String export_java_gson_stype(String class_name = "MyJsonModel");
  String export_rust_serders_style(String class_name = "MyJsonModel");

private:
  void up_indent();
  void down_indent();
  void append(String str);
  
  void cs_newtonsoftjson_object_internal(std::pair<String, jtree_value>& it);
  void cs_newtonsoftjson_object(jtree_object object, String class_name);
  void cs_newtonsoftjson_safe_array(jtree_safe_array array, String class_name);
};

} // namespace jsonhead

#endif