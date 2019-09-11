//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#ifndef _STRING_9bf1541fdf7efd41b7b39543fd870ac4_
#define _STRING_9bf1541fdf7efd41b7b39543fd870ac4_

#include <stdint.h>
#include <cstring>

// Get a pointer type can be sure.
#if defined(__x86_64__) || defined(__ia64__) || defined(_M_AMD64) \
  || defined(_M_IA64) || defined(_WIN64) || defined(__alpha__) \
  || defined(__s390__)
#define _X64_MODE
typedef uint64_t	ptr_type;
#else
typedef uint32_t	ptr_type;
#endif

#ifdef _MSC_VER
#define _COMPILER_MS
#elif __clang__
#define _COMPILER_LLVM
#elif __GNUC__
#define _COMPILER_GCC
#elif defined(__MINGW32__) || defined(__MINGW64__)
#define _COMPILER_MINGW
#endif

#if defined(_WIN32) || defined(_WIN64)
#define _OS_WINDOWS
#elif __linux__
#define _OS_LINUX
#endif

#include <memory>
#include <string>
#include <iostream>

namespace jsonhead
{
  
template<typename type>
class ArrayBase
{
public:

  ArrayBase()
    : m_size(0)
    , m_pointer(nullptr)
    , m_constptr(nullptr)
  {
  }

  ArrayBase(size_t size, type *pointer)
    : m_size(size)
    , m_pointer(pointer)
    , m_constptr(pointer)
  {
  }

  virtual ~ArrayBase()
  {
    if (!m_nodel)
    {
      _class_delete(*m_constptr, m_constptr);
      delete[] m_constptr;
      m_constptr = nullptr;
    }
  }

  template<typename tt> 
  void _class_delete(tt&, tt*&)
  {
  }
  template<typename tt>
  void _class_delete(tt*&, tt* const*& arr)
  {
    for (size_t i = 0; i < m_size; i++)
    {
      arr[i].~type();
      delete arr[i];
    }
  }

protected:

  bool   m_nodel = false;
  size_t m_size;
  type  *m_pointer;
  type  *m_constptr;

};

// Read Only Array [ const array iterator ]
template<typename type>
class ReadOnlyArray
  : public ArrayBase<type>
{
  typedef ReadOnlyArray<type> this_type;

public:
  ReadOnlyArray(type *ptr, size_t size) : ArrayBase<type>(size, ptr) { }
  ReadOnlyArray() : ArrayBase<type>() { }
  ~ReadOnlyArray() { }

  type operator*() const { return *this->m_pointer; }
  type* operator->() const { return **this; }
  type operator[](size_t off) const { return *(*this + off); }
  this_type& operator--() { --this->m_pointer; return *this; }

  this_type operator--(int)
  {
    this_type tmp = *this;
    tmp.m_nodel = true;
    --this->m_pointer;
    return tmp;
  }

  this_type& operator++()
  {
    ++this->m_pointer;
    return *this;
  }

  this_type operator++(int)
  {
    this_type tmp = *this;
    tmp.m_nodel = true;
    ++*this->m_pointer;
    return tmp;
  }

  this_type& operator+=(size_t size)
  {
    this->m_pointer += size;
    return *this;
  }

  this_type operator+(size_t size) const
  {
    this_type tmp = *this;
    tmp.m_nodel = true;
    return (tmp += size);
  }

  this_type& operator-=(size_t size)
  {
    this->m_pointer -= size;
    return *this;
  }

  this_type operator-(size_t size) const
  {
    this_type tmp = *this;
    tmp.m_nodel = true;
    return (tmp -= size);
  }

  bool operator==(const this_type& refer) const
  { return (this->m_pointer == refer.m_pointer); }
  bool operator!=(const this_type& refer) const
  { return (!(*this == refer)); }
  bool operator<(const this_type& refer) const
  { return (this->m_pointer < refer.m_pointer); }
  bool operator>(const this_type& refer) const
  { return (refer < *this); }
  bool operator<=(const this_type& refer) const
  { return (!(refer < *this)); }
  bool operator>=(const this_type& refer) const
  { return (!(*this < refer)); }
  void Reset() { this->m_pointer = this->m_constptr; }
  type* Array() const { return this->m_constptr; }
  size_t Size() const { return this->m_size; }

  // For Each <Item> In Type _ etc...
  template<typename func>
  void Each(func function)
  {
    for (size_t i = 0; i < this->m_size; i++)
      function(this->m_constptr[i]);
  }

};

class StringTools
{
public:
  static size_t strlen(const char *str);
  static char *strrnchr(char *ptr, size_t len, char ch);
  static char *strrnstrn(char *ptr, size_t ptrlen, 
    const char *dest, size_t destlen);
  static void strnset(char *ptr, wchar_t ch, size_t len);
  static size_t strcountch(char *ptr, char *last, char ch);
};

class String final
{
  char *first;
  char *last;
  size_t   length;

  /// If this flag is on, string-pointer is not deleted in destructor.
  bool tm = false;

public:
  using ArrayType = ReadOnlyArray<String *>;
  using Utf8Array = ReadOnlyArray<unsigned char>;

  static const size_t error = -1;

  String() : length(0), first(nullptr), last(first) { }
  String(const char *str) { InitString(str); }
  String(const char *str, size_t len) { InitString(str); }
  explicit String(char *str, size_t len, bool built_in = true);
  String(char ch, size_t count);
  String(char ch);
  String(int);
  String(long int);
  String(long long int);
  String(unsigned int);
  String(unsigned long int);
  String(unsigned long long int);
  String(float);
  String(double);
  String(String&& ws) : first(ws.first), last(ws.last), 
    length(ws.length) { ws.tm = true; }
  String(const String& cnt) : String((const char *)cnt.first, cnt.length) {}
  String(std::string& str) : String(&str[0], str.length()) { }
  String(const std::string& str) : String(str.c_str(), str.length()) { }
  ~String() { if (first != nullptr && !tm){delete[] first; first = nullptr;}}

  inline size_t Length() const { return length; }
  inline bool Empty() const { return length == 0; }
  inline bool Full() const { return length > 0; }
  inline bool Null() const { return first == nullptr; }
  inline const char *Reference() const { return first; }
  
  inline size_t CompareTo(const char *str) const 
  { return ::strcmp(first, str); }
  inline size_t CompareTo(const String& refer) const 
  { return CompareTo(refer.first); }
  inline static int Comparer(const String& r1, const String &r2) 
  { return ::strcmp(r1.first, r2.first); }

  inline bool Equal(const String& refer) const
  {
    if (refer.length != this->length)
      return false;
    return !memcmp(first, refer.first, length * sizeof(char));
  }

  inline bool operator==(const char *str) const { return Equal(str); }
  inline bool operator==(const String& refer) const { return Equal(refer); }
  inline bool operator!=(const char *str) const { return !Equal(str); }
  inline bool operator!=(const String& refer) const { return !Equal(refer); }

  inline char First(size_t pos) const { return first[pos]; }
  inline char Last(size_t pos) const { return *(last - pos); }
  inline char operator[](size_t index) const { return first[index]; }
  
  /// Concatenate a number of strings.
  static String Concat(const String& t1, const String& t2);
  static String Concat(const String& t1, const String& t2, 
    const String& t3);
  static String Concat(const String& t1, const String& t2, const String& t3,
    const String& t4);
  
  String Substring(size_t starts)
  { return String((const char *)(first + starts), length - starts); }
  String Substring(size_t starts, size_t len)
  { return String((const char *)(first + starts), len); }
  String SubstringReverse(size_t starts)
  { return String((const char *)(first), length - starts); }
  String SubstringReverse(size_t starts, size_t len)
  { return String((const char *)(first - starts - len + 1), len); }

  size_t TirmStartPos() const;
  size_t TrimEndPos() const;
  size_t TrimStartPos(char ch) const;
  size_t TrimEndPos(char ch) const;
  String TrimStart();
  String TrimEnd();
  String Trim();
  String TrimStrat(char ch);
  String TrimEnd(char ch);
  String Trim(char ch);

  String ToLower();
  String ToUpper();
  String Capitalize();
  String Title();

  String PadLeft(size_t len, char pad = L' ');
  String PadRight(size_t len, char pad = L' ');
  String PadCenter(size_t len, char pad = L' ', bool lefts = true);

  String Remove(size_t len) { return this->Substring(0, len); }
  String Remove(size_t starts, size_t len);
  String RemoveReverse(size_t len) { return this->SubstringReverse(0, len); }
  String RemoveReverse(size_t starts, size_t len)
  { return this->Remove(length - starts - len, len); }
  
  String Repeat(size_t count);
  String Reverse();
  String Slice(size_t first, size_t last);
  String Slice(size_t skip)
  { return String((const char *)(first + skip), length - (skip << 1)); }
  String Slicing(size_t jmp, size_t starts = 0, size_t len = 1, 
    bool remain = true);
  
  uint64_t Hash(uint64_t seed = 0x8538dcfb7617fe9f) const;
  bool IsNumeric() const;
  bool IsHexDigit() const;
  unsigned long long int ToHexDigit() const;
  inline char ToChar() const { return first[0];}
  long long int ToLongLong() const;
  unsigned long long int ToULongLong() const;
  long int ToLong() const { return (long)ToLongLong(); }
  unsigned long int ToULong() const { return (unsigned long)ToULongLong(); }
  int ToInteger() const { return (int)ToLongLong(); }
  unsigned int ToUInteger() const { return (unsigned int)ToULongLong(); }
  short int ToShort() const { return (short)ToLongLong(); }
  unsigned short int ToUShort() const { return (unsigned short)ToULongLong(); }

  char *ToArray() const;
  char *ToAnsi();
  Utf8Array ToUtf8(bool file_bom = false);

  String operator&(const String& concat) const
  { return this->Concat(*this, concat); }
  String operator+(const String& concat) const
  { return this->Concat(*this, concat); }
  inline bool operator>(const String& compare) const
  { return ::strcmp(first, compare.first) > 0; }
  inline bool operator<(const String& compare) const
  { return ::strcmp(first, compare.first) < 0; }
  inline bool operator>=(const String& compare) const
  { return !this->operator<(compare); }
  inline bool operator<=(const String& compare) const
  { return !this->operator>(compare);  }

  void Swap(String& refer);
  String& operator=(String&& refer);
  String& operator=(const String& refer);
  void CloneSet(const String& refer);
  String Clone();

  friend std::ostream& operator<<(std::ostream& os, const String& refer)
  {
    if (refer.Null()); // os << "Null-(0)"; // nothing
    else os << refer.Reference();
    return os;
  }
  
  String operator+=(const String&) = delete;
  String operator&=(const String&) = delete;

private:
  String AppendHelper(const char *str, size_t len);

  size_t FindFirstHelper(const char *str, size_t starts) const;
  size_t FindLastHelper(const char *str, size_t ends, size_t len) const;

  bool ContainsHelper(const char *str, size_t len, bool ignore =false) const;

  size_t CountHelper(const char *str, size_t len) const;

  ArrayType SplitHelper(const char *src, size_t srclen, size_t max);
  ArrayType SplitSlowHelper(const char *src, size_t srclen, size_t max);
  String SplitPositionHelper(const char *src, size_t srclen, size_t pos);
  ArrayType SplitReverseHelper(const char *src, size_t srclen, size_t max);

  String BetweenHelper(const char *left, size_t llen, const char *right, 
    size_t rlen, size_t starts);
  ArrayType BetweensHelper(const char *left, size_t llen, const char
    *right, size_t rlen, size_t starts);

  bool StartsWithHelper(const char *str, size_t starts, size_t len) const;
  bool EndsWithHelper(const char *str, size_t ends, size_t len) const;

  String InsertLeftHelper(size_t separation, const char *str, 
    size_t strlen);
  String InsertRightHelper(size_t separation, const char *str,
    size_t strlen);

  String ReplaceHelper(const char *src, const char *dest,
    size_t srclen, size_t destlen, size_t max);
  String ReplaceSlowHelper(const char *src, const char *dest,
    size_t srclen, size_t destlen, size_t max);

  String TrimHelper(const char *src, size_t srclen, size_t max);

  String InsertHelper(size_t starts, const char *str, size_t len);

  ArrayType LineSplitHelper(size_t len, const char *front, 
    size_t front_len, const char *end, size_t end_len);

  long double ToLongDoubleHelper(const char *ptr) const;

public:
  /// This function is the same implementation as string::append.
  /// To fastly append, use StringBuilder.
  String Append(const char *str)
  { return AppendHelper(str, StringTools::strlen(str)); }
  String Append(const String& refer)
  { return AppendHelper(refer.first, refer.length); }
  
  size_t FindFirst(const char *str, size_t starts = 0) const
  { return FindFirstHelper(str, starts); }
  size_t FindFirst(const String& refer, size_t starts = 0) const
  { return FindFirstHelper(refer.first, starts); }
  size_t FindLast(const char *str, size_t ends = 0) const
  { return FindLastHelper(str, ends, StringTools::strlen(str)); }
  size_t FindLast(const String& refer, size_t ends = 0) const
  { return FindLastHelper(refer.first, ends, refer.length); }
  size_t FindFirst(const char ch, size_t starts = 0) const;
  size_t FindLast(const char ch, size_t ends = 0) const;

  bool Contains(const char *str, bool ignore = false) const
  { return ContainsHelper(str, StringTools::strlen(str), ignore); }
  bool Contains(const String& refer, bool ignore = false) const
  { return ContainsHelper(refer.first, refer.length, ignore); }

  size_t Count(const char *str) const
  { return CountHelper(str, StringTools::strlen(str)); }
  size_t Count(const String& refer) const
  { return CountHelper(refer.first, refer.length); }
  size_t Count(const char ch) const
  { return StringTools::strcountch(first, last, ch); }

  ArrayType Split(const char *str, size_t max = SIZE_MAX)
  { return SplitHelper(str, StringTools::strlen(str), max); }
  ArrayType Split(const String &refer, size_t max = SIZE_MAX)
  { return SplitHelper(refer.first, refer.length, max); }
  ArrayType SplitSlow(const char *str, size_t max = SIZE_MAX)
  { return SplitSlowHelper(str, StringTools::strlen(str), max); }
  ArrayType SplitSlow(const String &refer, size_t max = SIZE_MAX)
  { return SplitSlowHelper(refer.first, refer.length, max); }
  String SplitPosition(const char *str, size_t pos)
  { return SplitPositionHelper(str, StringTools::strlen(str), pos); }
  String SplitPosition(const String& refer, size_t pos)
  { return SplitPositionHelper(refer.first, refer.length, pos); }
  ArrayType SplitReverse(const char *str, size_t max = SIZE_MAX)
  { return SplitReverseHelper(str, StringTools::strlen(str), max); }
  ArrayType SplitReverse(const String &refer, size_t max = SIZE_MAX)
  { return SplitReverseHelper(refer.first, refer.length, max); }
  
  String Between(const char *left, const char *right, size_t starts = 0)
  { return BetweenHelper(left, StringTools::strlen(left), right, 
    StringTools::strlen(right), starts); }
  String Between(const String& left, const String& right, size_t starts = 0)
  { return BetweenHelper(left.first, left.length, 
    right.first, right.length, starts); }
  ArrayType Betweens(const char *left, const char *right, 
    size_t starts = 0)
  { return BetweensHelper(left, StringTools::strlen(left), right, 
    StringTools::strlen(right), starts); }
  ArrayType Betweens(const String& left, const String& right, 
    size_t starts = 0)
  { return BetweensHelper(left.first, left.length, 
    right.first, right.length, starts); }
  String Between(char left, char right, size_t starts = 0);
  ArrayType Betweens(char left, char right, size_t starts = 0);
  
  bool StartsWith(const char *str, size_t starts = 0) const
  { return StartsWithHelper(str, starts, StringTools::strlen(str)); }
  bool StartsWith(const String& refer, size_t starts = 0) const
  { return StartsWithHelper(refer.first, starts, refer.length); }
  bool StartsWith(const char ch, size_t starts) const
  { return first[starts] == ch; }
  inline bool StartsWith(const char ch) const
  { return *first == ch; }
  bool EndsWith(const char *str, size_t ends = 0) const
  { return EndsWithHelper(str, ends, StringTools::strlen(str)); }
  bool EndsWith(const String& refer, size_t ends = 0) const
  { return EndsWithHelper(refer.first, ends, refer.length); }
  bool EndsWith(const char ch, size_t ends) const
  { return *(last - ends) == ch; }
  inline bool EndsWith(const char ch) const
  { return *last == ch; }

  String InsertLeft(size_t separation, const char *str)
  { return InsertLeftHelper(separation, str, StringTools::strlen(str)); }
  String InsertLeft(size_t separation, const String& refer)
  { return InsertLeftHelper(separation, refer.first, refer.length); }
  String InsertRight(size_t separation, const char *str)
  { return InsertRightHelper(separation, str, StringTools::strlen(str)); }
  String InsertRight(size_t separation, const String& refer)
  { return InsertRightHelper(separation, refer.first, refer.length); }
  String InsertLeft(size_t separation, char ch);
  String InsertRight(size_t separation, char ch);

  String Replace(const char *src, const char *dest, 
    size_t max = SIZE_MAX)
  { return ReplaceHelper(src, dest, 
    StringTools::strlen(src), StringTools::strlen(dest), max); }
  String Replace(const String& refer0, const String& refer1, 
    size_t max = SIZE_MAX)
  { return ReplaceHelper(refer0.first, refer1.first, refer0.length, 
    refer1.length, max); }
  String ReplaceSlow(const char *src, const char *dest, 
    size_t max = SIZE_MAX)
  { return ReplaceSlowHelper(src, dest, 
    StringTools::strlen(src), StringTools::strlen(dest), max); }
  String ReplaceSlow(const String& refer0, const String& refer1, 
    size_t max = SIZE_MAX)
  { return ReplaceSlowHelper(refer0.first, refer1.first, refer0.length, 
      refer1.length, max); }

  String Trim(const char *src) 
  { return TrimHelper(src, strlen(src), SIZE_MAX); }
  String Trim(const String& refer)
  { return TrimHelper(refer.first, refer.length, SIZE_MAX); }
  
  String Insert(size_t starts, const char *str, size_t len)
  { return InsertHelper(starts, str, len); }
  String Insert(size_t starts, const String& refer, size_t len)
  { return Insert(starts, refer.first, len); }
  String Insert(size_t starts, const char *str)
  { return Insert(starts, str, StringTools::strlen(str)); }
  String Insert(size_t starts, const String& refer)
  { return Insert(starts, refer.first, refer.length); }

  ArrayType LineSplit(size_t len)
  { return LineSplitHelper(len, nullptr, 0, nullptr, 0); }
  ArrayType LineSplit(size_t len, const String& front)
  { return LineSplitHelper(len, front.Reference(), front.length, nullptr, 0); }
  ArrayType LineSplit(size_t len, const char *front)
  { return LineSplitHelper(len, front, StringTools::strlen(front), 
    nullptr, 0); }
  ArrayType LineSplit(size_t len, const String& front, const String& end)
  { return LineSplitHelper(len, front.Reference(), front.length, 
    end.Reference(), end.length); }
  ArrayType LineSplit(size_t len, const char *front, const char *end)
  { return LineSplitHelper(len, front, StringTools::strlen(front), 
    end, StringTools::strlen(end)); }
  ArrayType LineSplit(bool last = false);
  String LineBreak(size_t len);

  long double ToLongDouble() const { return ToLongDoubleHelper(first); }
  double ToDouble() const { return (double)ToLongDouble(); }
  float ToFloat() const { return (float)ToLongDouble(); }

private:
  char *alloc(size_t c) { return new char[c]; }

  void AnsiToUnicode(const char *str, size_t len);
  char *UnicodeToAnsi();

  /// Copy to String pointer.
  void InitString(const char *str);
};

inline String operator"" _s(const char* str, size_t length)
{
  return String{ str, length };
}

inline String operator"" _s(unsigned long long i)
{
  return String{ i };
}

}

#endif