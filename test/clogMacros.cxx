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

class CLogMacroTest : public ::testing::Test {
protected:
  // longest string that shall fit in is "Fatal Error"
  // which requires 12 bytes
  static const size_t BufferSize = 12;
  static const size_t GuardSize = 3;
  char buffer[BufferSize + GuardSize];
  CLogAdapter adapters[1] = {{CLogMacroTest::filter, CLogMacroTest::printer}};
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

MockAdapter *CLogMacroTest::mock;

TEST_F(CLogMacroTest, filterMinLevel) {
  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);

  CLOG_TRC(&ctx, IO, "Trace")
  CLOG_DBG(&ctx, IO, "Debug")
  CLOG_INF(&ctx, IO, "Info")
  CLOG_WRN(&ctx, IO, "Warning")
  CLOG_ERR(&ctx, IO, "Error")
  CLOG_FTL(&ctx, IO, "Fatal Error")
}

TEST_F(CLogMacroTest, filterMinLevelFatal) {
  ctx.minLevel = CLOG_LFTL;

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_TRC(&ctx, IO, "Trace")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_DBG(&ctx, IO, "Debug")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_INF(&ctx, IO, "Info")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_WRN(&ctx, IO, "Warning")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_ERR(&ctx, IO, "Error")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(false));
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_FTL(&ctx, IO, "Fatal Error")
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMacroTest, filterMinLevelTrace) {
  ctx.minLevel = CLOG_LTRC;

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(false));
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_TRC(&ctx, IO, "Trace")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(false));
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_DBG(&ctx, IO, "Debug")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(false));
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_INF(&ctx, IO, "Info")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(false));
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_WRN(&ctx, IO, "Warning")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(false));
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_ERR(&ctx, IO, "Error")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(false));
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_FTL(&ctx, IO, "Fatal Error")
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMacroTest, filterMinLevelInfo) {
  ctx.minLevel = CLOG_LINF;

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_TRC(&ctx, IO, "Trace")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(0);
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_DBG(&ctx, IO, "Debug")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(false));
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_INF(&ctx, IO, "Info")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(false));
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_WRN(&ctx, IO, "Warning")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(false));
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_ERR(&ctx, IO, "Error")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(false));
  EXPECT_CALL(*mock, printer(_)).Times(0);
  CLOG_FTL(&ctx, IO, "Fatal Error")
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMacroTest, printWithoutParams) {
  ctx.minLevel = CLOG_LTRC;

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock, printer(Field(&CLogMessage::message, StrEq("Trace")))).Times(1);
  CLOG_TRC(&ctx, IO, "Trace")
  ASSERT_TRUE(isGuardOk());

  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock, printer(Field(&CLogMessage::message, StrEq("Debug")))).Times(1);
  CLOG_DBG(&ctx, IO, "Debug")
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMacroTest, printTestEndOfString) {
  ctx.minLevel = CLOG_LTRC;
  memset(ctx.messageBuffer, 0xFF, ctx.messageBufferSize);
  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock, printer(Field(&CLogMessage::message, StrEq("Trace")))).Times(1);
  CLOG_TRC(&ctx, IO, "Trace")
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMacroTest, printTestSomeParam) {
  ctx.minLevel = CLOG_LTRC;
  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock, printer(Field(&CLogMessage::message, StrEq("1, 2.5, abc")))).Times(1);
  CLOG_TRC(&ctx, IO, "%d, %2.1f, %s", 1, 2.5F, "abc")
  ASSERT_TRUE(isGuardOk());
}

TEST_F(CLogMacroTest, printTestOverflow) {
  ctx.minLevel = CLOG_LTRC;
  EXPECT_CALL(*mock, filter(_)).Times(1).WillRepeatedly(Return(true));
  // Buffer is 12 characters long (including terminator),
  // so we have to expect that the string is truncated after the 11th one
  EXPECT_CALL(*mock, printer(Field(&CLogMessage::message, StrEq("123456789AB")))).Times(1);
  CLOG_TRC(&ctx, IO, "123456789ABC")
  ASSERT_TRUE(isGuardOk());
}
