/*
 * sophgo, mtd, boot device, with boot files
 * flash devices with a simple file table
 */

#include <errno.h>
#include <libfdt.h>

#include <common/common.h>
#include <common/module.h>
#include <lib/container_of.h>
#include <driver/platform.h>
#include <driver/bootdev.h>
#include <driver/mtd.h>

#define DPT_MAGIC	0x55aa55aaU

struct sgmtd {
	struct bootdev *bootdev;
	struct mtd *mtd;
	unsigned long partition_table_offset;
};

struct part_info {
	/* disk partition table magic number */
	uint32_t magic;
	char name[32];
	uint32_t offset;
	uint32_t size;
	char reserve[4];
	/* load memory address*/
	uint64_t lma;
};

static int get_part_info(struct mtd *mtd, uint32_t addr, struct part_info *info)
{
	long err;

	err = mtd_read(mtd, addr, sizeof(struct part_info), info);
	if (err != sizeof(struct part_info))
		return -ENOENT;

	if (info->magic != DPT_MAGIC) {
		pr_debug("Bad partition magic\n");
		return -EINVAL;
	}

	return 0;
}

static int get_part_info_by_name(struct mtd *mtd, uint32_t addr, const char *name, struct part_info *info)
{
	int ret;

	do {
		ret = get_part_info(mtd, addr, info);

		addr += sizeof(struct part_info);

	} while (!ret && strcmp(info->name, name));

	return ret;
}

static long get_file_size(struct bootdev *bootdev, const char *file)
{
	struct sgmtd *sgmtd;
	struct mtd *mtd;
	struct part_info part_info;
	int err;

	sgmtd = bootdev->data;
	mtd = sgmtd->mtd;

	err = get_part_info_by_name(mtd, sgmtd->partition_table_offset, file, &part_info);

	if (err)
		return -ENOENT;

	return part_info.size;
}

static long load(struct bootdev *bootdev, const char *file, void *buf)
{
	struct sgmtd *sgmtd;
	struct mtd *mtd;
	struct part_info part_info;
	long err;

	sgmtd = bootdev->data;
	mtd = sgmtd->mtd;

	err = get_part_info_by_name(mtd, sgmtd->partition_table_offset, file, &part_info);

	if (err)
		return -ENOENT;

	return mtd_read(mtd, part_info.offset, part_info.size, buf);
}

static struct bootdev_ops ops = {
	.load = load,
	.get_file_size = get_file_size,
};

static int create_bootdev(struct mtd *mtd)
{
	struct bootdev *bootdev;
	struct sgmtd *sgmtd;
	int err;
	struct platform_device *pdev;
	const struct fdt_property *prop;
	int plen;

	/* check if it is mtd device that store boot file */
	/* assume mtd device is platform device */
	if (!mtd->hwdev) {
		pr_err("hardware device instance is NULL\n");
		return -EINVAL;
	}

	pdev = container_of(mtd->hwdev, struct platform_device, device);

	prop = fdt_get_property(pdev->of, pdev->of_node_offset, "sophgo-boot", &plen);
	if (!prop) {
		pr_debug("device %s doesn't contain partition table\n", mtd->hwdev->name);
		return -ENOENT;
	}

	if (plen < 8) {
		pr_err("no partition table offset in device %s\n", mtd->hwdev->name);
		return -EINVAL;
	}

	bootdev = bootdev_alloc();

	if (!bootdev) {
		pr_err("allocate boot device failed\n");
		return -ENOMEM;
	}

	sgmtd = malloc(sizeof(struct sgmtd));

	if (!sgmtd) {
		pr_err("allocate sgmtd instance failed\n");
		err = -ENOMEM;
		goto free_bootdev;
	}


	/* only support address cells 2 */
	sgmtd->partition_table_offset = fdt64_to_cpu(*(volatile uint64_t *)prop->data);
	sgmtd->bootdev = bootdev;
	sgmtd->mtd = mtd;
	bootdev->data = sgmtd;
	bootdev->size = mtd->total_size;
	bootdev->ops = &ops;

	device_set_child_name(&bootdev->device, &mtd->device, "boot");

	bootdev_add(bootdev);

	return 0;
free_bootdev:
	bootdev_free(bootdev);
	return err;
}

int sgmtd_init(void)
{
	int err = 0, tmp;
	struct mtd *mtd;

	for (mtd = mtd_first(); mtd; mtd = mtd_next(mtd)) {
		/* create new boot devices */
		tmp = create_bootdev(mtd);

		if (tmp == -ENOENT)
			continue;

		if (tmp) {
			err |= tmp;
			pr_err("cannot create sgmtd device for device %s\n", mtd->device.name);
			continue;
		}
	}

	return err;
}

late_init(sgmtd_init);
