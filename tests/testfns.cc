#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Base {
  int k;
  char const *name;

  typedef void (Base::*fn_t)();

  Base(int _k, char const *_n)
    : k(_k),
      name(_n)
  {}

  virtual int getk() const { return k; }

  void pop() {
    printf("pop: %d %d (%s)\n", k, getk(), name);
  }
};

struct A: public Base {
  char f[41];
  int x;

  A(int _x, char const *_n)
    : x(_x),
      Base(_x+10, _n)
  {}

  virtual int getk() const { return x; }

  virtual void foo() {
    pop();
    printf("foo: %d %d\n", x, getk());
    printf("  %p %p\n", dynamic_cast<Base *>(this), dynamic_cast<A *>(this));
  }
};

struct B: public A {
  char f[23];
  int x;

  B(int _x, char const *_n)
    : x(_x),
      A(_x-2, _n)
  {}

  virtual int getother() const { return x + 99; }
  virtual int getk() const { return x; }

  void bar() {
    pop();
    printf("bar: %d %d\n", x, getk());
  }
};

#define DO(a,b)		(a ->* fn##b)();printf("\n")

void main() {
  Base *x = new Base(3, "x");
  Base *y = new A(4, "y");
  Base *z = new B(7, "z");

  Base::fn_t fnx = &Base::pop;
  Base::fn_t fny = (Base::fn_t) &A::foo;
  Base::fn_t fnz = (Base::fn_t) &B::bar;

  x->pop();
  ((A *)y)->foo();
  ((B *)z)->bar();

  printf("----------------------\n");

  DO(x,x);
  //DO(x,y);
  //DO(x,z);

  DO(y,x);
  DO(y,y);
  //DO(y,z);

  DO(z,x);
  DO(z,y);
  DO(z,z);
}
