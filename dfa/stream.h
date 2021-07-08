#ifndef __STREAM_H__
#define __STREAM_H__

class Stream {
 public:
  Stream(const char *str);
  ~Stream();

  int  Read();
  int  Next();
  void Back();

 private:
  const char *str_;
  int         pos_;
};

#endif /* __STREAM_H__ */
