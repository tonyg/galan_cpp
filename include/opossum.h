/* -*- c++ -*- */
#ifndef Opossum_H
#define Opossum_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace Opossum {

class Object;
class Integer;
class Float;
class Character;
class Boolean;
class String;
class List;
class Map;

///////////////////////////////////////////////////////////////////////////
// Object

class IllformedExpressionException {
private:
  bool hasreason;
  string reason;

public:
  IllformedExpressionException()
    : hasreason(false),
      reason("Syntax Error")
  {}

  IllformedExpressionException(string const &_reason)
    : hasreason(true),
      reason(_reason)
  {}

  string const &getReason() const { return reason; }
  bool hasReason() const { return hasreason; }
};

class Object {
private:
  friend istream &operator>>(istream &i, Object *&dest);
  friend istream &operator>>(istream &i, Object &dest);
  friend ostream &operator<<(ostream &o, Object &src);

protected:
  typedef bool (*loader_t)(istream &i, Object *&dest);
  typedef vector<loader_t> loadervec_t;

  static loadervec_t loaders;

  static void addLoader(loader_t loader);
  static void removeLoader(loader_t loader);

public:
  static void initialise();

  Object() {}
  Object(Object &other) {}
  virtual ~Object() {}

  Object &operator=(Object const &other) { assign(other); return (*this); }

  virtual void assign(Object const &other) = 0;
  virtual string const &getKind() const = 0;

  virtual void readFrom(istream &stream) = 0;
  virtual void writeTo(ostream &stream) = 0;
};

extern istream &operator>>(istream &i, Object *&dest);
extern istream &operator>>(istream &i, Object &dest);
extern ostream &operator<<(ostream &o, Object &src);

///////////////////////////////////////////////////////////////////////////
// Others

template <class value_type> class GenericObject: public Object {
 protected:
  value_type value;

 public:
  typedef value_type value_t;

  GenericObject(): value() {}
  GenericObject(GenericObject<value_type> const &other) { assign(other); }
  GenericObject(value_type _v): value(_v) {}

  virtual void assign(Object const &other) {
    GenericObject<value_type> const &o(dynamic_cast<GenericObject<value_type> const &>(other));
    value = o.value;
  }

  virtual value_type const &getValue() const { return value; }
  virtual void setValue(value_type const &_v) { value = _v; }
};

#define DECLARE_OPOSSUM_GENERIC_CLASS_HEADER(CLS, TYPE)	\
class CLS: public GenericObject<TYPE>

#define DECLARE_OPOSSUM_GENERIC_CLASS_BODY(CLS, TYPE)						\
public:												\
  virtual string const &getKind() const { static string kind = "Opossum::" #CLS; return kind; }	\
  virtual void readFrom(istream &stream);							\
  virtual void writeTo(ostream &stream)

#define DECLARE_OPOSSUM_GENERIC_CLASS(CLS, TYPE)	\
DECLARE_OPOSSUM_GENERIC_CLASS_HEADER(CLS, TYPE) {	\
DECLARE_OPOSSUM_GENERIC_CLASS_BODY(CLS, TYPE);		\
}

typedef map<string, Object *> MapValue_t;

DECLARE_OPOSSUM_GENERIC_CLASS(Integer, int);
DECLARE_OPOSSUM_GENERIC_CLASS(Float, double);
DECLARE_OPOSSUM_GENERIC_CLASS(Character, char);
DECLARE_OPOSSUM_GENERIC_CLASS(String, string);
DECLARE_OPOSSUM_GENERIC_CLASS(Boolean, bool);

DECLARE_OPOSSUM_GENERIC_CLASS_HEADER(List, vector<Object *>) {
  DECLARE_OPOSSUM_GENERIC_CLASS_BODY(List, vector<Object *>);
  virtual ~List() {
    clear();
  }

  Object *&operator[](int index) { return value[index]; }
  Object * const &operator[](int index) const { return value[index]; }
  void push_back(Object *x) { value.push_back(x); }
  int size() const { return value.size(); }
  void erase(int index) { delete value[index]; value[index] = NULL; value.erase(&value[index]); }
  void clear() {
    for (int i = 0; i < value.size(); i++)
      delete value[i];
    value.clear();
  }
};

DECLARE_OPOSSUM_GENERIC_CLASS_HEADER(Map, MapValue_t) {
  DECLARE_OPOSSUM_GENERIC_CLASS_BODY(Map, MapValue_t);
  virtual ~Map() {
    clear();
  }

  Object *&operator[](string const &index) { return value[index]; }
  MapValue_t::iterator find(string const &index) { return value.find(index); }
  void erase(MapValue_t::iterator index) { delete (*index).second; value.erase(index); }
  void clear() {
    for (MapValue_t::iterator i = value.begin(); i != value.end(); i++)
      delete (*i).second;
    value.clear();
  }
};

extern Map *readIniFile(istream &stream);

}	// namespace

#endif
