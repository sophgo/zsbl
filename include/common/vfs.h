#ifndef __VFS_H__
#define __VFS_H__

#include <stddef.h>
#include <stdint.h>

/*
 * Minimal VFS for bare-metal:
 * - absolute-path exact match lookup
 * - dynamic node/file table with malloc
 * - mountable filesystems (vfs_fs_type) joined into one namespace
 *
 * The core depends only on libc (errno/string/stdlib) and never on any
 * driver-layer code, so it can be reused independently.
 */

#ifndef VFS_OFF_T_DEFINED
#define VFS_OFF_T_DEFINED
typedef long off_t;
#endif

#ifndef SEEK_SET
#define SEEK_SET	0	/* seek relative to start of file */
#define SEEK_CUR	1	/* seek relative to current position */
#define SEEK_END	2	/* seek relative to end of file */
#endif

enum vfs_node_type {
	VFS_NODE_UNSPEC = 0,
	VFS_NODE_REG,
	VFS_NODE_DIR,
	VFS_NODE_CHR,
	VFS_NODE_BLK,
};

struct vfs_file;
struct vfs_node;
struct vfs_mount;
struct vfs_fs_type;
struct vfs_super_ops;

struct vfs_file_ops {
	int (*open)(struct vfs_file *file);
	ssize_t (*read)(struct vfs_file *file, void *buf, size_t len);
	ssize_t (*write)(struct vfs_file *file, const void *buf, size_t len);
	int (*ioctl)(struct vfs_file *file, unsigned long cmd, void *arg);
	int (*close)(struct vfs_file *file);
};

struct vfs_node {
	const char *name;
	enum vfs_node_type type;
	const struct vfs_file_ops *fops;
	void *priv;
	struct vfs_node *parent;
	struct vfs_node *child;
	struct vfs_node *sibling;
	struct vfs_mount *mnt;		/* non-NULL: this dir is a mountpoint */
};

struct vfs_file {
	const struct vfs_node *node;
	size_t pos;
	unsigned int flags;
	void *private_data;
};

/*
 * Optional per-mount operations. All members may be NULL.
 *   lookup: resolve 'name' inside directory 'dir' on demand for filesystems
 *           whose entries are not pre-built (e.g. FAT). It should create the
 *           node in the tree via vfs_mknod(dir, name, ...) and return it, or
 *           return NULL if the entry does not exist. The node then lives in
 *           the tree like any other until umount.
 *   umount: release fs-private state before the core frees the node subtree.
 */
struct vfs_super_ops {
	struct vfs_node *(*lookup)(struct vfs_mount *mnt, struct vfs_node *dir,
				   const char *name);
	int (*umount)(struct vfs_mount *mnt);
};

/*
 * A registered filesystem type. mount() must build the fs root via
 * vfs_alloc_dir_node() (and populate children with vfs_mkdir/vfs_mknod),
 * store it into mnt->root, and may set mnt->sops / mnt->priv.
 */
struct vfs_fs_type {
	const char *name;
	int (*mount)(struct vfs_mount *mnt, const char *source, void *data);
	struct vfs_fs_type *next;	/* internal: registration list link */
};

struct vfs_mount {
	struct vfs_node *mountpoint;	/* covered directory in parent ns */
	struct vfs_node *root;		/* root node of the mounted fs */
	struct vfs_fs_type *type;
	const struct vfs_super_ops *sops;
	void *priv;			/* fs-private data */
};

/* ---- node / tree management ---- */

/* Return the root directory node, or NULL if VFS is not initialized. */
struct vfs_node *vfs_root(void);

/*
 * Create a directory under parent.
 * Returns the new node, or NULL on bad/duplicate name or parent not a dir.
 */
struct vfs_node *vfs_mkdir(struct vfs_node *parent, const char *name);

/*
 * Create a typed node (REG/CHR/BLK require a non-NULL fops; DIR ignores it)
 * under parent, attaching priv. Returns the node, or NULL on failure.
 */
struct vfs_node *vfs_mknod(struct vfs_node *parent, const char *name,
			   enum vfs_node_type type,
			   const struct vfs_file_ops *fops, void *priv);

/*
 * Allocate a detached (no parent) directory node, intended as a filesystem
 * root inside a vfs_fs_type.mount() callback. Returns NULL on OOM.
 */
struct vfs_node *vfs_alloc_dir_node(const char *name);

/*
 * Remove an empty leaf node 'name' under parent.
 * Errors: -ENOTDIR, -ENOENT, -ENOTEMPTY (has children), -EBUSY (open).
 */
int vfs_unlink(struct vfs_node *parent, const char *name);

/*
 * Resolve an absolute path (must start with '/'), crossing mountpoints.
 * Returns the node or NULL if invalid/not found.
 */
const struct vfs_node *vfs_lookup(const char *path);

/* ---- mount ---- */

/* Register a filesystem type. Errors: -EINVAL, -EEXIST (duplicate name). */
int vfs_register_filesystem(struct vfs_fs_type *type);

/*
 * Mount filesystem 'fstype' onto the existing directory 'target'.
 * 'source'/'data' are passed through to the fs mount() callback.
 * Errors: -EINVAL, -ENODEV (type), -ENOENT, -ENOTDIR, -EBUSY (already
 * mounted), -ENOMEM, or the fs callback's error.
 */
int vfs_mount(const char *source, const char *target,
	      const char *fstype, void *data);

/*
 * Unmount the filesystem at 'target'. The fs node subtree is freed.
 * Errors: -ENOENT, -EINVAL (not a mountpoint), -EBUSY (open files under it).
 */
int vfs_umount(const char *target);

/* ---- file I/O (fd semantics) ---- */

/*
 * Open the node at path. Returns a non-negative fd.
 * Errors: -ENOENT, -EISDIR (path is a directory), -EMFILE, -ENOMEM,
 * or the fops open() error.
 */
int vfs_open(const char *path, unsigned int flags);

/* Read up to len bytes, advancing the file position. -EBADF, -ENOSYS. */
ssize_t vfs_read(int fd, void *buf, size_t len);

/* Write up to len bytes, advancing the file position. -EBADF, -ENOSYS. */
ssize_t vfs_write(int fd, const void *buf, size_t len);

/*
 * Reposition the file offset. SEEK_SET/SEEK_CUR supported; SEEK_END is not
 * yet supported (-ENOSYS). Returns the new offset. -EBADF, -EINVAL.
 */
off_t vfs_lseek(int fd, off_t off, int whence);

/* Device control passthrough to fops ioctl(). -EBADF, -ENOSYS. */
int vfs_ioctl(int fd, unsigned long cmd, void *arg);

/* Close fd, invoking fops close(). -EBADF. */
int vfs_close(int fd);

#endif
