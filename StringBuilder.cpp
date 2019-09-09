//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#include "StringBuilder.h"

using namespace ofw;

void StringBuilder::Append(const String& refer)
{
  if (!refer.Empty())
  {
    if (capacity > refer.Length() + m_last->m_length)
    {
      Ensure();

      memcpy(m_last->m_ptr + m_last->m_length, refer.Reference(), refer.Length()
        * sizeof(char));

      m_last->m_length += refer.Length();
    }
    else
    {
      Expand();

      if (capacity <= refer.Length())
      {
        m_last->m_ptr = (char *)refer.ToArray();
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

void StringBuilder::Append(char ch)
{
  Ensure();

  if (m_last->m_length == capacity - 1)
    if (Expand())
      Ensure();

  m_last->m_ptr[m_last->m_length++] = ch;
}

void StringBuilder::Append(const char *str, size_t len)
{
  if (len > 0)
  {
    if (capacity > len + m_last->m_length)
    {
      Ensure();

      memcpy(m_last->m_ptr + m_last->m_length, str, len * sizeof(char));

      m_last->m_length += len;
    }
    else
    {
      Expand();

      m_last->m_length = len;
      m_last->m_ptr = new char[len];
      memcpy(m_last->m_ptr, str, len * sizeof(char));

      LinkTo();
    }
  }
}

void StringBuilder::Append(const char *str)
{
  size_t len = strlen(str);
  Append(str, len);
}

size_t StringBuilder::Find(const String& str)
{
  StringBuilderNode *iter = m_head;
  for (; iter != nullptr; iter = iter->m_next)
  {
    if (iter->m_length == 0) continue;

    char last = iter->m_ptr[iter->m_length - 1];
    iter->m_ptr[iter->m_length - 1] = 0;

    if (char *offset = strchr(iter->m_ptr, str[0]))
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

  return String::error;
}

void StringBuilder::Replace(const String & src, const String & tar)
{
  StringBuilderNode *iter = m_head;
  for (; iter != nullptr; iter = iter->m_next)
  {
    if (iter->m_length == 0) continue;

    char last = iter->m_ptr[iter->m_length - 1];
    char *offset;

    iter->m_ptr[iter->m_length - 1] = 0;
    offset = strchr(iter->m_ptr, src[0]);
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

size_t StringBuilder::Length() const
{
  return m_last->m_offset + m_last->m_length;
}

String StringBuilder::ToString()
{
  size_t len = m_last->m_offset + m_last->m_length;
  char *neString = new char[len + 1];

  StringBuilderNode *iter = m_head;
  for (; iter != nullptr; iter = iter->m_next)
  {
    memcpy(neString + iter->m_offset, iter->m_ptr, iter->m_length
      * sizeof(char));
  }
  neString[len] = 0;

  return String(neString, len, false);
}

void StringBuilder::Dispose()
{
  DisposeInternal();
  Init();
}

void StringBuilder::EnsureCpacity(size_t capacity)
{
  char *tmp = new char[capacity];

  memcpy(tmp, m_last->m_ptr, m_last->m_length * sizeof(char));
  delete[] m_last->m_ptr;

  m_last->m_ptr = tmp;
}

bool StringBuilder::MatchContinue(const String& str, size_t start_offset, 
  StringBuilderNode* start_node, size_t node_offset)
{
  if (start_node == nullptr)
  {
    if (str.Length() == start_offset)
      return true;
    return false;
  }

  if (str.Length() - start_offset < start_node->m_length - node_offset)
  {
    if (String(start_node->m_ptr, start_node->m_length).FindFirst(
      str.Reference() + start_offset, node_offset) != String::error)
      return true;
    return false;
  }

  char last = start_node->m_ptr[start_node->m_length - 1];
  start_node->m_ptr[start_node->m_length - 1] = 0;

  if (strcmp(start_node->m_ptr + node_offset, str.Reference() + start_offset) &&
    last != str[start_offset + start_node->m_length - node_offset - 1])
  {
    start_node->m_ptr[start_node->m_length - 1] = last;
    return false;
  }

  start_node->m_ptr[start_node->m_length - 1] = last;

  return MatchContinue(str, start_offset + start_node->m_length - node_offset,
    start_node->m_next, 0);
}

StringBuilder::StringBuilderNode *StringBuilder::ReplaceInternal(
  const String& src, const String& tar,
  StringBuilderNode * start_node, size_t node_offset)
{
  StringBuilderNode *node = Create();
  node->m_length = tar.Length();
  node->m_ptr = new char[node->m_length];
  memcpy(node->m_ptr, tar.Reference(), tar.Length() * sizeof(char));

  if (node_offset + src.Length() <= start_node->m_length)
  {
    StringBuilderNode *node1 = Create();
    node1->m_length = start_node->m_length - node_offset - src.Length();
    node1->m_ptr = new char[node1->m_length];

    memcpy(node1->m_ptr, start_node->m_ptr + node_offset + src.Length(), 
      (start_node->m_length - node_offset - src.Length()) * sizeof(char));

    start_node->m_length = node_offset;

    node1->m_next = start_node->m_next;
    start_node->m_next = node;
    node->m_next = node1;

    return node1;
  }
  else
  {
    StringBuilderNode *last = start_node->m_next;
    size_t count = src.Length() - start_node->m_length + node_offset;
    for (; last != nullptr; last = last->m_next)
    {
      if (count < last->m_length)
        break;
      count -= last->m_length;
    }

    StringBuilderNode *del = start_node->m_next;
    for (; del != last; )
    {
      StringBuilderNode *tmp = del->m_next;
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

StringBuilder::StringBuilderNode *StringBuilder::Create()
{
  StringBuilderNode *wsbn = new StringBuilderNode;
  wsbn->m_length = 0;
  wsbn->m_ptr = nullptr;
  wsbn->m_next = nullptr;
  return wsbn;
}

void StringBuilder::DisposeInternal()
{
  StringBuilderNode *iter = m_head;
  StringBuilderNode *prev = nullptr;

  for (; iter != nullptr; )
  {
    delete[] iter->m_ptr;
    prev = iter;
    iter = iter->m_next;
    delete prev;
  }

  m_head = m_last = nullptr;
}

void StringBuilder::Init()
{
  m_last = m_head = Create();
  m_last->m_offset = 0;
}

void StringBuilder::Ensure()
{
  if (m_last->m_ptr == nullptr)
  {
    m_last->m_ptr = new char[capacity];
  }
}

bool StringBuilder::Expand()
{
  if (m_last->m_length > 0)
  {
    LinkTo();
    return true;
  }
  return false;
}

void StringBuilder::LinkTo()
{
  StringBuilderNode *twsbn = Create();
  twsbn->m_offset = m_last->m_length + m_last->m_offset;
  m_last->m_next = twsbn;
  m_last = twsbn;
}

void StringBuilder::Migration()
{
  StringBuilderNode *iter = m_head;

  iter->m_offset = 0;

  while (iter->m_next != nullptr)
  {
    iter->m_next->m_offset = iter->m_length + iter->m_offset;
    iter = iter->m_next;
  }
}