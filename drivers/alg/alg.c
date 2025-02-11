#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <lib/container_of.h>
#include <framework/common.h>
#include <driver/alg.h>
#include <lib/cli.h>

static LIST_HEAD(alg_list);
static int alg_count;

void alg_release(struct akchipher_alg *alg)
{
	alg->ops->release(alg);	
}

struct akchipher_alg *alg_alloc()
{
	struct akchipher_alg *alg;

	alg = malloc(sizeof(struct akchipher_alg));
	if (!alg) {
		return NULL;
	}
	return alg;
}

struct akchipher_alg *alg_find_by_name(const char *name)
{
	struct list_head *p;
	struct base_alg *bp;
	struct akchipher_alg *alg;

	list_for_each(p, &alg_list) {
		bp = container_of(p, struct base_alg, list_head);
		alg = container_of(bp, struct akchipher_alg, base);
		if (strcmp(alg->name, name) == 0) {
			return alg;
		}
	}
	return NULL;

}

void alg_add(struct akchipher_alg *alg)
{
	list_add_tail(&alg->base.list_head, &alg_list);
}

int alg_register(struct akchipher_alg *alg)
{
	if (!alg->ops) {
		pr_err("failed: no alg ops\n");
		return -EINVAL;
	}

	alg_add(alg);
	return 0;
}

void alg_remove(struct akchipher_alg *alg)
{
	list_del(&alg->base.list_head);
}

void alg_unregister(struct akchipher_alg *alg)
{
	alg_remove(alg);
	alg_count--;
}

int alg_verify(struct akchipher_alg *alg, union akchipher_param *param)
{
	return alg->ops->verify(alg, param);
}

static void command_show_algs(struct command *c, int argc, const char *argv[])
{
	struct list_head *p;
	struct base_alg *bp;
	struct akchipher_alg *alg;
	int i;

	console_printf(command_get_console(c),
		       "%6s %14s %14s\n",
		       "Index", "Alg", "Alg driver");

	i = 0;
	list_for_each(p, &alg_list) {
		bp = container_of(p, struct base_alg, list_head);
		alg = container_of(bp, struct akchipher_alg, base);
		console_printf(command_get_console(c),
			       "%6d %14s %14s\n", i, alg->name, alg->base.name);
		++i;
	}
}

cli_command(lsalg, command_show_algs);