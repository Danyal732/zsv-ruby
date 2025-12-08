/* Parser wrapper implementation using zsv's callback API */
#include "parser.h"
#include <errno.h>
#include <string.h>

/* Forward declarations */
static void zsv_parser_init_common(zsv_ruby_parser_t *parser, VALUE opts_hash);
static void zsv_parser_finish_safe(zsv_ruby_parser_t *parser);

/* Row handler callback for zsv */
static void zsv_row_handler(void *ctx) {
    zsv_ruby_parser_t *parser = (zsv_ruby_parser_t *)ctx;

    /* Don't allocate Ruby objects during cleanup/GC */
    if (parser->in_cleanup) {
        return;
    }

    /* Reset row builder for new row */
    zsv_row_builder_reset(parser->row_builder);

    /* Get all cells in this row */
    size_t cell_count = zsv_cell_count(parser->zsv);
    for (size_t i = 0; i < cell_count; i++) {
        struct zsv_cell cell = zsv_get_cell(parser->zsv, i);
        zsv_row_builder_add_cell(parser->row_builder, cell.str, cell.len);
    }

    /* Skip lines if configured */
    if (parser->lines_skipped < (size_t)parser->options.skip_lines) {
        parser->lines_skipped++;
        return;
    }

    /* Process header row - only if headers:true and no custom headers provided */
    if (parser->options.headers &&
        NIL_P(parser->options.header_array) &&
        !parser->header_row_processed) {
        parser->headers = zsv_row_builder_to_array(parser->row_builder);
        zsv_row_builder_set_headers(parser->row_builder, parser->headers);
        parser->header_row_processed = true;
        return;  /* Don't store header row */
    }

    /* Build row as array or hash */
    VALUE row;
    if (!NIL_P(parser->headers)) {
        row = zsv_row_builder_to_hash(parser->row_builder);
    } else {
        row = zsv_row_builder_to_array(parser->row_builder);
    }

    /* Add row to buffer */
    rb_ary_push(parser->row_buffer, row);
    parser->row_count++;
}

/* Initialize common parser setup */
static void zsv_parser_init_common(zsv_ruby_parser_t *parser, VALUE opts_hash) {
    /* Parse options */
    zsv_options_parse(opts_hash, &parser->options);

    /* Create row builder */
    parser->row_builder = zsv_row_builder_new(parser->options.encoding);

    /* Initialize zsv parser with callback */
    struct zsv_opts zopts = {0};
    zopts.delimiter = parser->options.delimiter;
    zopts.row_handler = zsv_row_handler;
    zopts.ctx = parser;

    if (parser->file) {
        zopts.stream = parser->file;
        parser->zsv = zsv_new(&zopts);
    } else {
        /* IO object handling would require custom read callback */
        rb_raise(rb_eNotImpError, "IO object parsing not yet implemented");
    }

    /* Set headers: use custom headers if provided, otherwise will be read from first row */
    if (!NIL_P(parser->options.header_array)) {
        parser->headers = parser->options.header_array;
        zsv_row_builder_set_headers(parser->row_builder, parser->headers);
        parser->header_row_processed = true;  /* Skip reading headers from file */
    } else {
        parser->headers = Qnil;
    }

    parser->row_buffer = rb_ary_new();
    parser->closed = false;
    parser->eof_reached = false;
    parser->in_cleanup = false;
    parser->current_row = Qnil;
}

/* Create parser from file path */
zsv_ruby_parser_t *zsv_parser_new_from_path(const char *path, VALUE opts_hash) {
    zsv_ruby_parser_t *parser = ZSV_ALLOC(zsv_ruby_parser_t);
    memset(parser, 0, sizeof(zsv_ruby_parser_t));

    /* Open file */
    parser->file = fopen(path, "rb");
    if (!parser->file) {
        xfree(parser);
        rb_sys_fail(path);
    }

    zsv_parser_init_common(parser, opts_hash);
    return parser;
}

/* Create parser from Ruby IO */
zsv_ruby_parser_t *zsv_parser_new_from_io(VALUE io, VALUE opts_hash) {
    zsv_ruby_parser_t *parser = ZSV_ALLOC(zsv_ruby_parser_t);
    memset(parser, 0, sizeof(zsv_ruby_parser_t));

    parser->io = io;
    zsv_parser_init_common(parser, opts_hash);

    return parser;
}

/* Create parser from string */
zsv_ruby_parser_t *zsv_parser_new_from_string(VALUE string, VALUE opts_hash) {
    Check_Type(string, T_STRING);

    zsv_ruby_parser_t *parser = ZSV_ALLOC(zsv_ruby_parser_t);
    memset(parser, 0, sizeof(zsv_ruby_parser_t));

    /* Create memory-backed file */
    const char *str_ptr = RSTRING_PTR(string);
    size_t str_len = RSTRING_LEN(string);

    parser->file = fmemopen((void *)str_ptr, str_len, "rb");
    if (!parser->file) {
        xfree(parser);
        rb_raise(rb_eRuntimeError, "Failed to create memory stream");
    }

    zsv_parser_init_common(parser, opts_hash);
    return parser;
}

/* Free parser */
void zsv_parser_free(zsv_ruby_parser_t *parser) {
    if (!parser) return;

    zsv_parser_close(parser);

    if (parser->row_builder) {
        zsv_row_builder_free(parser->row_builder);
    }

    if (parser->error_message) {
        xfree(parser->error_message);
    }

    zsv_options_free(&parser->options);
    xfree(parser);
}

/* Parse next row - pull-based interface */
VALUE zsv_parser_shift(zsv_ruby_parser_t *parser) {
    if (parser->closed) {
        return Qnil;
    }

    /* If we have buffered rows, return the first one */
    if (RARRAY_LEN(parser->row_buffer) > 0) {
        return rb_ary_shift(parser->row_buffer);
    }

    /* If we've reached EOF, no more rows */
    if (parser->eof_reached) {
        return Qnil;
    }

    /* Parse more data */
    enum zsv_status status = zsv_parse_more(parser->zsv);

    /* Check for errors */
    if (status != zsv_status_ok && status != zsv_status_no_more_input) {
        RAISE_MALFORMED_CSV("CSV parsing error");
    }

    /* If EOF reached, finish to flush final row */
    if (status == zsv_status_no_more_input && !parser->eof_reached) {
        /* Call finish() BEFORE marking EOF to flush any pending row */
        zsv_parser_finish_safe(parser);
        parser->eof_reached = true;
    }

    /* Return first buffered row, or nil if buffer is empty */
    if (RARRAY_LEN(parser->row_buffer) > 0) {
        return rb_ary_shift(parser->row_buffer);
    }

    return Qnil;  /* EOF or no rows yet */
}

/* Parse all rows with block */
void zsv_parser_each(zsv_ruby_parser_t *parser) {
    VALUE row;
    while (!NIL_P(row = zsv_parser_shift(parser))) {
        rb_yield(row);
    }
}

/* Rewind parser */
void zsv_parser_rewind(zsv_ruby_parser_t *parser) {
    if (!parser->file) {
        rb_raise(rb_eIOError, "Cannot rewind non-file parser");
    }

    rewind(parser->file);
    zsv_finish(parser->zsv);
    zsv_delete(parser->zsv);

    /* Reinitialize parser */
    struct zsv_opts zopts = {0};
    zopts.delimiter = parser->options.delimiter;
    zopts.row_handler = zsv_row_handler;
    zopts.ctx = parser;
    zopts.stream = parser->file;

    parser->zsv = zsv_new(&zopts);

    parser->row_count = 0;
    parser->lines_skipped = 0;
    parser->header_row_processed = false;
    parser->eof_reached = false;
    parser->in_cleanup = false;
    parser->row_buffer = rb_ary_new();
    parser->current_row = Qnil;
}

/* Finish parsing safely (flush final row if needed) */
static void zsv_parser_finish_safe(zsv_ruby_parser_t *parser) {
    if (!parser->zsv) {
        return;  /* No parser */
    }

    /* Only finish if we can allocate (not in GC) */
    if (!parser->in_cleanup) {
        zsv_finish(parser->zsv);
    }
}

/* Close parser */
void zsv_parser_close(zsv_ruby_parser_t *parser) {
    if (!parser || parser->closed) return;

    /* Mark as closed first to prevent re-entry */
    parser->closed = true;

    if (parser->zsv) {
        /* Finish parsing to flush any final row */
        zsv_parser_finish_safe(parser);

        /* Mark cleanup to prevent further allocations */
        parser->in_cleanup = true;

        /* Delete parser - frees internal buffers */
        zsv_delete(parser->zsv);
        parser->zsv = NULL;
    }

    if (parser->file) {
        fclose(parser->file);
        parser->file = NULL;
    }
}

/* Get headers */
VALUE zsv_parser_headers(zsv_ruby_parser_t *parser) {
    return parser->headers;
}

/* Check if closed */
bool zsv_parser_closed(zsv_ruby_parser_t *parser) {
    return parser->closed;
}

/* Get row count */
size_t zsv_parser_row_count(zsv_ruby_parser_t *parser) {
    return parser->row_count;
}
