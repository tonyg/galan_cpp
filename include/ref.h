#ifndef Ref_H	/* -*- c++ -*- */
#define Ref_H

// Reference-counting garbage collector.
// Nifty C++ hack?

#define WANT_REF_DEBUG_CODE	0
#define WANT_REF_USING_RTTI	1

#include <iostream.h>
#include <assert.h>

#if WANT_REF_USING_RTTI
#include <typeinfo>
#endif

class InvalidDowncastException {
public:
  InvalidDowncastException() {}
};

template <class T> class c_auto_ptr {
private:
  T *ptr;

public:
  c_auto_ptr(): ptr(0) {}
  c_auto_ptr(T *p): ptr(p) {}

  ~c_auto_ptr() {
#if WANT_REF_DEBUG_CODE
    cerr << "Destroying c_auto_ptr " << this << " (" << (void *) ptr << " = " <<
      ptr << ")" << endl;
#endif
    if (ptr) free(ptr);
  }

  T &operator *() const { assert(ptr != NULL); return *ptr; }
  T *operator ->() const { assert(ptr != NULL); return ptr; }
  T *get() const { return ptr; }
};

struct ref_rep {
  int count;
  void *object;

  ref_rep(void *_o): object(_o), count(1) {}
};

template <class T> class ref {
private:
  ref_rep *rep;

  void become(ref<T> const &from) {
    rep = from.rep;
    if (from.valid())
      (rep->count)++;
#if WANT_REF_DEBUG_CODE
    cerr << "[ref.h] Cloned reference " << (*this) << endl;
#endif
  }

  void become(T *obj) {
    rep = new ref_rep(obj);
#if WANT_REF_DEBUG_CODE
    cerr << "[ref.h] New reference " << (*this) << endl;
#endif
  }

public:
  ref(T *obj) {
    become(obj);
  }

  ref() {
    rep = 0;
  }

  ref(ref<T> const &from) {
    become(from);
  }

  ref(ref_rep *_rep) {
    rep = _rep;
    rep->count++;
  }

  ~ref() {
    release();
  }

  ref const &operator =(ref const &from) {
    if (this != &from) {
      release();
      become(from);
    }
    return (*this);
  }

  void release() {
    if (rep != 0) {
      (rep->count)--;
#if WANT_REF_DEBUG_CODE
      cerr << "Releasing reference " << (*this) << endl;
#endif
      if (rep->count <= 0) {
#if WANT_REF_DEBUG_CODE
	cerr << "Destroying reference " << (*this) << endl;
#endif
	if (rep->object)
	  delete (T *) rep->object;
	delete rep;
      }
      rep = 0;
    }
  }

  bool valid() const {
    return rep != 0;
  }

  T &operator *() const { assert(rep && rep->object); return * (T *) (rep->object); }
  T *operator ->() const { assert(rep && rep->object); return (T *) (rep->object); }
  T *get() const { return (T *) (rep->object); }

  T *extract() {
    assert(rep && rep->object);

    T *result = (T *) rep->object;
    rep->object = 0;
    return result;
  }

  template <class DerivedT> ref<DerivedT> downcast() {
    assert(rep && rep->object);

    if (dynamic_cast<DerivedT *>((T *) rep->object) == 0)
      throw InvalidDowncastException();

    return ref<DerivedT>(rep);
  }

  template <class X> friend inline ostream &operator <<(ostream &o, ref<X> &r);
};

template <class X> inline ostream &operator <<(ostream &o, ref<X> &r) {
  o << "ref(";
#if WANT_REF_USING_RTTI
  o << typeid(r).name() << ":";
#endif
  if (r.rep)
    o << r.rep->object << ";" << r.rep->count;
  else
    o << "norep";
  return o << ")";
}

#endif
