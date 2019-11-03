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

#include "clog.h"
#include <stdarg.h>
#include <stdio.h>

static const char *EmptyTag = "";

static char *levelNames[] = {"TRC", "DBG", "INF", "WRN", "ERR", "FTL", "OFF", "UKN"};

#ifdef CLOG_COLOR
static const char *levelColors[] =
    {"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m", "\x1b[94m", "\x1b[35m"};
static const char *lineHeaderFormat = "%s%s:%s\x1b[0m \x1b[90m%s:%d(%s)\x1b[0m";
#else
static const char *lineHeaderFormat = "%s%s:%s %s:%d(%s)";
static const char *clog_getColor(const CLogLevel level);
#endif

bool clog_checkContext(const CLogContext *ctx) {
  if (NULL == ctx) {
    return false;
  }
  if (NULL == ctx) {
    return false;
  }

  if (NULL == ctx->adapters || 0U == ctx->adaptersSize) {
    return false;
  }

  if (NULL == ctx->tagNames || 0U == ctx->numberOfTags) {
    return false;
  }

  if (NULL == ctx->messageBuffer || ctx->messageBufferSize < 1U) {
    return false;
  }

  for (size_t i = 0; i < ctx->adaptersSize; i++) {
    if (!ctx->adapters[i].onMessage) {
      return false;
    }
  }

  return true;
}

CLogLevel clog_getMinLevel(CLogContext *ctx) {
  if (NULL == ctx) {
    return CLOG_LTRC;
  }
  return ctx->minLevel;
}

void clog_setMinLevel(CLogContext *ctx, CLogLevel level) {
  if (NULL == ctx) {
    return;
  }
  ctx->minLevel = level;
}

void clog_logMessage(CLogContext *const ctx,
                     const CLogLevel level,
                     const size_t tag,
                     const char *const file,
                     const unsigned int line,
                     const char *const function,
                     const char *const message,
                     ...) {
  if (NULL == ctx) {
    return;
  }

  if (NULL == ctx->adapters || 0U == ctx->adaptersSize) {
    return;
  }

  if (NULL == ctx->tagNames || 0U == ctx->numberOfTags) {
    return;
  }

  if (NULL == ctx->messageBuffer || ctx->messageBufferSize < 1U) {
    return;
  }

  if (level < ctx->minLevel) {
    return;
  }

  if (NULL == message) {
    return;
  }

  const char *tagName = NULL;

  if (tag < ctx->numberOfTags) {
    tagName = ctx->tagNames[tag];
  } else {
    tagName = EmptyTag;
  }

  const CLogLevel finalLevel = (level < CLOG_LOFF) ? level : CLOG_LUKN;

  CLogMessage msg = {file, line, function, ctx->messageBuffer, finalLevel, tagName};

  va_list args;

  va_start(args, message);
  vsnprintf(ctx->messageBuffer, ctx->messageBufferSize, message, args);
  va_end(args);

  ctx->messageBuffer[ctx->messageBufferSize - 1] = 0;

  for (size_t i = 0; i < ctx->adaptersSize; i++) {
    if (!ctx->adapters[i].onMessage) {
      continue;
    }
    if (!ctx->adapters[i].messageFilter || ctx->adapters[i].messageFilter(&msg)) {
      ctx->adapters[i].onMessage(&msg);
    }
  }
}

void clog_formatLineHeader(char buffer[], int *const bufferLength, const CLogMessage *const msg) {

  if (NULL == buffer || NULL == bufferLength || *bufferLength < 1 || NULL == msg) {
    return;
  }

  int maxLength = *bufferLength;

  *bufferLength = snprintf(buffer,
                           *bufferLength,
                           lineHeaderFormat,
                           clog_getColor(msg->level),
                           clog_getLevel(msg->level),
                           msg->tag,
                           msg->file,
                           msg->line,
                           msg->function);

  if (*bufferLength > maxLength) {
    *bufferLength = maxLength;
  }
}

void clog_formatMessage(char buffer[], int *const bufferLength, const CLogMessage *const msg) {
  if (NULL == buffer || NULL == bufferLength || *bufferLength < 2 || NULL == msg) {
    return;
  }

  int maxLength = *bufferLength;

  clog_formatLineHeader(buffer, bufferLength, msg);

  int usedBytes = *bufferLength;
  if (*bufferLength >= maxLength) {
    *bufferLength = maxLength;
    return;
  }

  *bufferLength = snprintf(&buffer[usedBytes], (maxLength - usedBytes), " %s\n", msg->message);

  *bufferLength += usedBytes;
  buffer[maxLength - 2] = '\n';
  buffer[maxLength - 1] = 0U;
}

const char *clog_getLevel(const CLogLevel level) {
  if (level < 0 || level >= sizeof(levelNames) / sizeof(levelNames[0])) {
    return levelNames[CLOG_LUKN];
  }
  return levelNames[level];
}

#ifdef CLOG_COLOR
const char *clog_getColor(const CLogLevel level) {
  if (level < 0 || level >= sizeof(levelColors) / sizeof(levelColors[0])) {
    return levelColors[CLOG_LUKN];
  }
  return levelColors[level];
}
#else
static const char *clog_getColor(const CLogLevel level) {
  // Suppress unused parameter warning
  (void)(level);

  return EmptyTag;
}
#endif
