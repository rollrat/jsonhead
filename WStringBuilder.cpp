//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#include "WStringBuilder.h"

using namespace jsonhead;

void WStringBuilder::Append(const WString& refer)
{
  if (!refer.Empty())
  {
    if (capacity > refer.Length() + m_last->m_length)
    {
      Ensure();

      memcpy(m_last->m_ptr + m_last->m_length, refer.Reference(), refer.Length()
        * sizeof(wchar_t));

      m_last->m_length += refer.Length();
    }
    else
    {
      Expand();

      if (capacity <= refer.Length())
      {
        m_last->m_ptr = (wchar_t *)refer.ToArray();
        m_last->m_length = refer.Length();
      }
      else
      {
        Append(refer);
        return;
      }

      LinkTo();
    }
  }
}

void WStringBuilder::Append(wchar_t ch)
{
  Ensure();

  if (m_last->m_length == capacity - 1)
    Expand();

  m_last->m_ptr[m_last->m_length++] = ch;
}

void WStringBuilder::Append(const wchar_t *str, size_t len)
{
  if (len > 0)
  {
    if (capacity > len + m_last->m_length)
    {
      Ensure();

      memcpy(m_last->m_ptr + m_last->m_length, str, len * sizeof(wchar_t));

      m_last->m_length += len;
    }
    else
    {
      Expand();

      m_last->m_length = len;
      m_last->m_ptr = new wchar_t[len];
      memcpy(m_last->m_ptr, str, len * sizeof(wchar_t));

      LinkTo();
    }
  }
}

void WStringBuilder::Append(const wchar_t *str)
{
  size_t len = wcslen(str);
  Append(str, len);
}

size_t WStringBuilder::Find(const WString& str)
{
  WStringBuilderNode *iter = m_head;
  for (; iter != nullptr; iter = iter->m_next)
  {
    if (iter->m_length == 0) continue;

    wchar_t last = iter->m_ptr[iter->m_length - 1];
    iter->m_ptr[iter->m_length - 1] = 0;

    if (wchar_t *offset = wcschr(iter->m_ptr, str[0]))
    {
      iter->m_ptr[iter->m_length - 1] = last;
      if (MatchContinue(str, 1, iter, offset - iter->m_ptr + 1) == true)
        return (size_t)(offset - iter->m_ptr) + iter->m_offset;
    }

    iter->m_ptr[iter->m_length - 1] = last;

    if (last == str[0])
    {
      if (MatchContinue(str, 1, iter->m_next, 0) == true)
        return iter->m_length - 1 + iter->m_offset;
    }
  }

  return WString::error;
}

void WStringBuilder::Replace(const WString & src, const WString & tar)
{
  WStringBuilderNode *iter = m_head;
  for (; iter != nullptr; iter = iter->m_next)
  {
    if (iter->m_length == 0) continue;

    wchar_t last = iter->m_ptr[iter->m_length - 1];
    wchar_t *offset;

    iter->m_ptr[iter->m_length - 1] = 0;
    offset = wcschr(iter->m_ptr, src[0]);
    iter->m_ptr[iter->m_length - 1] = last;

    if (offset)
    {
      iter->m_ptr[iter->m_length - 1] = last;
      if (MatchContinue(src, 1, iter, offset - iter->m_ptr + 1) == true)
        iter = ReplaceInternal(src, tar, iter, offset - iter->m_ptr);
    }

    if (last == src[0])
    {
      if (MatchContinue(src, 1, iter->m_next, 0) == true)
        iter = ReplaceInternal(src, tar, iter, iter->m_length - 1);
    }
  }
  Migration();
}

size_t WStringBuilder::Length() const
{
  return m_last->m_offset + m_last->m_length;
}

WString WStringBuilder::ToString()
{
  size_t len = m_last->m_offset + m_last->m_length;
  wchar_t *newString = new wchar_t[len + 1];

  WStringBuilderNode *iter = m_head;
  for (; iter != nullptr; iter = iter->m_next)
  {
    memcpy(newString + iter->m_offset, iter->m_ptr, iter->m_length
      * sizeof(wchar_t));
  }
  newString[len] = 0;

  return WString(newString, len, false);
}

void WStringBuilder::Dispose()
{
  DisposeInternal();
  Init();
}

void WStringBuilder::EnsureCpacity(size_t capacity)
{
  wchar_t *tmp = new wchar_t[capacity];

  memcpy(tmp, m_last->m_ptr, m_last->m_length * sizeof(wchar_t));
  delete[] m_last->m_ptr;

  m_last->m_ptr = tmp;
}

bool WStringBuilder::MatchContinue(const WString& str, size_t start_offset, 
  WStringBuilderNode* start_node, size_t node_offset)
{
  if (start_node == nullptr)
  {
    if (str.Length() == start_offset)
      return true;
    return false;
  }

  if (str.Length() - start_offset < start_node->m_length - node_offset)
  {
    if (WString(start_node->m_ptr, start_node->m_length).FindFirst(
      str.Reference() + start_offset, node_offset) != WString::error)
      return true;
    return false;
  }

  wchar_t last = start_node->m_ptr[start_node->m_length - 1];
  start_node->m_ptr[start_node->m_length - 1] = 0;

  if (wcscmp(start_node->m_ptr + node_offset, str.Reference() + start_offset) &&
    last != str[start_offset + start_node->m_length - node_offset - 1])
  {
    start_node->m_ptr[start_node->m_length - 1] = last;
    return false;
  }

  start_node->m_ptr[start_node->m_length - 1] = last;

  return MatchContinue(str, start_offset + start_node->m_length - node_offset,
    start_node->m_next, 0);
}

WStringBuilder::WStringBuilderNode *WStringBuilder::ReplaceInternal(
  const WString& src, const WString& tar,
  WStringBuilderNode * start_node, size_t node_offset)
{
  WStringBuilderNode *node = Create();
  node->m_length = tar.Length();
  node->m_ptr = new wchar_t[node->m_length];
  memcpy(node->m_ptr, tar.Reference(), tar.Length() * sizeof(wchar_t));

  if (node_offset + src.Length() <= start_node->m_length)
  {
    WStringBuilderNode *node1 = Create();
    node1->m_length = start_node->m_length - node_offset - src.Length();
    node1->m_ptr = new wchar_t[node1->m_length];

    memcpy(node1->m_ptr, start_node->m_ptr + node_offset + src.Length(), 
      (start_node->m_length - node_offset - src.Length()) * sizeof(wchar_t));

    start_node->m_length = node_offset;

    node1->m_next = start_node->m_next;
    start_node->m_next = node;
    node->m_next = node1;

    return node1;
  }
  else
  {
    WStringBuilderNode *last = start_node->m_next;
    size_t count = src.Length() - start_node->m_length + node_offset;
    for (; last != nullptr; last = last->m_next)
    {
      if (count < last->m_length)
        break;
      count -= last->m_length;
    }

    WStringBuilderNode *del = start_node->m_next;
    for (; del != last; )
    {
      WStringBuilderNode *tmp = del->m_next;
      delete[] del->m_ptr;
      delete tmp;
      del = tmp;
    }

    start_node->m_length = node_offset;
    start_node->m_next = node;

    node->m_next = last;

    last->m_ptr += count;
    last->m_length -= count;

    return node;
  }
}

WStringBuilder::WStringBuilderNode *WStringBuilder::Create()
{
  WStringBuilderNode *wsbn = new WStringBuilderNode;
  wsbn->m_length = 0;
  wsbn->m_ptr = nullptr;
  wsbn->m_next = nullptr;
  return wsbn;
}

void WStringBuilder::DisposeInternal()
{
  WStringBuilderNode *iter = m_head;
  WStringBuilderNode *prev = nullptr;

  for (; iter != nullptr; )
  {
    delete[] iter->m_ptr;
    prev = iter;
    iter = iter->m_next;
    delete prev;
  }

  m_head = m_last = nullptr;
}

void WStringBuilder::Init()
{
  m_last = m_head = Create();
  m_last->m_offset = 0;
}

void WStringBuilder::Ensure()
{
  if (m_last->m_ptr == nullptr)
  {
    m_last->m_ptr = new wchar_t[capacity];
  }
}

bool WStringBuilder::Expand()
{
  if (m_last->m_length > 0)
  {
    LinkTo();
    return true;
  }
  return false;
}

void WStringBuilder::LinkTo()
{
  WStringBuilderNode *twsbn = Create();
  twsbn->m_offset = m_last->m_length + m_last->m_offset;
  m_last->m_next = twsbn;
  m_last = twsbn;
}

void WStringBuilder::Migration()
{
  WStringBuilderNode *iter = m_head;

  iter->m_offset = 0;

  while (iter->m_next != nullptr)
  {
    iter->m_next->m_offset = iter->m_length + iter->m_offset;
    iter = iter->m_next;
  }
}