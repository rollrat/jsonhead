//===----------------------------------------------------------------------===//
//
//                      Json Parser for large data set
//
//===----------------------------------------------------------------------===//
//
//  Copyright (C) 2019. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <memory>
#include "jsonhead.h"

using namespace std;

int main()
{
  auto fn = R"(/home/rollrat/github/jsonhead/namuwiki_20190312.json)";
  //auto fn =  R"(C:\Users\rollrat\Desktop\namuwiki190312\namuwiki_20190312.json)";

  jsonhead::json_parser ps(fn);
  ps.skip_literal() = true;
  long long count = 0;
  while (ps.step()) {
    count++;
    if (count % 100000 == 0) {
      cout << ps.readsize() << '/' << ps.filesize() << '(' << ((long double)ps.readsize() / ps.filesize() * 100.0) << ')' << '\n';
    }
  }

#if 0
  ifstream file(fn, ios::binary | ios::ate);
  long long tsz = file.tellg();
  file.close();

  jsonhead::json_lexer lexer(fn);
  long long count = 0;
  while (lexer.next())
  {
    if(lexer.type() == jsonhead::json_token::eof)
      break;
    count++;
    if (count % 100000 == 0) {
      cout << lexer.stream().tellg() << '/' << tsz << '(' << ((double)lexer.stream().tellg() / tsz * 100.0) << ')' << '\n';
    }
  }

  cout << count << '\n';
#endif

  cout << "and";
}