#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <lib/container_of.h>
#include <common/common.h>
#include <driver/alg.h>
#include <lib/cli.h>

static LIST_HEAD(alg_list);
static int alg_count;

void alg_release(struct akcipher_alg *alg)
{
	alg->ops->release(alg);	
}

struct akcipher_alg *alg_alloc()
{
	struct akcipher_alg *alg;

	alg = malloc(sizeof(struct akcipher_alg));
	if (!alg) {
		return NULL;
	}
	return alg;
}

struct akcipher_alg *alg_find_by_name(const char *name)
{
	struct list_head *p;
	struct base_alg *bp;
	struct akcipher_alg *alg;

	list_for_each(p, &alg_list) {
		bp = container_of(p, struct base_alg, list_head);
		alg = container_of(bp, struct akcipher_alg, base);
		if (strcmp(alg->name, name) == 0) {
			return alg;
		}
	}
	return NULL;

}

void alg_add(struct akcipher_alg *alg)
{
	list_add_tail(&alg->base.list_head, &alg_list);
}

int alg_register(struct akcipher_alg *alg)
{
	if (!alg->ops) {
		pr_err("failed: no alg ops\n");
		return -EINVAL;
	}

	alg_add(alg);
	return 0;
}

void alg_remove(struct akcipher_alg *alg)
{
	list_del(&alg->base.list_head);
}

void alg_unregister(struct akcipher_alg *alg)
{
	alg_remove(alg);
	alg_count--;
}

int alg_verify(struct akcipher_alg *alg, union akcipher_param *param)
{
	return alg->ops->verify(alg, param);
}

static void command_show_algs(struct command *c, int argc, const char *argv[])
{
	struct list_head *p;
	struct base_alg *bp;
	struct akcipher_alg *alg;
	int i;

	console_printf(command_get_console(c),
		       "%6s %14s %14s\n",
		       "Index", "Alg", "Alg driver");

	i = 0;
	list_for_each(p, &alg_list) {
		bp = container_of(p, struct base_alg, list_head);
		alg = container_of(bp, struct akcipher_alg, base);
		console_printf(command_get_console(c),
			       "%6d %14s %14s\n", i, alg->name, alg->base.name);
		++i;
	}
}

cli_command(lsalg, command_show_algs);
