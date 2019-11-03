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
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

#include "clog.h"
#include "testUtils.h"

#define DEFAULT_TAGS(F) \
  F(COMMUNICATION)      \
  F(IO)

TEST(testLevels, testLevelNames) {
  ASSERT_STREQ("TRC", clog_getLevel(CLOG_LTRC));
  ASSERT_STREQ("DBG", clog_getLevel(CLOG_LDBG));
  ASSERT_STREQ("WRN", clog_getLevel(CLOG_LWRN));
  ASSERT_STREQ("INF", clog_getLevel(CLOG_LINF));
  ASSERT_STREQ("ERR", clog_getLevel(CLOG_LERR));
  ASSERT_STREQ("FTL", clog_getLevel(CLOG_LFTL));
  ASSERT_STREQ("OFF", clog_getLevel(CLOG_LOFF));
  ASSERT_STREQ("UKN", clog_getLevel(CLOG_LUKN));
  ASSERT_STREQ("UKN", clog_getLevel(static_cast<CLogLevel>(static_cast<int>(CLOG_LOFF) + 1)));
}

TEST(testMinLevel, testSetMinLevel) {
  const size_t BufferSize = 1000;
  std::array<char, BufferSize> buffer;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
  CLOG_ENUM_WITH_NAMES(DefaultTags, DEFAULT_TAGS);
#pragma GCC diagnostic pop

  CLogContext ctx = {nullptr, 0U, DefaultTagsNames, ARRAY_LENGTH(DefaultTagsNames), CLOG_LOFF, buffer.data(), 1000};

  ASSERT_EQ(CLOG_LOFF, clog_getMinLevel(&ctx));

  clog_setMinLevel(&ctx, CLOG_LERR);
  ASSERT_EQ(CLOG_LERR, clog_getMinLevel(&ctx));
}

TEST(testMinLevel, testNoContext) {
  // invoke the setter to provoke an exception eventually
  clog_setMinLevel(nullptr, CLOG_LWRN);

  ASSERT_EQ(CLOG_LTRC, clog_getMinLevel(nullptr));

}
