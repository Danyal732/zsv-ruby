/* Common header for zsv Ruby extension */
#ifndef ZSV_RUBY_COMMON_H
#define ZSV_RUBY_COMMON_H

#include <ruby.h>
#include <ruby/encoding.h>
#include <stdbool.h>
#include <zsv.h>

/* Module and class references */
extern VALUE mZSV;
extern VALUE cParser;
extern VALUE eZSVError;
extern VALUE eMalformedCSVError;
extern VALUE eInvalidEncodingError;

/* Error handling macros */
#define RAISE_ZSV_ERROR(msg) rb_raise(eZSVError, "%s", (msg))
#define RAISE_MALFORMED_CSV(msg) rb_raise(eMalformedCSVError, "%s", (msg))
#define RAISE_INVALID_ENCODING(msg) rb_raise(eInvalidEncodingError, "%s", (msg))

/* Memory management macros - use Ruby's built-in */
#define ZSV_ALLOC(type) RB_ALLOC(type)
#define ZSV_ALLOC_N(type, n) RB_ALLOC_N(type, n)
#define ZSV_REALLOC_N(var, type, n) RB_REALLOC_N(var, type, n)

/* Debug logging (disabled in production) */
#ifdef DEBUG
#define LOG_DEBUG(fmt, ...) fprintf(stderr, "[ZSV DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...) ((void)0)
#endif

#endif /* ZSV_RUBY_COMMON_H */
