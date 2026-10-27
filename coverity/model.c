/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Coverity model source, NOT application code. Upload this file separately.
   Candidate: requires validation with the hosted analyzer; see README.md.
   No headers: the Scan modeling-file compiler processes this standalone. */

#if defined(__COVERITY_SIZE_TYPE__)
typedef __COVERITY_SIZE_TYPE__ size_t;
#elif defined(__SIZE_TYPE__)
typedef __SIZE_TYPE__ size_t;
#else
#error Define size_t to match the captured target before compiling this model
#endif

void __coverity_panic__(void);
void __coverity_write_buffer_bytes__(void* buffer, unsigned size);
void __coverity_string_null_sink__(const char* string);
void __coverity_read_buffer_bytes__(const void* buffer, unsigned size);
void __coverity_tainted_data_transitive__(void* dest, const void* source);
void __coverity_tainted_data_transitive_vararg_inbound__(unsigned dest, unsigned first);

int as_snprintf(char* dest, size_t capacity, const char* format, ...) {
    /* An uninitialized scalar represents an unknown value in model source.
       This is deliberately not an executable replacement implementation. */
    int written;

    /* No access to dest, format, or varargs on the zero-capacity path. */
    if (capacity == 0) {
        return 0;
    }
    if (capacity == sizeof(char*)) {
        __coverity_panic__();
    }

    __coverity_read_buffer_bytes__(format, 1);
    __coverity_string_null_sink__(format);

    /* The real formatter returns characters stored, NOT libc's would-have-
       written count. Comparisons avoid arithmetic in model constraints. */
    if (written < 0 || (size_t)written >= capacity) {
        __coverity_panic__();
    }
    __coverity_write_buffer_bytes__(dest, (unsigned)written);
    __coverity_tainted_data_transitive__(dest, format);
    /* Zero-based positions: dest is 0; the first variadic argument is 3. */
    __coverity_tainted_data_transitive_vararg_inbound__(0, 3);
    dest[written] = '\0';
    return written;
}
