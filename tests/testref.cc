#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <hash_map>
#include <string>

#include "ref.h"

struct hash<string> {
  size_t operator()(const string &s) const { return hash<const char *>()(s.c_str()); }
};

class A {
public:
  int x;

  A(int q) {
    cout << "Created A " << q << endl;
    x = q;
  }

  virtual ~A() {
    cout << "Destroyed A " << x << endl;
  }

  void doit() {
    static A kapow(3344);
    cout << "We're in doit(" << x << ") now." << endl;
    cout << "Kapow! " << kapow.x << endl;
  }
};

class B: public A {
public:
  B(int q): A(q) {}

  virtual ~B() {
    cout << "Destroyed B" << endl;
  }
};

typedef hash_map< string, ref<A> > a_t;

static ref<A> foo;

void one() {
  a_t x;
  a_t::iterator i;
  ref< c_auto_ptr < char > > s = new c_auto_ptr<char>(strdup("Hello, world!\n"));

  x["first"] = new A(1);
  x["second"] = new A(2);
  x["third"] = new A(3);
  x["fourth"] = new A(4);

  foo = x["second"];

  for (i = x.begin(); i != x.end(); i++) {
    cout << (*i).first << ":" << endl;
    (*i).second->doit();
  }

  printf("s is %s", s.get()->get());
}

void two() {
  foo->doit();
}

void testDowncast() {
  ref<A> a = new A(321);
  ref<A> b = new B(123);

  try {
    ref<B> x1 = a.downcast<B>();
    x1->doit();
  } catch (InvalidDowncastException) {
    cerr << "Invalid Downcast (a)" << endl;
  }

  try {
    ref<B> x2 = b.downcast<B>();
    x2->doit();
  } catch (InvalidDowncastException) {
    cerr << "Invalid Downcast (b)" << endl;
  }
}

int main(int argc, char *argv[]) {
  ref<int> x = new int(1);
  ref<int> y = x;

  one();
  two();

  testDowncast();

  delete foo.extract();

  cout << *x << endl;
  *y = 2;
  cout << *x << endl;
}
