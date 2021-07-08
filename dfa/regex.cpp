#include <iostream>
#include <list>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include "regex.h"
#include "state.h"
#include "stream.h"
#include "tree.h"

using namespace std;

Regex::Regex()
  : root_(NULL) {
  Tree::Init();
  State::Init();
}

Regex::~Regex() {
  map<int, Tree*>::iterator tree_iter;
  for (tree_iter = tree_map_.begin();
       tree_iter != tree_map_.end(); ++tree_iter) {
    delete tree_iter->second;
  }
  map<int, State*>::iterator state_iter;
  for (state_iter = state_map_.begin();
       state_iter != state_map_.end(); ++state_iter) {
    delete state_iter->second;
  }
}

void Regex::AddTree(Tree *tree) {
  tree_map_[tree->get_index()] = tree;
}

Tree* Regex::NewCharNode(int c) {
  Tree *tree = new Tree(NORMAL, c);
  AddTree(tree);
  set<Tree*> pos;
  pos.insert(tree);
  // N = char node;fisrpos(N) = lastpos(N) = {N}
  tree->add_firstpos(pos);
  tree->add_lastpos(pos);

  chars_map_[c] = true;
  return tree;
}

void  Regex::ProcessCatPos(Tree *parent, Tree *left, Tree *right) {
  // for nullable
  if (left->get_nullable() && right->get_nullable()) {
    parent->set_nullable(true);
  }

  // for first pos
  // for cat node(N) = c1c2
  // if (c1->nullable()) then firstnode(N) = firstnode(c1) + firstnode(c2)
  // else firstnode(N) = firstnode(c1)
  if (left->get_nullable()) {
    parent->add_firstpos(left->get_firstpos());
    parent->add_firstpos(right->get_firstpos());
  } else {
    parent->add_firstpos(left->get_firstpos());
  }

  // for last pos
  // for cat node(N) = c1c2
  // if (c2->nullable()) then lastpos(N) = lastpos(c1) + lastpos(c2)
  // else lastpos(N) = lastpos(c2)
  if (right->get_nullable()) {
    parent->add_lastpos(left->get_lastpos());
    parent->add_lastpos(right->get_lastpos());
  } else {
    parent->add_lastpos(right->get_lastpos());
  }

  // for follow pos
  // for cat node(N)=c1c2: follow_pos(last_pos(c1)) = first_pos(c2)
  set<Tree*>::iterator iter;
  const set<Tree*>& last_pos = left->get_lastpos();
  for (iter = last_pos.begin(); iter != last_pos.end(); ++iter) {
    (*iter)->add_followpos(right->get_firstpos());
  }
}

Tree* Regex::ProcessChar(int c, Stream *stream, stack<Tree*> *nodes) {
  Tree *right = NewCharNode(c);

  if (stream->Next() == '*') {
    stream->Read();
    nodes->push(right);
    right = ProcessStar(c, stream, nodes);
    nodes->pop();
  }

  if (nodes->size() > 0) {
    Tree *left = nodes->top();nodes->pop();
    Tree *parent = new Tree(CAT);
    AddTree(parent);
    parent->set_left(left);
    parent->set_right(right);
    nodes->push(parent);
    ProcessCatPos(parent, left, right);
    return parent;
  } else {
    nodes->push(right);
    return right;
  }
}

Tree* Regex::ProcessAlter(int c, Stream *stream, stack<Tree*> *nodes) {
  int next;
  Tree *tree;

  next = stream->Read();
  if (isalpha(next) || next == '(') {
    // 取出节点栈顶的节点
    if (nodes->size() < 1) {
      return NULL;
    }
    Tree *left  = nodes->top(); nodes->pop();
    Tree *right = NULL;
    if (next == '(') {
      right = ProcessGroup(next, stream, nodes);
    } else {
      right = NewCharNode(next);
      if (stream->Next() == '*') {
        stream->Read();
        nodes->push(right);
        right = ProcessStar(c, stream, nodes);
        nodes->pop();
      }
    }
    Tree *parent = new Tree(ALTER);
    AddTree(parent);
    parent->set_left(left);
    parent->set_right(right);
    // for nullable
    if (left->get_nullable() || right->get_nullable()) {
      parent->set_nullable(true);
    }
    // for first pos
    parent->add_firstpos(left->get_firstpos());
    parent->add_firstpos(right->get_firstpos());
    // for last pos
    parent->add_lastpos(left->get_lastpos());
    parent->add_lastpos(right->get_lastpos());

    nodes->push(parent);
    return parent;
  }

  cout << "ProcessAlter error\n";
  return NULL;
}

Tree* Regex::ProcessGroup(int c, Stream *stream, stack<Tree*> *nodes) {
  Tree *tree;
  stack<Tree*> new_nodes;

  while ((c = stream->Read()) != '\0') {
    if (isalpha(c)) {
      tree = ProcessChar(c, stream, &new_nodes);
    } else {
      if (c == '(') {
        tree = ProcessGroup(c, stream, &new_nodes);
      } else if (c == ')') {
        break;
      } else if (c == '|') {
        tree = ProcessAlter(c, stream, &new_nodes);
      } else if (c == '*') {
        //tree = ProcessStar(c, stream, &new_nodes);
        cout << "ProcessGroup error\n";
        return NULL;
      }
    }
  }

  if (c != ')') {
    cout << "ProcessGroup error\n";
    return NULL;
  }
  //assert(new_nodes.size() == 1);

  if (stream->Next() == '*') {
    stream->Read();
    nodes->push(tree);
    tree = ProcessStar(c, stream, nodes);
  } else {
    nodes->push(tree);
  }
  return tree;
}

Tree* Regex::ProcessStar(int c, Stream *stream, stack<Tree*> *nodes) {
  if (nodes->size() < 1) {
    cout << "ProcessStar error\n";
    return NULL;
  }
  Tree *old_node = nodes->top(); nodes->pop();
  Tree *parent = new Tree(START);
  AddTree(parent);
  parent->set_left(old_node);
  // for nullable
  parent->set_nullable(true);
  // for first pos
  parent->add_firstpos(old_node->get_firstpos());
  // for last pos
  parent->add_lastpos(old_node->get_lastpos());
  // for follow pos
  // for star node N: follow_pos(last_pos(N)) = first_pos(N)
  const set<Tree*>& last_pos = parent->get_lastpos();
  set<Tree*>::iterator iter;
  for (iter = last_pos.begin(); iter != last_pos.end(); ++iter) {
    (*iter)->add_followpos(parent->get_firstpos());
  }
  nodes->push(parent);

  return parent;
}

Tree* Regex::ConstructTree(const char *str) {
  int c;
  Stream stream(str);
  stack<Tree*> nodes;
  Tree *tree;

  while ((c = stream.Read()) != '\0') {
    if (isalpha(c)) {
      tree = ProcessChar(c, &stream, &nodes);
    } else {
      if (c == '(') {
        tree = ProcessGroup(c, &stream, &nodes);
      } else if (c == ')') {
        cout << "error\n";
        return NULL;
      } else if (c == '|') {
        tree = ProcessAlter(c, &stream, &nodes);
      } else if (c == '*') {
        //tree = ProcessStar(c, &stream, &nodes);
        cout << "ConstructTree error\n";
        return NULL;
      }
    }
  }

  // CAT the last node and END node together as the root
  Tree *right = new Tree(END);
  set<Tree*> pos;
  pos.insert(right);
  right->add_firstpos(pos);
  right->add_lastpos(pos);

  Tree *parent = new Tree(CAT);
  AddTree(parent);
  parent->set_left(tree);
  AddTree(right);
  parent->set_right(right);

  ProcessCatPos(parent, tree, right);

  /*
  cout << "size: " << nodes.size() << endl;
  cout << "type: " << nodes.top()->get_type() << endl;
  //", type: " << nodes[1]->get_type() << endl;
  nodes.pop();
  cout << "type: " << nodes.top()->get_type() << endl;
  */
  //assert(nodes.size() == 1);

  return parent;
}

bool Regex::Compile(const char *str) {
  cout << "complie " << str << " into regex\n";
  root_ = ConstructTree(str);
  if (root_ == NULL) {
    return false;
  }
  return ConstructDFA();
}

void Regex::DoPrintNodePos(Tree *node) {
  node->PrintPos();
}

void Regex::DoPrintTreePos(Tree *root) {
  if (root == NULL) {
    return;
  }
  DoPrintNodePos(root);
  DoPrintTreePos(root->get_left());
  DoPrintTreePos(root->get_right());
}

void Regex::PrintTree() {
  DoPrintTreePos(root_);
}

bool Regex::ConstructDFA() {
  list<State *> unmarked_states;
  map<set<Tree*>, State*> existed_states;
  State *state;

  state = new State(root_->get_firstpos());
  existed_states[root_->get_firstpos()] = state;
  unmarked_states.push_back(state);
  while (!unmarked_states.empty()) {
    state = unmarked_states.front(); unmarked_states.pop_front();
    state_map_[state->get_index()] = state;
    const set<Tree*> &tree_set = state->get_tree_set();

    map<int, bool>::iterator iter;
    for (iter = chars_map_.begin(); iter != chars_map_.end(); ++iter) {
      int c = iter->first;
      set<Tree*> followset;
      set<Tree*>::const_iterator tree_iter;
      for (tree_iter = tree_set.begin();
           tree_iter != tree_set.end(); ++tree_iter) {
        Tree *tree = *tree_iter;
        if (tree->get_type() == NORMAL && tree->get_char() == c) {
          followset.insert((*tree_iter)->get_follow_pos().begin(),
                           (*tree_iter)->get_follow_pos().end());
        }
      }
      State *follow_state = NULL;
      /*
      // if this state already exist??
      map<int, State*>::iterator state_iter;
      for (state_iter = state_map_.begin();
           state_iter != state_map_.end(); ++state_iter) {
        if (state_iter->second->get_tree_set() == followset) {
          follow_state = state_iter->second;
          break;
        }
      }
      */
      // if this state already exist??
      map<set<Tree*>, State*>::iterator iter = existed_states.find(followset);
      if (iter == existed_states.end()) {
        follow_state = new State(followset);
        unmarked_states.push_back(follow_state);
        existed_states[followset] = follow_state;
      } else {
        follow_state = iter->second;
      }
      state->AddTransferState(c, follow_state);
    }
  }

  return true;
}

bool Regex::Match(const char *str) {
  State *state;

  cout << "Try match: " << str << ", result: ";
  state = state_map_[1];
  if (state == NULL) {
    return NULL;
  }
  while (str && *str) {
    state = state->get_transter_state(*str);
    if (state == NULL) {
      cout << "unmatch: " << *str << endl;
      return false;
    }
    ++str;
  }
  if (state != NULL && state->get_acceptable()) {
    return true;
  }
  return false;
}
