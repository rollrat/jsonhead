//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#define _CRT_SECURE_NO_WARNINGS

#include "String.h"
#include <memory.h>
#include <string.h>
#include <wchar.h>

using namespace ofw;

#define align_address(n) \
    ((ptr_type *)( (ptr_type)(n) & ~(ptr_type)( sizeof(ptr_type) - 1 ) ) + 1)

#define align_address2(n) \
    ((ptr_type *)( (ptr_type)(n) & ~(ptr_type)( sizeof(ptr_type) - 1 ) ))

// find zero in string
static const char *findzero(const char *str)
{
  if (!str[0]) return str;
  else if (!str[1]) return str + 1;
  else if (!str[2]) return str + 2;
  else if (!str[3]) return str + 3;

  // This block will optimized on your compiler.
  if (sizeof(ptr_type) == sizeof(uint64_t))
  {
    if (!str[4]) return str + 4;
    else if (!str[5]) return str + 5;
    else if (!str[6]) return str + 6;
    else if (!str[7]) return str + 7;
  }
}

/* this function provide counting string-length method by null-terminated string 
   but is too big, to use this function should be considered prior using.
   or you can make partition routines. */
size_t ofw::StringTools::strlen(const char *str)
{
  if (sizeof(char) != 1)
    // Call built-in function.
    return ::strlen(str);

#if !defined(_COMPILER_MS)
  // The strlen function, except for msvc, is optimized very well.
  return ::strlen(str);
#else

  ptr_type* trim;

  // set has zero checker byte
  const ptr_type less_magic = (ptr_type)(~0ULL / 0xff);
  const ptr_type most_magic = (less_magic << 7);

  trim = align_address(str);

  // Routine 1 Find 0 and 128~255 Byte value in pointer type value.
  if ((*(ptr_type *)str - less_magic) & most_magic)
  {
    if ((*(ptr_type *)str - less_magic) & (~*(ptr_type *)str & most_magic))
      return findzero(str) - str;
  }
  else
  {
    while (1)
    {
      if ((trim[0] - less_magic) & most_magic)
        { trim = &trim[0]; break; }
      if ((trim[1] - less_magic) & most_magic)
        { trim = &trim[1]; break; }
      if ((trim[2] - less_magic) & most_magic)
        { trim = &trim[2]; break; }
      if ((trim[3] - less_magic) & most_magic)
        { trim = &trim[3]; break; }
      if (sizeof(ptr_type) == sizeof(uint32_t))
      {
        if ((trim[4] - less_magic) & most_magic)
          { trim = &trim[4]; break; }
        if ((trim[5] - less_magic) & most_magic)
          { trim = &trim[5]; break; }
        if ((trim[6] - less_magic) & most_magic)
          { trim = &trim[6]; break; }
        if ((trim[7] - less_magic) & most_magic)
          { trim = &trim[7]; break; }
      }
      trim += (sizeof(uint64_t) + sizeof(uint32_t) - sizeof(ptr_type));
    }
  }

  // Routine 2 Find Zero Byte in pointer type value.
  if ((*trim - less_magic) & (~*trim & most_magic))
  {
    return findzero((const char *)trim) - str;
  }

  while (1)
  {
    if ((trim[0] - less_magic) & (~trim[0] & most_magic))
      return findzero((const char *)(trim + 0)) - str;
    if ((trim[1] - less_magic) & (~trim[1] & most_magic))
      return findzero((const char *)(trim + 1)) - str;
    if ((trim[2] - less_magic) & (~trim[2] & most_magic))
      return findzero((const char *)(trim + 2)) - str;
    if ((trim[3] - less_magic) & (~trim[3] & most_magic))
      return findzero((const char *)(trim + 3)) - str;
    if (sizeof(ptr_type) == sizeof(uint32_t))
    {
      if ((trim[4] - less_magic) & (~trim[4] & most_magic))
        return findzero((const char *)(trim + 4)) - str;
      if ((trim[5] - less_magic) & (~trim[5] & most_magic))
        return findzero((const char *)(trim + 5)) - str;
      if ((trim[6] - less_magic) & (~trim[6] & most_magic))
        return findzero((const char *)(trim + 6)) - str;
      if ((trim[7] - less_magic) & (~trim[7] & most_magic))
        return findzero((const char *)(trim + 7)) - str;
    }
    trim += (sizeof(uint64_t) + sizeof(uint32_t) - sizeof(ptr_type));
  }
#endif
}

char *ofw::StringTools::strrnchr(char * ptr, size_t len, char ch)
{
  static_assert(sizeof(char) == 1, 
    "The 'strrnchr' function is not supported on your environment.");

  char *lptr = ptr + len - 1;
  char *aptr = (char *)align_address2(lptr);

  ptr_type less_magic = (ptr_type)(~0ULL / 0xff);
  ptr_type most_magic = (less_magic << 7);

  ptr_type wide_checker = ((ptr_type)ch << 0)| ((ptr_type)ch << 8) | ((ptr_type)ch << 16)| ((ptr_type)ch << 24);
  if (sizeof(ptr_type) == sizeof(uint64_t))
    wide_checker |= ((ptr_type)ch << 32)  | ((ptr_type)ch << 40)| ((ptr_type)ch << 48) | ((ptr_type)ch << 56);

  if (aptr < ptr) aptr = ptr;

  for (; lptr >= aptr; --lptr)
    if (*lptr == ch) return lptr;

  const size_t count = sizeof(ptr_type) / sizeof(char);
  for (lptr = aptr - count; lptr >= ptr; lptr -= count)
  {
    ptr_type nptr = *(ptr_type *)lptr ^ wide_checker;

    if ((nptr - less_magic) & (~nptr & most_magic))
    {
      if (sizeof(ptr_type) == sizeof(uint64_t))
      {
        if (lptr[7] == ch) return lptr + 7;
        if (lptr[6] == ch) return lptr + 6;
        if (lptr[5] == ch) return lptr + 5;
        if (lptr[4] == ch) return lptr + 4;
      }
      if (lptr[3] == ch) return lptr + 3;
      if (lptr[2] == ch) return lptr + 2;
      if (lptr[1] == ch) return lptr + 1;
      return lptr;
    }
  }

  const size_t edge = sizeof(ptr_type) - 1;
  for (lptr += edge; lptr >= aptr; --lptr)
    if (*lptr == ch) return lptr;

  return nullptr;
}

char *ofw::StringTools::strrnstrn(char * ptr, size_t ptrlen, 
  const char * dest, size_t destlen)
{
  char *tptr;
  size_t len2 = destlen * sizeof(char);

  while (tptr = strrnchr(ptr, ptrlen, *dest))
  {
    ptrlen = tptr - ptr;
    if (!memcmp(tptr, dest, len2))
      return tptr;
  }

  return NULL;
}

size_t ofw::StringTools::strcountch(char * ptr, char * last, char ch)
{
  static_assert(sizeof(char) == 1, "Do not use 'wcountch' function!");

  size_t count = 0;
  char *trim = (char *)align_address(ptr);
  
  const ptr_type less_magic = (ptr_type)(~0ULL / 0xff);
  const ptr_type most_magic = (less_magic << 7);

  ptr_type wide_checker = ((ptr_type)ch << 0)| ((ptr_type)ch << 8) | ((ptr_type)ch << 16)| ((ptr_type)ch << 24);
  if (sizeof(ptr_type) == sizeof(uint64_t))
    wide_checker |= ((ptr_type)ch << 32)  | ((ptr_type)ch << 40)| ((ptr_type)ch << 48) | ((ptr_type)ch << 56);

  if (trim < ptr) trim = ptr;

  for (; ptr < trim; ptr++)
    if (*ptr == ch) count++;
  
  const size_t countc = sizeof(ptr_type) / 4;
  for (ptr = trim; last - countc >= ptr; ptr += countc)
  {
    ptr_type nptr = *(ptr_type *)ptr ^ wide_checker;
    
    if ((nptr - less_magic) & (~nptr & most_magic))
    {
      if (ptr[0] == ch) count++;
      if (ptr[1] == ch) count++;
      if (ptr[2] == ch) count++;
      if (ptr[3] == ch) count++;
      
      if (sizeof(ptr_type) == sizeof(uint64_t))
      {
        if (ptr[4] == ch) count++;
        if (ptr[5] == ch) count++;
        if (ptr[6] == ch) count++;
        if (ptr[7] == ch) count++;
      }
    }
  }

  return count;
}

///===-----------------------------------------------------------------------===
///
///               String  Constructor
///
///===-----------------------------------------------------------------------===

String::String(char *str, size_t len, bool built_in) 
  : length(len), 
    first(str), 
    last(str + len - 1),
    tm(built_in) 
{
}

String::String(char ch, size_t count)
  : length(count)
{
  first = alloc(length + 1);
  last = first + length - 1;
  _strnset(first, ch, count);
  first[length] = 0; 
}

String::String(char ch)
  : length (1)
{
  first = alloc(2);
  last = first;
  *first = ch;
  *(last + 1) = 0;
}

String::String(int num)
{
  char buffer[65];
  _itoa(num, buffer, 10);
  InitString((const char *)buffer);
}

String::String(long int num)
{
  char buffer[65];
  _ltoa(num, buffer, 10);
  InitString((const char *)buffer);
}

String::String(long long int num)
{
  char buffer[65];
  sprintf(buffer, "%lld", num);
  InitString((const char *)buffer);
}

String::String(unsigned int num)
{
  char buffer[65];
  sprintf(buffer, "%u", num);
  InitString((const char *)buffer);
}

String::String(unsigned long int num)
{
  char buffer[65];
  sprintf(buffer, "%lu", num);
  InitString((const char *)buffer);
}

String::String(unsigned long long int num)
{
  char buffer[65];
  sprintf(buffer, "%llu", num);
  InitString((const char *)buffer);
}

String::String(float num)
{
  char buffer[65];
  sprintf(buffer, "%g", num);
  InitString((const char *)buffer);
}

String::String(double num)
{
  char buffer[65];
  sprintf(buffer, "%lg", num);
  InitString((const char *)buffer);
}

String String::Concat(const String& t1, const String& t2)
{
  if (t1.Empty())
  {
      if (t2.Empty())
      {
          return String();
      }
      return String((const char *)t2.first, t2.length);
  }
  
  if (t2.Empty())
  {
      return String((const char *)t1.first, t1.length);
  }
  
  size_t newSize = t1.length + t2.length;
  char * mergerString;
  
  mergerString = new char[newSize + 1];
  memcpy(mergerString, t1.first, t1.length * sizeof(char));
  memcpy(mergerString + t1.length, t2.first, t2.length * sizeof(char));
  mergerString[newSize] = 0;
  
  return String(mergerString, newSize, false);
}

String String::Concat(const String& t1, const String& t2, const String& t3)
{
  if (t1.Empty() && t2.Empty() && t3.Empty())
  {
      return String();
  }
  
  size_t newSize = t1.length + t2.length + t3.length;
  char * mergerString;
  
  mergerString = new char[newSize + 1];
  memcpy(mergerString, t1.first, t1.length * sizeof(char));
  memcpy(mergerString + t1.length, t2.first, t2.length * sizeof(char));
  memcpy(mergerString + t1.length + t2.length, t3.first, t3.length
    * sizeof(char));
  mergerString[newSize] = 0;
  
  return String(mergerString, newSize, false);
}

String String::Concat(const String& t1, const String& t2, 
  const String& t3, const String& t4)
{
  if (t1.Empty() && t2.Empty() && t3.Empty() && t4.Empty())
  {
      return String();
  }
  
  size_t newSize = t1.length + t2.length + t3.length + t4.length;
  char * mergerString;
  
  mergerString = new char[newSize + 1];
  memcpy(mergerString, t1.first, t1.length * sizeof(char));
  memcpy(mergerString + t1.length, t2.first, t2.length * sizeof(char));
  memcpy(mergerString + t1.length + t2.length, t3.first, t3.length 
    * sizeof(char));
  memcpy(mergerString + t1.length + t2.length + t3.length, t4.first, 
    t4.length * sizeof(char));
  mergerString[newSize] = 0;

  return String(mergerString, newSize, false);
}

size_t String::TirmStartPos() const
{
  const char *ptr = first;
  while (*ptr)
  {
    if (!isspace((int)*ptr))
      break;
    else
      ptr++;
  }
  return ptr - first;
}

size_t String::TrimEndPos() const
{
  char *ptr = last;
  while (ptr >= first)
  {
    if (!isspace((int)*ptr))
      break;
    else
      ptr--;
  }
  return ptr - first;
}

size_t String::TrimStartPos(char ch) const
{
  const char *ptr = first;
  while (*ptr)
  {
    if (*ptr != ch)
      break;
    else
      ptr++;
  }
  return ptr - first;
}

size_t String::TrimEndPos(char ch) const
{
  char *ptr = last;
  while (ptr >= first)
  {
    if (*ptr != ch)
      break;
    else
      ptr--;
  }
  return ptr - first;
}

String String::TrimStart()
{
  const char *ptr = first;
  while (*ptr)
  {
    if (!isspace((int)*ptr))
      break;
    else
      ptr++;
  }
  return String(ptr, length + first - ptr);
}

String String::TrimEnd()
{
  char *ptr = last;
  while (ptr >= first)
  {
    if (!isspace((int)*ptr))
      break;
    else
      ptr--;
  }
  return String((const char *)first, ptr - first + 1);
}

String String::Trim()
{
  const char *fptr = first;
  char *ptr = first + length - 1;
  while (*fptr)
  {
    if (!isspace((int)*fptr))
      break;
    else
      fptr++;
  }
  while (ptr >= first)
  {
    if (!isspace((int)*ptr))
      break;
    else
      ptr--;
  }
  return String(fptr, ptr - fptr + 1);
}

String String::TrimStrat(char ch)
{
  const char *ptr = first;
  while (*ptr)
  {
    if (*ptr != ch)
      break;
    else
      ptr++;
  }
  return String(ptr, length + first - ptr);
}

String String::TrimEnd(char ch)
{
  char *ptr = last;
  while (ptr >= first)
  {
    if (*ptr != ch)
      break;
    else
      ptr--;
  }
  return String((const char *)first, ptr - first + 1);
}

String String::Trim(char ch)
{
  const char *fptr = first;
  char *ptr = first + length - 1;
  while (*fptr)
  {
    if (*fptr != ch)
      break;
    else
      fptr++;
  }
  while (ptr >= first)
  {
    if (*ptr != ch)
      break;
    else
      ptr--;
  }
  return String(fptr, ptr - fptr + 1);
}

String String::ToLower()
{
  char *ret = new char[length + 1];
  for (size_t i = 0; i < length; i++)
    ret[i] = tolower(first[i]);
  ret[length] = 0;
  return String(ret, length, false);
}

String String::ToUpper()
{
  char *ret = new char[length + 1];
  for (size_t i = 0; i < length; i++)
    ret[i] = toupper(first[i]);
  ret[length] = 0;
  return String(ret, length, false);
}

String String::Capitalize()
{
  char *ret = this->ToArray();
  *ret = toupper(*ret);
  return String(ret, length, false);
}

String String::Title()
{
  char *ret = this->ToArray();
  *ret = toupper(*ret);
  for (size_t i = 0; i < length - 1; i++)
    if (first[i] == L' ')
      ret[i + 1] = toupper(first[i + 1]);
  return String(ret, length, false);
}

String String::PadLeft(size_t len, char pad)
{
  if (len > length)
  {
    char* ret = new char[len + 1];
    size_t   padlen = len - length;

    _strnset(ret, pad, padlen);
    memcpy(ret + padlen, first, length * sizeof(char));

    ret[len] = 0;

    return String(ret, len, false);
  }
  else
  {
    return String((const char *)first, length);
  }
}

String String::PadRight(size_t len, char pad)
{
  if (len > length)
  {
    char *ret = new char[len + 1];

    memcpy(ret, first, length * sizeof(char));
    _strnset(ret + length, pad, len - length);

    ret[len] = 0;

    return String(ret, len, false);
  }
  else
  {
    return String((const char *)first, length);
  }
}

String String::PadCenter(size_t len, char pad, bool lefts)
{
  if (len > length)
  {
    size_t padlen = len - length;
    size_t lpadlen = padlen / 2 + ((padlen & 1) && lefts);
    size_t rpadlen = padlen - lpadlen;

    char *ret = new char[len + 1];

    _strnset(ret, pad, lpadlen);
    memcpy(ret + lpadlen, first, length * sizeof(char));
    _strnset(ret + lpadlen + length, pad, rpadlen);

    ret[len] = 0;

    return String(ret, len, false);
  }
  else
  {
    return String((const char *)first, length);
  }
}

String String::Remove(size_t starts, size_t len)
{
  size_t retlen = length - len;
  char *neString = new char[retlen + 1];

  memcpy(neString, first, starts * sizeof(char));
  memcpy(neString + starts, first + starts + len, (retlen - starts) 
    * sizeof(char));

  neString[retlen] = 0;

  return String(neString, retlen, false);
}

String String::Repeat(size_t count)
{
  size_t newLen = count * length;
  size_t dlength = length * sizeof(char);
  char *neString = new char[newLen + 1];

  for (size_t i = 0; i < count; i++)
  {
    memcpy(neString + i * length, first, dlength);
  }

  neString[newLen] = 0;

  return String(neString, newLen, false);
}

String String::Reverse()
{
  char *ret = this->ToArray();
  _strrev(ret);
  return String(ret, length, false);
}

String String::Slice(size_t first, size_t last)
{
  if ((int)last > 0)
  {
    return String((const char *)(first + first), last - first + 1);
  }
  else
  {
    return String((const char *)(first + first), length - first + last);
  }
}

String String::Slicing(size_t jmp, size_t starts, size_t len,
  bool remain)
{
  size_t   searchLen = length - starts;

  if (len <= searchLen)
  {
    size_t   chunkLen = jmp + len;
    size_t   fixedLen = (searchLen / chunkLen) * len;
    size_t   lastRemain = searchLen % chunkLen;
    size_t   remainLen = lastRemain >= len ? len : (remain ? lastRemain : 0);
    size_t   totalLen = fixedLen + remainLen;
    char* collect = new char[totalLen + 1];
    char* colptr = collect;

    size_t   countLen = starts;
    size_t   putLen2 = len * sizeof(char);

    size_t   copyLen = length - lastRemain;

    for (; countLen < copyLen; countLen += chunkLen)
    {
      memcpy(colptr, first + countLen, putLen2);
      colptr += len;
    }

    if (remainLen)
    {
      memcpy(colptr, first + countLen, remainLen * sizeof(char));
    }

    collect[totalLen] = 0;

    return String(collect, totalLen);
  }
  else if (remain)
  {
    return String((const char *)(first + starts), searchLen);
  }

  return String();
}

uint64_t String::Hash(uint64_t seed) const
{
  uint64_t num_hash = seed;
  size_t   length = this->length;
  size_t   count = 0;

  while (length-- >= count++)
  {
    num_hash += first[count];
    num_hash ^= first[length] * seed;
  }

  return num_hash * ((seed << 16) + (num_hash >> 16) + (num_hash << 32));
}

bool String::IsNumeric() const
{
  const char *ptr = first;

  if (*ptr == L'-' || *ptr == L'+') ptr++;
  while (iswdigit(*ptr) && *ptr) ptr++;
  if (*ptr == L'.') ptr++;
  while (iswdigit(*ptr) && *ptr)ptr++;

  if (*ptr == L'e' || *ptr == L'E')
  {
    ptr++;
    if (*ptr == L'+' || *ptr == L'-' || iswdigit(*ptr))
    { ptr++; while (iswdigit(*ptr) && *ptr) ptr++; }
  }

  return *ptr == 0;
}

bool String::IsHexDigit() const
{
  const char *ptr = first;
  if (*ptr == L'0' && (ptr[1] == L'x' || ptr[1] == L'X')) ptr += 2;
  while (iswxdigit(*ptr) && *ptr) ptr++;
  return *ptr == 0;
}

unsigned long long int String::ToHexDigit() const
{
  unsigned long long int ret = 0;
  const char *ptr = first;

  if (*ptr == L'0' && (ptr[1] == L'x' || ptr[1] == L'X'))
    ptr += 2;

  while (*ptr)
  {
    if (iswdigit(*ptr))
      ret = ret * 16 + *ptr++ - L'0';
    else // no check
      ret = ret * 16 + (*ptr++ & 15) + 9;
  }

  return ret;
}

long long int String::ToLongLong() const
{
  long long int ret = 0, mark = 1;
  const char *ptr = first;

  if (*ptr == L'-')
    mark = -1, ptr++;
  else if (*ptr == L'+')
    ptr++;

  while (*ptr)
    ret = ret * 10 + (*ptr++ & 0xf);

  return ret * mark;
}

unsigned long long int String::ToULongLong() const
{
  long long int ret = 0;
  const char *ptr = first;

  while (*ptr)
    ret = ret * 10 + (*ptr++ & 0xf);

  return ret;
}

char *String::ToArray() const
{
  char *ret = new char[length + 1];
  memcpy(ret, first, (length + 1) * sizeof(char));
  return ret;
}

char *String::ToAnsi()
{
  return UnicodeToAnsi();
}

String::Utf8Array String::ToUtf8(bool file_bom)
{
  size_t szReal = file_bom + (file_bom << 1);
  size_t size = szReal + length;
  unsigned long *tmp = new unsigned long[size];
  unsigned long *ptr = tmp;

  if (file_bom)
  {
    ptr[0] = 0xef; ptr[1] = 0xbb; ptr[2] = 0xbf;

    ptr += 3;
  }

  for (size_t i = 0; i < length; i++)
  {
    char ch = first[i];
    unsigned long put = ch;

    if (ch > 0x7f)
    {
      put = (ch & 0x3f) | ((ch << 2) & 0x3f00);
      if (ch < 0x800)
        szReal += 2, put |= 0xc080;
      else
        szReal += 3, put |= ((ch << 4) & 0x3f0000) | 0xe08080;
    }
    else
      szReal++;

    ptr[i] = put;
  }

  unsigned char *bytes = new unsigned char[szReal];

  for (size_t i = 0, j = 0; i < size; i++)
  {
    if (tmp[i] >= 0x10000)
    {
      bytes[j] = (unsigned char)((tmp[i] & 0xff0000) >> 16);
      bytes[j + 1] = (unsigned char)((tmp[i] & 0x00ff00) >> 8);
      bytes[j + 2] = (unsigned char)((tmp[i] & 0x0000ff));
      j += 3;
    }
    else if (tmp[i] >= 0x100)
    {
      bytes[j] = (unsigned char)((tmp[i] & 0xff00) >> 8);
      bytes[j + 1] = (unsigned char)((tmp[i] & 0xff));
      j += 2;
    }
    else
    {
      bytes[j] = (unsigned char)((tmp[i] & 0xff));
      j += 1;
    }
  }

  delete[] tmp;

  return String::Utf8Array(bytes, szReal);
}

void String::Swap(String& refer)
{
  // "swap" function will probably call "move" fucntion.
  std::swap(first, refer.first);
  std::swap(last, refer.last);
  std::swap(length, refer.length);
}

String& ofw::String::operator=(String && refer)
{
  this->Swap(refer);
  refer.tm = this->tm;
  this->tm = false;
  return *this;
}

String& String::operator=(const String& refer)
{
  if (first != nullptr)
    delete[] first;
  if (length = refer.length)
  {
    first = new char[length + 1];
    last = first + length - 1;
    memcpy(first, refer.first, (length + 1) * sizeof(char));
  }
  return *this;
}

void String::CloneSet(const String& refer)
{
  if (first != nullptr)
    delete[] first;
  first = nullptr;
  tm = true;
  first = refer.first;
  last = refer.last;
  length = refer.length;
}

String String::Clone()
{
  String nstr;
  nstr.CloneSet(*this);
  return nstr;
}

///===-----------------------------------------------------------------------===
///
///               String  Helper
///
///===-----------------------------------------------------------------------===

String String::AppendHelper(const char *str, size_t len)
{
  size_t newSize = length + len;
  char * appendString;
  
  appendString = new char[newSize + 1];
  
  if (length > 0)
      memcpy(appendString, first, length * sizeof(char));
  memcpy(appendString + length, str, len * sizeof(char));
  appendString[newSize] = 0;
  
  return String(appendString, newSize, false);
}

size_t String::FindFirstHelper(const char *str, size_t starts) const
{
  if (starts >= length)return error;
  const char *ptr = strstr(first + starts, str);
  return ptr != NULL ? ptr - first : error;
}

size_t String::FindLastHelper(const char *str, size_t ends, size_t len) const
{
  if (ends >= length)return error;
  const char *ptr = StringTools::strrnstrn(first, length - ends, str, len);
  return ptr != NULL ? ptr - first : error;
}

size_t String::FindFirst(const char ch, size_t starts) const
{
  const char *ptr = strchr(first + starts, ch);
  return ptr != NULL ? ptr - first : error;
}

size_t String::FindLast(const char ch, size_t ends) const
{
  const char *ptr = StringTools::strrnchr(first, length - ends, ch);
  return (ptr != NULL) ? ptr - first : error;
}

static char to_switch(char ch)
{
  if (ch <= L'z')
  {
    if (ch <= L'Z' && ch >= L'A') return ch | 0x0020;
    if (ch >= L'a') return ch ^ 0x0020;
  }
  return ch;
}

static char to_wlower(char ch)
{
  if (ch <= L'Z' && ch >= L'A') return ch ^ 0x0020;
  return ch;
}

bool String::ContainsHelper(const char *str, size_t len, bool ignore) const
{
  if (len > length) return false;
  if (!ignore) return strstr(first, str) != NULL;

  const char *s1, *s2, *ptr = first;
  size_t len_1 = len - 1;
  
  char *t1 = first;
  char *t2 = first;
  char *searchMax = last - len + 1;
  
  char strlast = to_wlower(str[len_1]);
  char w = to_switch(*str);
  
  t1 = strchr(t1, *str);
  t2 = strchr(t2, w);
  
#define _CONTAINS_HELPER_SKIP(t,ch) {\
  if (tolower(t[len_1]) == strlast) {\
    s1 = t + 1;s2 = str + 1;\
    while (*s1 && *s2 && !(tolower(*s1) - tolower(*s2)))\
      s1++, s2++;\
    if (*s2 == 0)return true;\
  } t = strchr(t + 1, ch);}

  while (true)
  {
    if (t1 && t2) {
      if (t1 < t2 && t1 < searchMax) _CONTAINS_HELPER_SKIP(t1, *str)
      else if (t2 < searchMax) _CONTAINS_HELPER_SKIP(t2, w)
      else  break;
    } else {
      if (t1)
        while (t1)
          if (t1 > searchMax) return false;
          else _CONTAINS_HELPER_SKIP(t1, *str)
      else if (t2)
        while (t2)
          if (t2 > searchMax) return false;
          else _CONTAINS_HELPER_SKIP(t2, w)
      else break;
    }
  }

  return false;
}

size_t String::CountHelper(const char *str, size_t len) const
{
  size_t ret = 0;
  const char *ptr = first;
  for (; ptr = strstr(ptr, str); ptr += len, ret++)
    ;
  return ret;
}

String::ArrayType String::SplitHelper(const char *src, 
  size_t srclen, size_t max)
{
  size_t         alloclen = max <= length ? max : length;
  char         **position = new char*[alloclen];
  size_t        *poslen = new size_t[alloclen];
  size_t         count = 0;
  char *ptr = first, *tptr;

  for (; (tptr = strstr(ptr, src)) && max; max--, count++)
  {
    position[count] = ptr;
    poslen[count] = tptr - ptr;
    ptr = tptr + srclen;
  }

  bool max_remain = max > 0;

  String **n = new String*[count + max_remain];

  for (size_t i = 0; i < count; i++)
  {
    n[i] = new String((const char *)(position[i]), poslen[i]);
  }

  if (max_remain)
  {
    n[count] = new String((const char *)(ptr), length + first - ptr);
  }

  delete[] position;
  delete[] poslen;

  return String::ArrayType(n, count + max_remain);
}

String::ArrayType String::SplitSlowHelper(const char *src, 
  size_t srclen, size_t max)
{
  size_t      max_dec = max;
  size_t      count = 0;
  size_t      i = 0;
  char    *ptr, *tptr;

  for (ptr = first; (tptr = strstr(ptr, src)) && max_dec; max_dec--, count++)
  {
    ptr = tptr + srclen;
  }

  bool max_remain = max > 0;

  String **n = new String*[count + max_remain];

  for (ptr = first; (tptr = strstr(ptr, src)) && max; max--, i++)
  {
    n[i] = new String((const char *)(ptr), tptr - ptr);
    ptr = tptr + srclen;
  }

  if (max_remain)
  {
    n[count] = new String((const char *)(ptr), length + first - ptr);
  }

  return ArrayType(n, count + max_remain);
}

String String::SplitPositionHelper(const char *src, 
  size_t srclen, size_t pos)
{
  char *ptr = first, *tptr;

  for (ptr = first; (tptr = strstr(ptr, src)) && pos; pos--)
  {
    ptr = tptr + srclen;
  }

  if (tptr)
  {
    return String((const char *)(ptr), tptr - ptr);
  }
  else
  {
    return String((const char *)(ptr), length + first - ptr);
  }
}

String::ArrayType String::SplitReverseHelper(const char *src, 
  size_t srclen, size_t max)
{
  size_t         alloclen = max <= length ? max : length;
  char         **position = new char*[alloclen];
  size_t        *poslen = new size_t[alloclen];
  size_t         count = 0;
  size_t         nowlen = length;
  char *ptr = first, *tptr, *prev = last + 1;

  for (; (tptr = StringTools::strrnstrn(first, nowlen, src, srclen))
    && max; max--, count++)
  {
    position[count] = tptr + srclen;
    poslen[count] = prev - tptr - srclen;
    nowlen = tptr - first;
    prev = tptr;
  }

  bool max_remain = max > 0;

  String **n = new String*[count + max_remain];

  for (size_t i = 0; i < count; i++)
  {
    n[i] = new String((const char *)(position[i]), poslen[i]);
  }

  if (max_remain)
  {
    n[count] = new String((const char *)first, prev - first);
  }

  delete[] position;
  delete[] poslen;

  return String::ArrayType(n, count + max_remain);
}

String String::BetweenHelper(const char *left, size_t llen, 
  const char *right, size_t rlen, size_t starts)
{
  size_t lefts = FindFirst(left, starts);
  size_t rights = FindFirst(right, lefts);

  if ((lefts == error) || (rights == error))
    return String();

  lefts += llen;

  if (lefts > rights)
    std::swap(lefts, rights);

  return  Slice(lefts, rights - 1);
}

String::ArrayType String::BetweensHelper(const char 
  *left, size_t llen, const char *right, size_t rlen, size_t starts)
{
  char **position = new char*[length];
  size_t   *poslen = new size_t[length];
  size_t    count = 0;

  char *ptr_starts = first + starts;
  char *ptr_ends = first + starts;

  while (ptr_starts = strstr(ptr_ends, left))
  {
    ptr_starts += llen;

    if (ptr_ends = strstr(ptr_starts, right))
    {
      position[count] = ptr_starts;
      poslen[count] = size_t(ptr_ends - ptr_starts);
      count++;
      ptr_ends += rlen;
    }
  }

  String **n = new String*[count];

  for (size_t i = 0; i < count; i++)
  {
    n[i] = new String((const char *)(position[i]), poslen[i]);
  }

  delete[] position;
  delete[] poslen;

  return ArrayType(n, count);
}

String String::Between(char left, char right, size_t starts)
{
  size_t lefts = FindFirst(left, starts) + 1;
  size_t rights = FindFirst(right, lefts);

  if (lefts > rights)
    std::swap(lefts, rights);

  return (lefts != error) && (rights != error) ? 
    Slice(lefts, rights - 1) : String();
}

String::ArrayType String::Betweens(char left, char right, 
  size_t starts)
{
  char **position = new char*[length];
  size_t   *poslen = new size_t[length];
  size_t    count = 0;

  char *ptr_starts = first + starts;
  char *ptr_ends = first + starts;

  while (ptr_starts = strchr(ptr_ends, left))
  {
    ptr_starts++;

    if (ptr_ends = strchr(ptr_starts, right))
    {
      position[count] = ptr_starts;
      poslen[count] = size_t(ptr_ends - ptr_starts);
      count++;
      ptr_ends++;
    }
  }

  String **n = new String*[count];

  for (size_t i = 0; i < count; i++)
  {
    n[i] = new String((const char *)(position[i]), poslen[i]);
  }

  delete[] position;
  delete[] poslen;

  return ArrayType(n, count);
}

bool String::StartsWithHelper(const char *str, size_t starts,
  size_t len) const
{
  if (starts >= length)
    return false;

  if (length < len + starts)
    return false;

  return !memcmp(first + starts, str, len * sizeof(char));
}

bool String::EndsWithHelper(const char *str, size_t ends, size_t len) const
{
  if (ends >= length)
    return false;

  if (length < len - ends)
    return false;

  return !memcmp(last - ends - len + 1, str, len * sizeof(char));
}

String String::InsertLeftHelper(size_t separation,
  const char *str, size_t strlen)
{
  size_t   sizeof_diff = (length - 1) / separation;
  size_t   len = sizeof_diff * strlen + length;
  size_t   dstrlen = strlen * sizeof(char);
  char *totalString = new char[len + 1];
  char *totalPtr = totalString + 1;

  for (size_t pos = 1; pos < length; pos++)
  {
    if (pos % separation == 0)
    {
      memcpy(totalPtr, str, dstrlen);
      totalPtr += strlen;
    }
    *totalPtr++ = first[pos];
  }

  *totalString = *first;
  totalString[len] = 0;

  return String(totalString, len, false);
}

String String::InsertRightHelper(size_t separation,
  const char *str, size_t strlen)
{
  size_t   sizeof_diff = (length - 1) / separation;
  size_t   len = sizeof_diff * strlen + length;
  size_t   dstrlen = strlen * sizeof(char);
  char *totalString = new char[len + 1];
  char *totalPtr = totalString + len;

  for (size_t pos = length - 1; pos > 0; pos--)
  {
    *--totalPtr = first[pos];
    if ((length - pos) && (length - pos) % separation == 0)
    {
      memcpy(totalPtr -= strlen, str, dstrlen);
    }
  }

  *totalString = *first;
  totalString[len] = 0;

  return String(totalString, len, false);
}

String String::InsertLeft(size_t separation, char ch)
{
  size_t   sizeof_diff = (length - 1) / separation;
  size_t   len = sizeof_diff + length;
  char *totalString = new char[len + 1];
  char *totalPtr = totalString + 1;

  for (size_t pos = 1; pos < length; pos++)
  {
    if (pos % separation == 0)
    {
      *totalPtr++ = ch;
    }
    *totalPtr++ = first[pos];
  }

  *totalString = *first;
  totalString[len] = 0;

  return String(totalString, len, false);
}

String String::InsertRight(size_t separation, char ch)
{
  size_t   sizeof_diff = (length - 1) / separation;
  size_t   len = sizeof_diff + length;
  char *totalString = new char[len + 1];
  char *totalPtr = totalString + len;

  for (size_t pos = length - 1; pos > 0; pos--)
  {
    *--totalPtr = first[pos];
    if ((length - pos) && (length - pos) % separation == 0)
    {
      *--totalPtr = ch;
    }
  }

  *totalString = *first;
  totalString[len] = 0;

  return String(totalString, len, false);
}

String String::ReplaceHelper(const char *src, const char *dest,
  size_t srclen, size_t destlen, size_t max)
{
  size_t         alloclen = max <= length ? max : length;
  char         **position = new char*[alloclen];
  size_t         count = 0;
  size_t         sourceLength;
  size_t         index = 0;
  size_t         ddestlen = destlen * sizeof(char);
  size_t         tlen;
  size_t         rest;
  const char *ptr = first;
  const char *tptr;
  char       *mergerString;
  char       *mergerPointer;
  const char *iter = first;

  for (; (tptr = strstr(ptr, src)) && max; )
  {
    position[count] = (char *)tptr;
    ptr = tptr + srclen;
    max--;
    count++;
  }

  sourceLength = length + (destlen - srclen) * count;
  mergerPointer = mergerString = new char[sourceLength + 1];

  for (; index < count;
    index++,
    iter += srclen + tlen,
    mergerPointer += destlen)
  {
    tlen = (size_t)((const char *)position[index] - iter);

    if (tlen > 0)
    {
      memcpy(mergerPointer, iter, tlen * sizeof(char));
    }

    memcpy(mergerPointer += tlen, dest, ddestlen);
  }

  rest = first + length - iter;

  if (rest > 0)
  {
    memcpy(mergerPointer, iter, rest * sizeof(char));
  }

  delete[] position;

  mergerString[sourceLength] = 0;

  return String(mergerString, sourceLength, false);
}

String String::ReplaceSlowHelper(const char *src, const char *dest,
  size_t srclen, size_t destlen, size_t max)
{
  size_t         tlen;
  size_t         rest;
  size_t         ddestlen = destlen * sizeof(char);
  size_t         max_dec = max;
  size_t         count = 0;
  size_t         sourceLength;
  char       *ptr, *tptr;
  char       *mergerString;
  char       *mergerPointer;
  const char *iter = first;

  for (ptr = first; (tptr = strstr(ptr, src)) && max_dec; max_dec--, count++)
  {
    ptr = tptr + srclen;
  }

  sourceLength = length + (destlen - srclen) * count;
  mergerPointer = mergerString = new char[sourceLength + 1];

  for (ptr = first; (tptr = strstr(ptr, src)) && max;
    max--,
    iter += srclen + tlen,
    ptr = tptr + srclen,
    mergerPointer += destlen)
  {
    tlen = (size_t)(tptr - ptr);

    if (tlen > 0)
    {
      memcpy(mergerPointer, iter, tlen * sizeof(char));
    }

    memcpy(mergerPointer += tlen, dest, ddestlen);
  }

  rest = first + length - iter;

  if (rest > 0)
  {
    memcpy(mergerPointer, iter, rest * sizeof(char));
  }

  mergerString[sourceLength] = 0;

  return String(mergerString, sourceLength, false);
}

String String::TrimHelper(const char *src, size_t srclen, size_t max)
{
  size_t         alloclen = max <= length ? max : length;
  char         **position = new char*[alloclen];
  size_t         count = 0;
  size_t         sourceLength;
  size_t         index = 0;
  size_t         tlen;
  size_t         rest;
  const char *ptr = first;
  const char *tptr;
  char       *mergerString;
  char       *mergerPointer;
  const char *iter = first;

  for (; (tptr = strstr(ptr, src)) && max; )
  {
    position[count] = (char *)tptr;
    ptr = tptr + srclen;
    max--;
    count++;
  }

  sourceLength = length - srclen * count;
  mergerPointer = mergerString = new char[sourceLength + 1];

  for (; index < count;
    index++,
    iter += srclen + tlen,
    mergerPointer += tlen)
  {
    tlen = (size_t)((const char *)position[index] - iter);

    if (tlen > 0)
    {
      memcpy(mergerPointer, iter, tlen * sizeof(char));
    }
  }

  rest = first + length - iter;

  if (rest > 0)
  {
    memcpy(mergerPointer, iter, rest * sizeof(char));
  }

  delete[] position;

  mergerString[sourceLength] = 0;

  return String(mergerString, sourceLength, false);
}

String String::InsertHelper(size_t starts, const char *str, size_t len)
{
  size_t newLen = length + len;

  if (newLen == 0)
    return *new String();

  char *neString = new char[newLen + 1];

  memcpy(neString, first, starts * sizeof(char));
  memcpy(neString + starts, str, len * sizeof(char));
  memcpy(neString + starts + len, first + starts, (length - starts)
    * sizeof(char));

  neString[newLen] = 0;

  return String(neString, newLen, false);
}

String::ArrayType String::LineSplitHelper(size_t len, const char *front,
  size_t front_len, const char *end, size_t end_len)
{
  size_t remainLen = length % len;
  size_t countLineLen = length / len + (remainLen != 0);
  size_t eachLineLen = len + front_len + end_len;

  size_t dfront_len = front_len * sizeof(char);
  size_t dend_len = end_len * sizeof(char);
  size_t dlen = len * sizeof(char);

  String **n = new String*[countLineLen];

  for (size_t i = 0; i < countLineLen; i++)
  {
    char *partialLine = new char[eachLineLen + 1];

    if (front_len)
      memcpy(partialLine, front, dfront_len);
    memcpy(partialLine + front_len, first + i * len, dlen);
    if (end_len)
      memcpy(partialLine + front_len + len, end, dend_len);

    partialLine[eachLineLen] = 0;

    n[i] = new String(partialLine, eachLineLen, false);
  }

  if (remainLen != 0)
  {
    if (end_len)
    {
      _strnset(n[countLineLen - 1]->first + front_len + 
        remainLen, L' ', len - remainLen);
    }
    else
    {
      n[countLineLen - 1]->length = remainLen + front_len;
    }
  }

  return String::ArrayType(n, countLineLen);
}

String::ArrayType String::LineSplit(bool last)
{
  char      **position = new char*[length];
  size_t        *poslen = new size_t[length];
  size_t         count = 0;
  size_t         szTotal;
  const char *ptr = first;
  const char *rptr = first;

  for (; *ptr; ptr++)
  {
    if (*ptr == L'\r' || *ptr == L'\n')
    {
      position[count] = (char *)rptr;
      poslen[count] = (size_t)(ptr - rptr);
      count++;

      if (ptr[1] == L'\n')
      {
        ptr += 1;
      }
      rptr = ptr + 1;
    }
  }

  szTotal = count + (last || (ptr - rptr));

  String **n = new String*[szTotal];

  for (size_t i = 0; i < count; i++)
  {
    n[i] = new String((const char *)(position[i]), poslen[i]);
  }

  if (last || (ptr - rptr))
  {
    n[count] = new String((const char *)(rptr), ptr - rptr);
  }

  delete[] position;
  delete[] poslen;

  return String::ArrayType(n, szTotal);
}

String String::LineBreak(size_t len)
{
  size_t remainLen = length % len;
  size_t fullinsertLen = length / len;
  size_t countLine = fullinsertLen + (remainLen != 0); // 모든 줄 수

  size_t totalLen = countLine * 2 - (countLine ? 2 : 0) + length;

  char *origin = new char[totalLen + 1];
  char *pointer = origin;

  const char *mpointer = first;

  size_t dlen = len * sizeof(char);

  if (fullinsertLen)
  {
    for (size_t line = 0; line < fullinsertLen - 1; line++)
    {
      memcpy(pointer, mpointer, dlen);
      pointer[len] = L'\r';
      pointer[len + 1] = L'\n';
      pointer += len + 2;
      mpointer += len;
    }

    memcpy(pointer, mpointer, dlen);

    if (remainLen)
    {
      pointer[len] = L'\r';
      pointer[len + 1] = L'\n';
      pointer += len + 2;
      mpointer += len;
      memcpy(pointer, mpointer, remainLen * sizeof(char));
    }
  }
  else
  {
    memcpy(origin, first, length * sizeof(char));
  }

  origin[totalLen] = 0;

  return String(origin, totalLen, false);
}

long double String::ToLongDoubleHelper(const char *ptr) const
{
  unsigned char sign = 0;
  long double   digit = 0.;
  long double   base = 1.;
  long double   step = 1.;
  char       token;

  while (token = *ptr++)
  {
    switch (token)
    {
    case L'-':
      sign = 0x80;
      break;
    case L'.':
      step = 10;
      break;
    case L'E':
    case L'e':
      ((unsigned char*)&digit)[sizeof(long double) - 1] |= sign;
      return  digit / base * powl(10., ToLongDoubleHelper(ptr));
    default:
      if (token >= L'0' && token <= L'9')
      {
        digit = digit * 10. + (token & 0xF);
        base *= step;
      }
    }
  }

  ((unsigned char*)&digit)[sizeof(long double) - 1] |= sign;
  return  digit / base;
}

///===-----------------------------------------------------------------------===
///
///               String  Private  Tools
///
///===-----------------------------------------------------------------------===

void String::AnsiToUnicode(const char *str, size_t len)
{
   char *ptr = first = new char[len + 1];
   size_t rlen = len;
   length = len;
   while (rlen--) *ptr++ = (char)*str++;
   *ptr = 0;
   last = first + length - 1;
}

char *String::UnicodeToAnsi()
{
  char *ret = new char[length + 1];
  char *ptr = ret;
  char *unicode = first;
  size_t rlen = length;
  while (rlen--)
    *ptr++ = (char)*unicode++;
  *ptr = 0;
  return ret;
}

void String::InitString(const char *str)
{
  length = StringTools::strlen(str);
  first = new char[length + 1];
  last = first + length - 1;
  memcpy(first, str, (length + 1) * sizeof(char));
}