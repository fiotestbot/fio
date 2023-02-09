/*
 * Note: This is similar to a very basic setup
 * of ZBD devices
 *
 * Specify fdp=1 (With char devices /dev/ng0n1)
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "file.h"
#include "fio.h"

#include "pshared.h"
#include <fdp.h>

/*
 * Maximum number of RUHS to fetch
 * TODO: Revisit this so that we can work with more than 1024 RUH's
 */
#define FDP_FETCH_RUHS_MAX	1024U

static int fdp_ruh_info(struct thread_data *td, struct fio_file *f,
			struct fio_ruhs_info *ruhs)
{
	int ret = -EINVAL;

	if (td->io_ops && td->io_ops->fdp_fetch_ruhs)
		ret = td->io_ops->fdp_fetch_ruhs(td, f, ruhs);
	else
		log_err("%s: engine (%s) lacks fetch ruhs.\n",
			f->file_name, td->io_ops->name);
	if (ret < 0) {
		td_verror(td, errno, "fdp fetch ruhs failed");
		log_err("%s: fdp fetch ruhs failed (%d).\n",
			f->file_name, errno);
	}

	return ret;
}

static int init_ruh_info(struct thread_data *td, struct fio_file *f)
{
	struct fio_ruhs_info *ruhs, *tmp;
	int i, ret;

	ruhs = calloc(1, sizeof(*ruhs) +
			   FDP_FETCH_RUHS_MAX * sizeof(*ruhs->plis));
	if (!ruhs)
		return -ENOMEM;

	ret = fdp_ruh_info(td, f, ruhs);
	if (ret) {
		log_info("fio: ruh info failed for %s (%d).\n",
			 f->file_name, -ret);
		goto out;
	}

	if (ruhs->nr_ruhs > FDP_FETCH_RUHS_MAX)
		ruhs->nr_ruhs = FDP_FETCH_RUHS_MAX;

	if (td->o.fdp_nrpli == 0) {
		f->ruhs_info = ruhs;
		return 0;
	}

	for (i = 0; i < td->o.fdp_nrpli; i++) {
		if (td->o.fdp_plis[i] > ruhs->nr_ruhs) {
			ret = -EINVAL;
			goto out;
		}
	}

	tmp = calloc(1, sizeof(*tmp) + ruhs->nr_ruhs * sizeof(*tmp->plis));
	tmp->nr_ruhs = td->o.fdp_nrpli;
	for (i = 0; i < td->o.fdp_nrpli; i++)
		tmp->plis[i] = ruhs->plis[td->o.fdp_plis[i]];
	f->ruhs_info = tmp;
out:
	free(ruhs);
	return ret;
}

int fdp_init(struct thread_data *td)
{
	struct fio_file *f;
	int i, ret = 0;

	for_each_file(td, f, i) {
		ret = init_ruh_info(td, f);
		if (ret)
			break;
	}
	return ret;
}

void fdp_free_ruhs_info(struct fio_file *f)
{
	if (!f->ruhs_info)
		return;
	free(f->ruhs_info);
	f->ruhs_info = NULL;
}

void fdp_fill_dspec_data(struct thread_data *td, struct io_u *io_u)
{
	struct fio_file *f = io_u->file;
	struct fio_ruhs_info *ruhs = f->ruhs_info;
	int dspec;

	if (!ruhs || io_u->ddir != DDIR_WRITE) {
		io_u->dtype = 0;
		io_u->dspec = 0;
		return;
	}

	dspec = ruhs->plis[ruhs->pli_loc++ % ruhs->nr_ruhs];
	io_u->dtype = 2;
	io_u->dspec = dspec;
}
