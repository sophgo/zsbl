#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <common/module.h>
#include <common/vfs.h>

static void print_indent(unsigned int depth)
{
	unsigned int i;

	for (i = 0; i < depth; i++)
		printf("  ");
}

static void print_tree(const struct vfs_node *node, unsigned int depth)
{
	const struct vfs_node *child;

	if (!node)
		return;

	print_indent(depth);
	printf("%s\n", node->name ? node->name : "(null)");

	for (child = node->child; child; child = child->sibling)
		print_tree(child, depth + 1);
}

static int register_tree_nodes(void)
{
	struct vfs_node *root;
	struct vfs_node *lvl2[3];
	unsigned int i;
	unsigned int j;
	char name[8];

	root = vfs_root();
	if (!root)
		return -1;

	for (i = 0; i < 3; i++) {
		snprintf(name, sizeof(name), "n%u", i);
		lvl2[i] = vfs_mkdir(root, name);
		if (!lvl2[i])
			return -1;

		for (j = 0; j < 2; j++) {
			snprintf(name, sizeof(name), "c%u", j);
			if (!vfs_mkdir(lvl2[i], name))
				return -1;
		}
	}

	return 0;
}

static int test_vfs_tree(void)
{
	int ret;
	const struct vfs_node *root;

	ret = register_tree_nodes();
	if (ret)
		return ret;

	root = vfs_lookup("/");
	if (!root)
		return -1;

	printf("vfs tree:\n");
	print_tree(root, 0);

	return 0;
}

test_case(test_vfs_tree);

/* ---- RAM-backed mock device, used to exercise the file I/O path ---- */

#define MOCK_BUF_SIZE	64
static char mock_buf[MOCK_BUF_SIZE];

static ssize_t mock_read(struct vfs_file *file, void *buf, size_t len)
{
	size_t pos = file->pos;

	if (pos >= MOCK_BUF_SIZE)
		return 0;
	if (len > MOCK_BUF_SIZE - pos)
		len = MOCK_BUF_SIZE - pos;

	memcpy(buf, mock_buf + pos, len);

	return (ssize_t)len;
}

static ssize_t mock_write(struct vfs_file *file, const void *buf, size_t len)
{
	size_t pos = file->pos;

	if (pos >= MOCK_BUF_SIZE)
		return 0;
	if (len > MOCK_BUF_SIZE - pos)
		len = MOCK_BUF_SIZE - pos;

	memcpy(mock_buf + pos, buf, len);

	return (ssize_t)len;
}

static const struct vfs_file_ops mock_fops = {
	.read = mock_read,
	.write = mock_write,
};

static int test_vfs_file(void)
{
	struct vfs_node *root;
	const char in[4] = { 'a', 'b', 'c', 'd' };
	char out[4];
	int fd;
	ssize_t n;

	root = vfs_root();
	if (!root)
		return -1;

	if (!vfs_mknod(root, "regtest", VFS_NODE_REG, &mock_fops, NULL))
		return -1;

	fd = vfs_open("/regtest", 0);
	if (fd < 0)
		return -1;

	/* write advances pos to 4 */
	n = vfs_write(fd, in, sizeof(in));
	if (n != (ssize_t)sizeof(in))
		return -1;

	/* rewind and read back */
	if (vfs_lseek(fd, 0, SEEK_SET) != 0)
		return -1;
	n = vfs_read(fd, out, sizeof(out));
	if (n != (ssize_t)sizeof(out))
		return -1;
	if (memcmp(out, in, sizeof(in)) != 0)
		return -1;

	/* SEEK_CUR: from 4 back to 2 */
	if (vfs_lseek(fd, -2, SEEK_CUR) != 2)
		return -1;

	/* SEEK_END not supported yet */
	if (vfs_lseek(fd, 0, SEEK_END) != -ENOSYS)
		return -1;

	if (vfs_close(fd) != 0)
		return -1;

	/* error paths */
	if (vfs_open("/", 0) != -EISDIR)
		return -1;
	if (vfs_read(999, out, sizeof(out)) != -EBADF)
		return -1;
	if (vfs_mkdir(root, "regtest"))		/* duplicate name */
		return -1;

	if (vfs_unlink(root, "regtest") != 0)
		return -1;

	printf("vfs file I/O test ok\n");

	return 0;
}

test_case(test_vfs_file);

/* ---- mock filesystem, used to exercise the mount framework ---- */

static int mockfs_mount(struct vfs_mount *mnt, const char *source, void *data)
{
	struct vfs_node *root;

	(void)source;
	(void)data;

	root = vfs_alloc_dir_node("mockfs");
	if (!root)
		return -ENOMEM;

	if (!vfs_mkdir(root, "foo"))
		return -1;
	if (!vfs_mknod(root, "bar", VFS_NODE_REG, &mock_fops, NULL))
		return -1;

	mnt->root = root;

	return 0;
}

static struct vfs_fs_type mockfs_type = {
	.name = "mockfs",
	.mount = mockfs_mount,
};

static int test_vfs_mount(void)
{
	struct vfs_node *root;
	int fd;
	int ret;

	root = vfs_root();
	if (!root)
		return -1;

	if (!vfs_mkdir(root, "mnt"))
		return -1;

	ret = vfs_register_filesystem(&mockfs_type);
	if (ret)
		return ret;

	ret = vfs_mount(NULL, "/mnt", "mockfs", NULL);
	if (ret)
		return ret;

	/* path resolution crosses the mountpoint into the mounted fs */
	if (!vfs_lookup("/mnt/foo"))
		return -1;
	if (!vfs_lookup("/mnt/bar"))
		return -1;

	/* a file inside the mounted fs is openable */
	fd = vfs_open("/mnt/bar", 0);
	if (fd < 0)
		return -1;
	if (vfs_close(fd) != 0)
		return -1;

	ret = vfs_umount("/mnt");
	if (ret)
		return ret;

	/* after umount the mountpoint is an empty directory again */
	if (vfs_lookup("/mnt/foo"))
		return -1;

	printf("vfs mount test ok\n");

	return 0;
}

test_case(test_vfs_mount);

/* ---- vfs_mkdir_p: nested mount-point creation ---- */

static int test_vfs_mkdir_p(void)
{
	struct vfs_node *leaf;
	struct vfs_node *dir;

	/* build a nested path from scratch */
	leaf = vfs_mkdir_p("/a/b/c");
	if (!leaf || leaf->type != VFS_NODE_DIR)
		return -1;
	if (!vfs_lookup("/a/b/c"))
		return -1;

	/* idempotent: reusing existing components returns the same leaf */
	if (vfs_mkdir_p("/a/b/c") != leaf)
		return -1;

	/* extend an existing prefix */
	dir = vfs_mkdir_p("/a/b/d");
	if (!dir)
		return -1;
	if (!vfs_lookup("/a/b/d"))
		return -1;

	/* a component that exists as a file is rejected */
	if (!vfs_mknod(dir, "f", VFS_NODE_REG, &mock_fops, NULL))
		return -1;
	if (vfs_mkdir_p("/a/b/d/f/x"))
		return -1;

	printf("vfs mkdir_p test ok\n");

	return 0;
}

test_case(test_vfs_mkdir_p);

/* ---- mock on-demand filesystem, exercises dynamic lookup ---- */

static ssize_t dynfs_read(struct vfs_file *file, void *buf, size_t len)
{
	const char *content = file->private_data;
	size_t clen;
	size_t pos = file->pos;

	if (!content)
		return 0;
	clen = strlen(content);
	if (pos >= clen)
		return 0;
	if (len > clen - pos)
		len = clen - pos;

	memcpy(buf, content + pos, len);

	return (ssize_t)len;
}

static const struct vfs_file_ops dynfs_fops = {
	.read = dynfs_read,
};

/*
 * A filesystem whose entries are NOT pre-built: nodes are materialized into
 * the tree on demand in lookup(). Layout: /foo (file), /sub/ (dir),
 * /sub/bar (file).
 */
static struct vfs_node *dynfs_lookup(struct vfs_mount *mnt, struct vfs_node *dir,
				     const char *name)
{
	(void)mnt;

	if (!strcmp(dir->name, "dynfs")) {
		if (!strcmp(name, "foo"))
			return vfs_mknod(dir, name, VFS_NODE_REG,
					 &dynfs_fops, strdup("FOO-CONTENT"));
		if (!strcmp(name, "sub"))
			return vfs_mkdir(dir, name);
		return NULL;
	}

	if (!strcmp(dir->name, "sub")) {
		if (!strcmp(name, "bar"))
			return vfs_mknod(dir, name, VFS_NODE_REG,
					 &dynfs_fops, strdup("BAR-CONTENT"));
		return NULL;
	}

	return NULL;
}

static const struct vfs_super_ops dynfs_sops = {
	.lookup = dynfs_lookup,
};

static int dynfs_mount(struct vfs_mount *mnt, const char *source, void *data)
{
	struct vfs_node *root;

	(void)source;
	(void)data;

	root = vfs_alloc_dir_node("dynfs");
	if (!root)
		return -ENOMEM;

	mnt->root = root;
	mnt->sops = &dynfs_sops;

	return 0;
}

static struct vfs_fs_type dynfs_type = {
	.name = "dynfs",
	.mount = dynfs_mount,
};

static int test_vfs_dynamic(void)
{
	struct vfs_node *root;
	char out[16];
	int fd;
	ssize_t n;

	root = vfs_root();
	if (!root)
		return -1;

	if (!vfs_mkdir(root, "dyn"))
		return -1;
	if (vfs_register_filesystem(&dynfs_type))
		return -1;
	if (vfs_mount(NULL, "/dyn", "dynfs", NULL))
		return -1;

	/* file materialized on demand at the mount root (no pre-built child) */
	fd = vfs_open("/dyn/foo", 0);
	if (fd < 0)
		return -1;
	n = vfs_read(fd, out, sizeof(out));
	if (n != 11 || memcmp(out, "FOO-CONTENT", 11) != 0)
		return -1;
	if (vfs_close(fd) != 0)
		return -1;

	/* nested lookup: intermediate dir materialized into tree, target readable */
	fd = vfs_open("/dyn/sub/bar", 0);
	if (fd < 0)
		return -1;
	n = vfs_read(fd, out, sizeof(out));
	if (n != 11 || memcmp(out, "BAR-CONTENT", 11) != 0)
		return -1;
	if (vfs_close(fd) != 0)
		return -1;

	/* non-existent entry */
	if (vfs_open("/dyn/nope", 0) != -ENOENT)
		return -1;

	printf("vfs dynamic lookup test ok\n");

	return 0;
}

test_case(test_vfs_dynamic);

