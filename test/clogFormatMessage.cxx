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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "clog.h"
#include "testUtils.h"

using namespace ::testing;

TEST(testFormatMessage, testFormatMessage) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  std::array<char, BufferLength> buffer;
  int bufferLength = BufferLength - GuardLength;
  std::fill_n(buffer.begin(), bufferLength, 0);
  std::fill_n(buffer.begin() + bufferLength, GuardLength, 0xFF);

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

  clog_formatMessage(buffer.data(), &bufferLength, &msg);

  // check guard
  ASSERT_EQ(buffer[BufferLength - GuardLength + 0], (char)0xFF);
  ASSERT_EQ(buffer[BufferLength - GuardLength + 1], (char)0xFF);
  ASSERT_EQ(buffer[BufferLength - GuardLength + 2], (char)0xFF);
  ASSERT_EQ(bufferLength, 57);
  ASSERT_STREQ(buffer.data(), "\033[33mWRN:IO\x1b[0m \x1b[90mmyFile.c:123(foo())\x1b[0m the message\n");
}

TEST(testFormatMessage, testFormatMessageNoBuffer) {
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

  clog_formatMessage(nullptr, &bufferLength, &msg);

  ASSERT_EQ(static_cast<size_t>(bufferLength), BufferLength - GuardLength);
}

TEST(testFormatMessage, testFormatMessageNoLength) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  std::array<char, BufferLength> buffer;
  std::array<char, BufferLength> bufferReference;
  int bufferLength = BufferLength - GuardLength;
  std::fill_n(buffer.begin(), bufferLength, 0);
  std::fill_n(buffer.begin() + bufferLength, GuardLength, 0xFF);
  bufferReference = buffer;

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

  clog_formatMessage(buffer.data(), nullptr, &msg);

  EXPECT_THAT(buffer, ContainerEq(bufferReference));
}

TEST(testFormatMessage, testFormatMessageBelowMinimumLength) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  std::array<char, BufferLength> buffer;
  std::array<char, BufferLength> bufferReference;
  int bufferLength = BufferLength - GuardLength;
  std::fill_n(buffer.begin(), bufferLength, 0);
  std::fill_n(buffer.begin() + bufferLength, GuardLength, 0xFF);
  bufferReference = buffer;

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

  clog_formatMessage(buffer.data(), &bufferLength, &msg);

  EXPECT_THAT(buffer, ContainerEq(bufferReference));
}

TEST(testFormatMessage, testFormatMessageNoMsg) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  std::array<char, BufferLength> buffer;
  std::array<char, BufferLength> bufferReference;
  int bufferLength = BufferLength - GuardLength;
  std::fill_n(buffer.begin(), bufferLength, 0);
  std::fill_n(buffer.begin() + bufferLength, GuardLength, 0xFF);
  bufferReference = buffer;

  clog_formatMessage(buffer.data(), &bufferLength, nullptr);

  ASSERT_EQ(static_cast<size_t>(bufferLength), BufferLength - GuardLength);
  EXPECT_THAT(buffer, ContainerEq(bufferReference));
}

TEST(testFormatMessage, testFormatMessageInsufficientBuffer) {
  const size_t BufferLength = 40;
  const size_t GuardLength = 3;
  std::array<char, BufferLength> buffer;
  std::array<char, BufferLength> bufferReference = {'\033', '[',    '3', '3', 'm', 'W', 'R',    'N',    ':',    'I',
                                                    'O',    '\x1b', '[', '0', 'm', ' ', '\x1b', '[',    '9',    '0',
                                                    'm',    'm',    'y', 'F', 'i', 'l', 'e',    '.',    'c',    ':',
                                                    '1',    '2',    '3', '(', 'f', 'o', '\x00', '\xFF', '\xFF', '\xFF'};
  int bufferLength = BufferLength - GuardLength;
  std::fill_n(buffer.begin(), bufferLength, 0);
  std::fill_n(buffer.begin() + bufferLength, GuardLength, 0xFF);

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

  clog_formatMessage(buffer.data(), &bufferLength, &msg);

  ASSERT_EQ(static_cast<size_t>(bufferLength), BufferLength - GuardLength);
  EXPECT_THAT(buffer, ContainerEq(bufferReference));
}

TEST(testFormatMessage, testFormatMessageNullPointersInMessage) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  std::array<char, BufferLength> buffer;
  int bufferLength = BufferLength - GuardLength;
  std::fill_n(buffer.begin(), bufferLength, 0);
  std::fill_n(buffer.begin() + bufferLength, GuardLength, 0xFF);

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

  clog_formatMessage(buffer.data(), &bufferLength, &msg);

  // check guard
  ASSERT_EQ(buffer[BufferLength - GuardLength + 0], (char)0xFF);
  ASSERT_EQ(buffer[BufferLength - GuardLength + 1], (char)0xFF);
  ASSERT_EQ(buffer[BufferLength - GuardLength + 2], (char)0xFF);
  ASSERT_EQ(bufferLength, 55);
  ASSERT_STREQ(buffer.data(), "\033[33mWRN:(null)\x1b[0m \x1b[90m(null):123((null))\x1b[0m (null)\n");
}

TEST(testFormatMessage, testFormatMessageIllegalLevel) {
  const size_t BufferLength = 128;
  const size_t GuardLength = 3;
  std::array<char, BufferLength> buffer;
  int bufferLength = BufferLength - GuardLength;
  std::fill_n(buffer.begin(), bufferLength, 0);
  std::fill_n(buffer.begin() + bufferLength, GuardLength, 0xFF);

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

  clog_formatMessage(buffer.data(), &bufferLength, &msg);

  // check guard
  ASSERT_EQ(buffer[BufferLength - GuardLength + 0], (char)0xFF);
  ASSERT_EQ(buffer[BufferLength - GuardLength + 1], (char)0xFF);
  ASSERT_EQ(buffer[BufferLength - GuardLength + 2], (char)0xFF);
  ASSERT_EQ(bufferLength, 57);
  ASSERT_STREQ(buffer.data(), "\033[35mUKN:IO\x1b[0m \x1b[90mmyFile.c:123(foo())\x1b[0m the message\n");
}
