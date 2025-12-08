/* Row handling and conversion implementation */
#include "row.h"

#define INITIAL_ROW_CAPACITY 32

zsv_row_builder_t *zsv_row_builder_new(rb_encoding *encoding)
{
    zsv_row_builder_t *builder = ZSV_ALLOC(zsv_row_builder_t);

    builder->capacity = INITIAL_ROW_CAPACITY;
    builder->cells = ZSV_ALLOC_N(VALUE, builder->capacity);
    builder->count = 0;
    builder->headers = Qnil;
    builder->header_cache = NULL;
    builder->header_count = 0;
    builder->encoding = encoding;

    return builder;
}

void zsv_row_builder_free(zsv_row_builder_t *builder)
{
    if (builder) {
        xfree(builder->cells);
        if (builder->header_cache) {
            xfree(builder->header_cache);
        }
        xfree(builder);
    }
}

void zsv_row_builder_reset(zsv_row_builder_t *builder)
{
    builder->count = 0;
}

static void zsv_row_builder_ensure_capacity(zsv_row_builder_t *builder, size_t needed)
{
    if (needed <= builder->capacity) {
        return;
    }

    /* Double capacity until we have enough */
    size_t new_capacity = builder->capacity;
    while (new_capacity < needed) {
        new_capacity *= 2;
    }

    ZSV_REALLOC_N(builder->cells, VALUE, new_capacity);
    builder->capacity = new_capacity;
}

void zsv_row_builder_add_cell(zsv_row_builder_t *builder, const unsigned char *data, size_t length)
{
    zsv_row_builder_ensure_capacity(builder, builder->count + 1);

    /* Create Ruby string with proper encoding */
    VALUE str = rb_enc_str_new((const char *)data, length, builder->encoding);

    /* Freeze string for memory efficiency (shared strings) */
    rb_str_freeze(str);

    builder->cells[builder->count++] = str;
}

VALUE zsv_row_builder_to_array(zsv_row_builder_t *builder)
{
    VALUE row = rb_ary_new_from_values(builder->count, builder->cells);
    return row;
}

VALUE zsv_row_builder_to_hash(zsv_row_builder_t *builder)
{
    if (NIL_P(builder->headers)) {
        rb_raise(rb_eRuntimeError, "Headers not set for hash conversion");
    }

    /* Use cached header count for efficiency */
    size_t header_count = builder->header_count;
    size_t cell_count = builder->count;

    /* Pre-allocate hash with expected size (Ruby 3.2+) */
#ifdef HAVE_RB_HASH_NEW_CAPA
    VALUE hash = rb_hash_new_capa(cell_count);
#else
    VALUE hash = rb_hash_new();
#endif

    /* Fast path: use cached header pointers (no rb_ary_entry calls) */
    size_t matched = (cell_count < header_count) ? cell_count : header_count;
    for (size_t i = 0; i < matched; i++) {
        rb_hash_aset(hash, builder->header_cache[i], builder->cells[i]);
    }

    /* Rare case: more cells than headers, add with numeric keys */
    for (size_t i = header_count; i < cell_count; i++) {
        rb_hash_aset(hash, SIZET2NUM(i), builder->cells[i]);
    }

    return hash;
}

void zsv_row_builder_set_headers(zsv_row_builder_t *builder, VALUE headers)
{
    Check_Type(headers, T_ARRAY);
    builder->headers = headers;

    /* Cache header count and pointers for fast access in to_hash */
    builder->header_count = RARRAY_LEN(headers);

    /* Free old cache if exists */
    if (builder->header_cache) {
        xfree(builder->header_cache);
    }

    /* Allocate and populate header cache */
    builder->header_cache = ZSV_ALLOC_N(VALUE, builder->header_count);
    const VALUE *header_ptr = RARRAY_CONST_PTR(headers);
    for (size_t i = 0; i < builder->header_count; i++) {
        builder->header_cache[i] = header_ptr[i];
    }
}
