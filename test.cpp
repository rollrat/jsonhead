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
  jsonhead::json_lexer lexer(R"(C:\Users\rollrat\Desktop\namuwiki190312\namuwiki_20190312.json)");

  int count = 0;
  while (lexer.next())
  {
    count++;
    //cout << lexer.str() << endl;
  }
}