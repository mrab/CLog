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
 *
 * # Introduction
 * A versatile logging module for C projects. The log function itself if not thread safe. If you want to use the library
 * in multithreaded environemtns you need to use different log contexts for concurring events. The module doesn't
 * allocate any memory during runtime. This allows to use the module in embedded systems with limited resources.
 *
 *
 * # Important Components
 *
 * @startuml
 * class CLogContext {
 *  All properties of a single logging context.
 *  The use multiple different contexts is required
 *  only in special cases of multithreaded environments.
 * }
 *
 * class CLogAdapter {
 *  An adapter consists of a filter function
 *  and an onMessage event handler.
 * }
 *
 * class CLogMessage {
 *  This class is needed to allow structured backends,
 *  e.g. storing all fields of a message separately
 *  instead of formatting everything into a single
 *  string.
 * }
 * @enduml
 *
 *
 * # Call sequence of a logging event
 *
 * @startuml
 *  entity "Function that generates\na log event" as function
 *  participant "CLOG_*\n(log macro)\ninvokes clog_logMessage()" as log
 *  function -> log: pass context and paramters
 *  activate log
 *  log -> log: check paramters
 *  log -> log: prepare CLogMessage
 *  loop for all adapters in the context
 *    alt messageFilter accepts message
 *      log -> adapter: CLogMessage
 *    end
 *  end
 * @enduml
 */

#ifndef INCLUDE_CLOG_H_
#define INCLUDE_CLOG_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def CLOG_MLTRC
 * Macro representing a log level. Use only at compile time. Use CLogLevel otherwise.
 * @see CLogLevel for more information.
 */
#define CLOG_MLTRC (0U)

/**
 * @def CLOG_MLDBG
 * Macro representing a log level. Use only at compile time. Use CLogLevel otherwise.
 * @see CLogLevel for more information.
 */
#define CLOG_MLDBG (CLOG_MLTRC + 1U)

/**
 * @def CLOG_MLINF
 * Macro representing a log level. Use only at compile time. Use CLogLevel otherwise.
 * @see CLogLevel for more information.
 */
#define CLOG_MLINF (CLOG_MLDBG + 1U)

/**
 * @def CLOG_MLWRN
 * Macro representing a log level. Use only at compile time. Use CLogLevel otherwise.
 * @see CLogLevel for more information.
 */
#define CLOG_MLWRN (CLOG_MLINF + 1U)

/**
 * @def CLOG_MLERR
 * Macro representing a log level. Use only at compile time. Use CLogLevel otherwise.
 * @see CLogLevel for more information.
 */
#define CLOG_MLERR (CLOG_MLWRN + 1U)

/**
 * @def CLOG_MLFTL
 * Macro representing a log level. Use only at compile time. Use CLogLevel otherwise.
 * @see CLogLevel for more information.
 */
#define CLOG_MLFTL (CLOG_MLERR + 1U)

/**
 * @def CLOG_MLOFF
 * Macro representing a log level. Use only at compile time. Use CLogLevel otherwise.
 * @see CLogLevel for more information.
 */
#define CLOG_MLOFF (CLOG_MLFTL + 1U)

/**
 * The enum defining the different levels of log messages.
 * Can be used to classify different log messages and to filter them.
 */
typedef enum _CLogLevel {
  CLOG_LTRC = CLOG_MLTRC, /**< trace, detailed information on a very specific topic, should
                               usually be turned off. */
  CLOG_LDBG = CLOG_MLDBG, /**< debug, information that helps software developers to
                               understand the context. */
  CLOG_LINF = CLOG_MLINF, /**< info, important information. */
  CLOG_LWRN = CLOG_MLWRN, /**< warning, something the user should be aware of and this could
                               lead to an error in future. */
  CLOG_LERR = CLOG_MLERR, /**< error, something went wrong, but the system is still behaving
                               as it is supposed to. */
  CLOG_LFTL = CLOG_MLFTL, /**< fatal error, something went wrong, recovering is not possible,
                               the system might behave unexpectedly. */
  CLOG_LOFF = CLOG_MLOFF, /**< off, dummy level to be able to turn logging off. DO NOT USE AS LEVEL IN MESSAGES! */
  CLOG_LUKN               /**< unknown, dummy level to handle illegal values internally. DO NOT USE AS LEVEL IN MESSAGES! */
} CLogLevel;

/**
 * @def CLOG_FILE
 * Internal macro providing the name of the current file.
 * Set it manually before including this header
 * in order to provide a custom definition.
 */
#ifndef CLOG_FILE
#define CLOG_FILE __FILE__
#endif

/**
 * @def CLOG_GLOBAL_MIN_LEVEL
 * The global minimum log level. Set it to the respective
 * value in order to remove all less important messages already
 * at compile time.
 */
#ifndef CLOG_GLOBAL_MIN_LEVEL
#define CLOG_GLOBAL_MIN_LEVEL (CLOG_LTRC)
#endif

/**
 * @def CLOG_LINE
 * Internal macro providing the current line number.
 * Set it manually before including this header
 * in order to provide a custom definition.
 */
#ifndef CLOG_LINE
#define CLOG_LINE __LINE__
#endif

/**
 * @def CLOG_FUNC
 * Internal macro providing the name of the current function.
 */
#define CLOG_FUNC __func__

/**
 * @def CLOG_ENUM_ELEMENT
 * macro internally used to produce an enum element.
 */
#define CLOG_ENUM_ELEMENT(NAME) NAME,

/**
 * @def CLOG_ENUM_NAME
 * macro internally used to produce a name string for an enum element
 */
#define CLOG_ENUM_NAME(NAME) #NAME,

/**
 * @def CLOG_ENUM_COUNT
 * macro internally used to count the number of elements of an enum
 */
#define CLOG_ENUM_COUNT(NAME) +1

/**
 * @def CLOG_ENUM_WITH_NAMES
 * Internal macro producing an enum and an array containing the enum elements as
 * strings.
 */
#define CLOG_ENUM_WITH_NAMES(TYPENAME, ELEMENTS)                  \
  typedef enum _##TYPENAME{ELEMENTS(CLOG_ENUM_ELEMENT)} TYPENAME; \
  const char *const TYPENAME##Names[0 ELEMENTS(CLOG_ENUM_COUNT)] = {ELEMENTS(CLOG_ENUM_NAME)};

/**
 * Structure containing all parameters for a single
 * log message.
 */
typedef struct _CLogMessage {
  const char *file;        ///< The name of the file in which the log message is being
                           ///< produced.
  const unsigned int line; ///< The line number where the log message is produced.
  const char *function;    ///< The function in which the log message is being produced.
  const char *message;     ///< The formatted message.
  const CLogLevel level;   ///< The log level.
  const char *tag;         ///< The tag of the message.
} CLogMessage;

/**
 * Function pointer prototype for message backends.
 * @param message the message to be handled by the backend.
 */
typedef void (*LogAdapterOnMessage)(const CLogMessage *message);

/**
 * Function pointer prototype for message filters.
 * @param message The message to be filtered or not.
 * @return true if the message shall be passed to the backend, false if it shall
 * be surpressed.
 */
typedef bool (*LogAdapterFilter)(const CLogMessage *message);

/**
 * Structure containing all parameters for a log adapter.
 * Use the messageFilter to change the messages passed to the backend
 * during runtime.
 */
typedef struct _CLogAdapter {
  LogAdapterFilter messageFilter; /**< Pointer to the filter function. Filters can be used to supress messages
                                       dynamically (can be changed during runtime). Can be set to NULL if not needed. */
  LogAdapterOnMessage onMessage;  /**< Pointer to the backend function. Must not be NULL. The function is called to
                                       process (e.g. print to stdout) the messages. */
} CLogAdapter;

/**
 * Structure defining a log context. It holds the internal state for
 * one logging instance. A single context cannot be used in a multithreaded
 * environment without external synchronisation as the message buffer is
 * being reused for all messages. So if there are multiple messages logged
 * at the "same" time the message buffer might get corrupted.
 *
 * Different tags can be used to separate log messages by their origin. E.g. a
 * communication module and a storage module can use different tags in their messages.
 */
typedef struct _CLogContext {
  const CLogAdapter *const adapters; /**< Pointer to the array of log adapters. At least one is needed. */
  const size_t adaptersSize;         /**< The size of the array of log adapters. */
  const char *const *tagNames;       /**< The array containing all tag names. Can be set to NULL if not needed. */
  const size_t numberOfTags;         /**< The size of the array containing the tag names. */
  CLogLevel minLevel;                /**< The minimum log level at runtime (messages with a
                                          lower level will be discarded). */
  char *const messageBuffer;         /**< The char buffer to hold the formatted message
                                          (message text and message parameters). */
  const size_t messageBufferSize;    /**< The size of the message buffer. */
} CLogContext;

/**
 * @def VA_ARGS
 * Internal macro needed to handle variable numbers of arguments in the log macros
 */
#define VA_ARGS(...) , ##__VA_ARGS__

/**
 * @def CLOG_MESSAGE
 * @param CTX     The log context to be used.
 * @param LEVEL   The log level of the message.
 * @param TAG     The tag of the message.
 * @param MESSAGE The message text / format string
 * @param ...     The additional parameters to be used when formatting the message.
 * The macro to log an arbitrary message.
 */
#define CLOG_MESSAGE(CTX, LEVEL, TAG, MESSAGE, ...)                                                  \
  if (LEVEL >= clog_getMinLevel(CTX)) {                                                              \
    clog_logMessage(CTX, LEVEL, TAG, CLOG_FILE, CLOG_LINE, CLOG_FUNC, MESSAGE VA_ARGS(__VA_ARGS__)); \
  }

#if CLOG_GLOBAL_MIN_LEVEL <= CLOG_MLTRC
/**
 * @def CLOG_TRC
 * @param CTX     The log context to be used.
 * @param TAG     The tag of the message.
 * @param MESSAGE The message text / format string
 * @param ...     The additional parameters to be used when formatting the message.
 * Macro to log a message at trace level.
 */
#define CLOG_TRC(CTX, TAG, MESSAGE, ...) CLOG_MESSAGE(CTX, CLOG_LTRC, TAG, MESSAGE VA_ARGS(__VA_ARGS__))
#else
#define CLOG_TRC(CTX, TAG, MESSAGE, ...)
#endif

#if CLOG_GLOBAL_MIN_LEVEL <= CLOG_MLDBG
/**
 * @def CLOG_DBG
 * @param CTX     The log context to be used.
 * @param TAG     The tag of the message.
 * @param MESSAGE The message text / format string
 * @param ...     The additional parameters to be used when formatting the message.
 * Macro to log a message at debug level.
 */
#define CLOG_DBG(CTX, TAG, MESSAGE, ...) CLOG_MESSAGE(CTX, CLOG_LDBG, TAG, MESSAGE VA_ARGS(__VA_ARGS__))
#else
#define CLOG_DBG(CTX, TAG, MESSAGE, ...)
#endif

#if CLOG_GLOBAL_MIN_LEVEL <= CLOG_MLINF
/**
 * @def CLOG_INF
 * @param CTX     The log context to be used.
 * @param TAG     The tag of the message.
 * @param MESSAGE The message text / format string
 * @param ...     The additional parameters to be used when formatting the message.
 * Macro to log a message at information level.
 */
#define CLOG_INF(CTX, TAG, MESSAGE, ...) CLOG_MESSAGE(CTX, CLOG_LINF, TAG, MESSAGE VA_ARGS(__VA_ARGS__))
#else
#define CLOG_INF(CTX, TAG, MESSAGE, ...)
#endif

#if CLOG_GLOBAL_MIN_LEVEL <= CLOG_MLWRN
/**
 * @def CLOG_WRN
 * @param CTX     The log context to be used.
 * @param TAG     The tag of the message.
 * @param MESSAGE The message text / format string
 * @param ...     The additional parameters to be used when formatting the message.
 * Macro to log a message at warning level.
 */
#define CLOG_WRN(CTX, TAG, MESSAGE, ...) CLOG_MESSAGE(CTX, CLOG_LWRN, TAG, MESSAGE VA_ARGS(__VA_ARGS__))
#else
#define CLOG_WRN(CTX, TAG, MESSAGE, ...)
#endif

#if CLOG_GLOBAL_MIN_LEVEL <= CLOG_MLERR
/**
 * @def CLOG_ERR
 * @param CTX     The log context to be used.
 * @param TAG     The tag of the message.
 * @param MESSAGE The message text / format string
 * @param ...     The additional parameters to be used when formatting the message.
 * Macro to log a message at error level.
 */
#define CLOG_ERR(CTX, TAG, MESSAGE, ...) CLOG_MESSAGE(CTX, CLOG_LERR, TAG, MESSAGE VA_ARGS(__VA_ARGS__))
#else
#define CLOG_ERR(CTX, TAG, MESSAGE, ...)
#endif

#if CLOG_GLOBAL_MIN_LEVEL <= CLOG_MLFTL
/**
 * @def CLOG_FTL
 * @param CTX     The log context to be used.
 * @param TAG     The tag of the message.
 * @param MESSAGE The message text / format string
 * @param ...     The additional parameters to be used when formatting the message.
 * Macro to log a message at fatal error level.
 */
#define CLOG_FTL(CTX, TAG, MESSAGE, ...) CLOG_MESSAGE(CTX, CLOG_LFTL, TAG, MESSAGE VA_ARGS(__VA_ARGS__))
#else
#define CLOG_FTL(CTX, TAG, MESSAGE, ...)
#endif

/**
 * @param ctx      The log context to be used.
 * @param level    The log level.
 * @param tag      The tag of the message.
 * @param file     The name of the file producing the log message.
 * @param line     The line number within the file.
 * @param function The name of the function that is producing the log message.
 * @param message  The message text / format string
 * @param ...      The additional parameters to be used when formatting the message.
 * The log function. Avoid using this function directly. Prefer the macros above as they are filling in the basic
 * parameters like e.g. file name.
 */

void clog_logMessage(CLogContext *const ctx,
                     const CLogLevel level,
                     const size_t tag,
                     const char *const file,
                     const unsigned int line,
                     const char *const function,
                     const char *const message,
                     ...);
/**
 * @param level        The log level.
 * @return const char* The text representation of the log level.
 * Resolves the text representation of a log level.
 */
const char *clog_getLevel(const CLogLevel level);

#ifdef CLOG_COLOR
/**
 * @param level        The log level.
 * @return const char* The color code.
 * Returns the string that switches the output color for the given level.
 */
const char *clog_getColor(const CLogLevel level);
#endif

/**
 * Formats the line header (The first part containing information about the file, function, ...) of a message. In case
 * of any invalid parameters (e.g. null pointers) it will silently return. The length of the buffer must be at least one
 * character. The function ensures a terminating null byte either at the end of the formatted line header or in the last
 * character of the buffer. You can use this function if you don't want to implement your own formatting of the line
 * header.
 * @param buffer        The buffer to fill the line header into.
 * @param bufferLength  In: The maximum size of buffer. Out: The number of characters actually used.
 * @param msg           The message to be formatted.
 */
void clog_formatLineHeader(char buffer[], int *const bufferLength, const CLogMessage *const msg);

/**
 * Formats a complete message line including the header. A terminating end of line character `\n` is added. In case
 * of any invalid parameters (e.g. null pointers) it will silently return. The length of the buffer must be at least two
 * characters. The function ensures a terminating null byte either at the end of the formatted message or in the
 * last character of the buffer.
 * You can use this function if you don't want to implement your own formatter.
 *
 * @param buffer        The buffer to fill the message into.
 * @param bufferLength  In: The maximum size of buffer. Out: The number of characters actually used.
 * @param msg           The message to be formatted.
 */
void clog_formatMessage(char buffer[], int *const bufferLength, const CLogMessage *const msg);

/**
 * Returns the minimum log level for the given context. If no context is given
 * the function will return trace (so gracefully all messages will be handled).
 *
 * @param ctx The pointer to the context.
 * @return CLogLevel The minimum level for messages to be handled in this
 * context.
 */
CLogLevel clog_getMinLevel(CLogContext *ctx);

/**
 * Sets the minimum log level for the given context.
 * All messages with a lower level shall not be handed to the backend.
 *
 * @param ctx   The context.
 * @param level The new minimum level.
 */
void clog_setMinLevel(CLogContext *ctx, CLogLevel level);

/**
 * Function that checks a log context. Returns true if all requirements are met.
 *
 * @param ctx  Pointer to the context
 * @return true If all fields in the context are initialized correctly.
 * @return false If any of the fields is not set up properly.
 */
bool clog_checkContext(const CLogContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_CLOG_H_ */
