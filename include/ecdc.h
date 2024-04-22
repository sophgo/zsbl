/**
 * Copyright (c) 2016 Bradley Kim Schleusner < bradschl@gmail.com >
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef ECDC_H_
#define ECDC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>


// --------------------------- Console allocation, deallocation, and processing


// ---------- ecdc_getc_fn return value
#define ECDC_GETC_EOF   (-1)


// ---------------- Configuration modes
enum ecdc_mode {
    ECDC_MODE_ANSI = 0
};


// ---------------- Configuration flags

// Enable local echo
#define ECDC_SET_LOCAL_ECHO     (1 << 0)


// --------- Internal console structure
struct ecdc_console;

/**
 * @brief Function pointer prototype for non-blocking character reads
 * @details When the console is pumped, this is polled for incoming characters.
 *          If there are no characters to read, then ECDC_GETC_EOF should be
 *          returned.
 *          It is expected that this is non-blocking
 *
 * @param console_hint Optional console hint parameter. This pointer may be
 *          used for whatever the implementation wants (such as a this pointer,
 *          or a buffer pointer)
 * @return User input character or ECDC_GETC_EOF
 */
typedef int (*ecdc_getc_fn)(void * console_hint);

/**
 * @brief Function pointer prototype for writing characters
 * @details It is expected that this is non-blocking
 *
 * @param console_hint Optional console hint parameter. This pointer may be
 *          used for whatever the implementation wants (such as a this pointer,
 *          or a buffer pointer)
 * @param c Character to write to the output console
 */
typedef void (*ecdc_putc_fn)(void * console_hint, char c);

/**
 * @brief Allocates a console structure on the heap
 * @details The console will be allocated with default settings and no
 *          registered commands.
 *          The default settings are ECDC_MODE_ANSI with local echo enabled
 *
 * @param console_hint Optional console hint parameter. The getc_fn and putc_fn
 *          functions will be called with this pointer. This pointer may be
 *          used for whatever the implementation wants (such as a this pointer,
 *          or a buffer pointer). Set to NULL if not used
 * @param getc_fn Character read function
 * @param putc_fn Character write function
 * @param max_arg_line_length Maximum length of an input line. This is the
 *          maximum size of the input line and its arguments. 80 characters is
 *          a sane default.
 * @param max_arg_count Maximum number of arguments allowed per command. There
 *          should be moderate ratio between the argument line length and the
 *          maximum number of arguments. 10 arguments is a sane default.
 *
 * @return Console pointer. It is the responsibility of the caller to deallocate
 *          this with a call to ecdc_free_console.
 */
struct ecdc_console *
ecdc_alloc_console(void * console_hint,
                   ecdc_getc_fn getc_fn,
                   ecdc_putc_fn putc_fn,
                   size_t max_arg_line_length,
                   size_t max_arg_count);

/**
 * @brief Deallocates a console
 * @details This will free all resources and unregistering all commands
 *
 * @param ecdc_console Console to deallocate
 */
void
ecdc_free_console(struct ecdc_console * console);

/**
 * @brief Periodic call to drive character receiving and parsing
 * @details This needs to be periodically called to drive the receiving and
 *          parsing of the commands by the console. Callbacks will also be
 *          driven by this call.
 *
 * @param ecdc_console Console to execute
 */
void
ecdc_pump_console(struct ecdc_console * console);

/**
 * @brief Modifies the console's configuration
 * @details This is used to modify the control sequence standard (mode) used by
 *          the console, as well as configure things like local echo.
 *
 * @param ecdc_console Console to configure
 * @param ecdc_mode Control sequence standard
 * @param flags Option flags
 */
void
ecdc_configure_console(struct ecdc_console * console,
                       enum ecdc_mode mode,
                       int flags);

/**
 * @brief Replaces prompt
 * @details This command will set the prompt or replaces it if previously set by user
 * 
 * @param ecdc_console Console to set prompt on
 * @param prompt Pointer to C-string containing the desired prompt
 * 
 * @return Pointer to newly allocated prompt string.  It is the responsibility
 *         of the user to deallocate this with the ecdc_free_prompt fucnction.  NULL
 *         is returned on failure.
 * 
 */
char const *
ecdc_replace_prompt(struct ecdc_console *console, 
                    char const *prompt);

/**
 * @brief Deallocated previously allocated prompt
 * @details This will deallocate any resources allocated by the ecdc_set_prompt command
 *          and resets prompt to default.
 * 
 * @param   ecdc_console Console for which to deallocate prompt
 */
void 
ecdc_free_prompt(struct ecdc_console *console);

// ------------------------------------------ Command allocation / deallocation

// --------- Internal command structure
struct ecdc_command;

/**
 * @brief Function pointer prototype for console command callbacks
 *
 * @param hint Optional command hint parameter. This pointer may be used for
 *          whatever the implementation wants (such as a this pointer)
 * @param argc Argument count
 * @param argv Argument values. First argument is always the command name
 */
typedef void (*ecdc_callback_fn)(void * hint, int argc, char const * argv[]);

/**
 * @brief Allocates a new command on the heap
 *
 * @param command_hint Optional command hint parameter. This will be passed to
 *          the command callback. This pointer may be used for whatever the
 *          implementation wants (such as a this pointer). If not used, set
 *          to NULL
 * @param ecdc_console Console to register the command with
 * @param command_name Name of the command. When a user inputs a string that
 *          matches this, the callback will be called. This string is copied, so
 *          its storage is allowed to be shorter than the returned pointer
 * @param callback Command callback
 *
 * @return Command structure. It is the responsibility of the caller to
 *          deallocate this with the ecdc_free_command function. NULL is
 *          returned on failure.
 */
struct ecdc_command *
ecdc_alloc_command(void * command_hint,
                   struct ecdc_console * console,
                   const char * command_name,
                   ecdc_callback_fn callback);

/**
 * @brief Deallocates a command structure
 * @details This will unregister the command and free any resources held by it
 *
 * @param ecdc_command Command to deallocate
 */
void
ecdc_free_command(struct ecdc_command * command);

// ------------------------------------------------- Built-in optional commands

/**
 * @brief Creates a list command
 * @details This command will print the name of every registered command
 *
 * @param ecdc_console Console to register the command with
 * @param command_name Name of the command, i.e. "list", "ls", "dir"
 *
 * @return Command structure. It is the responsibility of the caller to
 *          deallocate this with the ecdc_free_command function. NULL is
 *          returned on failure.
 */
struct ecdc_command *
ecdc_alloc_list_command(struct ecdc_console * console,
                        const char * command_name);

// --------------------------------------------------------------------- Extras

/**
 * @brief Writes a single character to the console output
 * @details The console may change the character to handle newlines correctly
 *
 * @param ecdc_console Console output to use
 * @param c Character to write
 */
void
ecdc_putc(struct ecdc_console * console, char c);

/**
 * @brief Writes a string to the console output
 * @details The console may insert sequences to handle newlines correctly
 *
 * @param ecdc_console Console output to use
 * @param str String to write
 */
void
ecdc_puts(struct ecdc_console * console, const char * str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ECDC_H_ */