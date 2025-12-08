/* Options parsing and conversion implementation */
#include "options.h"

/* Symbol constants for option keys */
static ID id_headers;
static ID id_col_sep;
static ID id_quote_char;
static ID id_skip_lines;
static ID id_encoding;
static ID id_liberal_parsing;
static ID id_buffer_size;

void zsv_options_init_symbols(void)
{
    id_headers = rb_intern("headers");
    id_col_sep = rb_intern("col_sep");
    id_quote_char = rb_intern("quote_char");
    id_skip_lines = rb_intern("skip_lines");
    id_encoding = rb_intern("encoding");
    id_liberal_parsing = rb_intern("liberal_parsing");
    id_buffer_size = rb_intern("buffer_size");
}

void zsv_options_init(zsv_ruby_options_t *opts)
{
    opts->delimiter = ',';
    opts->quote_char = '"';
    opts->headers = false;
    opts->header_array = Qnil;
    opts->skip_lines = 0;
    opts->liberal_parsing = false;
    opts->encoding = rb_utf8_encoding();
    opts->buffer_size = 256 * 1024; /* 256KB default */
}

static VALUE get_option(VALUE opts_hash, ID key, VALUE default_value)
{
    if (NIL_P(opts_hash)) {
        return default_value;
    }

    VALUE val = rb_hash_lookup(opts_hash, ID2SYM(key));
    return NIL_P(val) ? default_value : val;
}

void zsv_options_parse(VALUE opts_hash, zsv_ruby_options_t *opts)
{
    /* Initialize with defaults */
    zsv_options_init(opts);

    if (NIL_P(opts_hash)) {
        return;
    }

    Check_Type(opts_hash, T_HASH);

    /* Parse col_sep */
    VALUE col_sep = get_option(opts_hash, id_col_sep, Qnil);
    if (!NIL_P(col_sep)) {
        Check_Type(col_sep, T_STRING);
        if (RSTRING_LEN(col_sep) != 1) {
            rb_raise(rb_eArgError, "col_sep must be a single character");
        }
        opts->delimiter = RSTRING_PTR(col_sep)[0];
    }

    /* Parse quote_char */
    VALUE quote_char = get_option(opts_hash, id_quote_char, Qnil);
    if (!NIL_P(quote_char)) {
        Check_Type(quote_char, T_STRING);
        if (RSTRING_LEN(quote_char) != 1) {
            rb_raise(rb_eArgError, "quote_char must be a single character");
        }
        opts->quote_char = RSTRING_PTR(quote_char)[0];
    }

    /* Parse headers */
    VALUE headers = get_option(opts_hash, id_headers, Qfalse);
    if (TYPE(headers) == T_ARRAY) {
        opts->headers = true;
        opts->header_array = headers;
    } else {
        opts->headers = RTEST(headers);
    }

    /* Parse skip_lines */
    VALUE skip_lines = get_option(opts_hash, id_skip_lines, INT2FIX(0));
    opts->skip_lines = NUM2INT(skip_lines);

    /* Parse liberal_parsing */
    VALUE liberal = get_option(opts_hash, id_liberal_parsing, Qfalse);
    opts->liberal_parsing = RTEST(liberal);

    /* Parse encoding */
    VALUE encoding = get_option(opts_hash, id_encoding, Qnil);
    if (!NIL_P(encoding)) {
        opts->encoding = rb_to_encoding(encoding);
    }

    /* Parse buffer_size */
    VALUE buffer_size = get_option(opts_hash, id_buffer_size, Qnil);
    if (!NIL_P(buffer_size)) {
        opts->buffer_size = NUM2SIZET(buffer_size);
    }
}

void zsv_options_free(zsv_ruby_options_t *opts)
{
    (void)opts; /* Currently no dynamic allocations to free */
}

void zsv_options_apply(zsv_parser parser, zsv_ruby_options_t *opts)
{
    /* Note: zsv_set_delimiter doesn't exist in zsv API */
    /* The delimiter is set during parser creation via zsv_opts */
    /* Additional zsv-specific option application would go here */
    (void)parser; /* Suppress unused parameter warning */
    (void)opts;
}

/* Initialize symbols (call from Init_zsv) */
void Init_zsv_options(void)
{
    zsv_options_init_symbols();
}
