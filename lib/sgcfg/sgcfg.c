/*
 * sophgo, mtd, boot device, config file only
 * flash devices with a simple file table
 */

#include <errno.h>
#include <libfdt.h>

#include <framework/common.h>
#include <framework/module.h>
#include <lib/container_of.h>
#include <driver/platform.h>
#include <driver/bootdev.h>
#include <driver/mtd.h>

struct sgcfg {
	struct bootdev *bootdev;
	struct mtd *mtd;
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

/* support platform device only, we need some property in dtb */
static int create_bootdev(struct mtd *mtd)
{
	struct bootdev *bootdev;
	struct sgcfg *sgcfg;
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

	prop = fdt_get_property(pdev->of, pdev->of_node_offset, "sophgo-config", &plen);
	if (!prop) {
		pr_debug("device %s is not doesn't contain configs\n", mtd->hwdev->name);
		return -ENOENT;
	}

	bootdev = bootdev_alloc();

	if (!bootdev) {
		pr_err("allocate boot device failed\n");
		return -ENOMEM;
	}

	sgcfg = malloc(sizeof(struct sgcfg));

	if (!sgcfg) {
		pr_err("allocate sgcfg instance failed\n");
		err = -ENOMEM;
		goto free_bootdev;
	}

	sgcfg->bootdev = bootdev;
	sgcfg->mtd = mtd;
	bootdev->data = sgcfg;
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

int sgcfg_init(void)
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
			pr_err("cannot create sg mtd device for device %s\n", mtd->device.name);
			continue;
		}
	}

	return err;
}

late_init(sgcfg_init);
