//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#ifndef _WSTRING_9bf1541fdf7efd41b7b39543fd870ac4_
#define _WSTRING_9bf1541fdf7efd41b7b39543fd870ac4_

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <stdint.h>
#include <cstring>
#include <memory>
#include <string>
#include <iostream>
#include "String.h"

namespace jsonhead
{

class WStringTools
{
public:
  static size_t strlen(const char *str);
  static size_t wcslen(const wchar_t *str);
  static wchar_t *wcsrnchr(wchar_t *ptr, size_t len, wchar_t ch);
  static wchar_t *wcsrnstrn(wchar_t *ptr, size_t ptrlen, 
    const wchar_t *dest, size_t destlen);
  static void wcsnset(wchar_t *ptr, wchar_t ch, size_t len);
  static size_t wcountch(wchar_t *ptr, wchar_t *last, wchar_t ch);
};

class WString final
{
  wchar_t *first;
  wchar_t *last;
  size_t   length;

  /// If this flag is on, string-pointer is not deleted in destructor.
  bool tm = false;

public:
  using ArrayType = ReadOnlyArray<WString *>;
  using Utf8Array = ReadOnlyArray<unsigned char>;

  static const size_t error = -1;

  WString() : length(0), first(nullptr), last(first) { }
  WString(const char *str) { AnsiToUnicode(str, WStringTools::strlen(str)); }
  WString(const char *str, size_t len) { AnsiToUnicode(str, len); }
  WString(const wchar_t *str) { InitString(str); }
  explicit WString(wchar_t *str, size_t len, bool built_in = true);
  WString(const wchar_t *str, size_t len);
  explicit WString(wchar_t ch, size_t count);
  explicit WString(wchar_t ch);
  WString(char ch) : WString((wchar_t)ch) { };
  WString(unsigned char ch) : WString((wchar_t)ch) { }
  WString(int);
  WString(long int);
  WString(long long int);
  WString(unsigned int);
  WString(unsigned long int);
  WString(unsigned long long int);
  WString(float);
  WString(double);
  WString(const String& cnt) : WString((const char *)cnt.Reference(), cnt.Length()) { }
  WString(WString&& ws) : first(ws.first), last(ws.last), 
    length(ws.length) { ws.tm = true; }
  WString(const WString& cnt) : WString((const wchar_t *)cnt.first, cnt.length) {}
  WString(std::wstring& str) : WString(&str[0], str.length()) { }
  WString(const std::string& str) : WString(str.c_str(), str.length()) { }
  WString(const std::wstring& wstr) : WString(wstr.c_str(), wstr.length()) { }
  ~WString() { if (first != nullptr && !tm){delete[] first; first = nullptr;}}

  inline size_t Length() const { return length; }
  inline bool Empty() const { return length == 0; }
  inline bool Full() const { return length > 0; }
  inline bool Null() const { return first == nullptr; }
  inline const wchar_t *Reference() const { return first; }
  
  inline size_t CompareTo(const wchar_t *str) const 
  { return wcscmp(first, str); }
  inline size_t CompareTo(const WString& refer) const 
  { return CompareTo(refer.first); }
  inline static int Comparer(const WString& r1, const WString &r2) 
  { return wcscmp(r1.first, r2.first); }

  inline bool Equal(const WString& refer) const
  {
    if (refer.length != this->length)
      return false;
    return !memcmp(first, refer.first, length * sizeof(wchar_t));
  }

  inline bool operator==(const wchar_t *str) const { return Equal(str); }
  inline bool operator==(const WString& refer) const { return Equal(refer); }
  inline bool operator!=(const wchar_t *str) const { return !Equal(str); }
  inline bool operator!=(const WString& refer) const { return !Equal(refer); }

  inline wchar_t First(size_t pos) const { return first[pos]; }
  inline wchar_t Last(size_t pos) const { return *(last - pos); }
  inline wchar_t operator[](size_t index) const { return first[index]; }
  
  /// Concatenate a number of strings.
  static WString Concat(const WString& t1, const WString& t2);
  static WString Concat(const WString& t1, const WString& t2, 
    const WString& t3);
  static WString Concat(const WString& t1, const WString& t2, const WString& t3,
    const WString& t4);
  
  WString Substring(size_t starts)
  { return WString((const wchar_t *)(first + starts), length - starts); }
  WString Substring(size_t starts, size_t len)
  { return WString((const wchar_t *)(first + starts), len); }
  WString SubstringReverse(size_t starts)
  { return WString((const wchar_t *)(first), length - starts); }
  WString SubstringReverse(size_t starts, size_t len)
  { return WString((const wchar_t *)(first - starts - len + 1), len); }

  size_t TirmStartPos() const;
  size_t TrimEndPos() const;
  size_t TrimStartPos(wchar_t ch) const;
  size_t TrimEndPos(wchar_t ch) const;
  WString TrimStart();
  WString TrimEnd();
  WString Trim();
  WString TrimStrat(wchar_t ch);
  WString TrimEnd(wchar_t ch);
  WString Trim(wchar_t ch);

  WString ToLower();
  WString ToUpper();
  WString Capitalize();
  WString Title();

  WString PadLeft(size_t len, wchar_t pad = L' ');
  WString PadRight(size_t len, wchar_t pad = L' ');
  WString PadCenter(size_t len, wchar_t pad = L' ', bool lefts = true);

  WString Remove(size_t len) { return this->Substring(0, len); }
  WString Remove(size_t starts, size_t len);
  WString RemoveReverse(size_t len) { return this->SubstringReverse(0, len); }
  WString RemoveReverse(size_t starts, size_t len)
  { return this->Remove(length - starts - len, len); }
  
  WString Repeat(size_t count);
  WString Reverse();
  WString Slice(size_t first, size_t last);
  WString Slice(size_t skip)
  { return WString((const wchar_t *)(first + skip), length - (skip << 1)); }
  WString Slicing(size_t jmp, size_t starts = 0, size_t len = 1, 
    bool remain = true);
  
  uint64_t Hash(uint64_t seed = 0x8538dcfb7617fe9f) const;
  bool IsNumeric() const;
  bool IsHexDigit() const;
  unsigned long long int ToHexDigit() const;
  inline wchar_t ToChar() const { return first[0];}
  long long int ToLongLong() const;
  unsigned long long int ToULongLong() const;
  long int ToLong() const { return (long)ToLongLong(); }
  unsigned long int ToULong() const { return (unsigned long)ToULongLong(); }
  int ToInteger() const { return (int)ToLongLong(); }
  unsigned int ToUInteger() const { return (unsigned int)ToULongLong(); }
  short int ToShort() const { return (short)ToLongLong(); }
  unsigned short int ToUShort() const { return (unsigned short)ToULongLong(); }

  wchar_t *ToArray() const;
  char *ToAnsi();
  Utf8Array ToUtf8(bool file_bom = false);

  WString operator&(const WString& concat) const
  { return this->Concat(*this, concat); }
  WString operator+(const WString& concat) const
  { return this->Concat(*this, concat); }
  inline bool operator>(const WString& compare) const
  { return wcscmp(first, compare.first) > 0; }
  inline bool operator<(const WString& compare) const
  { return wcscmp(first, compare.first) < 0; }
  inline bool operator>=(const WString& compare) const
  { return !this->operator<(compare); }
  inline bool operator<=(const WString& compare) const
  { return !this->operator>(compare);  }

  void Swap(WString& refer);
  WString& operator=(WString&& refer);
  WString& operator=(const WString& refer);
  void CloneSet(const WString& refer);
  WString Clone();

  friend std::wostream& operator<<(std::wostream& os, const WString& refer)
  {
    if (refer.Null()) os << L"Null-(0)";
    else os << refer.Reference();
    return os;
  }
  
  WString operator+=(const WString&) = delete;
  WString operator&=(const WString&) = delete;

private:
  WString AppendHelper(const wchar_t *str, size_t len);

  size_t FindFirstHelper(const wchar_t *str, size_t starts) const;
  size_t FindLastHelper(const wchar_t *str, size_t ends, size_t len) const;

  bool ContainsHelper(const wchar_t *str, size_t len, bool ignore =false) const;

  size_t CountHelper(const wchar_t *str, size_t len) const;

  ArrayType SplitHelper(const wchar_t *src, size_t srclen, size_t max);
  ArrayType SplitSlowHelper(const wchar_t *src, size_t srclen, size_t max);
  WString SplitPositionHelper(const wchar_t *src, size_t srclen, size_t pos);
  ArrayType SplitReverseHelper(const wchar_t *src, size_t srclen, size_t max);

  WString BetweenHelper(const wchar_t *left, size_t llen, const wchar_t *right, 
    size_t rlen, size_t starts);
  ArrayType BetweensHelper(const wchar_t *left, size_t llen, const wchar_t
    *right, size_t rlen, size_t starts);

  bool StartsWithHelper(const wchar_t *str, size_t starts, size_t len) const;
  bool EndsWithHelper(const wchar_t *str, size_t ends, size_t len) const;

  WString InsertLeftHelper(size_t separation, const wchar_t *str, 
    size_t strlen);
  WString InsertRightHelper(size_t separation, const wchar_t *str,
    size_t strlen);

  WString ReplaceHelper(const wchar_t *src, const wchar_t *dest,
    size_t srclen, size_t destlen, size_t max);
  WString ReplaceSlowHelper(const wchar_t *src, const wchar_t *dest,
    size_t srclen, size_t destlen, size_t max);

  WString TrimHelper(const wchar_t *src, size_t srclen, size_t max);

  WString InsertHelper(size_t starts, const wchar_t *str, size_t len);

  ArrayType LineSplitHelper(size_t len, const wchar_t *front, 
    size_t front_len, const wchar_t *end, size_t end_len);

  long double ToLongDoubleHelper(const wchar_t *ptr) const;

public:
  /// This function is the same implementation as string::append.
  /// To fastly append, use WStringBuilder.
  WString Append(const wchar_t *str)
  { return AppendHelper(str, WStringTools::wcslen(str)); }
  WString Append(const WString& refer)
  { return AppendHelper(refer.first, refer.length); }
  
  size_t FindFirst(const wchar_t *str, size_t starts = 0) const
  { return FindFirstHelper(str, starts); }
  size_t FindFirst(const WString& refer, size_t starts = 0) const
  { return FindFirstHelper(refer.first, starts); }
  size_t FindLast(const wchar_t *str, size_t ends = 0) const
  { return FindLastHelper(str, ends, WStringTools::wcslen(str)); }
  size_t FindLast(const WString& refer, size_t ends = 0) const
  { return FindLastHelper(refer.first, ends, refer.length); }
  size_t FindFirst(const wchar_t ch, size_t starts = 0) const;
  size_t FindLast(const wchar_t ch, size_t ends = 0) const;

  bool Contains(const wchar_t *str, bool ignore = false) const
  { return ContainsHelper(str, WStringTools::wcslen(str), ignore); }
  bool Contains(const WString& refer, bool ignore = false) const
  { return ContainsHelper(refer.first, refer.length, ignore); }

  size_t Count(const wchar_t *str) const
  { return CountHelper(str, WStringTools::wcslen(str)); }
  size_t Count(const WString& refer) const
  { return CountHelper(refer.first, refer.length); }
  size_t Count(const wchar_t ch) const
  { return WStringTools::wcountch(first, last, ch); }

  ArrayType Split(const wchar_t *str, size_t max = SIZE_MAX)
  { return SplitHelper(str, WStringTools::wcslen(str), max); }
  ArrayType Split(const WString &refer, size_t max = SIZE_MAX)
  { return SplitHelper(refer.first, refer.length, max); }
  ArrayType SplitSlow(const wchar_t *str, size_t max = SIZE_MAX)
  { return SplitSlowHelper(str, WStringTools::wcslen(str), max); }
  ArrayType SplitSlow(const WString &refer, size_t max = SIZE_MAX)
  { return SplitSlowHelper(refer.first, refer.length, max); }
  WString SplitPosition(const wchar_t *str, size_t pos)
  { return SplitPositionHelper(str, WStringTools::wcslen(str), pos); }
  WString SplitPosition(const WString& refer, size_t pos)
  { return SplitPositionHelper(refer.first, refer.length, pos); }
  ArrayType SplitReverse(const wchar_t *str, size_t max = SIZE_MAX)
  { return SplitReverseHelper(str, WStringTools::wcslen(str), max); }
  ArrayType SplitReverse(const WString &refer, size_t max = SIZE_MAX)
  { return SplitReverseHelper(refer.first, refer.length, max); }
  
  WString Between(const wchar_t *left, const wchar_t *right, size_t starts = 0)
  { return BetweenHelper(left, WStringTools::wcslen(left), right, 
    WStringTools::wcslen(right), starts); }
  WString Between(const WString& left, const WString& right, size_t starts = 0)
  { return BetweenHelper(left.first, left.length, 
    right.first, right.length, starts); }
  ArrayType Betweens(const wchar_t *left, const wchar_t *right, 
    size_t starts = 0)
  { return BetweensHelper(left, WStringTools::wcslen(left), right, 
    WStringTools::wcslen(right), starts); }
  ArrayType Betweens(const WString& left, const WString& right, 
    size_t starts = 0)
  { return BetweensHelper(left.first, left.length, 
    right.first, right.length, starts); }
  WString Between(wchar_t left, wchar_t right, size_t starts = 0);
  ArrayType Betweens(wchar_t left, wchar_t right, size_t starts = 0);
  
  bool StartsWith(const wchar_t *str, size_t starts = 0) const
  { return StartsWithHelper(str, starts, WStringTools::wcslen(str)); }
  bool StartsWith(const WString& refer, size_t starts = 0) const
  { return StartsWithHelper(refer.first, starts, refer.length); }
  bool StartsWith(const wchar_t ch, size_t starts) const
  { return first[starts] == ch; }
  inline bool StartsWith(const wchar_t ch) const
  { return *first == ch; }
  bool EndsWith(const wchar_t *str, size_t ends = 0) const
  { return EndsWithHelper(str, ends, WStringTools::wcslen(str)); }
  bool EndsWith(const WString& refer, size_t ends = 0) const
  { return EndsWithHelper(refer.first, ends, refer.length); }
  bool EndsWith(const wchar_t ch, size_t ends) const
  { return *(last - ends) == ch; }
  inline bool EndsWith(const wchar_t ch) const
  { return *last == ch; }

  WString InsertLeft(size_t separation, const wchar_t *str)
  { return InsertLeftHelper(separation, str, WStringTools::wcslen(str)); }
  WString InsertLeft(size_t separation, const WString& refer)
  { return InsertLeftHelper(separation, refer.first, refer.length); }
  WString InsertRight(size_t separation, const wchar_t *str)
  { return InsertRightHelper(separation, str, WStringTools::wcslen(str)); }
  WString InsertRight(size_t separation, const WString& refer)
  { return InsertRightHelper(separation, refer.first, refer.length); }
  WString InsertLeft(size_t separation, wchar_t ch);
  WString InsertRight(size_t separation, wchar_t ch);

  WString Replace(const wchar_t *src, const wchar_t *dest, 
    size_t max = SIZE_MAX)
  { return ReplaceHelper(src, dest, 
    WStringTools::wcslen(src), WStringTools::wcslen(dest), max); }
  WString Replace(const WString& refer0, const WString& refer1, 
    size_t max = SIZE_MAX)
  { return ReplaceHelper(refer0.first, refer1.first, refer0.length, 
    refer1.length, max); }
  WString ReplaceSlow(const wchar_t *src, const wchar_t *dest, 
    size_t max = SIZE_MAX)
  { return ReplaceSlowHelper(src, dest, 
    WStringTools::wcslen(src), WStringTools::wcslen(dest), max); }
  WString ReplaceSlow(const WString& refer0, const WString& refer1, 
    size_t max = SIZE_MAX)
  { return ReplaceSlowHelper(refer0.first, refer1.first, refer0.length, 
      refer1.length, max); }

  WString Trim(const wchar_t *src) 
  { return TrimHelper(src, wcslen(src), SIZE_MAX); }
  WString Trim(const WString& refer)
  { return TrimHelper(refer.first, refer.length, SIZE_MAX); }
  
  WString Insert(size_t starts, const wchar_t *str, size_t len)
  { return InsertHelper(starts, str, len); }
  WString Insert(size_t starts, const WString& refer, size_t len)
  { return Insert(starts, refer.first, len); }
  WString Insert(size_t starts, const wchar_t *str)
  { return Insert(starts, str, WStringTools::wcslen(str)); }
  WString Insert(size_t starts, const WString& refer)
  { return Insert(starts, refer.first, refer.length); }

  ArrayType LineSplit(size_t len)
  { return LineSplitHelper(len, nullptr, 0, nullptr, 0); }
  ArrayType LineSplit(size_t len, const WString& front)
  { return LineSplitHelper(len, front.Reference(), front.length, nullptr, 0); }
  ArrayType LineSplit(size_t len, const wchar_t *front)
  { return LineSplitHelper(len, front, WStringTools::wcslen(front), 
    nullptr, 0); }
  ArrayType LineSplit(size_t len, const WString& front, const WString& end)
  { return LineSplitHelper(len, front.Reference(), front.length, 
    end.Reference(), end.length); }
  ArrayType LineSplit(size_t len, const wchar_t *front, const wchar_t *end)
  { return LineSplitHelper(len, front, WStringTools::wcslen(front), 
    end, WStringTools::wcslen(end)); }
  ArrayType LineSplit(bool last = false);
  WString LineBreak(size_t len);

  long double ToLongDouble() const { return ToLongDoubleHelper(first); }
  double ToDouble() const { return (double)ToLongDouble(); }
  float ToFloat() const { return (float)ToLongDouble(); }

private:
  wchar_t *alloc(size_t c) { return new wchar_t[c]; }

  void AnsiToUnicode(const char *str, size_t len);
  char *UnicodeToAnsi();

  /// Copy to WString pointer.
  void InitString(const char *str);
  void InitString(const wchar_t *str);
};

inline WString operator"" _ws(const wchar_t* str, size_t length)
{
  return WString{ str, length };
}

inline WString operator"" _ws(const char* str, size_t length)
{
  return WString{ str, length };
}

inline WString operator"" _ws(unsigned long long i)
{
  return WString{ i };
}

}

#endif