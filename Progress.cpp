//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#include "Progress.h"
#include <algorithm>

///===-----------------------------------------------------------------------===
///
///               Progress Base
///
///===-----------------------------------------------------------------------===

void jsonhead::ProgressBase::Dispose() {
  disposed = true;
  update_text(jsonhead::String(""));
}

void jsonhead::ProgressBase::update_text(jsonhead::String&& text) {
using namespace jsonhead;
  int prefix_len = 0;
  int length = std::min(current_text.Length(), text.Length());

  while (prefix_len < length && text[prefix_len] == current_text[prefix_len])
    prefix_len++;

  StringBuilder builder;
  builder.Append(String('\b', current_text.Length() - prefix_len));
  builder.Append(text.Substring(prefix_len));

  int overlap = current_text.Length() - text.Length();
  if (overlap > 0)
  {
    builder.Append(String(' ', overlap));
    builder.Append(String('\b', overlap));
  }

  std::cout << builder.ToString();
  current_text = text;
}

///===-----------------------------------------------------------------------===
///
///               Wait Progress
///
///===-----------------------------------------------------------------------===

jsonhead::WaitProgress::WaitProgress() {
  timer = std::thread([&]() {
    while (true) {
      if (disposed) break;
      auto delta = std::chrono::steady_clock::now() + 
                   std::chrono::milliseconds(125);
      Tick();
      std::this_thread::sleep_until(delta);
    }
  });
}

void jsonhead::WaitProgress::Tick() {
  update_text(animation[index++ % animation.Length()]);
}

void jsonhead::WaitProgress::Dispose() {
  disposed = true;
  timer.detach();
  update_text(jsonhead::String(""));
}

///===-----------------------------------------------------------------------===
///
///               Progress Bar
///
///===-----------------------------------------------------------------------===

void jsonhead::ProgressBar::Update(double progress, String message) {
using namespace jsonhead;
  if (disposed) return;
  int progress_block_count = (int)(progress * block_count);
  int percent = (int)(progress * 100);

  StringBuilder builder;
  builder.Append('[');
  builder.Append(String('#', progress_block_count));
  builder.Append(String('-', (size_t)block_count - progress_block_count));
  builder.Append("] ");
  builder.Append(percent);
  builder.Append("% [");
  builder.Append(message);
  builder.Append(']');

  update_text(builder.ToString());
}
