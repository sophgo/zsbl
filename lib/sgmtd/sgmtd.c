/*
 * sophgo, mtd, boot device, with boot files
 * flash devices with a simple file table
 */

#include <errno.h>
#include <libfdt.h>

#include <framework/common.h>
#include <lib/container_of.h>
#include <driver/platform.h>
#include <driver/bootdev.h>
#include <driver/mtd.h>

struct sgmtd {
	struct bootdev *bootdev;
	struct mtd *mtd;
	unsigned long partition_table_offset;
};

static long get_file_size(struct bootdev *bootdev, const char *file)
{
	return 0;
}

static long load(struct bootdev *bootdev, const char *file, void *buf)
{
	return 0;
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

	/* check if it is mtd device that store config file */
	/* assume mtd device is platform device */
	if (!mtd->hwdev) {
		pr_err("hardware device instance is NULL\n");
		return -EINVAL;
	}

	pdev = container_of(mtd->hwdev, struct platform_device, device);

	prop = fdt_get_property(pdev->of, pdev->of_node_offset, "sophgo-boot", &plen);
	if (!prop) {
		pr_debug("device %s is not doesn't contain partition table\n", mtd->hwdev->name);
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

	err = snprintf(bootdev->device.name, sizeof(bootdev->device.name), "%s", mtd->device.name);
	if (err == sizeof(bootdev->device.name))
		pr_warn("boot device name overflow\n");

	bootdev_add(bootdev);

	return 0;
free_bootdev:
	bootdev_free(bootdev);
	return err;
}

int sgmtd_init(void)
{
	int err = 0;
	struct mtd *mtd;

	for (mtd = mtd_first(); mtd; mtd = mtd_next(mtd)) {
		pr_info("%s\n", mtd->device.name);
		/* create new boot devices */
		err |= create_bootdev(mtd);
		if (err) {
			pr_err("cannot create sg mtd device for device %s\n", mtd->device.name);
			continue;
		}
	}

	return err;
}
