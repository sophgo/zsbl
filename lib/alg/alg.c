#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <lib/container_of.h>
#include <framework/common.h>
#include <lib/alg.h>

static LIST_HEAD(alg_list);
static int alg_count;

void alg_release(struct akchipher_alg *alg)
{
	alg->ops->release(alg);	
}

int alg_list_all(void)
{
	struct list_head *p;
	struct base_alg *bp;
	struct akchipher_alg *alg;
	int i = 0;

	list_for_each(p, &alg_list) {
		bp = container_of(p, struct base_alg, list_head);
		alg = container_of(bp, struct akchipher_alg, base);
		pr_info("%s ", alg->base.name);
		i++;
	}

	return i;
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

	if(!alg->priv) {
		pr_err("failed: no alg drivers\n");
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

struct akchipher_alg *alg_first(void)
{
	return container_of(list_first_entry_or_null(&alg_list, struct base_alg, list_head), struct akchipher_alg, base);
}

struct akchipher_alg *alg_next(struct akchipher_alg *cur)
{
	if (list_is_last(&cur->base.list_head, &alg_list))
		return NULL;

	return container_of(list_next_entry(&cur->base, list_head), struct akchipher_alg, base);
}

int alg_sign(struct akchipher_alg *alg, struct akchipher_param *param)
{
	return alg->ops->sign(alg, param);
}

int alg_verify(struct akchipher_alg *alg, struct akchipher_param *param)
{
	return alg->ops->verify(alg, param);
}

int alg_set_pub_key(struct akchipher_alg *alg, struct akchipher_param *param)
{
	return alg->ops->set_pub_key(alg, param);
}

int alg_set_priv_key(struct akchipher_alg *alg, struct akchipher_param *param)
{
	return alg->ops->set_priv_key(alg, param);
}