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

#define DEFAULT_TAGS(F) \
  F(COMMUNICATION)      \
  F(IO)

class CLogMessageTest : public ::testing::Test {
protected:
  // longest string that shall fit in is "Fatal Error"
  // which requires 12 bytes
  static const size_t BufferSize = 12;
  static const size_t GuardSize = 3;
  char buffer[BufferSize + GuardSize];
  CLogAdapter adapters[1] = {{CLogMessageTest::filter, CLogMessageTest::printer}};
  CLogContext ctx = {
      adapters,
      ARRAY_LENGTH(adapters),
      TagsNames,
      ARRAY_LENGTH(TagsNames),
      CLOG_LOFF,
      buffer,
      BufferSize,
  };

  CLOG_ENUM_WITH_NAMES(Tags, DEFAULT_TAGS)

  void SetUp() override {
    memset(buffer, 0, ARRAY_LENGTH(buffer));
    for (size_t i = 0; i < GuardSize; i++) {
      buffer[BufferSize + i] = '\xFF';
    }
    mock = new MockAdapter();
  }

  void TearDown() override {
    delete mock;
    mock = nullptr;
  }

  bool isGuardOk() {
    for (size_t i = 0; i < GuardSize; i++) {
      if (buffer[BufferSize + i] != '\xFF') {
        return false;
      }
    }
    return true;
  }

  static MockAdapter *mock;

  static bool filter(const CLogMessage *message) {
    return mock->filter(message);
  };

  static void printer(const CLogMessage *message) {
    mock->printer(message);
  };
};

MockAdapter *CLogMessageTest::mock;

TEST_F(CLogMessageTest, logMsgParamsAllSet) {
  CLogContext *pCtx = &ctx;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LWRN;
  size_t tag = 0U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock,
              printer(AllOf(Field(&CLogMessage::file, StrEq(file)),
                            Field(&CLogMessage::function, StrEq(function)),
                            Field(&CLogMessage::level, Eq(CLOG_LWRN)),
                            Field(&CLogMessage::line, Eq(line)),
                            Field(&CLogMessage::message, StrEq(message)),
                            Field(&CLogMessage::tag, StrEq(ctx.tagNames[0])))))
      .Times(1);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, logMsgNoContext) {
  CLogContext *pCtx = nullptr;

  CLogLevel level = CLOG_LWRN;
  size_t tag = 0U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());

  // Buffer shall be untouched
  ASSERT_THAT(std::vector<char>(buffer, buffer + BufferSize), Each(0));
}

TEST_F(CLogMessageTest, logMsgNoAdapter) {
  CLogContext customCtx = {
      nullptr,
      5, // deliberately set an invalid adapter length
      TagsNames,
      ARRAY_LENGTH(TagsNames),
      CLOG_LTRC,
      buffer,
      BufferSize,
  };

  CLogContext *pCtx = &customCtx;

  CLogLevel level = CLOG_LWRN;
  size_t tag = 0U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());

  // Buffer shall be untouched
  ASSERT_THAT(std::vector<char>(buffer, buffer + BufferSize), Each(0));
}

TEST_F(CLogMessageTest, logMsgZeroAdapterLength) {
  CLogContext customCtx = {
      adapters,
      0, // deliberately set adapter length to 0
      TagsNames,
      ARRAY_LENGTH(TagsNames),
      CLOG_LTRC,
      buffer,
      BufferSize,
  };

  CLogContext *pCtx = &customCtx;

  CLogLevel level = CLOG_LWRN;
  size_t tag = 0U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());

  // Buffer shall be untouched
  ASSERT_THAT(std::vector<char>(buffer, buffer + BufferSize), Each(0));
}

TEST_F(CLogMessageTest, logMsgFileNullPtr) {
  CLogContext *pCtx = &ctx;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LWRN;
  size_t tag = 0U;
  const char *file = nullptr;
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock,
              printer(AllOf(Field(&CLogMessage::file, Eq(file)),
                            Field(&CLogMessage::function, StrEq(function)),
                            Field(&CLogMessage::level, Eq(CLOG_LWRN)),
                            Field(&CLogMessage::line, Eq(line)),
                            Field(&CLogMessage::message, StrEq(message)),
                            Field(&CLogMessage::tag, StrEq(ctx.tagNames[0])))))
      .Times(1);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, logMsgFunctionNullPtr) {
  CLogContext *pCtx = &ctx;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LWRN;
  size_t tag = 0U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = nullptr;
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock,
              printer(AllOf(Field(&CLogMessage::file, StrEq(file)),
                            Field(&CLogMessage::function, Eq(function)),
                            Field(&CLogMessage::level, Eq(CLOG_LWRN)),
                            Field(&CLogMessage::line, Eq(line)),
                            Field(&CLogMessage::message, StrEq(message)),
                            Field(&CLogMessage::tag, StrEq(ctx.tagNames[0])))))
      .Times(1);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, logMsgAdapterPrinterIsNull) {
  CLogAdapter adapter[] = {{CLogMessageTest::filter, nullptr}};
  CLogContext customCtx = {
      adapter,
      1,
      TagsNames,
      ARRAY_LENGTH(TagsNames),
      CLOG_LTRC,
      buffer,
      BufferSize,
  };

  CLogContext *pCtx = &customCtx;
  CLogLevel level = CLOG_LWRN;
  size_t tag = 0U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = nullptr;
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());

  // Buffer shall be untouched
  ASSERT_THAT(std::vector<char>(buffer, buffer + BufferSize), Each(0));
}

TEST_F(CLogMessageTest, logMsgMsgNullPtr) {
  CLogContext *pCtx = &ctx;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LWRN;
  size_t tag = 0U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = nullptr;

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, logMsgTagOutOfRange) {
  CLogContext *pCtx = &ctx;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LWRN;
  size_t tag = 123U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock,
              printer(AllOf(Field(&CLogMessage::file, StrEq(file)),
                            Field(&CLogMessage::function, StrEq(function)),
                            Field(&CLogMessage::level, Eq(CLOG_LWRN)),
                            Field(&CLogMessage::line, Eq(line)),
                            Field(&CLogMessage::message, StrEq(message)),
                            Field(&CLogMessage::tag, StrEq("")))))
      .Times(1);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, logMsgLevelOutOfRange) {
  CLogContext *pCtx = &ctx;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = static_cast<CLogLevel>(static_cast<int>(CLOG_LUKN) + 1);
  size_t tag = 1U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock,
              printer(AllOf(Field(&CLogMessage::file, StrEq(file)),
                            Field(&CLogMessage::function, StrEq(function)),
                            Field(&CLogMessage::level, Eq(CLOG_LUKN)),
                            Field(&CLogMessage::line, Eq(line)),
                            Field(&CLogMessage::message, StrEq(message)),
                            Field(&CLogMessage::tag, StrEq("IO")))))
      .Times(1);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, logMsgLevelBelowMinLevel) {
  CLogContext *pCtx = &ctx;
  ctx.minLevel = CLOG_LERR;
  CLogLevel level = CLOG_LWRN;
  size_t tag = 1U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, logMsgLevelUnknown) {
  CLogContext *pCtx = &ctx;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LUKN;
  size_t tag = 1U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock,
              printer(AllOf(Field(&CLogMessage::file, StrEq(file)),
                            Field(&CLogMessage::function, StrEq(function)),
                            Field(&CLogMessage::level, Eq(CLOG_LUKN)),
                            Field(&CLogMessage::line, Eq(line)),
                            Field(&CLogMessage::message, StrEq(message)),
                            Field(&CLogMessage::tag, StrEq("IO")))))
      .Times(1);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, logMsgLevelOff) {
  CLogContext *pCtx = &ctx;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LOFF;
  size_t tag = 1U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock,
              printer(AllOf(Field(&CLogMessage::file, StrEq(file)),
                            Field(&CLogMessage::function, StrEq(function)),
                            Field(&CLogMessage::level, Eq(CLOG_LUKN)),
                            Field(&CLogMessage::line, Eq(line)),
                            Field(&CLogMessage::message, StrEq(message)),
                            Field(&CLogMessage::tag, StrEq("IO")))))
      .Times(1);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, noAdapters) {
  CLogContext context = {
      nullptr,
      5,
      TagsNames,
      ARRAY_LENGTH(TagsNames),
      CLOG_LTRC,
      buffer,
      BufferSize,
  };

  CLogContext *pCtx = &context;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LOFF;
  size_t tag = 1U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, noAdapterLength) {
  CLogContext context = {
      adapters,
      0,
      TagsNames,
      ARRAY_LENGTH(TagsNames),
      CLOG_LTRC,
      buffer,
      BufferSize,
  };

  CLogContext *pCtx = &context;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LOFF;
  size_t tag = 1U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, logAdapterNoPrinter) {
    CLogAdapter testAdapters[1] = {{CLogMessageTest::filter, nullptr}};
  CLogContext context = {
      testAdapters,
      ARRAY_LENGTH(testAdapters),
      TagsNames,
      ARRAY_LENGTH(TagsNames),
      CLOG_LTRC,
      buffer,
      BufferSize,
  };

  CLogContext *pCtx = &context;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LWRN;
  size_t tag = 0U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock,
              printer(_))
      .Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, logAdapterNoFilter) {
  CLogAdapter testAdapters[1] = {{nullptr, CLogMessageTest::printer}};;
  CLogContext context = {
      testAdapters,
      ARRAY_LENGTH(testAdapters),
      TagsNames,
      ARRAY_LENGTH(TagsNames),
      CLOG_LTRC,
      buffer,
      BufferSize,
  };

  CLogContext *pCtx = &context;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LWRN;
  size_t tag = 0U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock,
              printer(AllOf(Field(&CLogMessage::file, StrEq(file)),
                            Field(&CLogMessage::function, StrEq(function)),
                            Field(&CLogMessage::level, Eq(CLOG_LWRN)),
                            Field(&CLogMessage::line, Eq(line)),
                            Field(&CLogMessage::message, StrEq(message)),
                            Field(&CLogMessage::tag, StrEq("COMMUNICATION")))))
      .Times(1);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, noTags) {
  CLogContext context = {
      adapters,
      ARRAY_LENGTH(adapters),
      nullptr,
      ARRAY_LENGTH(TagsNames),
      CLOG_LOFF,
      buffer,
      BufferSize,
  };

  CLogContext *pCtx = &context;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LOFF;
  size_t tag = 1U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, noTagsLength) {
  CLogContext context = {
      adapters,
      ARRAY_LENGTH(adapters),
      TagsNames,
      0,
      CLOG_LOFF,
      buffer,
      BufferSize,
  };

  CLogContext *pCtx = &context;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LOFF;
  size_t tag = 1U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, noBuffer) {
  CLogContext context = {
      adapters,
      ARRAY_LENGTH(adapters),
      TagsNames,
      ARRAY_LENGTH(TagsNames),
      CLOG_LOFF,
      nullptr,
      BufferSize,
  };

  CLogContext *pCtx = &context;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LOFF;
  size_t tag = 1U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMessageTest, noBufferLength) {
  CLogContext context = {
      adapters,
      ARRAY_LENGTH(adapters),
      TagsNames,
      ARRAY_LENGTH(TagsNames),
      CLOG_LOFF,
      buffer,
      0,
  };

  CLogContext *pCtx = &context;
  ctx.minLevel = CLOG_LTRC;
  CLogLevel level = CLOG_LOFF;
  size_t tag = 1U;
  const char *file = "myFile.c";
  const unsigned int line = 123U;
  const char *function = "foo()";
  const char *message = "some msg";

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);

  clog_logMessage(pCtx, level, tag, file, line, function, message);
  ASSERT_TRUE(isGuardOk());
}
