#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "opossum.h"

namespace Opossum {

Object::loadervec_t Object::loaders;

void Object::addLoader(loader_t loader) {
  loaders.push_back(loader);
}

void Object::removeLoader(loader_t loader) {
  for (loadervec_t::iterator i = Object::loaders.begin();
       i != Object::loaders.end();
       i++) {
    if ((*i) == loader)
      loaders.erase(i);
  }
}

istream &operator>>(istream &stream, Object *&dest) {
  for (Object::loadervec_t::iterator i = Object::loaders.begin();
       i != Object::loaders.end();
       i++) {
    Object::loader_t loader = (*i);
    streampos pos = stream.tellg();

    stream.clear();
    if (loader(stream, dest))
      return stream;

    stream.seekg(pos);
  }

  stream.set(ios::badbit);
  return stream;
}

istream &operator>>(istream &i, Object &dest) {
  dest.readFrom(i);
  return i;
}

ostream &operator<<(ostream &o, Object &src) {
  src.writeTo(o);
  return o;
}

///////////////////////////////////////////////////////////////////////////

void Integer::readFrom(istream &stream) {
  if (!(stream >> value))
    throw IllformedExpressionException();
}

void Integer::writeTo(ostream &stream) {
  stream << value;
}

void Float::readFrom(istream &stream) {
  if (!(stream >> value))
    throw IllformedExpressionException();
}

void Float::writeTo(ostream &stream) {
  stream << value;
}

void Boolean::readFrom(istream &stream) {
  char keyword_buf[6];
  string keyword;

  if (!(stream.scan("%5[a-z]", keyword_buf)))
    throw IllformedExpressionException();

  keyword = keyword_buf;

  if (keyword == "true")	{ value = true; return; }
  if (keyword == "on")		{ value = true; return; }

  if (keyword == "false")	{ value = false; return; }
  if (keyword == "off")		{ value = false; return; }

  throw IllformedExpressionException();
}

void Boolean::writeTo(ostream &stream) {
  stream << (value ? "true" : "false");
}

void Character::readFrom(istream &stream) {
  char ch;
  stream >> ch; if (ch != '\'') throw IllformedExpressionException();
  stream >> value;
  stream >> ch; if (ch != '\'') throw IllformedExpressionException();
}

void Character::writeTo(ostream &stream) {
  stream << "'" << value << "'";
}

void String::readFrom(istream &stream) {
  char ch;

  value.erase();

  stream >> ch;
  // Don't turn off whitespace-skipping until *after* we read the first char.
  ios::fmtflags flags = stream.unsetf(ios::skipws);

  if (ch != '"') {
    stream.putback(ch);

    while (1) {
      if (!(stream >> ch))
	break;

      if (isspace(ch) && ch != '\n')
	continue;

      if (!isalnum(ch)) {
	stream.putback(ch);
	break;
      }

      value.append(1, ch);
    }

    stream.flags(flags);

    if (value.length() == 0)
      throw IllformedExpressionException();

    return;
  }

  while (stream >> ch) {
    switch (ch) {
    case '"':
      stream.flags(flags);
      return;

    case '\\':
      stream >> ch;
      break;

    default:
      break;
    }

    value.append(1, ch);
  }

  stream.flags(flags);
  throw IllformedExpressionException();
}

void String::writeTo(ostream &stream) {
  stream << '"' << value << '"';
}

void List::readFrom(istream &stream) {
  char ch;
  stream >> ch; if (ch != '[') throw IllformedExpressionException();

  clear();
  while (1) {
    Object *o;

    if (!(stream >> o)) {
      stream.clear();

      stream >> ch;
      if (ch == ']')
	return;

      throw IllformedExpressionException();
    }

    value.push_back(o);

    stream >> ch;
    switch (ch) {
    case ',':
      continue;

    case ']':
      return;

    default:
      clear();
      throw IllformedExpressionException();
    }
  }
}

void List::writeTo(ostream &stream) {
  stream << '[';
  for (int i = 0; i < ((int) value.size()) - 1; i++)
    stream << *value[i] << ", ";
  if (value.size() > 0)
    stream << *value[value.size() - 1];
  stream << ']';
}

void Map::readFrom(istream &stream) {
  char ch;
  stream >> ch; if (ch != '{') throw IllformedExpressionException();

  clear();
  while (1) {
    string key;
    Object *val;

    stream >> ch;
    if (ch == '}')
      return;
    stream.putback(ch);

    if (!getline(stream, key, '='))
      throw IllformedExpressionException("Bad key format");

    if (key.size() <= 0)
      throw IllformedExpressionException("Key too short");

    while (isspace(key[key.size() - 1]))
      key.erase(key.size() - 1);

    if (!(stream >> val))
      throw IllformedExpressionException("Bad value format");

    value[key] = val;

    stream >> ch;
    switch (ch) {
    case ',':
      continue;

    case '}':
      break;

    default:
      clear();
      throw IllformedExpressionException("Bad separator");
    }

    break;
  }
}

void Map::writeTo(ostream &stream) {
  stream << '{';

  MapValue_t::iterator i = value.begin();

  if (i != value.end()) {
    stream << (*i).first << '=' << *(*i).second;
    i++;
  }

  while (i != value.end()) {
    stream << ", " << (*i).first << '=' << *(*i).second;
    i++;
  }

  stream << '}';
}

///////////////////////////////////////////////////////////////////////////

template <class T> bool loader_function(istream &stream, Object *&dest) {
  Object *result = new T();
  try {
    result->readFrom(stream);
    dest = result;
    return true;
  } catch (IllformedExpressionException e) {
    if (e.hasReason())
      cerr << "IllformedExpressionException(" << e.getReason() << ")" << endl;
    delete result;
    return false;
  }
}

void Object::initialise() {
  Object::addLoader(loader_function<List>);
  Object::addLoader(loader_function<Map>);
  Object::addLoader(loader_function<Float>);
  Object::addLoader(loader_function<Integer>);
  Object::addLoader(loader_function<Character>);
  Object::addLoader(loader_function<Boolean>);
  Object::addLoader(loader_function<String>);
}

}	// namespace

