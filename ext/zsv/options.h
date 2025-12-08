/* Options parsing and conversion */
#ifndef ZSV_RUBY_OPTIONS_H
#define ZSV_RUBY_OPTIONS_H

#include "common.h"

/* Parser options structure */
typedef struct {
    char delimiter;
    char quote_char;
    bool headers;
    VALUE header_array;  /* Custom headers as Ruby array */
    int skip_lines;
    bool liberal_parsing;
    rb_encoding *encoding;
    size_t buffer_size;
} zsv_ruby_options_t;

/* Initialize options with defaults */
void zsv_options_init(zsv_ruby_options_t *opts);

/* Parse Ruby hash into options structure */
void zsv_options_parse(VALUE opts_hash, zsv_ruby_options_t *opts);

/* Free any allocated option resources */
void zsv_options_free(zsv_ruby_options_t *opts);

/* Apply options to zsv parser */
void zsv_options_apply(zsv_parser parser, zsv_ruby_options_t *opts);

#endif /* ZSV_RUBY_OPTIONS_H */
