#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <hash_map>
#include <string>

#include "galan/registry.h"

GALAN_USE_NAMESPACE

class A: public Registrable {
public:
  int x;

  A(int q) {
    cout << "Created A " << q << endl;
    x = q;
  }

  ~A() {
    cout << "Destroyed A " << x << endl;
  }

  void doit() {
    static A kapow(3344);
    cout << "We're in doit(" << x << ") now." << endl;
    cout << "Kapow! " << kapow.x << endl;
  }
};

int main(int argc, char *argv[]) {
  Registry r;
  RegistryIterator i;

  r.bind("Hello/World", new A(1));
  r.bind("Hello/There", new A(2));
  r.bind("Foo/Bar/Baz", new A(999));
  delete r["Foo/Bar/Baz"];
  r.bind("Foo/Bar/Baz", new A(1010));

  dynamic_cast<A *>(r["/Hello/World"])->doit();

  for (i = r.deep_begin(); i != r.end(); i++) {
    A *x = dynamic_cast<A *>(*i);

    cout << x->getFullpath() << "... ";
    x->doit();
  }
}
