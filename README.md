# JsonHead - Fast and Simple json parser

## Example

``` c++
#include <iostream>
#include "jsonhead.h"

using namespace std;

int main()
{
  auto fn = R"(namuwiki_20190312.json)";
  //auto fn = R"(korquad2.0_train_00.json)";
  jsonhead::json_parser ps(fn);
  long long count = 0;
  while (ps.step()) {
    count++;
    if (count % 100000 == 0) {
      cout << ps.position() << '/' << ps.filesize() << '(' << 
             ((long double)ps.position() / ps.filesize() * 100.0) << ')' << '\n';
    }
  }
  
  // Print formatted json
  ofstream ofs("formatted_namuwiki_20190312.json");
  ps.entry()->print(ofs, true);
  
  // Print json structure
  jsonhead::json_tree tr(ps.entry());
  tr.tree_entry()->print(cout);
}
```

KorQuAD Structrue Example
 
``` json
{
  "data": [
    {
      "context": string,
      "qas": [
        {
          "question": string,
          "id": string,
          "answer": {
            "text": string,
            "html_answer_text": string,
            "answer_start": numeric,
            "html_answer_start": numeric
          }
        }
      ],
      "title": string,
      "url": string,
      "raw_html": string
    }
  ],
  "version": string
}
```
