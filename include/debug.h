#ifndef __DEBUG_H__
#define __DEBUG_H__

#define LOG_FMT "(%s:%d) %s "

#define ERROR_BIT (0x8000000)

#define LOW (0x1)
#define MED (0x2)
#define HIGH (0x3)
#define CRITICAL (0x4)

#define ERROR(x)    (x|ERROR_BIT)
#define IS_ERROR(x) (x&ERROR_BIT)

#define LEVEL(x)    ((~ERROR_BIT)&x)

#define CSTR(x)     (x.toStdString().c_str())

/**
 * @def debug(level, fmt, ...)
 * Shortcut for log which automatically inserts the file and line.
 */
#define debug(level, ...) log(level, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @fn void set_debug_level(int level)
 * Used to set the current debug level of the application.
 * This will restrict the printed messaged to those meeting the specified
 * target level or being more critical (higher number).
 * If the set level has the ERROR_BIT set, then any call to log without the
 * ERROR_BIT being set will be rejected.
 *
 * @param level Level to restrict the debug output to.
 */
void set_debug_level(int level);

/**
 * @fn int log(int level, char* file, int line, char* fmt, ...)
 * Logs to stdout or stderr, depending on ERROR_BIT.  Will print the line
 * and file associated with a given call, if specified.
 * @param level Level associated with this print.
 * @param file String name of the file in which this print is occuring.
 * @param line Line number of the line on which this print is occuring.
 * @param fmt The desired output string.
 * @param ... The values to be placed in the fmt string.
 *
 * @return The number of bytes printed; -1 if the call was rejected.
 */
int log(int level, const char* file, int line, const char* fmt, ...);

#endif /* __DEBUG_H__ */
