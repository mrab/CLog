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
#include <cstddef>
#include <cstring>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "clog.h"
#include "testUtils.h"

using namespace ::testing;

TEST(testLineHeader, testFormatHeader) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  char buffer[BufferLength];
  int bufferLength = BufferLength - GuardLength;
  memset(buffer, 0, bufferLength);
  memset(&(buffer[bufferLength]), 0xFF, GuardLength);

  const char *file = "myFile.c";
  const unsigned int line = 123;
  const char *function = "foo()";
  const char *message = "the message";
  const CLogLevel level = CLOG_LWRN;
  const char *tag = "IO";
  const CLogMessage msg = {
      file,     // const char * file;
      line,     // const unsigned int line;
      function, // const char *function;
      message,  // const char *message;
      level,    // const CLogLevel level;
      tag       // const char *tag;
  };

  clog_formatLineHeader(buffer, &bufferLength, &msg);

  // check guard
  ASSERT_EQ(buffer[BufferLength - GuardLength + 0], (char)0xFF);
  ASSERT_EQ(buffer[BufferLength - GuardLength + 1], (char)0xFF);
  ASSERT_EQ(buffer[BufferLength - GuardLength + 2], (char)0xFF);
  ASSERT_EQ(bufferLength, 44);
  ASSERT_STREQ(buffer, "\033[33mWRN:IO\x1b[0m \x1b[90mmyFile.c:123(foo())\x1b[0m");
}

TEST(testLineHeader, testFormatHeaderNoBuffer) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  int bufferLength = BufferLength - GuardLength;

  const char *file = "myFile.c";
  const unsigned int line = 123;
  const char *function = "foo()";
  const char *message = "the message";
  const CLogLevel level = CLOG_LWRN;
  const char *tag = "IO";
  const CLogMessage msg = {
      file,     // const char * file;
      line,     // const unsigned int line;
      function, // const char *function;
      message,  // const char *message;
      level,    // const CLogLevel level;
      tag       // const char *tag;
  };

  clog_formatLineHeader(nullptr, &bufferLength, &msg);

  ASSERT_EQ(static_cast<size_t>(bufferLength), BufferLength - GuardLength);
}

TEST(testLineHeader, testFormatHeaderNoLength) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  char buffer[BufferLength];
  char bufferReference[BufferLength];
  int bufferLength = BufferLength - GuardLength;
  memset(buffer, 0, bufferLength);
  memset(&(buffer[bufferLength]), 0xFF, GuardLength);
  memcpy(bufferReference, buffer, BufferLength);

  const char *file = "myFile.c";
  const unsigned int line = 123;
  const char *function = "foo()";
  const char *message = "the message";
  const CLogLevel level = CLOG_LWRN;
  const char *tag = "IO";
  const CLogMessage msg = {
      file,     // const char * file;
      line,     // const unsigned int line;
      function, // const char *function;
      message,  // const char *message;
      level,    // const CLogLevel level;
      tag       // const char *tag;
  };

  clog_formatLineHeader(buffer, nullptr, &msg);

  ASSERT_TRUE(ArraysEqual(bufferReference, buffer, BufferLength));
}

TEST(testLineHeader, testFormatHeaderBelowMinimumLength) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  char buffer[BufferLength];
  char bufferReference[BufferLength];
  int bufferLength = BufferLength - GuardLength;
  memset(buffer, 0, bufferLength);
  memset(&(buffer[bufferLength]), 0xFF, GuardLength);
  memcpy(bufferReference, buffer, BufferLength);

  bufferLength = 0;
  const char *file = "myFile.c";
  const unsigned int line = 123;
  const char *function = "foo()";
  const char *message = "the message";
  const CLogLevel level = CLOG_LWRN;
  const char *tag = "IO";
  const CLogMessage msg = {
      file,     // const char * file;
      line,     // const unsigned int line;
      function, // const char *function;
      message,  // const char *message;
      level,    // const CLogLevel level;
      tag       // const char *tag;
  };

  clog_formatLineHeader(buffer, &bufferLength, &msg);

  ASSERT_TRUE(ArraysEqual(bufferReference, buffer, BufferLength));
}

TEST(testLineHeader, testFormatHeaderNoMsg) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  char buffer[BufferLength];
  char bufferReference[BufferLength];
  int bufferLength = BufferLength - GuardLength;
  memset(buffer, 0, bufferLength);
  memset(&(buffer[bufferLength]), 0xFF, GuardLength);
  memcpy(bufferReference, buffer, BufferLength);

  clog_formatLineHeader(buffer, &bufferLength, nullptr);

  ASSERT_EQ(static_cast<size_t>(bufferLength), BufferLength - GuardLength);
  ASSERT_TRUE(ArraysEqual(bufferReference, buffer, BufferLength));
}

TEST(testLineHeader, testFormatHeaderInsufficientBuffer) {
  const size_t BufferLength = 40;
  const size_t GuardLength = 3;
  char buffer[BufferLength];
  char bufferReference[BufferLength] = {'\033', '[',    '3', '3', 'm', 'W', 'R',    'N',    ':',    'I',
                                        'O',    '\x1b', '[', '0', 'm', ' ', '\x1b', '[',    '9',    '0',
                                        'm',    'm',    'y', 'F', 'i', 'l', 'e',    '.',    'c',    ':',
                                        '1',    '2',    '3', '(', 'f', 'o', '\x00', '\xFF', '\xFF', '\xFF'};
  int bufferLength = BufferLength - GuardLength;
  memset(buffer, 0, bufferLength);
  memset(&(buffer[bufferLength]), 0xFF, GuardLength);

  const char *file = "myFile.c";
  const unsigned int line = 123;
  const char *function = "foo()";
  const char *message = "the message";
  const CLogLevel level = CLOG_LWRN;
  const char *tag = "IO";
  const CLogMessage msg = {
      file,     // const char * file;
      line,     // const unsigned int line;
      function, // const char *function;
      message,  // const char *message;
      level,    // const CLogLevel level;
      tag       // const char *tag;
  };

  clog_formatLineHeader(buffer, &bufferLength, &msg);

  ASSERT_EQ(static_cast<size_t>(bufferLength), BufferLength - GuardLength);
  ASSERT_TRUE(ArraysEqual(bufferReference, buffer, BufferLength));
}

TEST(testLineHeader, testFormatHeaderNullPointersInMessage) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  char buffer[BufferLength];
  int bufferLength = BufferLength - GuardLength;
  memset(buffer, 0, bufferLength);
  memset(&(buffer[bufferLength]), 0xFF, GuardLength);

  const char *file = nullptr;
  const unsigned int line = 123;
  const char *function = nullptr;
  const char *message = nullptr;
  const CLogLevel level = CLOG_LWRN;
  const char *tag = nullptr;
  const CLogMessage msg = {
      file,     // const char * file;
      line,     // const unsigned int line;
      function, // const char *function;
      message,  // const char *message;
      level,    // const CLogLevel level;
      tag       // const char *tag;
  };

  clog_formatLineHeader(buffer, &bufferLength, &msg);

  // check guard
  ASSERT_EQ(buffer[BufferLength - GuardLength + 0], (char)0xFF);
  ASSERT_EQ(buffer[BufferLength - GuardLength + 1], (char)0xFF);
  ASSERT_EQ(buffer[BufferLength - GuardLength + 2], (char)0xFF);
  ASSERT_EQ(bufferLength, 47);
  ASSERT_STREQ(buffer, "\033[33mWRN:(null)\x1b[0m \x1b[90m(null):123((null))\x1b[0m");
}

TEST(testLineHeader, testFormatHeaderIllegalLevel) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  char buffer[BufferLength];
  int bufferLength = BufferLength - GuardLength;
  memset(buffer, 0, bufferLength);
  memset(&(buffer[bufferLength]), 0xFF, GuardLength);

  const char *file = "myFile.c";
  const unsigned int line = 123;
  const char *function = "foo()";
  const char *message = "the message";
  const CLogLevel level = static_cast<CLogLevel>(1000U);
  const char *tag = "IO";
  const CLogMessage msg = {
      file,     // const char * file;
      line,     // const unsigned int line;
      function, // const char *function;
      message,  // const char *message;
      level,    // const CLogLevel level;
      tag       // const char *tag;
  };

  clog_formatLineHeader(buffer, &bufferLength, &msg);

  // check guard
  ASSERT_EQ(buffer[BufferLength - GuardLength + 0], (char)0xFF);
  ASSERT_EQ(buffer[BufferLength - GuardLength + 1], (char)0xFF);
  ASSERT_EQ(buffer[BufferLength - GuardLength + 2], (char)0xFF);
  ASSERT_EQ(bufferLength, 44);
  ASSERT_STREQ(buffer, "\033[35mUKN:IO\x1b[0m \x1b[90mmyFile.c:123(foo())\x1b[0m");
}
