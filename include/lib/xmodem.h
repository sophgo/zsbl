#ifndef __XMODEM_H__
#define __XMODEM_H__

#include <common/common.h>
#include <stdlib.h>

/**
 * get_char: return EOF if no input device or end of file, an nagtive number
 * means error occur, or a character is returned.
 * put_char: return EOF if no output device or end of file, an nagtive number
 * means error, occur, on success character 'c' should be returned and 'c'
 * should output to specific device or file.
 */
struct xmodem {
	int (*get_char)(struct xmodem *);
	int (*put_char)(struct xmodem *, int);
	void *priv;
};

int xmodem_init(struct xmodem *ctx,
		int (*get_char)(struct xmodem *),
		int (*put_char)(struct xmodem *, int),
		void *priv);

long xmodem_receive(struct xmodem *ctx,
		    int (*save)(void *priv, void *data,unsigned long len),
		    void *priv);

#endif
