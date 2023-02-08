/*
 * nvme structure declarations and helper functions for the
 * io_uring_cmd engine.
 */

#ifndef FIO_NVME_H
#define FIO_NVME_H

#include <linux/nvme_ioctl.h>
#include "../fio.h"

/*
 * If the uapi headers installed on the system lacks nvme uring command
 * support, use the local version to prevent compilation issues.
 */
#ifndef CONFIG_NVME_URING_CMD
struct nvme_uring_cmd {
	__u8	opcode;
	__u8	flags;
	__u16	rsvd1;
	__u32	nsid;
	__u32	cdw2;
	__u32	cdw3;
	__u64	metadata;
	__u64	addr;
	__u32	metadata_len;
	__u32	data_len;
	__u32	cdw10;
	__u32	cdw11;
	__u32	cdw12;
	__u32	cdw13;
	__u32	cdw14;
	__u32	cdw15;
	__u32	timeout_ms;
	__u32   rsvd2;
};

#define NVME_URING_CMD_IO	_IOWR('N', 0x80, struct nvme_uring_cmd)
#define NVME_URING_CMD_IO_VEC	_IOWR('N', 0x81, struct nvme_uring_cmd)
#endif /* CONFIG_NVME_URING_CMD */

#define NVME_DEFAULT_IOCTL_TIMEOUT 0
#define NVME_IDENTIFY_DATA_SIZE 4096
#define NVME_IDENTIFY_CSI_SHIFT 24

#define NVME_ZNS_ZRA_REPORT_ZONES 0
#define NVME_ZNS_ZRAS_FEAT_ERZ (1 << 16)
#define NVME_ZNS_ZSA_RESET 0x4
#define NVME_ZONE_TYPE_SEQWRITE_REQ 0x2

enum nvme_identify_cns {
	NVME_IDENTIFY_CNS_NS		= 0x00,
	NVME_IDENTIFY_CNS_CTRL		= 0x01,
	NVME_IDENTIFY_CNS_CSI_NS	= 0x05,
	NVME_IDENTIFY_CNS_CSI_CTRL	= 0x06,
};

enum nvme_csi {
	NVME_CSI_NVM			= 0,
	NVME_CSI_KV			= 1,
	NVME_CSI_ZNS			= 2,
};

enum nvme_admin_opcode {
	nvme_admin_identify		= 0x06,
	nvme_admin_directive_recv	= 0x1a,
};

enum nvme_io_opcode {
	nvme_cmd_write			= 0x01,
	nvme_cmd_read			= 0x02,
	nvme_cmd_io_mgmt_recv		= 0x12,
	nvme_zns_cmd_mgmt_send		= 0x79,
	nvme_zns_cmd_mgmt_recv		= 0x7a,
};

enum nvme_zns_zs {
	NVME_ZNS_ZS_EMPTY		= 0x1,
	NVME_ZNS_ZS_IMPL_OPEN		= 0x2,
	NVME_ZNS_ZS_EXPL_OPEN		= 0x3,
	NVME_ZNS_ZS_CLOSED		= 0x4,
	NVME_ZNS_ZS_READ_ONLY		= 0xd,
	NVME_ZNS_ZS_FULL		= 0xe,
	NVME_ZNS_ZS_OFFLINE		= 0xf,
};

struct nvme_data {
	__u32 nsid;
	__u32 lba_shift;
};

struct nvme_id_psd {
	__le16			mp;
	__u8			rsvd2;
	__u8			flags;
	__le32                  enlat;
	__le32                  exlat;
	__u8			rrt;
	__u8			rrl;
	__u8			rwt;
	__u8			rwl;
	__le16			idlp;
	__u8			ips;
	__u8			rsvd19;
	__le16			actp;
	__u8			apws;
	__u8			rsvd23[9];
};

struct nvme_id_ctrl {
	__le16			vid;
	__le16			ssvid;
	char			sn[20];
	char			mn[40];
	char			fr[8];
	__u8			rab;
	__u8			ieee[3];
	__u8			cmic;
	__u8			mdts;
	__le16			cntlid;
	__le32			ver;
	__le32			rtd3r;
	__le32			rtd3e;
	__le32			oaes;
	__le32			ctratt;
	__le16			rrls;
	__u8			rsvd102[9];
	__u8			cntrltype;
	__u8			fguid[16];
	__le16			crdt1;
	__le16			crdt2;
	__le16			crdt3;
	__u8			rsvd134[119];
	__u8			nvmsr;
	__u8			vwci;
	__u8			mec;
	__le16			oacs;
	__u8			acl;
	__u8			aerl;
	__u8			frmw;
	__u8			lpa;
	__u8			elpe;
	__u8			npss;
	__u8			avscc;
	__u8			apsta;
	__le16			wctemp;
	__le16			cctemp;
	__le16			mtfa;
	__le32			hmpre;
	__le32			hmmin;
	__u8			tnvmcap[16];
	__u8			unvmcap[16];
	__le32			rpmbs;
	__le16			edstt;
	__u8			dsto;
	__u8			fwug;
	__le16			kas;
	__le16			hctma;
	__le16			mntmt;
	__le16			mxtmt;
	__le32			sanicap;
	__le32			hmminds;
	__le16			hmmaxd;
	__le16			nsetidmax;
	__le16			endgidmax;
	__u8			anatt;
	__u8			anacap;
	__le32			anagrpmax;
	__le32			nanagrpid;
	__le32			pels;
	__le16			domainid;
	__u8			rsvd358[10];
	__u8			megcap[16];
	__u8			rsvd384[128];
	__u8			sqes;
	__u8			cqes;
	__le16			maxcmd;
	__le32			nn;
	__le16			oncs;
	__le16			fuses;
	__u8			fna;
	__u8			vwc;
	__le16			awun;
	__le16			awupf;
	__u8			icsvscc;
	__u8			nwpc;
	__le16			acwu;
	__le16			ocfs;
	__le32			sgls;
	__le32			mnan;
	__u8			maxdna[16];
	__le32			maxcna;
	__u8			rsvd564[204];
	char			subnqn[256];
	__u8			rsvd1024[768];

	/* Fabrics Only */
	__le32			ioccsz;
	__le32			iorcsz;
	__le16			icdoff;
	__u8			fcatt;
	__u8			msdbd;
	__le16			ofcs;
	__u8			dctype;
	__u8			rsvd1807[241];

	struct nvme_id_psd	psd[32];
	__u8			vs[1024];
};


struct nvme_lbaf {
	__le16			ms;
	__u8			ds;
	__u8			rp;
};

struct nvme_id_ns {
	__le64			nsze;
	__le64			ncap;
	__le64			nuse;
	__u8			nsfeat;
	__u8			nlbaf;
	__u8			flbas;
	__u8			mc;
	__u8			dpc;
	__u8			dps;
	__u8			nmic;
	__u8			rescap;
	__u8			fpi;
	__u8			dlfeat;
	__le16			nawun;
	__le16			nawupf;
	__le16			nacwu;
	__le16			nabsn;
	__le16			nabo;
	__le16			nabspf;
	__le16			noiob;
	__u8			nvmcap[16];
	__le16			npwg;
	__le16			npwa;
	__le16			npdg;
	__le16			npda;
	__le16			nows;
	__le16			mssrl;
	__le32			mcl;
	__u8			msrc;
	__u8			rsvd81[11];
	__le32			anagrpid;
	__u8			rsvd96[3];
	__u8			nsattr;
	__le16			nvmsetid;
	__le16			endgid;
	__u8			nguid[16];
	__u8			eui64[8];
	struct nvme_lbaf	lbaf[16];
	__u8			rsvd192[192];
	__u8			vs[3712];
};

static inline int ilog2(uint32_t i)
{
	int log = -1;

	while (i) {
		i >>= 1;
		log++;
	}
	return log;
}

struct nvme_zns_lbafe {
	__le64	zsze;
	__u8	zdes;
	__u8	rsvd9[7];
};

struct nvme_zns_id_ns {
	__le16			zoc;
	__le16			ozcs;
	__le32			mar;
	__le32			mor;
	__le32			rrl;
	__le32			frl;
	__le32			rrl1;
	__le32			rrl2;
	__le32			rrl3;
	__le32			frl1;
	__le32			frl2;
	__le32			frl3;
	__le32			numzrwa;
	__le16			zrwafg;
	__le16			zrwasz;
	__u8			zrwacap;
	__u8			rsvd53[2763];
	struct nvme_zns_lbafe	lbafe[64];
	__u8			vs[256];
};

struct nvme_zns_desc {
	__u8	zt;
	__u8	zs;
	__u8	za;
	__u8	zai;
	__u8	rsvd4[4];
	__le64	zcap;
	__le64	zslba;
	__le64	wp;
	__u8	rsvd32[32];
};

struct nvme_zone_report {
	__le64			nr_zones;
	__u8			rsvd8[56];
	struct nvme_zns_desc	entries[];
};

struct nvme_id_directives {
	__u8	supported[32];
	__u8	enabled[32];
	__u8	rsvd64[4032];
};

struct nvme_fdp_ruh_status_desc {
        __u16 pid;
        __u16 ruhid;
        __u32 earutr;
        __u64 ruamw;
        __u8  rsvd16[16];
};

struct nvme_fdp_ruh_status {
        __u8  rsvd0[14];
        __le16 nruhsd;
        struct nvme_fdp_ruh_status_desc ruhss[];
};

int fio_nvme_iomgmt_ruhs(struct thread_data *td, struct fio_file *f,
			 struct nvme_fdp_ruh_status *ruhs, __u32 bytes);

int fio_nvme_is_fdp(struct thread_data *td, struct fio_file *f, bool *fdp);

int fio_nvme_get_info(struct fio_file *f, __u32 *nsid, __u32 *lba_sz,
		      __u64 *nlba);

int fio_nvme_uring_cmd_prep(struct nvme_uring_cmd *cmd, struct io_u *io_u,
			    struct iovec *iov);

int fio_nvme_get_zoned_model(struct thread_data *td, struct fio_file *f,
			     enum zbd_zoned_model *model);

int fio_nvme_report_zones(struct thread_data *td, struct fio_file *f,
			  uint64_t offset, struct zbd_zone *zbdz,
			  unsigned int nr_zones);

int fio_nvme_reset_wp(struct thread_data *td, struct fio_file *f,
		      uint64_t offset, uint64_t length);

int fio_nvme_get_max_open_zones(struct thread_data *td, struct fio_file *f,
				unsigned int *max_open_zones);

#endif
