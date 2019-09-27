// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright 2011, 2015 Red Hat, Inc.
 */

#ifndef NM_VERSION_H
#define NM_VERSION_H

#include <glib.h>

#include "nm-version-macros.h"

/* Deprecation / Availability macros */

#if !defined (NM_VERSION_MIN_REQUIRED) || (NM_VERSION_MIN_REQUIRED == 0)
# undef NM_VERSION_MIN_REQUIRED
# define NM_VERSION_MIN_REQUIRED (NM_API_VERSION)
#endif

#if !defined (NM_VERSION_MAX_ALLOWED) || (NM_VERSION_MAX_ALLOWED == 0)
# undef NM_VERSION_MAX_ALLOWED
# define NM_VERSION_MAX_ALLOWED (NM_API_VERSION)
#endif

/* sanity checks */
#if NM_VERSION_MIN_REQUIRED > NM_API_VERSION
#error "NM_VERSION_MIN_REQUIRED must be <= NM_API_VERSION"
#endif
#if NM_VERSION_MAX_ALLOWED < NM_VERSION_MIN_REQUIRED
#error "NM_VERSION_MAX_ALLOWED must be >= NM_VERSION_MIN_REQUIRED"
#endif
#if NM_VERSION_MIN_REQUIRED < NM_VERSION_0_9_8
#error "NM_VERSION_MIN_REQUIRED must be >= NM_VERSION_0_9_8"
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_0_9_10
# define NM_DEPRECATED_IN_0_9_10        G_DEPRECATED
# define NM_DEPRECATED_IN_0_9_10_FOR(f) G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_0_9_10
# define NM_DEPRECATED_IN_0_9_10_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_0_9_10
# define NM_AVAILABLE_IN_0_9_10         G_UNAVAILABLE(0.9,10)
#else
# define NM_AVAILABLE_IN_0_9_10
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_1_0
# define NM_DEPRECATED_IN_1_0           G_DEPRECATED
# define NM_DEPRECATED_IN_1_0_FOR(f)    G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_1_0
# define NM_DEPRECATED_IN_1_0_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_1_0
# define NM_AVAILABLE_IN_1_0            G_UNAVAILABLE(1,0)
#else
# define NM_AVAILABLE_IN_1_0
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_1_2
# define NM_DEPRECATED_IN_1_2           G_DEPRECATED
# define NM_DEPRECATED_IN_1_2_FOR(f)    G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_1_2
# define NM_DEPRECATED_IN_1_2_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_1_2
# define NM_AVAILABLE_IN_1_2            G_UNAVAILABLE(1,2)
#else
# define NM_AVAILABLE_IN_1_2
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_1_4
# define NM_DEPRECATED_IN_1_4           G_DEPRECATED
# define NM_DEPRECATED_IN_1_4_FOR(f)    G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_1_4
# define NM_DEPRECATED_IN_1_4_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_1_4
# define NM_AVAILABLE_IN_1_4            G_UNAVAILABLE(1,4)
#else
# define NM_AVAILABLE_IN_1_4
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_1_6
# define NM_DEPRECATED_IN_1_6           G_DEPRECATED
# define NM_DEPRECATED_IN_1_6_FOR(f)    G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_1_6
# define NM_DEPRECATED_IN_1_6_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_1_6
# define NM_AVAILABLE_IN_1_6            G_UNAVAILABLE(1,6)
#else
# define NM_AVAILABLE_IN_1_6
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_1_8
# define NM_DEPRECATED_IN_1_8           G_DEPRECATED
# define NM_DEPRECATED_IN_1_8_FOR(f)    G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_1_8
# define NM_DEPRECATED_IN_1_8_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_1_8
# define NM_AVAILABLE_IN_1_8            G_UNAVAILABLE(1,8)
#else
# define NM_AVAILABLE_IN_1_8
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_1_10
# define NM_DEPRECATED_IN_1_10           G_DEPRECATED
# define NM_DEPRECATED_IN_1_10_FOR(f)    G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_1_10
# define NM_DEPRECATED_IN_1_10_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_1_10
# define NM_AVAILABLE_IN_1_10            G_UNAVAILABLE(1,10)
#else
# define NM_AVAILABLE_IN_1_10
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_1_12
# define NM_DEPRECATED_IN_1_12           G_DEPRECATED
# define NM_DEPRECATED_IN_1_12_FOR(f)    G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_1_12
# define NM_DEPRECATED_IN_1_12_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_1_12
# define NM_AVAILABLE_IN_1_12            G_UNAVAILABLE(1,12)
#else
# define NM_AVAILABLE_IN_1_12
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_1_14
# define NM_DEPRECATED_IN_1_14           G_DEPRECATED
# define NM_DEPRECATED_IN_1_14_FOR(f)    G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_1_14
# define NM_DEPRECATED_IN_1_14_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_1_14
# define NM_AVAILABLE_IN_1_14            G_UNAVAILABLE(1,14)
#else
# define NM_AVAILABLE_IN_1_14
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_1_16
# define NM_DEPRECATED_IN_1_16           G_DEPRECATED
# define NM_DEPRECATED_IN_1_16_FOR(f)    G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_1_16
# define NM_DEPRECATED_IN_1_16_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_1_16
# define NM_AVAILABLE_IN_1_16            G_UNAVAILABLE(1,16)
#else
# define NM_AVAILABLE_IN_1_16
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_1_18
# define NM_DEPRECATED_IN_1_18           G_DEPRECATED
# define NM_DEPRECATED_IN_1_18_FOR(f)    G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_1_18
# define NM_DEPRECATED_IN_1_18_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_1_18
# define NM_AVAILABLE_IN_1_18            G_UNAVAILABLE(1,18)
#else
# define NM_AVAILABLE_IN_1_18
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_1_20
# define NM_DEPRECATED_IN_1_20           G_DEPRECATED
# define NM_DEPRECATED_IN_1_20_FOR(f)    G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_1_20
# define NM_DEPRECATED_IN_1_20_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_1_20
# define NM_AVAILABLE_IN_1_20            G_UNAVAILABLE(1,20)
#else
# define NM_AVAILABLE_IN_1_20
#endif

#if NM_VERSION_MIN_REQUIRED >= NM_VERSION_1_22
# define NM_DEPRECATED_IN_1_22           G_DEPRECATED
# define NM_DEPRECATED_IN_1_22_FOR(f)    G_DEPRECATED_FOR(f)
#else
# define NM_DEPRECATED_IN_1_22
# define NM_DEPRECATED_IN_1_22_FOR(f)
#endif

#if NM_VERSION_MAX_ALLOWED < NM_VERSION_1_22
# define NM_AVAILABLE_IN_1_22            G_UNAVAILABLE(1,22)
#else
# define NM_AVAILABLE_IN_1_22
#endif

/* libnm's NMClient maintains a cache of NetworkManager's D-Bus interface.
 * Issuing blocking calls from libnm API will only invoke the D-Bus method
 * and return it's result, without updating the cache (of course, otherwise
 * if it would emit signals and change the cache content while waiting, it
 * wouldn't be very blocking).
 *
 * When a blocking call returns (from g_dbus_connection_call_sync()), the
 * response is processed out of order from other events that populate the
 * object cache. That is bad.
 *
 * Even worse, the cache is no longer up-to-date, when the blocking call
 * returns. It will only get sync'ed when you iterate the main context again.
 * At this point, why did you call the blocking method? It doesn't make sense.
 *
 * For that reason, blocking API is deprecated. They are odd to use. You cannot
 * glue a synchronous API on top of D-Bus (which is inherrently asynchronous).
 * At least not, if you also have other state (the object cache), that should stay
 * in sync.
 *
 * These methods are effectively deprecated since 1.22. However, at this point
 * we don't yet mark them as such, because it might just cause unnecessary compiler
 * warnings. Let's first deprecate them for a long time, before we enable the
 * compiler warning. */
#define _NM_DEPRECATED_SYNC_METHOD            /*NM_DEPRECATED_IN_1_22*/
#define _NM_DEPRECATED_SYNC_WRITABLE_PROPERTY /*NM_DEPRECATED_IN_1_22*/

#endif  /* NM_VERSION_H */
