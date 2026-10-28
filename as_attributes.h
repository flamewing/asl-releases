/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#ifndef AS_ATTRIBUTES_H
#define AS_ATTRIBUTES_H

#ifdef __cplusplus
#    define AS_NORETURN [[noreturn]]
#else
#    define AS_NORETURN _Noreturn
#endif

/* Explicitly mark intentional transitions between switch cases. */
#if defined(__has_attribute)
#    if __has_attribute(fallthrough)
#        define AS_FALLTHROUGH __attribute__((fallthrough))
#    endif
#endif
#ifndef AS_FALLTHROUGH
#    define AS_FALLTHROUGH ((void)0)
#endif

/* A returned pointer/view borrows storage reachable through this parameter. */
#if defined(__has_attribute)
#    if __has_attribute(lifetimebound)
#        define AS_LIFETIMEBOUND __attribute__((lifetimebound))
#    endif
#endif
#ifndef AS_LIFETIMEBOUND
#    define AS_LIFETIMEBOUND
#endif

#if defined(__GNUC__)
#    define AS_MAYBE_UNUSED __attribute__((unused))
#    define AS_MALLOC __attribute__((malloc))
#    define AS_ALLOC_SIZE(count, size) __attribute__((alloc_size(count, size)))
#else
#    define AS_MAYBE_UNUSED
#    define AS_MALLOC
#    define AS_ALLOC_SIZE(count, size)
#endif

#endif
