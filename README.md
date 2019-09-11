# JsonHead - Fast and Simple json parser

## Example

``` c++
#include <iostream>
#include "jsonhead.h"

using namespace std;

int main()
{
  auto fn =  R"(C:\Users\rollrat\Desktop\namuwiki190312\namuwiki_20190312.json)";
  jsonhead::json_parser ps(fn);
  long long count = 0;
  while (ps.step()) {
    count++;
    if (count % 100000 == 0) {
      cout << ps.position() << '/' << ps.filesize() << '(' << 
             ((long double)ps.position() / ps.filesize() * 100.0) << ')' << '\n';
    }
  }
  
  ofstream ofs("formatted_namuwiki_20190312.json");
  ps.entry()->print(ofs, true);
}
```
