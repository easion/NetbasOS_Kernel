
#ifndef __xen32_BOOT_H__
#define __xen32_BOOT_H__


#include <type.h>
struct  page_specifier;
typedef u32_t pfn_t;
typedef u32_t evtchn_t;

typedef u8_t ipl_t;

typedef u32_t unative_t;
typedef int native_t;
typedef struct  page_specifier page_specifier_t;


#define GUEST_CMDLINE	1024
#define VIRT_CPUS		32
#define START_INFO_SIZE	1104

#define BOOT_OFFSET		0x0000
#define TEMP_STACK_SIZE 0x1000

#define XEN_VIRT_START	0xFC000000
#define XEN_CS			0xe019




typedef struct {
	u32_t version;
	u32_t pad0;
	u64_t tsc_timestamp;   /**< TSC at last update of time vals */
	u64_t system_time;     /**< Time, in nanosecs, since boot */
	u32_t tsc_to_system_mul;
	u8_t tsc_shift;
	u8_t pad1[3];
} vcpu_time_info_t;

typedef struct {
	u32_t cr2;
	u32_t pad[5];
} arch_vcpu_info_t;

typedef struct arch_shared_info {
	pfn_t max_pfn;                  /**< max pfn that appears in table */
	u32_t pfn_to_mfn_frame_list_list;
    u32_t nmi_reason;
} arch_shared_info_t;

typedef struct {
	uint8_t evtchn_upcall_pending;
	ipl_t evtchn_upcall_mask;
	evtchn_t evtchn_pending_sel;
	arch_vcpu_info_t arch;
	vcpu_time_info_t time;
} vcpu_info_t;

typedef struct {
	vcpu_info_t vcpu_info[VIRT_CPUS];
	evtchn_t evtchn_pending[32];
	evtchn_t evtchn_mask[32];
	
	u32_t wc_version;                  /**< Version counter */
	u32_t wc_sec;                      /**< Secs  00:00:00 UTC, Jan 1, 1970 */
	u32_t wc_nsec;                     /**< Nsecs 00:00:00 UTC, Jan 1, 1970 */
	
	arch_shared_info_t arch;
} shared_info_t;

typedef struct {
	u8_t magic[32];           /**< "xen-<version>-<platform>" */
	u32_t frames;            /**< Available frames */
	shared_info_t *shared_info; /**< Shared info structure (machine address) */
	u32_t flags;             /**< SIF_xxx flags */
	pfn_t store_mfn;            /**< Shared page (machine page) */
	evtchn_t store_evtchn;      /**< Event channel for store communication */
	pfn_t console_mfn;          /**< Console page (machine page) */
	evtchn_t console_evtchn;    /**< Event channel for console messages */
	page_specifier_t *ptl0;                /**< Boot PTL0 (kernel address) */
	u32_t pt_frames;         /**< Number of bootstrap page table frames */
	pfn_t *pm_map;              /**< Physical->machine frame map (kernel address) */
	void *mod_start;            /**< Modules start (kernel address) */
	u32_t mod_len;           /**< Modules size (bytes) */
	u8_t cmd_line[GUEST_CMDLINE];
} start_info_t;

typedef struct {
	pfn_t start;
	pfn_t size;
	pfn_t reserved;
} memzone_t;

extern start_info_t start_info;
extern shared_info_t shared_info;
extern memzone_t meminfo;


#endif
