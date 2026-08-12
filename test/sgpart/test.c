/*
 * sgpart filesystem test.
 *
 * Backs a mock block device with an in-RAM sophgo partition table (two
 * partitions) plus their data, mounts sgpart on it, and checks that each
 * partition surfaced as a file, that reads return the partition's bytes
 * (including offset/size windowing and short reads at EOF), and that a
 * mount at an offset with no DPT magic is rejected.
 *
 * Self-contained: it does not depend on real flash contents, so it runs
 * anywhere the VFS and block layers are built.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <common/module.h>
#include <common/common.h>
#include <common/vfs.h>
#include <driver/blkdev.h>
#include <fs/sgpart.h>

#define DPT_MAGIC	0x55aa55aaU

struct part_info {
	uint32_t magic;
	char name[32];
	uint32_t offset;
	uint32_t size;
	char reserve[4];
	uint64_t lma;
};

/* Mock medium layout: table at 0, then two partition payloads. */
#define IMG_SIZE	512
#define P0_OFF		128
#define P1_OFF		256
static unsigned char img[IMG_SIZE];

static const char p0_data[] = "opensbi-payload";
static const char p1_data[] = "edk2-fd";

static long mock_read(struct blkdev *blkdev, unsigned long offset,
		      unsigned long size, void *buf)
{
	(void)blkdev;
	if (offset >= IMG_SIZE)
		return 0;
	if (size > IMG_SIZE - offset)
		size = IMG_SIZE - offset;
	memcpy(buf, img + offset, size);
	return size;
}

static long mock_write(struct blkdev *blkdev, unsigned long offset,
		       unsigned long size, void *buf)
{
	(void)blkdev; (void)offset; (void)size; (void)buf;
	return -ENOSYS;
}

static struct blkops mock_blkops = {
	.read = mock_read,
	.write = mock_write,
};

static void put_part(unsigned int idx, const char *name,
		     uint32_t offset, uint32_t size)
{
	struct part_info pi;

	memset(&pi, 0, sizeof(pi));
	pi.magic = DPT_MAGIC;
	strncpy(pi.name, name, sizeof(pi.name));
	pi.offset = offset;
	pi.size = size;
	memcpy(img + idx * sizeof(pi), &pi, sizeof(pi));
}

static void build_image(void)
{
	memset(img, 0, sizeof(img));
	put_part(0, "opensbi", P0_OFF, sizeof(p0_data) - 1);
	put_part(1, "edk2", P1_OFF, sizeof(p1_data) - 1);
	/* entry 2 left zero -> magic mismatch terminates the table */
	memcpy(img + P0_OFF, p0_data, sizeof(p0_data) - 1);
	memcpy(img + P1_OFF, p1_data, sizeof(p1_data) - 1);
}

static int read_file(const char *path, char *buf, size_t len)
{
	int fd;
	ssize_t n;

	fd = vfs_open(path, 0);
	if (fd < 0)
		return fd;
	n = vfs_read(fd, buf, len);
	vfs_close(fd);
	return (int)n;
}

static int test_sgpart(void)
{
	struct blkdev blkdev;
	struct sgpart_data sd;
	struct vfs_node *root;
	char buf[32];
	int n;

	build_image();

	memset(&blkdev, 0, sizeof(blkdev));
	blkdev.ops = &mock_blkops;
	blkdev.block_size = 1;
	blkdev.total_size = IMG_SIZE;

	root = vfs_root();
	if (!root)
		return -1;
	if (!vfs_mkdir(root, "sgtest"))
		return -1;

	/* wrong offset (no DPT magic) must be rejected */
	sd.blkdev = &blkdev;
	sd.offset = P0_OFF;
	if (vfs_mount(NULL, "/sgtest", "sgpart", &sd) == 0) {
		pr_err("sgpart: mount at bogus offset should fail\n");
		return -1;
	}

	/* real table at offset 0 */
	sd.offset = 0;
	if (vfs_mount(NULL, "/sgtest", "sgpart", &sd)) {
		pr_err("sgpart: mount failed\n");
		return -1;
	}

	/* both partitions are files under the mount point */
	if (!vfs_lookup("/sgtest/opensbi") || !vfs_lookup("/sgtest/edk2")) {
		pr_err("sgpart: partition file missing\n");
		return -1;
	}

	/* contents match, windowed by the partition's offset/size */
	n = read_file("/sgtest/opensbi", buf, sizeof(buf));
	if (n != (int)sizeof(p0_data) - 1 || memcmp(buf, p0_data, n)) {
		pr_err("sgpart: opensbi content wrong (n=%d)\n", n);
		return -1;
	}
	n = read_file("/sgtest/edk2", buf, sizeof(buf));
	if (n != (int)sizeof(p1_data) - 1 || memcmp(buf, p1_data, n)) {
		pr_err("sgpart: edk2 content wrong (n=%d)\n", n);
		return -1;
	}

	if (vfs_umount("/sgtest")) {
		pr_err("sgpart: umount failed\n");
		return -1;
	}

	printf("sgpart test ok\n");

	return 0;
}

test_case(test_sgpart);
