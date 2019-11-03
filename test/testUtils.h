/**
 * @copyright (c) 2019 mrab
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 *
 * @author Melchior Rabe (oss@mrab.de)
 * @brief
 * @date 2019-09-16
 *
 * @file
 */
#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <cstddef>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "clog.h"

using ::testing::AssertionResult;

#define ARRAY_LENGTH(A) (sizeof(A) / sizeof(A[0]))

template <typename T>::testing::AssertionResult ArraysEqual(T expected[], T actual[], size_t size) {
  // for the sake of having better diagnostics check elements one by one
  for (size_t i = 0; i < size; i++) {
    if (expected[i] != actual[i]) {
      return ::testing::AssertionFailure()
             << "Elemet mismatch: expected[" << i << "]: " << expected[i] << ", actual[" << i << "]: " << actual[i];
    }
  }

  return ::testing::AssertionSuccess();
}

class Adapter {
public:
  virtual ~Adapter(){};
  virtual bool filter(const CLogMessage *) {
    return true;
  };
  virtual void printer(const CLogMessage *){};
};
class MockAdapter : public Adapter {
public:
  virtual ~MockAdapter(){};

  MOCK_METHOD(bool, filter, (const CLogMessage *), (override));
  MOCK_METHOD(void, printer, (const CLogMessage *), (override));
};

#endif // TESTUTILS_H