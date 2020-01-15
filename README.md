# JsonHead - Fast and Simple json structure analyzer

If you don't want a json scheme generator, use `simdjson` or another json library.

## Motivation

It may be difficult to analyze the `json` file because of its large size, 
its structure is complicated, or it may be difficult to grasp the overall 
structure because parts are omitted (ex. null type exists).

This project began to automate the task of analyzing any type of `json` file 
and creating a model.

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
  // If you are not formatting json or intending to use it in c++, 
  // enable this syntax to reduce memory usage.
  //ps.skip_literal() = true;
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
  
  std::cout << std::endl;
  
  // Print C# Style json model code
  jsonhead::json_tree_exporter jte(tr.tree_entry());
  std::cout << jte.export_cs_newtonsoftjson_style("hiddendata") << '\n';
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

Automate created model of KorQuAD

``` c#
public class KorQuAD_Model
{
    public class KorQuAD_Model_sub_0
    {
        public class KorQuAD_Model_sub_1
        {
            public string context;
            public class KorQuAD_Model_sub_3
            {
                public string question;
                public string id;
                public class KorQuAD_Model_sub_5
                {
                    public string text;
                    public string html_answer_text;
                    public double answer_start;
                    public double html_answer_start;
                }

                public KorQuAD_Model_sub_5 answer;
            }

            public List<KorQuAD_Model_sub_3> qas;
            public string title;
            public string url;
            public string raw_html;
        }

        public List<KorQuAD_Model_sub_1> data;
        public string version;
    }

    public KorQuAD_Model_sub_0 entry;
}
```
