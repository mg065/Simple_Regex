#include <iostream>
#include "regex.h"

using namespace std;

int main(int argc, char *argv[]) {
  {
    Regex re;

    re.Compile("a");

    cout << re.Match("b") << endl;
    cout << re.Match("a") << endl;
  }

  {
    Regex re;

    re.Compile("ab|c");

    cout << re.Match("ab") << endl;
    cout << re.Match("c") << endl;
    cout << re.Match("bc") << endl;
  }

  {
    Regex re;

    re.Compile("a|bc");

    cout << re.Match("a") << endl;
    cout << re.Match("bc") << endl;
    cout << re.Match("ab") << endl;
  }

  {
    Regex re;

    re.Compile("a|b*cd");

    cout << re.Match("a") << endl;
    cout << re.Match("bcd") << endl;
    cout << re.Match("cd") << endl;
    cout << re.Match("bc") << endl;
    cout << re.Match("ab") << endl;
  }

  {
    Regex re;

    re.Compile("1a*b*|c");

    cout << re.Match("1a") << endl;
    cout << re.Match("1b") << endl;
    cout << re.Match("1aab") << endl;
    cout << re.Match("1abb") << endl;
    cout << re.Match("1abbc") << endl;
    cout << re.Match("1c") << endl;
  }

  return 0;
}
