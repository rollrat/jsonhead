//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#ifndef _STRING_BUILDER_9bf1541fdf7efd41b7b39543fd870ac4_
#define _STRING_BUILDER_9bf1541fdf7efd41b7b39543fd870ac4_

#include <memory.h>

#include "String.h"

namespace ofw 
{

class StringBuilder
{
  size_t          capacity = 256;

  typedef struct _StringBuilderNode
  {
    size_t                      m_offset;
    size_t                      m_length;
    char                       *m_ptr;
    struct _StringBuilderNode  *m_next;
  } StringBuilderNode;

  StringBuilderNode *m_last;
  StringBuilderNode *m_head;

public:

  StringBuilder() { Init(); }
  StringBuilder(size_t capacity) : StringBuilder() 
    { this->capacity = capacity; }
  StringBuilder(String& refer) : StringBuilder() { Append(refer); }
  StringBuilder(String& refer, size_t capacity)
    : StringBuilder(capacity) { Append(refer); }
  ~StringBuilder() { DisposeInternal(); }

  void Append(const String& refer);
  void Append(char ch);
  template<typename wt_over>
  void Append(wt_over over) { Append(String(over)); }
  void Append(const char *str, size_t len);
  void Append(const char *str);
  size_t Find(const String& str);
  void Replace(const String& src, const String& tar);
  size_t Length() const;
  String ToString();
  void Dispose();
  size_t& Capacity() { return capacity; }
  void EnsureCpacity(size_t capacity);

private:

  bool MatchContinue(const String& str, size_t start_offset, 
    StringBuilderNode*  start_node, size_t node_offset);
  StringBuilderNode *ReplaceInternal(const String& src, const String& tar,
    StringBuilderNode*  start_node, size_t node_offset);
  StringBuilderNode *Create();
  void DisposeInternal();
  void Init();
  void Ensure();
  bool Expand();
  void LinkTo();
  void Migration();
};

}

#endif