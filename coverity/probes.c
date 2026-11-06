/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Analyzer-only acceptance probes; do not run the intentionally invalid ones.
   Analyze against strutil.c, with and without the uploaded model. */
#include "strutil.h"
#include "code78k_common.h"

#include <string.h>

extern void consume_string(const char* text);

void coverity_probe_78k_mem_tail(int count) {
    char dest[2];
    char const source[3] = {'a', 'b', 'c'};
    memcpy(dest, source + 1, as_78k_mem_tail_count(count)); /* No overrun. */
}

void coverity_probe_78k_mem_tail_overrun(void) {
    char dest[1];
    char const source[3] = {'a', 'b', 'c'};
    /* Must retain an overrun: count 3 means two bytes, not one. */
    memcpy(dest, source + 1, as_78k_mem_tail_count(3));
}

void coverity_probe_banner(void) {
    String tmp;
    as_snprintf(tmp, sizeof(tmp), "(%s-%s)", "x86_64", "Linux");
    consume_string(tmp); /* No STRING_NULL expected. */
}

void coverity_probe_one_byte(void) {
    char tmp[1];
    as_snprintf(tmp, sizeof(tmp), "(%s-%s)", "x86_64", "Linux");
    consume_string(tmp); /* Empty, terminated string. */
}

void coverity_probe_zero_capacity(void) {
    as_snprintf(0, 0, 0); /* No access, no null-dereference diagnostic. */
}

void coverity_probe_null_destination(void) {
    as_snprintf(0, 32, "%s", "text"); /* Must retain a null-write diagnostic. */
}

void coverity_probe_oversized_capacity(void) {
    char tmp[16];
    as_snprintf(tmp, 1024, "%s", "text that does not fit in sixteen bytes");
    consume_string(tmp); /* Must retain a destination-bounds diagnostic. */
}

void coverity_probe_unterminated_format(void) {
    char tmp[32];
    char format[3] = {'a', 'b', 'c'};
    as_snprintf(tmp, sizeof(tmp), format, 0); /* Must retain STRING_NULL. */
}

void coverity_probe_unterminated_argument(void) {
    char tmp[32];
    char text[3] = {'a', 'b', 'c'};
    /* The candidate does not parse conversions: verify this is still found
       before accepting it for general use. */
    as_snprintf(tmp, sizeof(tmp), "%s", text);
}
