/* Parser wrapper around zsv */
#ifndef ZSV_RUBY_PARSER_H
#define ZSV_RUBY_PARSER_H

#include "common.h"
#include "options.h"
#include "row.h"

/* Parser state structure */
typedef struct {
    zsv_parser zsv;                 /* zsv parser instance */
    FILE *file;                     /* File handle (if parsing file) */
    VALUE io;                       /* Ruby IO object (if parsing IO) */
    zsv_ruby_options_t options;     /* Parser options */
    zsv_row_builder_t *row_builder; /* Row builder */
    VALUE current_row;              /* Current row being built */
    VALUE row_buffer;               /* Buffer of parsed rows (Ruby array) */
    VALUE headers;                  /* Parsed headers (if header mode) */
    size_t row_count;               /* Number of rows parsed */
    size_t lines_skipped;           /* Number of lines skipped */
    bool header_row_processed;      /* Whether header row was processed */
    bool closed;                    /* Whether parser is closed */
    bool eof_reached;               /* Whether EOF was reached */
    bool in_cleanup;                /* True during zsv_finish cleanup */
    int error_code;                 /* Last error code */
    char *error_message;            /* Last error message */
} zsv_ruby_parser_t;

/* Create new parser from file path */
zsv_ruby_parser_t *zsv_parser_new_from_path(const char *path,
                                              VALUE opts_hash);

/* Create new parser from Ruby IO object */
zsv_ruby_parser_t *zsv_parser_new_from_io(VALUE io, VALUE opts_hash);

/* Create new parser from string */
zsv_ruby_parser_t *zsv_parser_new_from_string(VALUE string, VALUE opts_hash);

/* Free parser and associated resources */
void zsv_parser_free(zsv_ruby_parser_t *parser);

/* Parse next row, returns Ruby array or hash */
VALUE zsv_parser_shift(zsv_ruby_parser_t *parser);

/* Parse all rows, yielding to block */
void zsv_parser_each(zsv_ruby_parser_t *parser);

/* Rewind parser to beginning */
void zsv_parser_rewind(zsv_ruby_parser_t *parser);

/* Close parser */
void zsv_parser_close(zsv_ruby_parser_t *parser);

/* Get headers */
VALUE zsv_parser_headers(zsv_ruby_parser_t *parser);

/* Check if parser is closed */
bool zsv_parser_closed(zsv_ruby_parser_t *parser);

/* Get row count */
size_t zsv_parser_row_count(zsv_ruby_parser_t *parser);

#endif /* ZSV_RUBY_PARSER_H */
