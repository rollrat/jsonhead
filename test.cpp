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
  auto fn =  R"(/home/rollrat/github/jsonhead/namuwiki_20190312.json)";
  ifstream file(fn, ios::binary | ios::ate);
  long long tsz = file.tellg();
  file.close();

  jsonhead::json_lexer lexer(fn);
  long long count = 0;
  while (lexer.next())
  {
    count++;
    if (count % 100000 == 0) {
      cout << lexer.stream().tellg() << '/' << tsz << '(' << ((double)lexer.stream().tellg() / tsz * 100.0) << ')' << '\n';
    }
  }

  cout << count << '\n';
}