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
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * @def ARRAY_LENGTH
 * @param A The array
 * Macro to calculate the length of an array.
 */
#define ARRAY_LENGTH(A) (sizeof(A) / sizeof(A[0]))

// The buffer is needed to keep the formatted message.
// Only the message text and the following parameters.
#define BufferSize (200U)

/**
 * @def DEFAULT_TAGS
 * Some tags to be used with the log messages
 */
#define DEFAULT_TAGS(F) \
  F(COMM)               \
  F(PLUGIN)

// Here an enum named Tags and an array of C strings
// named TagsNames are being produced using the DEFAULT_TAGS
CLOG_ENUM_WITH_NAMES(Tags, DEFAULT_TAGS)

/***
 * A struct to keep the state of the filter.
 */
typedef struct {
  bool enableComm;    ///< Indicates if messages from the COMM modules shall be printed
  bool enablePlugin;  ///< Indicates if messages from the PLUGIN modules shall be printed
  CLogLevel minLevel; ///< The minimum level for a message to be printed
} FilterSettings;

/**
 * The instance of the filter state. In the initial state all tags are turned on and only messages
 * with level error or higher are logged.
 */
static FilterSettings stdoutFilterSettings = {.enableComm = true, .enablePlugin = true, .minLevel = CLOG_LERR};

/**
 * A Filter function to filter messages.
 *
 * @param message The message to be processed
 * @return true If the message matches the filter rules
 * @return false If the message does not match the filter rules
 */
bool stdoutFilter(const CLogMessage *message) {
  assert(message);

  // suppress COMM messages if not enabled
  if (!stdoutFilterSettings.enableComm && (0 == strcmp(message->tag, TagsNames[COMM]))) {
    return false;
  }

  // suppress PLUGIN messages if not enabled
  if (!stdoutFilterSettings.enablePlugin && (0 == strcmp(message->tag, TagsNames[PLUGIN]))) {
    return false;
  }

  // filter by level
  if (message->level < stdoutFilterSettings.minLevel) {
    return false;
  }

  return true;
}

/**
 * A simple onMessage handler that prints messages to stdout.
 *
 * @param message The message to be printed.
 */
void stdoutPrinter(const CLogMessage *message) {
  assert(message);

  char buffer[BufferSize];
  int bufferSize = BufferSize;

  // first format the message into buffer
  clog_formatMessage(buffer, &bufferSize, message);

  // then print it
  printf("%s", buffer);
}

int main(int argc, char **argv) {
  assert(argc);
  assert(argv);

  char buffer[BufferSize];

  // Then we need a list of adapters (here only one),
  // each consisting of a filter and a printer.
  CLogAdapter adapters[1] = {{stdoutFilter, stdoutPrinter}};

  // The context for the log messages.
  CLogContext ctx = {
      adapters,
      ARRAY_LENGTH(adapters),
      TagsNames,
      ARRAY_LENGTH(TagsNames),
      CLOG_LTRC,
      buffer,
      ARRAY_LENGTH(buffer),
  };

  // let's print some messages
  // By default all tags are enabled and anything above a warning is printed

  CLOG_TRC(&ctx, COMM, "This message will not be printed due do a low log level.");
  CLOG_ERR(&ctx, COMM, "This error message will be printed.");

  CLOG_TRC(&ctx, PLUGIN, "This message will not be printed due do a low log level.");
  CLOG_ERR(&ctx, PLUGIN, "This error message will be printed.");

  // Now let's turn off messages from the COMM module
  stdoutFilterSettings.enableComm = false;
  CLOG_ERR(&ctx, COMM, "Now this error message will not be printed as the COMM module is disabled.");

  // Now turn on trace mode
  stdoutFilterSettings.minLevel = CLOG_LTRC;

  CLOG_TRC(&ctx, PLUGIN, "Now even trace messages will be printed.");
  CLOG_TRC(&ctx, COMM, "... but the COMM module is still disabled.");

  // Now we print some messages with parameters.
  // Anything tht works with printf should work here as well.

  CLOG_TRC(&ctx, PLUGIN, "Integer is %d and string is %s", 123, "blubber");
  CLOG_DBG(&ctx, PLUGIN, "Here is a debug float %f", 2.3F);
  CLOG_INF(&ctx, PLUGIN, "I am some informational hex number %x", 12234U);
  CLOG_FTL(&ctx,
           PLUGIN,
           "Something went horribly wrong here (%d, %x). The address of stdoutFilterSettings is %p",
           -123,
           100U,
           &stdoutFilterSettings);
}
