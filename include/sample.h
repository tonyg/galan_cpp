#ifndef Sample_H	/* -*- c++ -*- */
#define Sample_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "global.h"

GALAN_BEGIN_NAMESPACE

/**
 * Represents a single sample in the system.
 **/
class Sample {
 public:
  static int const rate = 44100;	///< Sample rate the system operates at
  static int const unit = 1;		///< The unit sample
  static int const max = 1;		///< Maximum representable sample without clipping
  static int const min = -1;		///< Minimum representable sample without clipping
  static int const zero = 0;		///< The zero sample

 public:
  typedef double value_t;		///< Internal representation of samples

  /// Zero constructor
  Sample(): value(0) {}

  /// Allow implicit conversion from value_t to Sample
  Sample(value_t val): value(val) {}

  /// Allow implicit conversion from Sample to value_t
  operator value_t() const {
    return value;
  }

  /** @name Operators
   * Basic arithmetic operators for Sample instances
   **/
  //@{
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
  //@}

  /// Clips this sample to lie between Sample::min and Sample::max (inclusive).
  Sample clip() const {
    return Sample(value > max ? max : (value < min ? min : value));
  }

 private:
  value_t value;			///< Value of this sample
};

/**
 * Represents a buffer of Samples.
 **/
class SampleBuf {
public:
  /**
   * Construct a SampleBuf of a given length.
   **/
  SampleBuf(int _length = 0) {
    length = _length;
    buffer = (Sample *) malloc(sizeof(Sample) * length);
    clear();
  }

  /**
   * Clone another SampleBuf (creates a fresh copy of the buffer).
   **/
  SampleBuf(SampleBuf const &from) {
    buffer = (Sample *) malloc(0);
    length = 0;
    assign(from);
  }

  /**
   * Become identical (via buffer copying) to another SampleBuf.
   **/
  SampleBuf const &operator =(SampleBuf const &from) { 
    assign(from);
    return *this;
  }

  ~SampleBuf() {
    free(buffer);
  }

  /**
   * Become identical (via buffer copying) to another SampleBuf.
   **/
  void assign(SampleBuf const &from) {
    if (length != from.length)
      resize(from.length);
    memcpy(buffer, from.buffer, length);
  }

  /**
   * Sick hack to make this buffer refer directly to a position
   * internal to it. Yuck! Don't use this elsewhere!
   *
   * @see SampleCache::read()
   **/
  void wind(int offset) {
    // Gross. <shrug>
    buffer += offset;
    length -= offset;
  }

  /// Returns the number of samples in this buffer.
  int getLength() const { return length; }

  /// Retrieve an individual sample (non-const).
  Sample &operator[](int index) { return buffer[index]; }
  /// Retrieve an individual sample (const).
  Sample const &operator[](int index) const { return buffer[index]; }

  /**
   * Performs vector addition on this SampleBuf.  Every element in
   * this is pairwise incremented by the corresponding element in
   * 'other'.
   *
   * @note 'other' MUST have the same length as this!
   *
   * @param other another (equally-sized) SampleBuf
   **/
  void operator +=(SampleBuf const &other) {
    assert(length == other.length);
    for (int i = 0; i < length; i++)
      buffer[i] = buffer[i] + other.buffer[i];
  }

  /**
   * Perform vector translation on this SampleBuf. Every element in
   * this is incremented by 'value'.
   *
   * @param value the value to add to each element
   **/
  void operator +=(Sample value) {
    for (int i = 0; i < length; i++)
      buffer[i] = buffer[i] + value;
  }

  /**
   * Zeroes out the entire buffer. (Currently, cheats rather then
   * using Sample::zero.)
   **/
  void clear() {
    memset(buffer, 0, sizeof(Sample) * length);
  }

  /**
   * Sets each element in this buffer to the given Sample value.
   **/
  void fill_with(Sample value) {
    for (int i = 0; i < length; i++)
      buffer[i] = value;
  }

  /**
   * Essentially performs a realloc on the contents of this buffer,
   * keeping as much of the old contents as possible, and zeroing out
   * any newly-available space (again, cheating rather than using
   * Sample::zero).
   **/
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

  /**
   * Rolls this buffer over by 'howmany' bytes. Quite grubby. Again,
   * the culprit is SampleCache::read().
   * @see SampleCache::read()
   **/
  void delete_front(int howmany) {
    assert(howmany <= length);

    Sample *n = (Sample *) malloc(sizeof(Sample) * (length - howmany));
    memcpy(n, buffer + howmany, sizeof(Sample) * (length - howmany));
    free(buffer);
    buffer = n;
    length -= howmany;
  }

private:
  Sample *buffer;			///< The samples in this buffer.
  int length;				///< The number of samples in 'buffer'.
};

/**
 * Representation of our internal timing unit.
 **/
typedef gint32 sampletime_t;

GALAN_END_NAMESPACE

#endif
