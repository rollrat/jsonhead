//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#ifndef _WSTRING_BUILDER_9bf1541fdf7efd41b7b39543fd870ac4_
#define _WSTRING_BUILDER_9bf1541fdf7efd41b7b39543fd870ac4_

#include <memory.h>

#include "WString.h"

namespace jsonhead 
{

class WStringBuilder
{
  size_t          capacity = 1024;

  typedef struct _WStringBuilderNode
  {
    size_t                      m_offset;
    size_t                      m_length;
    wchar_t                    *m_ptr;
    struct _WStringBuilderNode *m_next;
  } WStringBuilderNode;

  WStringBuilderNode *m_last;
  WStringBuilderNode *m_head;

public:

  WStringBuilder() { Init(); }
  WStringBuilder(size_t capacity) : WStringBuilder() 
    { this->capacity = capacity; }
  WStringBuilder(WString& refer) : WStringBuilder() { Append(refer); }
  WStringBuilder(WString& refer, size_t capacity)
    : WStringBuilder(capacity) { Append(refer); }
  ~WStringBuilder() { DisposeInternal(); }

  void Append(const WString& refer);
  void Append(wchar_t ch);
  template<typename wt_over>
  void Append(wt_over over) { Append(WString(over)); }
  void Append(const wchar_t *str, size_t len);
  void Append(const wchar_t *str);
  size_t Find(const WString& str);
  void Replace(const WString& src, const WString& tar);
  size_t Length() const;
  WString ToString();
  void Dispose();
  size_t& Capacity() { return capacity; }
  void EnsureCpacity(size_t capacity);

private:

  bool MatchContinue(const WString& str, size_t start_offset, 
    WStringBuilderNode*  start_node, size_t node_offset);
  WStringBuilderNode *ReplaceInternal(const WString& src, const WString& tar,
    WStringBuilderNode*  start_node, size_t node_offset);
  WStringBuilderNode *Create();
  void DisposeInternal();
  void Init();
  void Ensure();
  bool Expand();
  void LinkTo();
  void Migration();
};

}

#endif