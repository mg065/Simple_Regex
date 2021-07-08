#include <list>
#include <assert.h>
#include "regex.h"
#include "tree.h"
#include "state.h"
#include "stream.h"

#define SENTRY 256

Regex::Regex()
  : root_(NULL),
    last_char_(-1) {
  op_map_['*'] = new opHandler(3, &Regex::ProcessStar);
  op_map_['+'] = new opHandler(2, &Regex::ProcessCat);
  op_map_['|'] = new opHandler(1, &Regex::ProcessAlter);
  op_map_[SENTRY] = new opHandler(0, &Regex::ProcessSentry);
  Tree::Init();
  State::Init();
}

Regex::~Regex() {
}

void Regex::AddTree(Tree *tree) {
  tree_map_[tree->get_index()] = tree;
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

void Regex::PushOperator(int opc, Stream *stream, stack<int> *operater,
                          stack<Tree*> *nodes) {
  while (operater->size() > 0) {
    opHandler *old_op = op_map_[operater->top()];
    opHandler *op = op_map_[opc];
    if (old_op->priority_ > op->priority_) {
      Handler handler = old_op->handler_;
      Tree* tree = (this->*handler)(opc, stream, operater, nodes);
    } else {
      break;
    }
  }

  operater->push(opc);
}

Tree* Regex::ProcessChar(int c, Stream *stream,
                         stack<int> *operater,
                         stack<Tree*> *nodes) {
  Tree *right = NewCharNode(c);
  if (last_char_ == '|') {
    goto out;
  }

  if (nodes->empty()) {
    goto out;
  }

  PushOperator('+', stream, operater, nodes);

out:
  nodes->push(right);
  return right;
}

Tree* Regex::ProcessStar(int c, Stream *stream,
                         stack<int> *operater,
                         stack<Tree*> *nodes) {
  if (nodes->size() < 1) {
    cout << "ProcessStar error\n";
    return NULL;
  }
  assert(operater->top() == '*');
  operater->pop();

  Tree *old_node = nodes->top(); nodes->pop();
  Tree *parent = new Tree(STAR);
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

Tree* Regex::ProcessCat(int c, Stream *stream,
                        stack<int> *operater,
                        stack<Tree*> *nodes) {
  if (nodes->size() < 2) {
    cout << "ProcessCat error\n";
    return NULL;
  }
  assert(operater->top() == '+');
  operater->pop();
  Tree *right = nodes->top(); nodes->pop();
  Tree *left = nodes->top(); nodes->pop();
  Tree *parent = new Tree(CAT);
  AddTree(parent);
  parent->set_left(left);
  parent->set_right(right);
  nodes->push(parent);
  ProcessCatPos(parent, left, right);
  return parent;
}

Tree* Regex::ProcessAlter(int c, Stream *stream,
                          stack<int> *operater,
                          stack<Tree*> *nodes) {
  if (nodes->size() < 2) {
    cout << "ProcessAlter error\n";
    return NULL;
  }
  assert(operater->top() == '|');
  operater->pop();
  Tree *right = nodes->top(); nodes->pop();
  Tree *left  = nodes->top(); nodes->pop();
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

bool  Regex::isOperator(int c) {
  return (c == '*' || c == '|');
}

Tree* Regex::ProcessSentry(int c, Stream *stream, stack<int> *operater,
                           stack<Tree*> *nodes) {
  assert(c == SENTRY);
  operater->pop();
  // CAT the last node and END node together as the root
  Tree *left  = nodes->top(); nodes->pop();
  Tree *right = new Tree(END);
  set<Tree*> pos;
  pos.insert(right);
  right->add_firstpos(pos);
  right->add_lastpos(pos);

  Tree *parent = new Tree(CAT);
  AddTree(parent);
  parent->set_left(left);
  AddTree(right);
  parent->set_right(right);

  ProcessCatPos(parent, left, right);

  return parent;
}

Tree* Regex::ConstructTree(const char *str) {
  stack<int> operater;
  stack<Tree*> nodes;
  Tree *tree;
  Stream stream(str);
  int c;

  // first push SENTRY operator as operator stack
  operater.push(SENTRY);

  do {
    c = stream.Read();
    if (isalnum(c)) {
      tree = ProcessChar(c, &stream, &operater, &nodes);
      last_char_ = c;
    } else if (c == '(') {
      //operater.push(c);
      //tree = ProcessGroup(c, &stream, &operater, &nodes);
    } else if (isOperator(c)) {
      PushOperator(c, &stream, &operater, &nodes);
      last_char_ = c;
    } else if (c != '\0') {
      cout << "ConstructTree error: " << char(c) << endl;
      return NULL;
    }
  } while(c != '\0' && tree != NULL);

  if (tree == NULL) {
    return NULL;
  }
  if (c != '\0') {
    return NULL;
  }

  while (!operater.empty()) {
    c = operater.top();
    opHandler *op = op_map_[c];
    Handler handler = op->handler_;
    tree = (this->*handler)(c, &stream, &operater, &nodes);
  }

  return tree;

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
