#ifndef Sample_H	/* -*- c++ -*- */
#define Sample_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "global.h"

class Sample {
  // Class members
 public:
  static int const rate = 44100;
  static int const unit = 1;
  static int const max = 1;
  static int const min = -1;
  static int const zero = 0;

  // Instance members
 private:
  typedef double value_t;

  value_t value;

 public:
  Sample(): value(0) {}
  Sample(value_t val): value(val) {}

  operator value_t() const {
    return value;
  }

  Sample operator +(Sample const &other) const {
    return Sample(value + other.value);
  }

  Sample operator +(value_t &other) const {
    return Sample(value + other);
  }

  Sample operator *(Sample const &other) const {
    return Sample(value * other.value);
  }

  Sample operator *(value_t &other) const {
    return Sample(value * other);
  }

  Sample clip() const {
    return Sample(value > max ? max : (value < min ? min : value));
  }
};

class SampleBuf {
private:
  Sample *buffer;
  int length;

public:
  SampleBuf(int _length = 0) {
    length = _length;
    buffer = (Sample *) malloc(sizeof(Sample) * length);
    clear();
  }

  SampleBuf(SampleBuf const &from) {
    buffer = (Sample *) malloc(0);
    length = 0;
    assign(from);
  }

  SampleBuf const &operator =(SampleBuf const &from) { 
    assign(from);
    return *this;
  }

  ~SampleBuf() {
    free(buffer);
  }

  void assign(SampleBuf const &from) {
    if (length != from.length)
      resize(from.length);
    memcpy(buffer, from.buffer, length);
  }

  void wind(int offset) {
    // Gross. <shrug>
    buffer += offset;
    length -= offset;
  }

  int getLength() const { return length; }

  Sample &operator[](int index) { return buffer[index]; }
  Sample const &operator[](int index) const { return buffer[index]; }

  void operator +=(SampleBuf const &other) {
    assert(length == other.length);
    for (int i = 0; i < length; i++)
      buffer[i] = buffer[i] + other.buffer[i];
  }

  void operator +=(Sample const &value) {
    for (int i = 0; i < length; i++)
      buffer[i] = buffer[i] + value;
  }

  void clear() {
    memset(buffer, 0, sizeof(Sample) * length);
  }

  void fill_with(Sample value) {
    for (int i = 0; i < length; i++)
      buffer[i] = value;
  }

  void resize(int newlen) {
    Sample *n = (Sample *) malloc(sizeof(Sample) * newlen);

    if (newlen >= length) {
      memcpy(n, buffer, sizeof(Sample) * length);
      memset(n + length, 0, newlen - length);
    } else {
      memcpy(n, buffer, sizeof(Sample) * newlen);
    }

    free(buffer);
    buffer = n;
    length = newlen;
  }

  void delete_front(int howmany) {
    assert(howmany <= length);

    Sample *n = (Sample *) malloc(sizeof(Sample) * (length - howmany));
    memcpy(n, buffer + howmany, sizeof(Sample) * (length - howmany));
    free(buffer);
    buffer = n;
    length -= howmany;
  }
};

typedef gint32 sampletime_t;

#endif
