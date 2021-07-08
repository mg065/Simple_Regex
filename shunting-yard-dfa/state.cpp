#include <iostream>
#include "state.h"
#include "tree.h"

int State::gIndex_ = 1;

void State::Init() {
  gIndex_ = 1;
}

State::State(const set<Tree*> &tree_set)
  : acceptable_(false) {
  index_ = gIndex_;
  ++gIndex_;
  tree_set_ = tree_set;
  set<Tree*>::iterator iter;
  for (iter = tree_set_.begin();
       iter != tree_set_.end(); ++iter) {
    if ((*iter)->get_type() == END) {
      acceptable_ = true;
      break;
    }
  }
}

State::~State() {
}

void State::AddTransferState(int c, State* state) {
  transfer_map_[c] = state;
}
