/*
 * Minimal virtio-blk driver over virtio-mmio (modern / version 2).
 *
 * Polling only, single outstanding request at a time — sufficient for a
 * bootloader. Registers each block-capable virtio-mmio slot as a blkdev.
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>

#include <lib/mmio.h>
#include <driver/platform.h>
#include <driver/blkdev.h>
#include <common/common.h>
#include <common/module.h>

/* virtio-mmio register offsets */
#define VIRTIO_MMIO_MAGIC_VALUE		0x000
#define VIRTIO_MMIO_VERSION		0x004
#define VIRTIO_MMIO_DEVICE_ID		0x008
#define VIRTIO_MMIO_DEVICE_FEATURES	0x010
#define VIRTIO_MMIO_DEVICE_FEATURES_SEL	0x014
#define VIRTIO_MMIO_DRIVER_FEATURES	0x020
#define VIRTIO_MMIO_DRIVER_FEATURES_SEL	0x024
#define VIRTIO_MMIO_QUEUE_SEL		0x030
#define VIRTIO_MMIO_QUEUE_NUM_MAX	0x034
#define VIRTIO_MMIO_QUEUE_NUM		0x038
#define VIRTIO_MMIO_QUEUE_READY		0x044
#define VIRTIO_MMIO_QUEUE_NOTIFY	0x050
#define VIRTIO_MMIO_INTERRUPT_STATUS	0x060
#define VIRTIO_MMIO_INTERRUPT_ACK	0x064
#define VIRTIO_MMIO_STATUS		0x070
#define VIRTIO_MMIO_QUEUE_DESC_LOW	0x080
#define VIRTIO_MMIO_QUEUE_DESC_HIGH	0x084
#define VIRTIO_MMIO_QUEUE_DRIVER_LOW	0x090
#define VIRTIO_MMIO_QUEUE_DRIVER_HIGH	0x094
#define VIRTIO_MMIO_QUEUE_DEVICE_LOW	0x0a0
#define VIRTIO_MMIO_QUEUE_DEVICE_HIGH	0x0a4
#define VIRTIO_MMIO_CONFIG		0x100

#define VIRTIO_MAGIC			0x74726976	/* "virt" */
#define VIRTIO_VERSION_MODERN		2
#define VIRTIO_ID_BLOCK			2

/* device status bits */
#define VIRTIO_STATUS_ACKNOWLEDGE	1
#define VIRTIO_STATUS_DRIVER		2
#define VIRTIO_STATUS_DRIVER_OK		4
#define VIRTIO_STATUS_FEATURES_OK	8

/* VIRTIO_F_VERSION_1 is feature bit 32 -> bit 0 of feature word 1 */
#define VIRTIO_F_VERSION_1_HI		(1u << 0)

/* split virtqueue */
#define VIRTQ_DESC_F_NEXT		1
#define VIRTQ_DESC_F_WRITE		2

struct virtq_desc {
	uint64_t addr;
	uint32_t len;
	uint16_t flags;
	uint16_t next;
};

struct virtq_avail {
	uint16_t flags;
	uint16_t idx;
	uint16_t ring[];
};

struct virtq_used_elem {
	uint32_t id;
	uint32_t len;
};

struct virtq_used {
	uint16_t flags;
	uint16_t idx;
	struct virtq_used_elem ring[];
};

/* virtio-blk request header */
#define VIRTIO_BLK_T_IN			0
#define VIRTIO_BLK_T_OUT		1

struct virtio_blk_req {
	uint32_t type;
	uint32_t reserved;
	uint64_t sector;
};

#define VIRTIO_BLK_QSIZE		8

struct virtio_blk {
	uintptr_t base;
	unsigned int qsize;
	struct virtq_desc *desc;
	struct virtq_avail *avail;
	struct virtq_used *used;
	uint16_t last_used_idx;
	struct virtio_blk_req *req;
	volatile uint8_t *status;
	struct blkdev *blkdev;
};

static inline void virtio_fence(void)
{
	asm volatile ("fence rw, rw" ::: "memory");
}

static long virtio_blk_rw(struct virtio_blk *vb, unsigned long sector,
			  unsigned long count, void *buf, int write)
{
	unsigned long bytes = count * 512;

	vb->req->type = write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN;
	vb->req->reserved = 0;
	vb->req->sector = sector;
	*vb->status = 0xff;

	/* header (device-readable) -> data -> status (device-writable) */
	vb->desc[0].addr = (uint64_t)(uintptr_t)vb->req;
	vb->desc[0].len = sizeof(*vb->req);
	vb->desc[0].flags = VIRTQ_DESC_F_NEXT;
	vb->desc[0].next = 1;

	vb->desc[1].addr = (uint64_t)(uintptr_t)buf;
	vb->desc[1].len = bytes;
	vb->desc[1].flags = VIRTQ_DESC_F_NEXT | (write ? 0 : VIRTQ_DESC_F_WRITE);
	vb->desc[1].next = 2;

	vb->desc[2].addr = (uint64_t)(uintptr_t)vb->status;
	vb->desc[2].len = 1;
	vb->desc[2].flags = VIRTQ_DESC_F_WRITE;
	vb->desc[2].next = 0;

	vb->avail->ring[vb->avail->idx % vb->qsize] = 0;
	virtio_fence();
	vb->avail->idx++;
	virtio_fence();

	mmio_write_32(vb->base + VIRTIO_MMIO_QUEUE_NOTIFY, 0);

	while (vb->used->idx == vb->last_used_idx)
		virtio_fence();
	virtio_fence();
	vb->last_used_idx++;

	if (*vb->status != 0)
		return -EIO;

	return bytes;
}

static long virtio_blk_read(struct blkdev *blkdev, unsigned long offset,
			    unsigned long size, void *buf)
{
	return virtio_blk_rw(blkdev->data, offset / 512, size / 512, buf, 0);
}

static long virtio_blk_write(struct blkdev *blkdev, unsigned long offset,
			     unsigned long size, void *buf)
{
	return virtio_blk_rw(blkdev->data, offset / 512, size / 512, buf, 1);
}

static struct blkops virtio_blkops = {
	.read = virtio_blk_read,
	.write = virtio_blk_write,
};

static int virtio_blk_setup_queue(struct virtio_blk *vb)
{
	unsigned int qmax;

	mmio_write_32(vb->base + VIRTIO_MMIO_QUEUE_SEL, 0);

	if (mmio_read_32(vb->base + VIRTIO_MMIO_QUEUE_READY) != 0) {
		pr_err("virtio-blk: queue 0 already in use\n");
		return -EBUSY;
	}

	qmax = mmio_read_32(vb->base + VIRTIO_MMIO_QUEUE_NUM_MAX);
	if (qmax == 0)
		return -ENODEV;

	vb->qsize = qmax < VIRTIO_BLK_QSIZE ? qmax : VIRTIO_BLK_QSIZE;

	vb->desc = malloc(sizeof(struct virtq_desc) * vb->qsize);
	vb->avail = malloc(sizeof(struct virtq_avail) + sizeof(uint16_t) * (vb->qsize + 1));
	vb->used = malloc(sizeof(struct virtq_used) + sizeof(struct virtq_used_elem) * vb->qsize);
	vb->req = malloc(sizeof(struct virtio_blk_req));
	vb->status = malloc(1);
	if (!vb->desc || !vb->avail || !vb->used || !vb->req || !vb->status)
		return -ENOMEM;

	memset(vb->desc, 0, sizeof(struct virtq_desc) * vb->qsize);
	memset(vb->avail, 0, sizeof(struct virtq_avail) + sizeof(uint16_t) * (vb->qsize + 1));
	memset(vb->used, 0, sizeof(struct virtq_used) + sizeof(struct virtq_used_elem) * vb->qsize);
	vb->last_used_idx = 0;

	mmio_write_32(vb->base + VIRTIO_MMIO_QUEUE_NUM, vb->qsize);

	mmio_write_32(vb->base + VIRTIO_MMIO_QUEUE_DESC_LOW,
		      (uint32_t)(uintptr_t)vb->desc);
	mmio_write_32(vb->base + VIRTIO_MMIO_QUEUE_DESC_HIGH,
		      (uint32_t)((uint64_t)(uintptr_t)vb->desc >> 32));
	mmio_write_32(vb->base + VIRTIO_MMIO_QUEUE_DRIVER_LOW,
		      (uint32_t)(uintptr_t)vb->avail);
	mmio_write_32(vb->base + VIRTIO_MMIO_QUEUE_DRIVER_HIGH,
		      (uint32_t)((uint64_t)(uintptr_t)vb->avail >> 32));
	mmio_write_32(vb->base + VIRTIO_MMIO_QUEUE_DEVICE_LOW,
		      (uint32_t)(uintptr_t)vb->used);
	mmio_write_32(vb->base + VIRTIO_MMIO_QUEUE_DEVICE_HIGH,
		      (uint32_t)((uint64_t)(uintptr_t)vb->used >> 32));

	mmio_write_32(vb->base + VIRTIO_MMIO_QUEUE_READY, 1);

	return 0;
}

static int probe(struct platform_device *pdev)
{
	struct virtio_blk *vb;
	struct blkdev *blkdev;
	uintptr_t base = pdev->reg_base;
	uint32_t status;
	uint64_t capacity;
	int ret;

	if (mmio_read_32(base + VIRTIO_MMIO_MAGIC_VALUE) != VIRTIO_MAGIC)
		return -ENODEV;
	if (mmio_read_32(base + VIRTIO_MMIO_VERSION) != VIRTIO_VERSION_MODERN)
		return -ENODEV;
	if (mmio_read_32(base + VIRTIO_MMIO_DEVICE_ID) != VIRTIO_ID_BLOCK)
		return -ENODEV;	/* empty slot or non-block device */

	vb = malloc(sizeof(*vb));
	if (!vb)
		return -ENOMEM;
	memset(vb, 0, sizeof(*vb));
	vb->base = base;

	/* reset and bring up the device */
	mmio_write_32(base + VIRTIO_MMIO_STATUS, 0);
	status = VIRTIO_STATUS_ACKNOWLEDGE;
	mmio_write_32(base + VIRTIO_MMIO_STATUS, status);
	status |= VIRTIO_STATUS_DRIVER;
	mmio_write_32(base + VIRTIO_MMIO_STATUS, status);

	/* negotiate: accept only VIRTIO_F_VERSION_1 */
	mmio_write_32(base + VIRTIO_MMIO_DRIVER_FEATURES_SEL, 0);
	mmio_write_32(base + VIRTIO_MMIO_DRIVER_FEATURES, 0);
	mmio_write_32(base + VIRTIO_MMIO_DRIVER_FEATURES_SEL, 1);
	mmio_write_32(base + VIRTIO_MMIO_DRIVER_FEATURES, VIRTIO_F_VERSION_1_HI);

	status |= VIRTIO_STATUS_FEATURES_OK;
	mmio_write_32(base + VIRTIO_MMIO_STATUS, status);
	if (!(mmio_read_32(base + VIRTIO_MMIO_STATUS) & VIRTIO_STATUS_FEATURES_OK)) {
		pr_err("virtio-blk: FEATURES_OK rejected\n");
		ret = -ENODEV;
		goto err;
	}

	ret = virtio_blk_setup_queue(vb);
	if (ret)
		goto err;

	status |= VIRTIO_STATUS_DRIVER_OK;
	mmio_write_32(base + VIRTIO_MMIO_STATUS, status);

	/* virtio_blk_config.capacity: first u64 in config space, in 512B sectors */
	capacity = mmio_read_64(base + VIRTIO_MMIO_CONFIG);

	blkdev = blkdev_alloc();
	if (!blkdev) {
		ret = -ENOMEM;
		goto err;
	}

	blkdev->block_size = 512;
	blkdev->total_size = capacity * 512;
	blkdev->ops = &virtio_blkops;
	blkdev->data = vb;
	vb->blkdev = blkdev;

	device_set_child_name(&blkdev->device, &pdev->device, "block");

	ret = blkdev_register(blkdev);
	if (ret)
		goto err;

	pr_info("virtio-blk: %lu sectors (%lu MiB) at %lx\n",
		(unsigned long)capacity,
		(unsigned long)(capacity * 512 / (1024 * 1024)), base);

	return 0;

err:
	free(vb);
	return ret;
}

static struct of_device_id match_table[] = {
	{
		.compatible = "virtio,mmio",
		.data = NULL,
	},
	{},
};

static struct platform_driver driver = {
	.driver.name = "virtio-blk",
	.probe = probe,
	.of_match_table = match_table,
};

static int virtio_blk_init(void)
{
	platform_driver_register(&driver);
	return 0;
}

module_init(virtio_blk_init);
