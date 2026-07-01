#include <common/vfs.h>
#include <common/module.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <lib/cli.h>

/* UTF-8 box drawing (same spirit as Linux `tree`) */
#define TREE_TE "\342\224\234\342\224\200\342\224\200 "
#define TREE_EL "\342\224\224\342\224\200\342\224\200 "
#define TREE_PN "\342\224\202   "
#define TREE_EM "    "

struct vfs_node_item {
	struct vfs_node node;
	char *name;
};

struct vfs_file_item {
	int fd;
	struct vfs_file file;
	struct vfs_file_item *next;
};

static struct vfs_node_item *vfs_root_node;
static struct vfs_file_item *vfs_file_head;
static struct vfs_fs_type *vfs_fs_types;
static int vfs_next_fd;
static unsigned int vfs_ready;

static struct vfs_node *vfs_follow_mount_mnt(struct vfs_node *node,
					     struct vfs_mount **mnt)
{
	while (node && node->mnt) {
		*mnt = node->mnt;
		node = node->mnt->root;
	}

	return node;
}

static struct vfs_node *vfs_follow_mount(struct vfs_node *node)
{
	struct vfs_mount *mnt = NULL;

	return vfs_follow_mount_mnt(node, &mnt);
}

static struct vfs_node_item *vfs_item_from_node(const struct vfs_node *node)
{
	if (!node)
		return NULL;

	return (struct vfs_node_item *)node;
}

static struct vfs_node *vfs_root_ptr(void)
{
	if (!vfs_root_node)
		return NULL;

	return &vfs_root_node->node;
}

static struct vfs_file_item *vfs_find_file(int fd)
{
	struct vfs_file_item *item;

	for (item = vfs_file_head; item; item = item->next) {
		if (item->fd == fd)
			return item;
	}

	return NULL;
}

static struct vfs_node *vfs_find_child_by_name(struct vfs_node *parent, const char *name)
{
	struct vfs_node *child;

	if (!parent || !name)
		return NULL;

	for (child = parent->child; child; child = child->sibling) {
		if (!strcmp(child->name, name))
			return child;
	}

	return NULL;
}

static int vfs_alloc_fd(void)
{
	int start_fd;

	if (vfs_next_fd < 0)
		vfs_next_fd = 0;

	start_fd = vfs_next_fd;
	do {
		if (!vfs_find_file(vfs_next_fd)) {
			int fd = vfs_next_fd;

			vfs_next_fd++;
			if (vfs_next_fd < 0)
				vfs_next_fd = 0;
			return fd;
		}

		vfs_next_fd++;
		if (vfs_next_fd < 0)
			vfs_next_fd = 0;
	} while (vfs_next_fd != start_fd);

	return -EMFILE;
}

static int vfs_is_path_valid(const char *path)
{
	if (!path)
		return 0;

	if (*path != '/')
		return 0;

	return 1;
}

static struct vfs_node *vfs_lookup_node_ex(const char *path, int follow_last)
{
	const char *cursor;
	const char *slash;
	size_t seg_len;
	char *seg_name;
	struct vfs_node *cur_node;
	struct vfs_mount *cur_mnt = NULL;

	if (!vfs_root_node)
		return NULL;

	cur_node = &vfs_root_node->node;

	if (!strcmp(path, "/"))
		return follow_last ? vfs_follow_mount_mnt(cur_node, &cur_mnt) : cur_node;

	cur_node = vfs_follow_mount_mnt(cur_node, &cur_mnt);
	cursor = path + 1;
	while (*cursor) {
		struct vfs_node *next;
		int last;

		slash = strchr(cursor, '/');
		seg_len = slash ? (size_t)(slash - cursor) : strlen(cursor);
		if (!seg_len)
			return NULL;

		/* final segment: no slash, or only a trailing slash follows */
		last = (!slash || *(slash + 1) == '\0');

		seg_name = malloc(seg_len + 1);
		if (!seg_name)
			return NULL;
		memcpy(seg_name, cursor, seg_len);
		seg_name[seg_len] = '\0';

		next = vfs_find_child_by_name(cur_node, seg_name);
		/* not pre-built: let the filesystem materialize it into the tree */
		if (!next && cur_mnt && cur_mnt->sops && cur_mnt->sops->lookup)
			next = cur_mnt->sops->lookup(cur_mnt, cur_node, seg_name);
		free(seg_name);
		if (!next)
			return NULL;
		cur_node = next;

		if (last) {
			if (follow_last)
				cur_node = vfs_follow_mount_mnt(cur_node, &cur_mnt);
			break;
		}

		cur_node = vfs_follow_mount_mnt(cur_node, &cur_mnt);
		cursor = slash + 1;
	}

	return cur_node;
}

static struct vfs_node *vfs_lookup_node(const char *path)
{
	return vfs_lookup_node_ex(path, 1);
}

static int vfs_name_valid(const char *name)
{
	if (!name || !name[0])
		return 0;

	if (strchr(name, '/'))
		return 0;

	return 1;
}

static struct vfs_node *vfs_alloc_node(const char *name, enum vfs_node_type type,
				       const struct vfs_file_ops *fops, void *priv)
{
	struct vfs_node_item *item;

	item = malloc(sizeof(*item));
	if (!item)
		return NULL;

	memset(item, 0, sizeof(*item));
	item->name = strdup(name);
	if (!item->name) {
		free(item);
		return NULL;
	}

	item->node.name = item->name;
	item->node.type = type;
	item->node.fops = fops;
	item->node.priv = priv;

	return &item->node;
}

static struct vfs_node *vfs_create_node(struct vfs_node *parent, const char *name,
					enum vfs_node_type type,
					const struct vfs_file_ops *fops, void *priv)
{
	struct vfs_node *node;

	if (!vfs_ready || !vfs_root_node)
		return NULL;

	if (!parent || !vfs_name_valid(name))
		return NULL;

	if (parent->type != VFS_NODE_DIR)
		return NULL;

	if (vfs_find_child_by_name(parent, name))
		return NULL;

	if (type != VFS_NODE_DIR && !fops)
		return NULL;

	node = vfs_alloc_node(name, type, fops, priv);
	if (!node)
		return NULL;

	node->parent = parent;
	node->sibling = parent->child;
	parent->child = node;

	return node;
}

static void vfs_free_subtree(struct vfs_node *node)
{
	struct vfs_node *child;
	struct vfs_node *next;
	struct vfs_node_item *item;

	if (!node)
		return;

	for (child = node->child; child; child = next) {
		next = child->sibling;
		vfs_free_subtree(child);
	}

	item = vfs_item_from_node(node);
	free(item->name);
	free(item);
}

static int vfs_init(void)
{
	struct vfs_file_item *file;

	if (vfs_root_node) {
		vfs_free_subtree(&vfs_root_node->node);
		vfs_root_node = NULL;
	}

	file = vfs_file_head;
	while (file) {
		struct vfs_file_item *next = file->next;

		free(file);
		file = next;
	}

	vfs_root_node = malloc(sizeof(*vfs_root_node));
	if (!vfs_root_node)
		return -ENOMEM;
	memset(vfs_root_node, 0, sizeof(*vfs_root_node));
	vfs_root_node->name = strdup("/");
	if (!vfs_root_node->name) {
		free(vfs_root_node);
		vfs_root_node = NULL;
		return -ENOMEM;
	}
	vfs_root_node->node.name = vfs_root_node->name;
	vfs_root_node->node.type = VFS_NODE_DIR;

	vfs_file_head = NULL;
	vfs_next_fd = 0;
	vfs_ready = 1;

	return 0;
}

struct vfs_node *vfs_root(void)
{
	return vfs_root_ptr();
}

struct vfs_node *vfs_mkdir(struct vfs_node *parent, const char *name)
{
	return vfs_create_node(parent, name, VFS_NODE_DIR, NULL, NULL);
}

struct vfs_node *vfs_mknod(struct vfs_node *parent, const char *name,
			   enum vfs_node_type type,
			   const struct vfs_file_ops *fops, void *priv)
{
	return vfs_create_node(parent, name, type, fops, priv);
}

struct vfs_node *vfs_alloc_dir_node(const char *name)
{
	if (!vfs_name_valid(name))
		return NULL;

	return vfs_alloc_node(name, VFS_NODE_DIR, NULL, NULL);
}

int vfs_unlink(struct vfs_node *parent, const char *name)
{
	struct vfs_node_item *item;
	struct vfs_node *node;
	struct vfs_node *walk;
	struct vfs_node *prev;
	struct vfs_file_item *file;

	if (!parent || !vfs_name_valid(name))
		return -EINVAL;
	if (parent->type != VFS_NODE_DIR)
		return -ENOTDIR;

	node = vfs_find_child_by_name(parent, name);
	if (!node)
		return -ENOENT;
	if (node->child)
		return -ENOTEMPTY;

	for (file = vfs_file_head; file; file = file->next) {
		if (file->file.node == node)
			return -EBUSY;
	}

	parent = node->parent;
	if (!parent)
		return -EINVAL;

	prev = NULL;
	for (walk = parent->child; walk; walk = walk->sibling) {
		if (walk == node)
			break;
		prev = walk;
	}
	if (!walk)
		return -ENOENT;

	if (prev)
		prev->sibling = node->sibling;
	else
		parent->child = node->sibling;

	item = vfs_item_from_node(node);
	if (!item)
		return -ENOENT;
	free(item->name);
	free(item);

	return 0;
}

const struct vfs_node *vfs_lookup(const char *path)
{
	struct vfs_node *node;

	if (!vfs_is_path_valid(path))
		return NULL;

	node = vfs_lookup_node(path);
	if (node)
		return node;

	return NULL;
}

int vfs_open(const char *path, unsigned int flags)
{
	int fd;
	int ret;
	const struct vfs_node *node;
	struct vfs_file_item *item;
	struct vfs_file *file;

	node = vfs_lookup(path);
	if (!node)
		return -ENOENT;

	if (node->type == VFS_NODE_DIR)
		return -EISDIR;

	fd = vfs_alloc_fd();
	if (fd < 0)
		return fd;

	item = malloc(sizeof(*item));
	if (!item)
		return -ENOMEM;

	memset(item, 0, sizeof(*item));
	item->fd = fd;
	file = &item->file;
	memset(file, 0, sizeof(*file));
	file->node = node;
	file->flags = flags;
	file->private_data = node->priv;

	if (node->fops && node->fops->open) {
		ret = node->fops->open(file);
		if (ret) {
			free(item);
			return ret;
		}
	}

	item->next = vfs_file_head;
	vfs_file_head = item;

	return fd;
}

ssize_t vfs_read(int fd, void *buf, size_t len)
{
	struct vfs_file_item *item;
	struct vfs_file *file;
	ssize_t ret;

	item = vfs_find_file(fd);
	if (!item)
		return -EBADF;

	if (!buf && len)
		return -EINVAL;

	file = &item->file;
	if (!file->node->fops || !file->node->fops->read)
		return -ENOSYS;

	ret = file->node->fops->read(file, buf, len);
	if (ret > 0)
		file->pos += (size_t)ret;

	return ret;
}

ssize_t vfs_write(int fd, const void *buf, size_t len)
{
	struct vfs_file_item *item;
	struct vfs_file *file;
	ssize_t ret;

	item = vfs_find_file(fd);
	if (!item)
		return -EBADF;

	if (!buf && len)
		return -EINVAL;

	file = &item->file;
	if (!file->node->fops || !file->node->fops->write)
		return -ENOSYS;

	ret = file->node->fops->write(file, buf, len);
	if (ret > 0)
		file->pos += (size_t)ret;

	return ret;
}

off_t vfs_lseek(int fd, off_t off, int whence)
{
	struct vfs_file_item *item;
	struct vfs_file *file;
	off_t base;
	off_t npos;

	item = vfs_find_file(fd);
	if (!item)
		return -EBADF;

	file = &item->file;

	switch (whence) {
	case SEEK_SET:
		base = 0;
		break;
	case SEEK_CUR:
		base = (off_t)file->pos;
		break;
	case SEEK_END:
		/* no reliable size source yet; revisit with block devices */
		return -ENOSYS;
	default:
		return -EINVAL;
	}

	npos = base + off;
	if (npos < 0)
		return -EINVAL;

	file->pos = (size_t)npos;

	return npos;
}

int vfs_ioctl(int fd, unsigned long cmd, void *arg)
{
	struct vfs_file_item *item;
	struct vfs_file *file;

	item = vfs_find_file(fd);
	if (!item)
		return -EBADF;

	file = &item->file;
	if (!file->node->fops || !file->node->fops->ioctl)
		return -ENOSYS;

	return file->node->fops->ioctl(file, cmd, arg);
}

int vfs_close(int fd)
{
	int ret = 0;
	struct vfs_file_item *item;
	struct vfs_file_item *prev = NULL;
	struct vfs_file *file;

	for (item = vfs_file_head; item; item = item->next) {
		if (item->fd == fd)
			break;
		prev = item;
	}
	if (!item)
		return -EBADF;

	file = &item->file;
	if (file->node->fops && file->node->fops->close)
		ret = file->node->fops->close(file);

	if (prev)
		prev->next = item->next;
	else
		vfs_file_head = item->next;

	memset(file, 0, sizeof(*file));
	free(item);

	return ret;
}

static struct vfs_fs_type *vfs_find_filesystem(const char *name)
{
	struct vfs_fs_type *type;

	for (type = vfs_fs_types; type; type = type->next) {
		if (!strcmp(type->name, name))
			return type;
	}

	return NULL;
}

int vfs_register_filesystem(struct vfs_fs_type *type)
{
	if (!type || !type->name || !type->mount)
		return -EINVAL;

	if (vfs_find_filesystem(type->name))
		return -EEXIST;

	type->next = vfs_fs_types;
	vfs_fs_types = type;

	return 0;
}

static int vfs_try_mount(struct vfs_node *mp, struct vfs_fs_type *type,
			 const char *source, void *data)
{
	struct vfs_mount *mnt;
	int ret;

	mnt = malloc(sizeof(*mnt));
	if (!mnt)
		return -ENOMEM;
	memset(mnt, 0, sizeof(*mnt));
	mnt->mountpoint = mp;
	mnt->type = type;

	ret = type->mount(mnt, source, data);
	if (ret) {
		free(mnt);
		return ret;
	}
	if (!mnt->root) {
		free(mnt);
		return -EINVAL;
	}

	mp->mnt = mnt;

	return 0;
}

int vfs_mount(const char *source, const char *target,
	      const char *fstype, void *data)
{
	struct vfs_fs_type *type;
	struct vfs_node *mp;

	if (!target || !vfs_is_path_valid(target))
		return -EINVAL;

	/* resolve the mountpoint itself, not what is mounted on it */
	mp = vfs_lookup_node_ex(target, 0);
	if (!mp)
		return -ENOENT;
	if (mp->type != VFS_NODE_DIR)
		return -ENOTDIR;
	if (mp->mnt)
		return -EBUSY;

	if (fstype) {
		type = vfs_find_filesystem(fstype);
		if (!type)
			return -ENODEV;
		return vfs_try_mount(mp, type, source, data);
	}

	/* auto-detect: try each block-backed filesystem until one accepts */
	for (type = vfs_fs_types; type; type = type->next) {
		if (!(type->flags & VFS_FS_REQUIRES_DEV))
			continue;
		if (vfs_try_mount(mp, type, source, data) == 0)
			return 0;
	}

	return -ENODEV;
}

static int vfs_node_under_root(const struct vfs_node *node,
			       const struct vfs_node *root)
{
	while (node) {
		if (node == root)
			return 1;
		node = node->parent;
	}

	return 0;
}

int vfs_umount(const char *target)
{
	struct vfs_node *mp;
	struct vfs_mount *mnt;
	struct vfs_file_item *file;

	if (!target || !vfs_is_path_valid(target))
		return -EINVAL;

	mp = vfs_lookup_node_ex(target, 0);
	if (!mp)
		return -ENOENT;
	if (!mp->mnt)
		return -EINVAL;

	mnt = mp->mnt;

	for (file = vfs_file_head; file; file = file->next) {
		if (vfs_node_under_root(file->file.node, mnt->root))
			return -EBUSY;
	}

	if (mnt->sops && mnt->sops->umount)
		mnt->sops->umount(mnt);

	mp->mnt = NULL;
	vfs_free_subtree(mnt->root);
	free(mnt);

	return 0;
}

static unsigned int vfs_child_count(const struct vfs_node *parent)
{
	unsigned int n = 0;
	const struct vfs_node *c;

	for (c = parent->child; c; c = c->sibling)
		n++;

	return n;
}

static void vfs_show_tree_recurse(const struct vfs_node *parent, const char *prefix)
{
	const struct vfs_node *child;
	unsigned int total;
	unsigned int i;

	/* descend into a mounted fs if this dir is a mountpoint */
	parent = vfs_follow_mount((struct vfs_node *)parent);

	total = vfs_child_count(parent);
	i = 0;

	for (child = parent->child; child; child = child->sibling, i++) {
		unsigned int last = (i == total - 1);
		const char *branch = last ? TREE_EL : TREE_TE;
		char next[256];
		const char *ext = last ? TREE_EM : TREE_PN;

		printf("%s%s%s\n", prefix, branch, child->name ? child->name : "?");

		if (snprintf(next, sizeof(next), "%s%s", prefix, ext) >= (int)sizeof(next))
			continue;

		vfs_show_tree_recurse(child, next);
	}
}

static void vfs_show_tree(void)
{
	struct vfs_node *root;

	root = vfs_root_ptr();
	if (!root || !vfs_ready) {
		printf("(vfs not ready)\n");
		return;
	}

	printf("%s\n", root->name ? root->name : "/");
	vfs_show_tree_recurse(root, "");
}

static void command_vfstree(struct command *c, int argc, const char *argv[])
{
	(void)c;
	(void)argc;
	(void)argv;

	vfs_show_tree();
}

cli_command(vfstree, command_vfstree);

static int vfs_subsys_init(void)
{
	return vfs_init();
}

subsys_init(vfs_subsys_init);
