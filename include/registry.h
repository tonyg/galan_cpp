#ifndef Registry_H	/* -*- c++ -*- */
#define Registry_H

///////////////////////////////////////////////////////////////////////////
// DESCRIPTION
// Interface

#include <map>
#include <memory>
#include <string>

#include "global.h"

class Registrable;	// a cell in the tree - may be a leaf or a node
class Registry;		// a node in the tree (has child cells)

//////////////////////////////////////////////////////////////////////////////

/**
 * A cell in a registry-tree - may be either a leaf or a node.
 **/
class Registrable {
private:
  friend class Registry;

  Registry *parent;		///< Parent node
  std::string localname;	///< Name of this child

public:
  Registrable(): parent(0), localname() {}
  virtual ~Registrable() { unbind(); }

  /// Retrieve our parent node.
  Registry *getParent() const { return parent; }

  /// Query this Registrable for its own name.
  std::string const &getLocalname() const { return localname; }

  /// Returns true if this node has a parent.
  bool isBound() const { return parent != 0; }

  /// Retrieve the complete path from the root node to this node.
  std::string getFullpath() const;

  /// Unbind this node from its parent.
  void unbind();
};

//////////////////////////////////////////////////////////////////////////////

/**
 * Implements a non-leaf node in a Registry tree.
 **/
class Registry: public Registrable {
public:
  typedef std::map<std::string, Registrable *> children_t;
  typedef pair<Registry *, std::string> leaf_t;

public:
  typedef children_t::iterator iterator;

  /**
   * Deep iterator over a registry.
   **/
  class RegistryIterator {
  private:
    friend class Registry;

    Registry::iterator position;	///< Current position at this level
    Registry::iterator end;		///< Final position at this level
    RegistryIterator *nest;		///< Iterator at some sublevel
    Registry *current;			///< Registry for this level

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

  static class Registry *root;		///< *THE* root Registry.

  Registry(): Registrable() {}
  ~Registry() {}

  /// Get a shallow iterator over this.
  iterator begin() { return children.begin(); }
  /// Get a deep iterator over this.
  RegistryIterator deep_begin() { return RegistryIterator(children.begin(), children.end()); }
  /// Get the end (shallow and/or deep) iterator for this.
  iterator end() { return children.end(); }

  /**
   * Binds a subpath to a Registrable, replacing an existing entry if
   * 'override' is nonfalse.
   *
   * @param path the subpath to bind
   * @param what the object to bind
   * @param override if true, replaces existing entries, otherwise refuses to bind
   * @return true if the bind succeeded; false otherwise
   **/
  bool bind(string const &path, Registrable *what, bool override = false);

  /**
   * Unbinds a subpath from a Registrable.
   *
   * @param path the subpath to unbind
   * @return true if specified entry existed and was unbound; false otherwise
   **/
  bool unbind(string const &path);

  /**
   * Unbind a child from this Registry. If the child's parent object
   * is not this, refuse, and return false.
   *
   * @param what the object to unbind
   * @return true if what was a direct child of this and was unbound; false otherwise
   **/
  bool unbind(Registrable *what);

  /**
   * Retrieve a bound object.
   * @path the path to retrieve
   * @return the object, if found; otherwise 0
   **/
  Registrable *lookup(string const &path);

  /// Synonym for Registry::lookup().
  Registrable *operator[](string const &path) { return lookup(path); }

private:
  children_t children;		///< All children of this registry.

  Registry(Registry const &from);
  Registry &operator =(Registry const &from);

  /**
   * Internal: searches for the leaf node at path, creating missing
   * intermediate nodes if create_missing_nodes is true. If any of the
   * intermediate nodes are not Registry instances, returns
   * leaf_t(0, "").
   *
   * @param path the path to find
   * @param create_missing_nodes true to create intermediaries
   * @return pair with first==node's parent and second==node's leafname
   **/
  leaf_t find_leaf(string const &path, bool create_missing_nodes = false);
};

//////////////////////////////////////////////////////////////////////////////

typedef class Registry::RegistryIterator RegistryIterator;

//////////////////////////////////////////////////////////////////////////////

inline void Registrable::unbind() {
  if (parent) parent->unbind(this);
}

#endif
