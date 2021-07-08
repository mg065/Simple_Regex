#include <iostream>
#include "regex.h"

using namespace std;

int main(int argc, char *argv[]) {
  {
    Regex re;

    re.Compile("(a|b)*abb");

    cout << re.Match("aabb") << endl;
    cout << re.Match("abb") << endl;
    cout << re.Match("b") << endl;
    cout << re.Match("babb") << endl;
    cout << re.Match("bbababb") << endl;
  }

  {
    Regex re;
    re.Compile("(a|b)a*c");
    cout << re.Match("aac") << endl;
    cout << re.Match("bac") << endl;
    cout << re.Match("ac") << endl;
    cout << re.Match("bc") << endl;
    cout << re.Match("c") << endl;
  }

  return 0;
}
