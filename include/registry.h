#ifndef Registry_H	/* -*- c++ -*- */
#define Registry_H

///////////////////////////////////////////////////////////////////////////
// DESCRIPTION
// Interface

#include <hash_map>
#include <memory>
#include <string>

#include "global.h"

class Registrable;	// a cell in the tree - may be a leaf or a node
class Registry;		// a node in the tree (has child cells)

//////////////////////////////////////////////////////////////////////////////

class Registrable {
private:
  friend class Registry;

  Registry *parent;
  string localname;

public:
  Registrable(): parent(0), localname() {}
  virtual ~Registrable() { unbind(); }

  Registry *getParent() const { return parent; }
  string const &getLocalname() const { return localname; }
  bool isBound() const { return parent != 0; }

  string getFullpath() const;

  void unbind();
};

//////////////////////////////////////////////////////////////////////////////

class Registry: public Registrable {
private:
  struct hash<string> {
    size_t operator()(const string &s) const { return hash<const char *>()(s.c_str()); }
  };

  typedef hash_map<string, Registrable *> children_t;
  typedef pair<Registry *, string> leaf_t;

  children_t children;

  Registry(Registry const &from) {}
  Registry &operator =(Registry const &from) {}

  leaf_t find_leaf(string const &path, bool create_missing_nodes = false);

public:
  typedef children_t::iterator iterator;

  class RegistryIterator {
  private:
    friend class Registry;

    Registry::iterator position;
    Registry::iterator end;
    RegistryIterator *nest;
    Registry *current;

  public:
    RegistryIterator(): nest(0) {}

    RegistryIterator(RegistryIterator const &from): nest(0) { assign(from); }
    RegistryIterator(Registry::iterator const &from,
		     Registry::iterator const &_end): nest(0) { assign(from, _end); }
    RegistryIterator &operator =(RegistryIterator const &from) { assign(from); return (*this); }
    RegistryIterator &operator =(Registry::iterator const &from) {
      assign(from, end);
      return (*this);
    }

    ~RegistryIterator();

    void assign(RegistryIterator const &from);
    void assign(Registry::iterator const &from, Registry::iterator const &_end);

    bool operator==(RegistryIterator const &other);
    bool operator==(Registry::iterator const &other);
    template <class T> bool operator!=(T const &other) { return !((*this) == other); }

    Registrable * &operator*();
    RegistryIterator &operator++(int);	// postfix operator has a dummy int
  };

  static class Registry *root;

  Registry(): Registrable() {}
  ~Registry() {}

  iterator begin() { return children.begin(); }
  RegistryIterator deep_begin() { return RegistryIterator(children.begin(), children.end()); }
  iterator end() { return children.end(); }

  bool bind(string const &path, Registrable *what, bool override = false);
  bool unbind(string const &path);
  bool unbind(Registrable *what);

  Registrable *lookup(string const &path);
  Registrable *operator[](string const &path) { return lookup(path); }
};

//////////////////////////////////////////////////////////////////////////////

typedef class Registry::RegistryIterator RegistryIterator;

//////////////////////////////////////////////////////////////////////////////

inline void Registrable::unbind() {
  if (parent) parent->unbind(this);
}

#endif
