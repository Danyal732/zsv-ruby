/* Row handling and conversion */
#ifndef ZSV_RUBY_ROW_H
#define ZSV_RUBY_ROW_H

#include "common.h"

/* Row builder for efficient conversion from zsv cells to Ruby arrays/hashes */
typedef struct {
    VALUE *cells;          /* Array of Ruby string cells */
    size_t count;          /* Number of cells in current row */
    size_t capacity;       /* Allocated capacity */
    VALUE headers;         /* Header array (for hash mode) */
    rb_encoding *encoding; /* String encoding */
} zsv_row_builder_t;

/* Initialize row builder */
zsv_row_builder_t *zsv_row_builder_new(rb_encoding *encoding);

/* Free row builder */
void zsv_row_builder_free(zsv_row_builder_t *builder);

/* Reset row builder for next row */
void zsv_row_builder_reset(zsv_row_builder_t *builder);

/* Add cell to current row */
void zsv_row_builder_add_cell(zsv_row_builder_t *builder, const unsigned char *data, size_t length);

/* Build Ruby array from current row */
VALUE zsv_row_builder_to_array(zsv_row_builder_t *builder);

/* Build Ruby hash from current row (requires headers) */
VALUE zsv_row_builder_to_hash(zsv_row_builder_t *builder);

/* Set headers for hash mode */
void zsv_row_builder_set_headers(zsv_row_builder_t *builder, VALUE headers);

#endif /* ZSV_RUBY_ROW_H */
