#include "ioengines.h"
#include "fio.h"
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <time.h>

struct fio_page_fault_data {
    void *mmap_ptr;
    size_t mmap_sz;
    off_t mmap_off;
#ifdef CONFIG_HAVE_THP
    pthread_t mmap_thread;
    pthread_mutex_t mmap_lock;
    pthread_cond_t mmap_cond;
    int mmap_thread_exit;
    int mmap_thread_started;
    unsigned int hugepage_delay;
#endif
};

#ifdef CONFIG_HAVE_THP
static void *mmap_delay_thread(void *data)
{
    struct fio_page_fault_data *fpd = data;
    struct timespec req;
    int ret;

    clock_gettime(CLOCK_REALTIME, &req);
    req.tv_sec += fpd->hugepage_delay / 1000;
    req.tv_nsec += (fpd->hugepage_delay % 1000) * 1000000;
    if (req.tv_nsec >= 1000000000) {
        req.tv_sec++;
        req.tv_nsec -= 1000000000;
    }

    pthread_mutex_lock(&fpd->mmap_lock);
    while (!fpd->mmap_thread_exit) {
        ret = pthread_cond_timedwait(&fpd->mmap_cond, &fpd->mmap_lock, &req);
        if (ret == ETIMEDOUT)
            break;
    }

    if (!fpd->mmap_thread_exit) {
        dprint(FD_MEM, "fio: madvising hugepage\n");
        ret = madvise(fpd->mmap_ptr, fpd->mmap_sz, MADV_HUGEPAGE);
        if (ret < 0)
            log_err("fio: madvise hugepage failed: %d\n", errno);
    }
    pthread_mutex_unlock(&fpd->mmap_lock);

    return NULL;
}
#endif

static int fio_page_fault_init(struct thread_data *td)
{
    size_t total_io_size;
    struct fio_page_fault_data *fpd = calloc(1, sizeof(*fpd));
    if (!fpd)
        return 1;

    total_io_size = td->o.size;
    fpd->mmap_sz = total_io_size;
    fpd->mmap_off = 0;
    fpd->mmap_ptr = mmap(NULL, total_io_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (fpd->mmap_ptr == MAP_FAILED)
    {
        free(fpd);
        return 1;
    }

    if (td->o.hugepage_delay) {
#ifdef CONFIG_HAVE_THP
        fpd->hugepage_delay = td->o.hugepage_delay;
        madvise(fpd->mmap_ptr, fpd->mmap_sz, MADV_NOHUGEPAGE);

        pthread_mutex_init(&fpd->mmap_lock, NULL);
        pthread_cond_init(&fpd->mmap_cond, NULL);
        fpd->mmap_thread_exit = 0;
        if (pthread_create(&fpd->mmap_thread, NULL, mmap_delay_thread, fpd)) {
            log_err("fio: failed to create mmap delay thread\n");
            pthread_cond_destroy(&fpd->mmap_cond);
            pthread_mutex_destroy(&fpd->mmap_lock);
            fpd->hugepage_delay = 0;
            fpd->mmap_thread_started = 0;
        } else {
            fpd->mmap_thread_started = 1;
        }
#endif
    }

    FILE_SET_ENG_DATA(td->files[0], fpd);
	return 0;
}

static int fio_page_fault_prep(struct thread_data *td, struct io_u *io_u)
{
	return 0;
}

static enum fio_q_status fio_page_fault_queue(struct thread_data *td, struct io_u *io_u)
{
    void * mmap_head;
    struct fio_page_fault_data *fpd = FILE_ENG_DATA(io_u->file);
    if (!fpd)
        return 1;

    if (io_u->offset + io_u->buflen > fpd->mmap_sz)
        return 1;

    mmap_head = fpd->mmap_ptr + io_u->offset;
    switch (io_u->ddir)
    {
        case DDIR_READ:
            for (size_t i = 0; i < io_u->buflen; i++)
            {
                ((unsigned char *)(io_u->xfer_buf))[i] = ((unsigned char *)(mmap_head))[i];
            }
            break;
        case DDIR_WRITE:
            for (size_t i = 0; i < io_u->buflen; i++)
            {
                ((unsigned char *)(mmap_head))[i] = ((unsigned char *)(io_u->xfer_buf))[i];
            }
            break;
        default:
            return 1;
    }

	return FIO_Q_COMPLETED;
}

static int fio_page_fault_open_file(struct thread_data *td, struct fio_file *f)
{
	return 0;
}

static int fio_page_fault_close_file(struct thread_data *td, struct fio_file *f)
{
    struct fio_page_fault_data *fpd = FILE_ENG_DATA(f);

    if (fpd) {
#ifdef CONFIG_HAVE_THP
        if (fpd->mmap_thread_started) {
            pthread_mutex_lock(&fpd->mmap_lock);
            fpd->mmap_thread_exit = 1;
            pthread_cond_signal(&fpd->mmap_cond);
            pthread_mutex_unlock(&fpd->mmap_lock);
            pthread_join(fpd->mmap_thread, NULL);
            pthread_cond_destroy(&fpd->mmap_cond);
            pthread_mutex_destroy(&fpd->mmap_lock);
            fpd->mmap_thread_started = 0;
        }
#endif

        if (fpd->mmap_ptr && fpd->mmap_sz)
            munmap(fpd->mmap_ptr, fpd->mmap_sz);
        free(fpd);
    }

	return 0;
}

static struct ioengine_ops ioengine = {
	.name		= "page_fault",
	.version	= FIO_IOOPS_VERSION,
	.init		= fio_page_fault_init,
	.prep		= fio_page_fault_prep,
	.queue		= fio_page_fault_queue,
	.open_file	= fio_page_fault_open_file,
	.close_file	= fio_page_fault_close_file,
	.get_file_size	= generic_get_file_size,
	.flags		= FIO_SYNCIO | FIO_NOEXTEND | FIO_DISKLESSIO,
};

static void fio_init fio_page_fault_register(void)
{
	register_ioengine(&ioengine);
}

static void fio_exit fio_page_fault_unregister(void)
{
	unregister_ioengine(&ioengine);
}