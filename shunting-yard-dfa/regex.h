#ifndef __REGEX_H__
#define __REGEX_H__

#include <stack>
#include <map>

using namespace std;

class Tree;
class Stream;
class State;

class Regex {
public:
  Regex();
  ~Regex();

  bool Compile(const char *str);
  bool Match(const char * str);

private:
  Tree* ConstructTree(const char *str);
  Tree* ProcessSentry(int c, Stream *stream, stack<int> *operater,
                    stack<Tree*> *nodes);
  Tree* ProcessChar(int c, Stream *stream, stack<int> *operater,
                    stack<Tree*> *nodes);
  Tree* ProcessGroup(int c, Stream *stream, stack<int> *operater,
                     stack<Tree*> *nodes);
  Tree* ProcessAlter(int c, Stream *stream, stack<int> *operater,
                     stack<Tree*> *nodes);
  Tree* ProcessStar(int c, Stream *stream, stack<int> *operater,
                    stack<Tree*> *nodes);
  Tree* ProcessCat(int c, Stream *stream, stack<int> *operater,
                   stack<Tree*> *nodes);

  void  PushOperator(int c, Stream *stream, stack<int> *operater,
                     stack<Tree*> *nodes);

  bool ConstructDFA();
  void  ProcessCatPos(Tree *parent, Tree *left, Tree *right);
  Tree* NewCharNode(int c);
  void AddTree(Tree *tree);
  bool  isOperator(int c);

  typedef Tree* (Regex::*Handler)(int, Stream*, stack<int> *, stack<Tree*>*);
  struct opHandler {
    int      priority_;
    Handler  handler_;

    opHandler(int priority, Handler handler)
      : priority_(priority),
        handler_(handler) {
    }
  };

private:
  Tree* root_;
  int   last_char_;
  map<int, opHandler*> op_map_;
  map<int, bool> chars_map_;
  map<int, Tree*> tree_map_;
  map<int, State*> state_map_;
};

#endif  // __REGEX_H__
