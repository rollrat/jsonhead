//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#include "WString.h"
#include <memory.h>
#include <wchar.h>
#include <cmath>


using namespace jsonhead;

///===-----------------------------------------------------------------------===
///
///               String Base
///
///===-----------------------------------------------------------------------===

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
size_t StringTools::strlen(const char *str)
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

static const wchar_t *wfindzero(const wchar_t *str)
{    
  if (!str[0]) return str;
  else if (!str[1]) return str + 1;
  if (sizeof(ptr_type) == sizeof(uint64_t))
  {
    if (!str[3]) return str + 3;
    else if (!str[4]) return str + 4;
  }
}

size_t StringTools::wcslen(const wchar_t * str)
{
#if !defined(_COMPILER_MS)
  return ::wcslen(str);
#else
  if (sizeof(wchar_t) != 2)
    return ::wcslen(str);

  ptr_type* trim;

  // set has zero checker byte
  const ptr_type less_magic = (ptr_type)(~0ULL / 0xffff);
  const ptr_type most_magic = (less_magic << 15);

  trim = align_address(str);
  
  if ((*(ptr_type *)str  - less_magic) & (~*(ptr_type *)str  & most_magic))
  {
    return wfindzero(str) - str;
  }

  // Find Zero Byte in pointer type value.
  while (1)
  {
    if ((trim[0] - less_magic) & (~trim[0] & most_magic))
      return wfindzero((const wchar_t *)(trim + 0)) - str;
    if ((trim[1] - less_magic) & (~trim[1] & most_magic))
      return wfindzero((const wchar_t *)(trim + 1)) - str;
    if ((trim[2] - less_magic) & (~trim[2] & most_magic))
      return wfindzero((const wchar_t *)(trim + 2)) - str;
    if ((trim[3] - less_magic) & (~trim[3] & most_magic))
      return wfindzero((const wchar_t *)(trim + 3)) - str;
    if (sizeof(ptr_type) == sizeof(uint32_t))
    {
      if ((trim[4] - less_magic) & (~trim[4] & most_magic))
        return wfindzero((const wchar_t *)(trim + 4)) - str;
      if ((trim[5] - less_magic) & (~trim[5] & most_magic))
        return wfindzero((const wchar_t *)(trim + 5)) - str;
      if ((trim[6] - less_magic) & (~trim[6] & most_magic))
        return wfindzero((const wchar_t *)(trim + 6)) - str;
      if ((trim[7] - less_magic) & (~trim[7] & most_magic))
        return wfindzero((const wchar_t *)(trim + 7)) - str;
    }
    trim += (sizeof(uint64_t) + sizeof(uint32_t) - sizeof(ptr_type));
  }
#endif
}

wchar_t *StringTools::wcsrnchr(wchar_t * ptr, size_t len, wchar_t ch)
{
  static_assert(sizeof(wchar_t) == 2 || sizeof(wchar_t) == 4, 
    "The 'wcsrnchr' function is not supported on your environment.");

  wchar_t *lptr = ptr + len - 1;
  wchar_t *aptr = (wchar_t *)align_address2(lptr);

  ptr_type less_magic = (ptr_type)(~0ULL / 0xffff);
  ptr_type most_magic = (less_magic << 15);

  if (sizeof(wchar_t) != 2) {
    less_magic = (ptr_type)(~0ULL / 0xffffffff);
    most_magic = (less_magic << 31);
  }

  ptr_type wide_checker = ((ptr_type)ch << 0) | ((ptr_type)ch << 16);
  if (sizeof(ptr_type) == sizeof(uint64_t))
    wide_checker |= ((ptr_type)ch << 32) | ((ptr_type)ch << 48);

  if (sizeof(wchar_t) != 2) {
    wide_checker = ((ptr_type)ch);
    if (sizeof(ptr_type) == sizeof(uint64_t))
      wide_checker |= ((ptr_type)ch << 32);
  }

  if (aptr < ptr) aptr = ptr;

  for (; lptr >= aptr; --lptr)
    if (*lptr == ch) return lptr;

  const size_t count = sizeof(ptr_type) / sizeof(wchar_t);
  for (lptr = aptr - count; lptr >= ptr; lptr -= count)
  {
    ptr_type nptr = *(ptr_type *)lptr ^ wide_checker;

    if ((nptr - less_magic) & (~nptr & most_magic))
    {
      if (sizeof(ptr_type) == sizeof(uint64_t))
      {
        if (sizeof(wchar_t) == 2) {
          if (lptr[3] == ch) return lptr + 3;
          if (lptr[2] == ch) return lptr + 2;
        } else {
          if (lptr[1] == ch) return lptr + 1;
        }
      }
      if (sizeof(wchar_t) == 2 && lptr[1] == ch) return lptr + 1;
      return lptr;
    }
  }

  const size_t edge = sizeof(ptr_type) - 1;
  for (lptr += edge; lptr >= aptr; --lptr)
    if (*lptr == ch) return lptr;

  return nullptr;
}

wchar_t *StringTools::wcsrnstrn(wchar_t * ptr, size_t ptrlen, 
  const wchar_t * dest, size_t destlen)
{
  wchar_t *tptr;
  size_t len2 = destlen * sizeof(wchar_t);

  while (tptr = wcsrnchr(ptr, ptrlen, *dest))
  {
    ptrlen = tptr - ptr;
    if (!memcmp(tptr, dest, len2))
      return tptr;
  }

  return NULL;
}

void StringTools::wcsnset(wchar_t * ptr, wchar_t ch, size_t len)
{
  if (sizeof(wchar_t) != 2) {
    //::_wcsnset(ptr, ch, len);

  } else {
    wchar_t *trim = (wchar_t *)align_address(ptr);

    if (trim < ptr) trim = ptr;

    for (; ptr < trim && len; ptr++, len--)
      *ptr = ch;

    if (len > 0)
    {
      ptr_type put = ((ptr_type)ch << 0) | ((ptr_type)ch << 16);
      if (sizeof(ptr_type) == sizeof(uint64_t))
        put |= ((ptr_type)ch << 32) | ((ptr_type)ch << 48);

      while ( len >= sizeof(ptr_type)/2 )
      {
        *(ptr_type *)trim = put;
        trim += sizeof(ptr_type) / 2;
        len  -= sizeof(ptr_type) / 2;
      }

      if (sizeof(ptr_type) == sizeof(uint64_t))
      {
        if ( len == 3 ) {*(uint32_t *)trim = put; trim[2] = ch; return;}
        if ( len == 2 ) {*(uint32_t *)trim = put; return;}
      }
      if ( len == 1 ) {*(uint16_t *)trim = put; return;}
    }
  }
}

size_t StringTools::wcountch(wchar_t * ptr, wchar_t * last, wchar_t ch)
{
#ifdef _COMPILER_MS
  static_assert(sizeof(wchar_t) == 2, "Do not use 'wcountch' function!");

  size_t count = 0;
  wchar_t *trim = (wchar_t *)align_address(ptr);
  
  const ptr_type less_magic = (ptr_type)(~0ULL / 0xffff);
  const ptr_type most_magic = (less_magic << 15);

  ptr_type wide_checker = ((ptr_type)ch << 0) | ((ptr_type)ch << 16);
  if (sizeof(ptr_type) == sizeof(uint64_t))
    wide_checker |= ((ptr_type)ch << 32) | ((ptr_type)ch << 48);

  if (trim < ptr) trim = ptr;

  for (; ptr < trim; ptr++)
    if (*ptr == ch) count++;
  
  const size_t countc = sizeof(ptr_type) / 2;
  for (ptr = trim; last - countc >= ptr; ptr += countc)
  {
    ptr_type nptr = *(ptr_type *)ptr ^ wide_checker;
    
    if ((nptr - less_magic) & (~nptr & most_magic))
    {
      if (ptr[0] == ch) count++;
      if (ptr[1] == ch) count++;
      
      if (sizeof(ptr_type) == sizeof(uint64_t))
      {
        if (ptr[2] == ch) count++;
        if (ptr[3] == ch) count++;
      }
    }
  }

  return count;
#else
  return -1;
#endif
}

///===-----------------------------------------------------------------------===
///
///               WString  Constructor
///
///===-----------------------------------------------------------------------===

WString::WString(wchar_t *str, size_t len, bool built_in) 
  : length(len), 
    first(str), 
    last(str + len - 1),
    tm(built_in) 
{
}

WString::WString(const wchar_t *str, size_t len)
  : length(len)
{
  first = alloc(length + 1);
  last = first + length - 1;
  memcpy(first, str, length * sizeof(wchar_t));
  last[1] = 0;
}

WString::WString(wchar_t ch, size_t count)
  : length(count)
{
  first = alloc(length + 1);
  last = first + length - 1;
  StringTools::wcsnset(first, ch, count);
  first[length] = 0; 
}

WString::WString(wchar_t ch)
  : length (1)
{
  first = alloc(2);
  last = first;
  *first = ch;
  *(last + 1) = 0;
}

WString::WString(int num)
{
  wchar_t buffer[65];
  swprintf(buffer, 65, L"%lld", num);
  InitString((const wchar_t *)buffer);
}

WString::WString(long int num)
{
  wchar_t buffer[65];
  swprintf(buffer, 65, L"%lld", num);
  InitString((const wchar_t *)buffer);
}

WString::WString(long long int num)
{
  wchar_t buffer[65];
  swprintf(buffer, 65, L"%lld", num);
  InitString((const wchar_t *)buffer);
}

WString::WString(unsigned int num)
{
  wchar_t buffer[65];
  swprintf(buffer, 65, L"%u", num);
  InitString((const wchar_t *)buffer);
}

WString::WString(unsigned long int num)
{
  wchar_t buffer[65];
  swprintf(buffer, 65, L"%lu", num);
  InitString((const wchar_t *)buffer);
}

WString::WString(unsigned long long int num)
{
  wchar_t buffer[65];
  swprintf(buffer, 65, L"%llu", num);
  InitString((const wchar_t *)buffer);
}

WString::WString(float num)
{
  wchar_t buffer[65];
  swprintf(buffer, 65, L"%g", num);
  InitString((const wchar_t *)buffer);
}

WString::WString(double num)
{
  wchar_t buffer[65];
  swprintf(buffer, 65, L"%lg", num);
  InitString((const wchar_t *)buffer);
}

WString WString::Concat(const WString& t1, const WString& t2)
{
  if (t1.Empty())
  {
      if (t2.Empty())
      {
          return WString();
      }
      return WString((const wchar_t *)t2.first, t2.length);
  }
  
  if (t2.Empty())
  {
      return WString((const wchar_t *)t1.first, t1.length);
  }
  
  size_t newSize = t1.length + t2.length;
  wchar_t * mergerString;
  
  mergerString = new wchar_t[newSize + 1];
  memcpy(mergerString, t1.first, t1.length * sizeof(wchar_t));
  memcpy(mergerString + t1.length, t2.first, t2.length * sizeof(wchar_t));
  mergerString[newSize] = 0;
  
  return WString(mergerString, newSize, false);
}

WString WString::Concat(const WString& t1, const WString& t2, const WString& t3)
{
  if (t1.Empty() && t2.Empty() && t3.Empty())
  {
      return WString();
  }
  
  size_t newSize = t1.length + t2.length + t3.length;
  wchar_t * mergerString;
  
  mergerString = new wchar_t[newSize + 1];
  memcpy(mergerString, t1.first, t1.length * sizeof(wchar_t));
  memcpy(mergerString + t1.length, t2.first, t2.length * sizeof(wchar_t));
  memcpy(mergerString + t1.length + t2.length, t3.first, t3.length
    * sizeof(wchar_t));
  mergerString[newSize] = 0;
  
  return WString(mergerString, newSize, false);
}

WString WString::Concat(const WString& t1, const WString& t2, 
  const WString& t3, const WString& t4)
{
  if (t1.Empty() && t2.Empty() && t3.Empty() && t4.Empty())
  {
      return WString();
  }
  
  size_t newSize = t1.length + t2.length + t3.length + t4.length;
  wchar_t * mergerString;
  
  mergerString = new wchar_t[newSize + 1];
  memcpy(mergerString, t1.first, t1.length * sizeof(wchar_t));
  memcpy(mergerString + t1.length, t2.first, t2.length * sizeof(wchar_t));
  memcpy(mergerString + t1.length + t2.length, t3.first, t3.length 
    * sizeof(wchar_t));
  memcpy(mergerString + t1.length + t2.length + t3.length, t4.first, 
    t4.length * sizeof(wchar_t));
  mergerString[newSize] = 0;

  return WString(mergerString, newSize, false);
}

size_t WString::TirmStartPos() const
{
  const wchar_t *ptr = first;
  while (*ptr)
  {
    if (!isspace((int)*ptr))
      break;
    else
      ptr++;
  }
  return ptr - first;
}

size_t WString::TrimEndPos() const
{
  wchar_t *ptr = last;
  while (ptr >= first)
  {
    if (!isspace((int)*ptr))
      break;
    else
      ptr--;
  }
  return ptr - first;
}

size_t WString::TrimStartPos(wchar_t ch) const
{
  const wchar_t *ptr = first;
  while (*ptr)
  {
    if (*ptr != ch)
      break;
    else
      ptr++;
  }
  return ptr - first;
}

size_t WString::TrimEndPos(wchar_t ch) const
{
  wchar_t *ptr = last;
  while (ptr >= first)
  {
    if (*ptr != ch)
      break;
    else
      ptr--;
  }
  return ptr - first;
}

WString WString::TrimStart()
{
  const wchar_t *ptr = first;
  while (*ptr)
  {
    if (!isspace((int)*ptr))
      break;
    else
      ptr++;
  }
  return WString(ptr, length + first - ptr);
}

WString WString::TrimEnd()
{
  wchar_t *ptr = last;
  while (ptr >= first)
  {
    if (!isspace((int)*ptr))
      break;
    else
      ptr--;
  }
  return WString((const wchar_t *)first, ptr - first + 1);
}

WString WString::Trim()
{
  const wchar_t *fptr = first;
  wchar_t *ptr = first + length - 1;
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
  return WString(fptr, ptr - fptr + 1);
}

WString WString::TrimStrat(wchar_t ch)
{
  const wchar_t *ptr = first;
  while (*ptr)
  {
    if (*ptr != ch)
      break;
    else
      ptr++;
  }
  return WString(ptr, length + first - ptr);
}

WString WString::TrimEnd(wchar_t ch)
{
  wchar_t *ptr = last;
  while (ptr >= first)
  {
    if (*ptr != ch)
      break;
    else
      ptr--;
  }
  return WString((const wchar_t *)first, ptr - first + 1);
}

WString WString::Trim(wchar_t ch)
{
  const wchar_t *fptr = first;
  wchar_t *ptr = first + length - 1;
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
  return WString(fptr, ptr - fptr + 1);
}

WString WString::ToLower()
{
  wchar_t *ret = new wchar_t[length + 1];
  for (size_t i = 0; i < length; i++)
    ret[i] = towlower(first[i]);
  ret[length] = 0;
  return WString(ret, length, false);
}

WString WString::ToUpper()
{
  wchar_t *ret = new wchar_t[length + 1];
  for (size_t i = 0; i < length; i++)
    ret[i] = towupper(first[i]);
  ret[length] = 0;
  return WString(ret, length, false);
}

WString WString::Capitalize()
{
  wchar_t *ret = this->ToArray();
  *ret = towupper(*ret);
  return WString(ret, length, false);
}

WString WString::Title()
{
  wchar_t *ret = this->ToArray();
  *ret = towupper(*ret);
  for (size_t i = 0; i < length - 1; i++)
    if (first[i] == L' ')
      ret[i + 1] = towupper(first[i + 1]);
  return WString(ret, length, false);
}

WString WString::PadLeft(size_t len, wchar_t pad)
{
  if (len > length)
  {
    wchar_t* ret = new wchar_t[len + 1];
    size_t   padlen = len - length;

    StringTools::wcsnset(ret, pad, padlen);
    memcpy(ret + padlen, first, length * sizeof(wchar_t));

    ret[len] = 0;

    return WString(ret, len, false);
  }
  else
  {
    return WString((const wchar_t *)first, length);
  }
}

WString WString::PadRight(size_t len, wchar_t pad)
{
  if (len > length)
  {
    wchar_t *ret = new wchar_t[len + 1];

    memcpy(ret, first, length * sizeof(wchar_t));
    StringTools::wcsnset(ret + length, pad, len - length);

    ret[len] = 0;

    return WString(ret, len, false);
  }
  else
  {
    return WString((const wchar_t *)first, length);
  }
}

WString WString::PadCenter(size_t len, wchar_t pad, bool lefts)
{
  if (len > length)
  {
    size_t padlen = len - length;
    size_t lpadlen = padlen / 2 + ((padlen & 1) && lefts);
    size_t rpadlen = padlen - lpadlen;

    wchar_t *ret = new wchar_t[len + 1];

    StringTools::wcsnset(ret, pad, lpadlen);
    memcpy(ret + lpadlen, first, length * sizeof(wchar_t));
    StringTools::wcsnset(ret + lpadlen + length, pad, rpadlen);

    ret[len] = 0;

    return WString(ret, len, false);
  }
  else
  {
    return WString((const wchar_t *)first, length);
  }
}

WString WString::Remove(size_t starts, size_t len)
{
  size_t retlen = length - len;
  wchar_t *newString = new wchar_t[retlen + 1];

  memcpy(newString, first, starts * sizeof(wchar_t));
  memcpy(newString + starts, first + starts + len, (retlen - starts) 
    * sizeof(wchar_t));

  newString[retlen] = 0;

  return WString(newString, retlen, false);
}

WString WString::Repeat(size_t count)
{
  size_t newLen = count * length;
  size_t dlength = length * sizeof(wchar_t);
  wchar_t *newString = new wchar_t[newLen + 1];

  for (size_t i = 0; i < count; i++)
  {
    memcpy(newString + i * length, first, dlength);
  }

  newString[newLen] = 0;

  return WString(newString, newLen, false);
}

WString WString::Reverse()
{
  throw new std::runtime_error("this function not implemented!");

  wchar_t *ret = this->ToArray();
  //_wcsrev(ret);
  return WString(ret, length, false);
}

WString WString::Slice(size_t first, size_t last)
{
  if ((int)last > 0)
  {
    return WString((const wchar_t *)(first + first), last - first + 1);
  }
  else
  {
    return WString((const wchar_t *)(first + first), length - first + last);
  }
}

WString WString::Slicing(size_t jmp, size_t starts, size_t len,
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
    wchar_t* collect = new wchar_t[totalLen + 1];
    wchar_t* colptr = collect;

    size_t   countLen = starts;
    size_t   putLen2 = len * sizeof(wchar_t);

    size_t   copyLen = length - lastRemain;

    for (; countLen < copyLen; countLen += chunkLen)
    {
      memcpy(colptr, first + countLen, putLen2);
      colptr += len;
    }

    if (remainLen)
    {
      memcpy(colptr, first + countLen, remainLen * sizeof(wchar_t));
    }

    collect[totalLen] = 0;

    return WString(collect, totalLen);
  }
  else if (remain)
  {
    return WString((const wchar_t *)(first + starts), searchLen);
  }

  return WString();
}

uint64_t WString::Hash(uint64_t seed) const
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

bool WString::IsNumeric() const
{
  const wchar_t *ptr = first;

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

bool WString::IsHexDigit() const
{
  const wchar_t *ptr = first;
  if (*ptr == L'0' && (ptr[1] == L'x' || ptr[1] == L'X')) ptr += 2;
  while (iswxdigit(*ptr) && *ptr) ptr++;
  return *ptr == 0;
}

unsigned long long int WString::ToHexDigit() const
{
  unsigned long long int ret = 0;
  const wchar_t *ptr = first;

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

long long int WString::ToLongLong() const
{
  long long int ret = 0, mark = 1;
  const wchar_t *ptr = first;

  if (*ptr == L'-')
    mark = -1, ptr++;
  else if (*ptr == L'+')
    ptr++;

  while (*ptr)
    ret = ret * 10 + (*ptr++ & 0xf);

  return ret * mark;
}

unsigned long long int WString::ToULongLong() const
{
  long long int ret = 0;
  const wchar_t *ptr = first;

  while (*ptr)
    ret = ret * 10 + (*ptr++ & 0xf);

  return ret;
}

wchar_t *WString::ToArray() const
{
  wchar_t *ret = new wchar_t[length + 1];
  memcpy(ret, first, (length + 1) * sizeof(wchar_t));
  return ret;
}

char *WString::ToAnsi()
{
  return UnicodeToAnsi();
}

WString::Utf8Array WString::ToUtf8(bool file_bom)
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
    wchar_t ch = first[i];
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

  return WString::Utf8Array(bytes, szReal);
}

void WString::Swap(WString& refer)
{
  // "swap" function will probably call "move" fucntion.
  std::swap(first, refer.first);
  std::swap(last, refer.last);
  std::swap(length, refer.length);
}

WString& WString::operator=(WString && refer)
{
  this->Swap(refer);
  refer.tm = this->tm;
  this->tm = false;
  return *this;
}

WString& WString::operator=(const WString& refer)
{
  if (first != nullptr)
    delete[] first;
  if (length = refer.length)
  {
    first = new wchar_t[length + 1];
    last = first + length - 1;
    memcpy(first, refer.first, (length + 1) * sizeof(wchar_t));
  }
  return *this;
}

void WString::CloneSet(const WString& refer)
{
  if (first != nullptr)
    delete[] first;
  first = nullptr;
  tm = true;
  first = refer.first;
  last = refer.last;
  length = refer.length;
}

WString WString::Clone()
{
  WString nstr;
  nstr.CloneSet(*this);
  return nstr;
}

///===-----------------------------------------------------------------------===
///
///               WString  Helper
///
///===-----------------------------------------------------------------------===

WString WString::AppendHelper(const wchar_t *str, size_t len)
{
  size_t newSize = length + len;
  wchar_t * appendString;
  
  appendString = new wchar_t[newSize + 1];
  
  if (length > 0)
      memcpy(appendString, first, length * sizeof(wchar_t));
  memcpy(appendString + length, str, len * sizeof(wchar_t));
  appendString[newSize] = 0;
  
  return WString(appendString, newSize, false);
}

size_t WString::FindFirstHelper(const wchar_t *str, size_t starts) const
{
  if (starts >= length)return error;
  const wchar_t *ptr = wcsstr(first + starts, str);
  return ptr != NULL ? ptr - first : error;
}

size_t WString::FindLastHelper(const wchar_t *str, size_t ends, size_t len) const
{
  if (ends >= length)return error;
  const wchar_t *ptr = StringTools::wcsrnstrn(first, length - ends, str, len);
  return ptr != NULL ? ptr - first : error;
}

size_t WString::FindFirst(const wchar_t ch, size_t starts) const
{
  const wchar_t *ptr = wcschr(first + starts, ch);
  return ptr != NULL ? ptr - first : error;
}

size_t WString::FindLast(const wchar_t ch, size_t ends) const
{
  const wchar_t *ptr = StringTools::wcsrnchr(first, length - ends, ch);
  return (ptr != NULL) ? ptr - first : error;
}

static wchar_t to_switch(wchar_t ch)
{
  if (ch <= L'z')
  {
    if (ch <= L'Z' && ch >= L'A') return ch | 0x0020;
    if (ch >= L'a') return ch ^ 0x0020;
  }
  return ch;
}

static wchar_t to_wlower(wchar_t ch)
{
  if (ch <= L'Z' && ch >= L'A') return ch ^ 0x0020;
  return ch;
}

bool WString::ContainsHelper(const wchar_t *str, size_t len, bool ignore) const
{
  if (len > length) return false;
  if (!ignore) return wcsstr(first, str) != NULL;

  const wchar_t *s1, *s2, *ptr = first;
  size_t len_1 = len - 1;
  
  wchar_t *t1 = first;
  wchar_t *t2 = first;
  wchar_t *searchMax = last - len + 1;
  
  wchar_t strlast = to_wlower(str[len_1]);
  wchar_t w = to_switch(*str);
  
  t1 = wcschr(t1, *str);
  t2 = wcschr(t2, w);
  
#define _CONTAINS_HELPER_SKIP(t,ch) {\
  if (to_wlower(t[len_1]) == strlast) {\
    s1 = t + 1;s2 = str + 1;\
    while (*s1 && *s2 && !(towlower(*s1) - towlower(*s2)))\
      s1++, s2++;\
    if (*s2 == 0)return true;\
  } t = wcschr(t + 1, ch);}

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

size_t WString::CountHelper(const wchar_t *str, size_t len) const
{
  size_t ret = 0;
  const wchar_t *ptr = first;
  for (; ptr = wcsstr(ptr, str); ptr += len, ret++)
    ;
  return ret;
}

WString::ArrayType WString::SplitHelper(const wchar_t *src, 
  size_t srclen, size_t max)
{
  size_t         alloclen = max <= length ? max : length;
  wchar_t      **position = new wchar_t*[alloclen];
  size_t        *poslen = new size_t[alloclen];
  size_t         count = 0;
  wchar_t *ptr = first, *tptr;

  for (; (tptr = wcsstr(ptr, src)) && max; max--, count++)
  {
    position[count] = ptr;
    poslen[count] = tptr - ptr;
    ptr = tptr + srclen;
  }

  bool max_remain = max > 0;

  WString **n = new WString*[count + max_remain];

  for (size_t i = 0; i < count; i++)
  {
    n[i] = new WString((const wchar_t *)(position[i]), poslen[i]);
  }

  if (max_remain)
  {
    n[count] = new WString((const wchar_t *)(ptr), length + first - ptr);
  }

  delete[] position;
  delete[] poslen;

  return WString::ArrayType(n, count + max_remain);
}

WString::ArrayType WString::SplitSlowHelper(const wchar_t *src, 
  size_t srclen, size_t max)
{
  size_t      max_dec = max;
  size_t      count = 0;
  size_t      i = 0;
  wchar_t    *ptr, *tptr;

  for (ptr = first; (tptr = wcsstr(ptr, src)) && max_dec; max_dec--, count++)
  {
    ptr = tptr + srclen;
  }

  bool max_remain = max > 0;

  WString **n = new WString*[count + max_remain];

  for (ptr = first; (tptr = wcsstr(ptr, src)) && max; max--, i++)
  {
    n[i] = new WString((const wchar_t *)(ptr), tptr - ptr);
    ptr = tptr + srclen;
  }

  if (max_remain)
  {
    n[count] = new WString((const wchar_t *)(ptr), length + first - ptr);
  }

  return ArrayType(n, count + max_remain);
}

WString WString::SplitPositionHelper(const wchar_t *src, 
  size_t srclen, size_t pos)
{
  wchar_t *ptr = first, *tptr;

  for (ptr = first; (tptr = wcsstr(ptr, src)) && pos; pos--)
  {
    ptr = tptr + srclen;
  }

  if (tptr)
  {
    return WString((const wchar_t *)(ptr), tptr - ptr);
  }
  else
  {
    return WString((const wchar_t *)(ptr), length + first - ptr);
  }
}

WString::ArrayType WString::SplitReverseHelper(const wchar_t *src, 
  size_t srclen, size_t max)
{
  size_t         alloclen = max <= length ? max : length;
  wchar_t      **position = new wchar_t*[alloclen];
  size_t        *poslen = new size_t[alloclen];
  size_t         count = 0;
  size_t         nowlen = length;
  wchar_t *ptr = first, *tptr, *prev = last + 1;

  for (; (tptr = StringTools::wcsrnstrn(first, nowlen, src, srclen))
    && max; max--, count++)
  {
    position[count] = tptr + srclen;
    poslen[count] = prev - tptr - srclen;
    nowlen = tptr - first;
    prev = tptr;
  }

  bool max_remain = max > 0;

  WString **n = new WString*[count + max_remain];

  for (size_t i = 0; i < count; i++)
  {
    n[i] = new WString((const wchar_t *)(position[i]), poslen[i]);
  }

  if (max_remain)
  {
    n[count] = new WString((const wchar_t *)first, prev - first);
  }

  delete[] position;
  delete[] poslen;

  return WString::ArrayType(n, count + max_remain);
}

WString WString::BetweenHelper(const wchar_t *left, size_t llen, 
  const wchar_t *right, size_t rlen, size_t starts)
{
  size_t lefts = FindFirst(left, starts);
  size_t rights = FindFirst(right, lefts);

  if ((lefts == error) || (rights == error))
    return WString();

  lefts += llen;

  if (lefts > rights)
    std::swap(lefts, rights);

  return  Slice(lefts, rights - 1);
}

WString::ArrayType WString::BetweensHelper(const wchar_t 
  *left, size_t llen, const wchar_t *right, size_t rlen, size_t starts)
{
  wchar_t **position = new wchar_t*[length];
  size_t   *poslen = new size_t[length];
  size_t    count = 0;

  wchar_t *ptr_starts = first + starts;
  wchar_t *ptr_ends = first + starts;

  while (ptr_starts = wcsstr(ptr_ends, left))
  {
    ptr_starts += llen;

    if (ptr_ends = wcsstr(ptr_starts, right))
    {
      position[count] = ptr_starts;
      poslen[count] = size_t(ptr_ends - ptr_starts);
      count++;
      ptr_ends += rlen;
    }
  }

  WString **n = new WString*[count];

  for (size_t i = 0; i < count; i++)
  {
    n[i] = new WString((const wchar_t *)(position[i]), poslen[i]);
  }

  delete[] position;
  delete[] poslen;

  return ArrayType(n, count);
}

WString WString::Between(wchar_t left, wchar_t right, size_t starts)
{
  size_t lefts = FindFirst(left, starts) + 1;
  size_t rights = FindFirst(right, lefts);

  if (lefts > rights)
    std::swap(lefts, rights);

  return (lefts != error) && (rights != error) ? 
    Slice(lefts, rights - 1) : WString();
}

WString::ArrayType WString::Betweens(wchar_t left, wchar_t right, 
  size_t starts)
{
  wchar_t **position = new wchar_t*[length];
  size_t   *poslen = new size_t[length];
  size_t    count = 0;

  wchar_t *ptr_starts = first + starts;
  wchar_t *ptr_ends = first + starts;

  while (ptr_starts = wcschr(ptr_ends, left))
  {
    ptr_starts++;

    if (ptr_ends = wcschr(ptr_starts, right))
    {
      position[count] = ptr_starts;
      poslen[count] = size_t(ptr_ends - ptr_starts);
      count++;
      ptr_ends++;
    }
  }

  WString **n = new WString*[count];

  for (size_t i = 0; i < count; i++)
  {
    n[i] = new WString((const wchar_t *)(position[i]), poslen[i]);
  }

  delete[] position;
  delete[] poslen;

  return ArrayType(n, count);
}

bool WString::StartsWithHelper(const wchar_t *str, size_t starts,
  size_t len) const
{
  if (starts >= length)
    return false;

  if (length < len + starts)
    return false;

  return !memcmp(first + starts, str, len * sizeof(wchar_t));
}

bool WString::EndsWithHelper(const wchar_t *str, size_t ends, size_t len) const
{
  if (ends >= length)
    return false;

  if (length < len - ends)
    return false;

  return !memcmp(last - ends - len + 1, str, len * sizeof(wchar_t));
}

WString WString::InsertLeftHelper(size_t separation,
  const wchar_t *str, size_t strlen)
{
  size_t   sizeof_diff = (length - 1) / separation;
  size_t   len = sizeof_diff * strlen + length;
  size_t   dstrlen = strlen * sizeof(wchar_t);
  wchar_t *totalString = new wchar_t[len + 1];
  wchar_t *totalPtr = totalString + 1;

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

  return WString(totalString, len, false);
}

WString WString::InsertRightHelper(size_t separation,
  const wchar_t *str, size_t strlen)
{
  size_t   sizeof_diff = (length - 1) / separation;
  size_t   len = sizeof_diff * strlen + length;
  size_t   dstrlen = strlen * sizeof(wchar_t);
  wchar_t *totalString = new wchar_t[len + 1];
  wchar_t *totalPtr = totalString + len;

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

  return WString(totalString, len, false);
}

WString WString::InsertLeft(size_t separation, wchar_t ch)
{
  size_t   sizeof_diff = (length - 1) / separation;
  size_t   len = sizeof_diff + length;
  wchar_t *totalString = new wchar_t[len + 1];
  wchar_t *totalPtr = totalString + 1;

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

  return WString(totalString, len, false);
}

WString WString::InsertRight(size_t separation, wchar_t ch)
{
  size_t   sizeof_diff = (length - 1) / separation;
  size_t   len = sizeof_diff + length;
  wchar_t *totalString = new wchar_t[len + 1];
  wchar_t *totalPtr = totalString + len;

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

  return WString(totalString, len, false);
}

WString WString::ReplaceHelper(const wchar_t *src, const wchar_t *dest,
  size_t srclen, size_t destlen, size_t max)
{
  size_t         alloclen = max <= length ? max : length;
  wchar_t      **position = new wchar_t*[alloclen];
  size_t         count = 0;
  size_t         sourceLength;
  size_t         index = 0;
  size_t         ddestlen = destlen * sizeof(wchar_t);
  size_t         tlen;
  size_t         rest;
  const wchar_t *ptr = first;
  const wchar_t *tptr;
  wchar_t       *mergerString;
  wchar_t       *mergerPointer;
  const wchar_t *iter = first;

  for (; (tptr = wcsstr(ptr, src)) && max; )
  {
    position[count] = (wchar_t *)tptr;
    ptr = tptr + srclen;
    max--;
    count++;
  }

  sourceLength = length + (destlen - srclen) * count;
  mergerPointer = mergerString = new wchar_t[sourceLength + 1];

  for (; index < count;
    index++,
    iter += srclen + tlen,
    mergerPointer += destlen)
  {
    tlen = (size_t)((const wchar_t *)position[index] - iter);

    if (tlen > 0)
    {
      memcpy(mergerPointer, iter, tlen * sizeof(wchar_t));
    }

    memcpy(mergerPointer += tlen, dest, ddestlen);
  }

  rest = first + length - iter;

  if (rest > 0)
  {
    memcpy(mergerPointer, iter, rest * sizeof(wchar_t));
  }

  delete[] position;

  mergerString[sourceLength] = 0;

  return WString(mergerString, sourceLength, false);
}

WString WString::ReplaceSlowHelper(const wchar_t *src, const wchar_t *dest,
  size_t srclen, size_t destlen, size_t max)
{
  size_t         tlen;
  size_t         rest;
  size_t         ddestlen = destlen * sizeof(wchar_t);
  size_t         max_dec = max;
  size_t         count = 0;
  size_t         sourceLength;
  wchar_t       *ptr, *tptr;
  wchar_t       *mergerString;
  wchar_t       *mergerPointer;
  const wchar_t *iter = first;

  for (ptr = first; (tptr = wcsstr(ptr, src)) && max_dec; max_dec--, count++)
  {
    ptr = tptr + srclen;
  }

  sourceLength = length + (destlen - srclen) * count;
  mergerPointer = mergerString = new wchar_t[sourceLength + 1];

  for (ptr = first; (tptr = wcsstr(ptr, src)) && max;
    max--,
    iter += srclen + tlen,
    ptr = tptr + srclen,
    mergerPointer += destlen)
  {
    tlen = (size_t)(tptr - ptr);

    if (tlen > 0)
    {
      memcpy(mergerPointer, iter, tlen * sizeof(wchar_t));
    }

    memcpy(mergerPointer += tlen, dest, ddestlen);
  }

  rest = first + length - iter;

  if (rest > 0)
  {
    memcpy(mergerPointer, iter, rest * sizeof(wchar_t));
  }

  mergerString[sourceLength] = 0;

  return WString(mergerString, sourceLength, false);
}

WString WString::TrimHelper(const wchar_t *src, size_t srclen, size_t max)
{
  size_t         alloclen = max <= length ? max : length;
  wchar_t      **position = new wchar_t*[alloclen];
  size_t         count = 0;
  size_t         sourceLength;
  size_t         index = 0;
  size_t         tlen;
  size_t         rest;
  const wchar_t *ptr = first;
  const wchar_t *tptr;
  wchar_t       *mergerString;
  wchar_t       *mergerPointer;
  const wchar_t *iter = first;

  for (; (tptr = wcsstr(ptr, src)) && max; )
  {
    position[count] = (wchar_t *)tptr;
    ptr = tptr + srclen;
    max--;
    count++;
  }

  sourceLength = length - srclen * count;
  mergerPointer = mergerString = new wchar_t[sourceLength + 1];

  for (; index < count;
    index++,
    iter += srclen + tlen,
    mergerPointer += tlen)
  {
    tlen = (size_t)((const wchar_t *)position[index] - iter);

    if (tlen > 0)
    {
      memcpy(mergerPointer, iter, tlen * sizeof(wchar_t));
    }
  }

  rest = first + length - iter;

  if (rest > 0)
  {
    memcpy(mergerPointer, iter, rest * sizeof(wchar_t));
  }

  delete[] position;

  mergerString[sourceLength] = 0;

  return WString(mergerString, sourceLength, false);
}

WString WString::InsertHelper(size_t starts, const wchar_t *str, size_t len)
{
  size_t newLen = length + len;

  if (newLen == 0)
    return *new WString();

  wchar_t *newString = new wchar_t[newLen + 1];

  memcpy(newString, first, starts * sizeof(wchar_t));
  memcpy(newString + starts, str, len * sizeof(wchar_t));
  memcpy(newString + starts + len, first + starts, (length - starts)
    * sizeof(wchar_t));

  newString[newLen] = 0;

  return WString(newString, newLen, false);
}

WString::ArrayType WString::LineSplitHelper(size_t len, const wchar_t *front,
  size_t front_len, const wchar_t *end, size_t end_len)
{
  size_t remainLen = length % len;
  size_t countLineLen = length / len + (remainLen != 0);
  size_t eachLineLen = len + front_len + end_len;

  size_t dfront_len = front_len * sizeof(wchar_t);
  size_t dend_len = end_len * sizeof(wchar_t);
  size_t dlen = len * sizeof(wchar_t);

  WString **n = new WString*[countLineLen];

  for (size_t i = 0; i < countLineLen; i++)
  {
    wchar_t *partialLine = new wchar_t[eachLineLen + 1];

    if (front_len)
      memcpy(partialLine, front, dfront_len);
    memcpy(partialLine + front_len, first + i * len, dlen);
    if (end_len)
      memcpy(partialLine + front_len + len, end, dend_len);

    partialLine[eachLineLen] = 0;

    n[i] = new WString(partialLine, eachLineLen, false);
  }

  if (remainLen != 0)
  {
    if (end_len)
    {
      StringTools::wcsnset(n[countLineLen - 1]->first + front_len + 
        remainLen, L' ', len - remainLen);
    }
    else
    {
      n[countLineLen - 1]->length = remainLen + front_len;
    }
  }

  return WString::ArrayType(n, countLineLen);
}

WString::ArrayType WString::LineSplit(bool last)
{
  wchar_t      **position = new wchar_t*[length];
  size_t        *poslen = new size_t[length];
  size_t         count = 0;
  size_t         szTotal;
  const wchar_t *ptr = first;
  const wchar_t *rptr = first;

  for (; *ptr; ptr++)
  {
    if (*ptr == L'\r' || *ptr == L'\n')
    {
      position[count] = (wchar_t *)rptr;
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

  WString **n = new WString*[szTotal];

  for (size_t i = 0; i < count; i++)
  {
    n[i] = new WString((const wchar_t *)(position[i]), poslen[i]);
  }

  if (last || (ptr - rptr))
  {
    n[count] = new WString((const wchar_t *)(rptr), ptr - rptr);
  }

  delete[] position;
  delete[] poslen;

  return WString::ArrayType(n, szTotal);
}

WString WString::LineBreak(size_t len)
{
  size_t remainLen = length % len;
  size_t fullinsertLen = length / len;
  size_t countLine = fullinsertLen + (remainLen != 0); // ��� �� ��

  size_t totalLen = countLine * 2 - (countLine ? 2 : 0) + length;

  wchar_t *origin = new wchar_t[totalLen + 1];
  wchar_t *pointer = origin;

  const wchar_t *mpointer = first;

  size_t dlen = len * sizeof(wchar_t);

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
      memcpy(pointer, mpointer, remainLen * sizeof(wchar_t));
    }
  }
  else
  {
    memcpy(origin, first, length * sizeof(wchar_t));
  }

  origin[totalLen] = 0;

  return WString(origin, totalLen, false);
}

long double WString::ToLongDoubleHelper(const wchar_t *ptr) const
{
  unsigned char sign = 0;
  long double   digit = 0.;
  long double   base = 1.;
  long double   step = 1.;
  wchar_t       token;

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
///               WString  Private  Tools
///
///===-----------------------------------------------------------------------===

void WString::AnsiToUnicode(const char *str, size_t len)
{
   wchar_t *ptr = first = new wchar_t[len + 1];
   size_t rlen = len;
   length = len;
   while (rlen--) *ptr++ = (wchar_t)*str++;
   *ptr = 0;
   last = first + length - 1;
}

char *WString::UnicodeToAnsi()
{
  char *ret = new char[length + 1];
  char *ptr = ret;
  wchar_t *unicode = first;
  size_t rlen = length;
  while (rlen--)
    *ptr++ = (char)*unicode++;
  *ptr = 0;
  return ret;
}

void WString::InitString(const char *str)
{
  //length = StringTools::strlen(str);
  first = new wchar_t[length + 1];
  mbstowcs(first, str, SIZE_MAX);
}

void WString::InitString(const wchar_t *str)
{
  length = StringTools::wcslen(str);
  first = new wchar_t[length + 1];
  last = first + length - 1;
  memcpy(first, str, (length + 1) * sizeof(wchar_t));
}