#include <iostream>
#include "tree.h"

int Tree::gIndex = 1;

void Tree::Init() {
  gIndex = 1;
}

void Tree::PrintPos() {
  cout << "index: " << index_ << endl;
  PrintPos(first_pos_, "first pos");
  PrintPos(last_pos_, "last pos");
  PrintPos(follow_pos_, "follow pos");
}

void Tree::PrintPos(const set<Tree*> &pos, const char *tag) {
  cout << tag << ": (";
  set<Tree*>::iterator iter;
  for (iter = pos.begin(); iter != pos.end(); ++iter) {
    cout << " " << (*iter)->get_index();
  }
  cout << " )" << endl;
}
