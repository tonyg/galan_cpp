#ifndef XMLDict_H	/* -*- c++ -*- */
#define XMLDict_H

///////////////////////////////////////////////////////////////////////////
// DESCRIPTION
// Interface

#include "registry.h"

class XMLValue: public Registrable {
private:
  string value;

  XMLValue() {}

public:
  XMLValue(XMLValue const &from) { assign(from); }
  XMLValue const &operator=(XMLValue const &from) { assign(from); return *this; }

  virtual void assign(XMLValue const &from) {
    value = from.value;
  }

  string getValue() { return value; }
};

class XMLDict: public Registry {
private:
  XMLDict() {}
  XMLDict(XMLDict const &from) {}
  XMLDict const &operator =(XMLDict const &from) {}

public:
};

#endif
