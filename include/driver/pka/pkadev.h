// ------------------------------------------------------------------------
//
//                (C) COPYRIGHT 2011 - 2015 SYNOPSYS, INC.
//                          ALL RIGHTS RESERVED
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  version 2 as published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <https://gnu.org/licenses/>.
//
// ------------------------------------------------------------------------

#ifndef ELPPKA_DEVICE_H_
#define ELPPKA_DEVICE_H_

#include <stdio.h>

/*
 * Open the PKA character device specified by name.  If name is NULL, attempt
 * to find a device automatically.  Returns the opened file descriptor, or -1
 * on error.
 */
int elppka_device_open(const char *name);

/*
 * Get/set PKA operands.  If func is the empty string or NULL, the name
 * parameter designates an absolute operand (such as A0 or D3).  Otherwise,
 * the parameter names are specific to the particular function.
 *
 * Parameter data is size bytes long, and stored most-significant byte first.
 */
int elppka_set_operand(int fd, const char *func, const char *name,
                               unsigned size, const void *data);
int elppka_get_operand(int fd, const char *func, const char *name,
                               unsigned size, void *data);

/*
 * Copy a PKA operand from one location to another.  Same effect as calling
 *
 *   elppka_get_operand(fd, src_func, src_name, size, tmp_buf);
 *   elppka_set_operand(fd, dst_func, dst_name, size, tmp_buf);
 *
 * except that no temporary buffer is required.  Returns 0 on success, or -1
 * on failure.
 */
int elppka_copy_operand(int fd, const char *dst_func, const char *dst_name,
                                const char *src_func, const char *src_name,
                                unsigned size);

/*
 * Tests a PKA flag.  Flags are essentially treated like boolean operands.
 * If func is the empty string or NULL, the name parameter designates a flag
 * directly (one of Z, M, B, C, or F0--F3).  Otherwise, the flag names are
 * specific to the particular function.
 *
 * Returns 0 if the flag is unset, a positive value if the flag is set, or -1
 * on error.
 */
int elppka_test_flag(int fd, const char *func, const char *name);

/*
 * Set a PKA flag for the next operation.
 *
 * Returns 0 if the flag was previously unset, a positive value if it was
 * previously set, or -1 on error.
 */
int elppka_set_flag(int fd, const char *func, const char *name);

/*
 * Run the PKA.  Takes the function name and operand size (in bytes).  Each
 * successive pair of arguments in the variadic part indicates the operands,
 * the first argument is a pointer to the parameter name and the second is
 * a pointer to the data (or, in the case of output parameters, a buffer in
 * which to store the data).  All operand buffers must be at least size
 * bytes long.  The parameter list is terminated by (char *)NULL.
 *
 * Input and output parameters may be specified in any order.  All input
 * parameters are processed prior to any output parameters, but otherwise
 * paramters are handled in the same order that they are specified.  Thus,
 * the same buffer may be used multiple times.
 *
 * Output parameters are indicated by prefixing their name by a single =, and
 * absolute parameter locations are indicated by prefixing the name by a
 * single %.  For absolute output parameters, the = must precede the %.
 * Other names are specific to the particular function being called.
 *
 * Example: call the function "foo" with a named input "bar" and an absolute
 * input "D0", with named outputs "baz" and absolute output "A1".
 *
 *   elppka_run(fd, "foo", 32,
 *              "bar",  &in_bar,
 *              "=baz", &out_baz,
 *              "%D0",  &in_d0,
 *              "=%A1", &out_a1,
 *              (char *)NULL);
 *
 * Returns -1 on failure, 0 on success, or a positive value indicating some
 * other error.  If a non-zero value is returned, none of the output parameters
 * are written.
 *
 * The elppka_vrun function is similar, except it takes a va_list instead.
 */
int elppka_vrun(int fd, const char *func, unsigned size, va_list ap);
int elppka_run(int fd, const char *func, unsigned size, ...);

/*
 * Close a previously opened PKA character device.
 */
int elppka_device_close(int fd);

#endif
