#ifndef __TREE_H__
#define __TREE_H__

#include <iostream>
#include <set>

using namespace std;

enum TreeType {
  NORMAL = 0,
  CAT,
  START,
  ALTER,
  END,
};

class Tree {
public:
  Tree(TreeType type, int c = -1)
    : type_(type),
    c_(c),
    left_(NULL),
    right_(NULL),
    parent_(NULL),
    nullable_(false) {
    index_ = gIndex;
    ++gIndex;
    //cout << index_ << " tree type: " << type << ", c: " << (char)c << "\n";
  }

  static void Init();
  void PrintPos();

  TreeType get_type() const { return type_; }
  int      get_char() const { return c_; }

  int      get_index() const { return index_; }

  bool get_nullable() const { return nullable_; }
  void set_nullable(bool nullable) { nullable_ = nullable; }

  const set<Tree*>& get_firstpos() const { return first_pos_;}
  void add_firstpos(const set<Tree*>& first_pos) {
    first_pos_.insert(first_pos.begin(), first_pos.end());
  }

  const set<Tree*>& get_lastpos() const { return last_pos_;}
  void add_lastpos(const set<Tree*>& last_pos) {
    last_pos_.insert(last_pos.begin(), last_pos.end());
  }

  void add_followpos(const set<Tree*>& follow_pos) {
    follow_pos_.insert(follow_pos.begin(), follow_pos.end());
  }
  const set<Tree*>& get_follow_pos() const { return follow_pos_;}

  void set_left(Tree* left) {
    left_ = left;
    left_->set_parent(this);
  }

  void set_right(Tree *right) {
    right_ = right;
    left_->set_parent(this);
  }

  void set_parent(Tree *parent) {
    parent_ = parent;
  }

  Tree* get_left() {
    return left_;
  }

  Tree* get_right() {
    return right_;
  }

private:
  void PrintPos(const set<Tree*> &pos, const char *tag);

private:
  static int gIndex;

  int      index_;
  TreeType type_;
  int      c_;
  Tree    *left_, *right_;
  Tree    *parent_;

  bool       nullable_;
  set<Tree*> first_pos_;
  set<Tree*> last_pos_;
  set<Tree*> follow_pos_;
};

#endif  // __TREE_H__
