#include <assert.h>
#include <iostream>
#include <fstream>
#include "opossum.h"

//using namespace Opossum;

int main(int argc, char *argv[]) {
  Opossum::Object::initialise();
  char buf[2048];

  assert(argc > 1);
  ifstream i(argv[1]);

  while (!i.eof()) {
    try {
      Opossum::Object *x;
      if (i >> x) {
	cout << "Read in: " << x->getKind() << " = " << *x << endl;
	delete x;
      } else {
	cout << "Read failed." << endl;
	i.clear();
	if (i.getline(buf, 2048)) {
	  cout << "Offending line: " << buf << endl;
	}
      }
    } catch (Opossum::IllformedExpressionException e) {
      cout << "Ill formed expression." << endl;
    }
  }

  cout << "... EOF reached." << endl;
  return 0;
}
