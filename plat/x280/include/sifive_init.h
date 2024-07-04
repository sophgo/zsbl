/* Copyright 2019 SiFive Inc. */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef METAL_INIT
#define METAL_INIT

/*!
 * @file init.h
 * API for Metal Initialization
 */

/*!
 * @brief Initialize devices shared between multiple harts
 *
 * This function is called by the default metal_init_run() implementation.
 */
void metal_init(void);

/*!
 * @brief Weak function to call metal_init()
 *
 * By default, this function calls metal_init(). If you wish to replace or
 * augment this call to the primary initialization, you can redefine
 * metal_init_run()
 *
 * On __metal_init_hart, this function is called before
 * metal_secondary_init_run by the _start function in init.c
 */
void metal_init_run(void) __attribute__((weak));

/*!
 * @brief Weak function to initialize WorldGuard checkers
 *
 * By default, this function configures a 'bypass' WG configuration where all
 * WIDs are allowed on all WG filters and WG PMPs.
 *
 * On __metal_init_hart, this function is called after metal_init_run by the
 * _start function in init.c
 *
 * If you wish to replace this behavior, you can redefine
 * metal_init_worldguard()
 */
void metal_init_worldguard(void) __attribute__((weak));

/*!
 * @brief Initialize hart specific devices
 *
 * This function is called by the default metal_secondary_init_run()
 * implementation.
 */
void metal_secondary_init(void);

/*!
 * @brief Weak function to call metal_secondary_init()
 *
 * By default, this function calls metal_secondary_init(). If you wish to
 * replace or augment this call to the primary initialization, you can
 * redefine metal_secondary_run()
 *
 * This function is called before secondary_main() by the _start function in
 * init.c
 */
void metal_secondary_init_run(void) __attribute__((weak));

/*!
 * @brief Weak function to initialize WorldGuard CSRs
 *
 * By default, this function configures:
 * - mwiddeleg: all legal WIDs enabled
 * - mlwid/slwid: highest legal WID value
 *
 * On all harts, this function is called after metal_secondary_init_run by the
 * _start function in init.c
 *
 * If you wish to replace this behavior, you can redefine
 * metal_secondary_init_worldguard()
 */
void metal_secondary_init_worldguard(void) __attribute__((weak));

/*!
 * @brief Weak function to call main()
 *
 * By default, this function will:
 * - on the boot hart, call main()
 * - on the other hart(s), spin forever
 *
 * If you wish to replace or augment this behavior, you can redefine
 * secondary_main()
 */
int secondary_main(int argc, const char *argv[], const char *envp[]) __attribute__((weak));

#endif /* METAL_INIT */
