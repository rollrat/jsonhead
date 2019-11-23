//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#ifndef _PROGRESS_
#define _PROGRESS_

#include "String.h"
#include "StringBuilder.h"
#include <iostream>
#include <thread>

namespace jsonhead {

class ProgressBase {
  String current_text;
public:
  virtual void Dispose();
protected:
  bool disposed;
  void update_text(String&& text);
};

class WaitProgress : public ProgressBase {
  String animation = "|/-\\";
  int index = 0;
  std::thread timer;
public:
  WaitProgress();
  virtual void Dispose();
private:
  void Tick();
};

class ProgressBar : public ProgressBase {
  int block_count = 20;
public:
  ProgressBar(int block_count = 20) : block_count(block_count) { }
  void Update(double progress, String message);
};

}

#endif