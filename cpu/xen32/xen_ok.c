# 1 "xen.c"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "xen.c"
# 1 "boot.h" 1





# 1 "../../sys/type.h" 1




typedef unsigned long pte_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

typedef unsigned long uLongf;
# 30 "../../sys/type.h"
typedef enum {FALSE=0,TRUE=1} BOOL ;
typedef enum { false=0, true=1 } bool;
# 48 "../../sys/type.h"
typedef void (*func_t) (void);
typedef int (*fun_t) (char*);
typedef void daemon_thread_t (void*);

typedef int semaphore_t;

typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned long u32_t;
typedef unsigned long long int u64_t;

typedef char s8_t;
typedef short s16_t;
typedef long s32_t;
typedef long long int s64_t;

typedef long tid_t;
typedef short uid_t;
typedef int pid_t;
typedef unsigned long size_t;

typedef unsigned char gid_t;
typedef unsigned short dev_t;
typedef unsigned short ino_t;
typedef unsigned short mode_t;
typedef unsigned long time_t;
typedef long off_t;
typedef unsigned reg_t;
typedef unsigned short prot_t;

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
# 95 "../../sys/type.h"
typedef void *linear_t;




typedef unsigned char uint8;
typedef signed char sbyte,int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint,uint32;
typedef signed int int32;
typedef unsigned long long qword,uint64;
typedef signed long long int64;
typedef void *pvoid;
typedef int32 HFILE;
typedef int8 DOSRESULT;
typedef const char *string;
# 7 "boot.h" 2
struct page_specifier;
typedef u32_t pfn_t;
typedef u32_t evtchn_t;
typedef u32_t uintptr_t;

typedef u8_t ipl_t;

typedef u32_t unative_t;
typedef int native_t;
typedef struct page_specifier page_specifier_t;
# 32 "boot.h"
typedef struct {
        u32_t version;
        u32_t pad0;
        u64_t tsc_timestamp;
        u64_t system_time;
        u32_t tsc_to_system_mul;
        u8_t tsc_shift;
        u8_t pad1[3];
} vcpu_time_info_t;

typedef struct {
        u32_t cr2;
        u32_t pad[5];
} arch_vcpu_info_t;

typedef struct arch_shared_info {
        pfn_t max_pfn;
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
        vcpu_info_t vcpu_info[32];
        evtchn_t evtchn_pending[32];
        evtchn_t evtchn_mask[32];

        u32_t wc_version;
        u32_t wc_sec;
        u32_t wc_nsec;

        arch_shared_info_t arch;
} shared_info_t;

typedef struct {
        u8_t magic[32];
        u32_t frames;
        shared_info_t *shared_info;
        u32_t flags;
        pfn_t store_mfn;
        evtchn_t store_evtchn;
        pfn_t console_mfn;
        evtchn_t console_evtchn;
        page_specifier_t *ptl0;
        u32_t pt_frames;
        pfn_t *pm_map;
        void *mod_start;
        u32_t mod_len;
        u8_t cmd_line[1024];
} start_info_t;

typedef struct {
        pfn_t start;
        pfn_t size;
        pfn_t reserved;
} memzone_t;

extern start_info_t start_info;
extern shared_info_t shared_info;
extern memzone_t meminfo;
# 2 "xen.c" 2
# 1 "memstr.h" 1
# 23 "memstr.h"
static inline void * memcpy(void * dst, const void * src, size_t cnt)
{
        unative_t d0, d1, d2;

        __asm__ __volatile__(

                "rep movsl\n\t"

                "movl %4, %%ecx\n\t"

                "andl $3, %%ecx\n\t"

                "jz 1f\n\t"

                "rep movsb\n\t"

                "1:\n"
                : "=&c" (d0), "=&D" (d1), "=&S" (d2)
                : "0" ((unative_t) (cnt / 4)), "g" ((unative_t) cnt), "1" ((unative_t) dst), "2" ((unative_t) src)
                : "memory");

        return dst;
}
# 59 "memstr.h"
static inline int memcmp(const void * src, const void * dst, size_t cnt)
{
        uint32_t d0, d1, d2;
        int ret;

        __asm__ (
                "repe cmpsb\n\t"
                "je 1f\n\t"
                "movl %3, %0\n\t"
                "addl $1, %0\n\t"
                "1:\n"
                : "=a" (ret), "=%S" (d0), "=&D" (d1), "=&c" (d2)
                : "0" (0), "1" ((unative_t) src), "2" ((unative_t) dst), "3" ((unative_t) cnt)
        );

        return ret;
}
# 86 "memstr.h"
static inline void memsetw(uintptr_t dst, size_t cnt, uint16_t x)
{
        uint32_t d0, d1;

        __asm__ __volatile__ (
                "rep stosw\n\t"
                : "=&D" (d0), "=&c" (d1), "=a" (x)
                : "0" (dst), "1" (cnt), "2" (x)
                : "memory"
        );

}
# 108 "memstr.h"
static inline void memsetb(uintptr_t dst, size_t cnt, uint8_t x)
{
        uint32_t d0, d1;

        __asm__ __volatile__ (
                "rep stosb\n\t"
                : "=&D" (d0), "=&c" (d1), "=a" (x)
                : "0" (dst), "1" (cnt), "2" (x)
                : "memory"
        );

}
# 3 "xen.c" 2
# 1 "hypercall.h" 1






typedef u16_t domid_t;


typedef struct {
        u8_t vector;
        u8_t flags;
        u16_t cs;
        void *address;
} trap_info_t;


typedef struct {
        evtchn_t port;
} evtchn_send_t;

typedef struct {
        u32_t cmd;
        union {
                evtchn_send_t send;
    };
} evtchn_op_t;
# 194 "hypercall.h"
static inline int xen_console_io(const unsigned int cmd, const unsigned int count, const char *str)
{
        return ({ unative_t ret, __ign1, __ign2, __ign3; asm volatile ( "call hypercall_page + (" "18" " * 32)\n" : "=a" (ret), "=b" (__ign1), "=c" (__ign2), "=d" (__ign3) : "1" (cmd), "2" (count), "3" (str) : "memory" ); ret; });
}

static inline int xen_vm_assist(const unsigned int cmd, const unsigned int type)
{
    return ({ unative_t ret, __ign1, __ign2; asm volatile ( "call hypercall_page + (" "21" " * 32)\n" : "=a" (ret), "=b" (__ign1), "=c" (__ign2) : "1" (cmd), "2" (type) : "memory" ); ret; });
}

static inline int xen_set_callbacks(const unsigned int event_selector, const void *event_address, const unsigned int failsafe_selector, void *failsafe_address)
{
        return ({ unative_t ret, __ign1, __ign2, __ign3, __ign4; asm volatile ( "call hypercall_page + (" "4" " * 32)\n" : "=a" (ret), "=b" (__ign1), "=c" (__ign2), "=d" (__ign3), "=S" (__ign4) : "1" (event_selector), "2" (event_address), "3" (failsafe_selector), "4" (failsafe_address) : "memory" ); ret; });
}

static inline int xen_set_trap_table(const trap_info_t *table)
{
        return ({ unative_t ret, __ign1; asm volatile ( "call hypercall_page + (" "0" " * 32)\n" : "=a" (ret), "=b" (__ign1) : "1" (table) : "memory" ); ret; });
}

static inline int xen_version(const unsigned int cmd, const void *arg)
{
        return ({ unative_t ret, __ign1, __ign2; asm volatile ( "call hypercall_page + (" "17" " * 32)\n" : "=a" (ret), "=b" (__ign1), "=c" (__ign2) : "1" (cmd), "2" (arg) : "memory" ); ret; });
}

static inline int xen_notify_remote(evtchn_t channel)
{
    evtchn_op_t op;

    op.cmd = 4;
    op.send.port = channel;
    return ({ unative_t ret, __ign1; asm volatile ( "call hypercall_page + (" "16" " * 32)\n" : "=a" (ret), "=b" (__ign1) : "1" (&op) : "memory" ); ret; });
}
# 4 "xen.c" 2
# 1 "page.h" 1



struct page_specifier{
        unsigned present : 1;
        unsigned writeable : 1;
        unsigned uaccessible : 1;
        unsigned page_write_through : 1;
        unsigned page_cache_disable : 1;
        unsigned accessed : 1;
        unsigned dirty : 1;
        unsigned pat : 1;
        unsigned global : 1;
        unsigned soft_valid : 1;
        unsigned avl : 2;
        unsigned frame_address : 20;
} __attr_packet ;

typedef struct {
        u64_t ptr;
        union {
                u64_t val;
                page_specifier_t pte;
        };
} mmu_update_t;
# 134 "page.h"
typedef struct {
        unsigned int cmd;
        union {
                unsigned long mfn;
                unsigned long linear_addr;
        };
        union {
                unsigned int nr_ents;
                void *vcpumask;
        };
} mmuext_op_t;

static inline uintptr_t PFN2ADDR(pfn_t frame)
{
        return (uintptr_t)(frame << 12);
}

static inline pfn_t ADDR2PFN(uintptr_t addr)
{
        return (pfn_t)(addr >> 12);
}

static inline int xen_update_va_mapping(const void *va, const page_specifier_t pte, const unsigned int flags)
{
        return ({ unative_t ret, __ign1, __ign2, __ign3, __ign4; asm volatile ( "call hypercall_page + (" "14" " * 32)\n" : "=a" (ret), "=b" (__ign1), "=c" (__ign2), "=d" (__ign3), "=S" (__ign4) : "1" (va), "2" (pte), "3" (0), "4" (flags) : "memory" ); ret; });
}

static inline int xen_mmu_update(const mmu_update_t *req, const unsigned int count, unsigned int *success_count, domid_t domid)
{
        return ({ unative_t ret, __ign1, __ign2, __ign3, __ign4; asm volatile ( "call hypercall_page + (" "1" " * 32)\n" : "=a" (ret), "=b" (__ign1), "=c" (__ign2), "=d" (__ign3), "=S" (__ign4) : "1" (req), "2" (count), "3" (success_count), "4" (domid) : "memory" ); ret; });
}

static inline int xen_mmuext_op(const mmuext_op_t *op, const unsigned int count, unsigned int *success_count, domid_t domid)
{
        return ({ unative_t ret, __ign1, __ign2, __ign3, __ign4; asm volatile ( "call hypercall_page + (" "26" " * 32)\n" : "=a" (ret), "=b" (__ign1), "=c" (__ign2), "=d" (__ign3), "=S" (__ign4) : "1" (op), "2" (count), "3" (success_count), "4" (domid) : "memory" ); ret; });
}


extern uintptr_t stack_safe;
static inline int get_pt_flags(page_specifier_t *pt, int i)
{
        page_specifier_t *p = &pt[i];

        return (
                (!p->page_cache_disable)<<0 |
                (!p->present)<<1 |
                p->uaccessible<<2 |
                1<<3 |
                p->writeable<<4 |
                1<<5 |
                p->global<<6
        );
}

static inline void set_pt_flags(page_specifier_t *pt, int i, int flags)
{
        page_specifier_t *p = &pt[i];

        p->page_cache_disable = !(flags & (1<<0));
        p->present = !(flags & (1<<1));
        p->uaccessible = (flags & (1<<2)) != 0;
        p->writeable = (flags & (1<<4)) != 0;
        p->global = (flags & (1<<6)) != 0;




        p->soft_valid = true;
}
# 5 "xen.c" 2
# 1 "xconsole.h" 1






typedef struct {
        char in[1024];
        char out[2048];
    u32_t in_cons;
        u32_t in_prod;
    u32_t out_cons;
        u32_t out_prod;
} xencons_t;

extern xencons_t console_page;

extern void xen_console_init(void);
# 6 "xen.c" 2
start_info_t start_info;
memzone_t meminfo;

extern void xen_callback(void);
extern void xen_failsafe_callback(void);

void arch_pre_main(void)
{
        xen_vm_assist(0, 2);

        page_specifier_t pte;
        memsetb((uintptr_t) &pte, sizeof(pte), 0);

        pte.present = 1;
        pte.writeable = 1;
        pte.frame_address = ADDR2PFN((uintptr_t) start_info.shared_info);
        xen_update_va_mapping(&shared_info, pte, 2);

        pte.present = 1;
        pte.writeable = 1;
        pte.frame_address = start_info.console_mfn;
        xen_update_va_mapping(&console_page, pte, 2);

        xen_set_callbacks(0xe019, xen_callback, 0xe019, xen_failsafe_callback);



        meminfo.start = ADDR2PFN(((((((uintptr_t) (start_info.ptl0)) - 0x80000000)) + ((4096) - 1)) & ~((4096) - 1))) + start_info.pt_frames;
        meminfo.size = start_info.frames - meminfo.start;
        meminfo.reserved = 0;

        uintptr_t pa;
        int last_ptl0 = 0;
        for (pa = PFN2ADDR(meminfo.start); pa < PFN2ADDR(meminfo.start + meminfo.size); pa += (1 << 12)) {
                uintptr_t va = (((uintptr_t) (pa)) + 0x80000000);

                if (((((va) >> 22) & 0x3ff) != last_ptl0) && (get_pt_flags((page_specifier_t *) (start_info.ptl0), (int)((((va) >> 22) & 0x3ff))) & (1<<1))) {

                        uintptr_t tpa = PFN2ADDR(meminfo.start + meminfo.reserved);
                        uintptr_t tva = (((uintptr_t) (tpa)) + 0x80000000);

                        memsetb(tva, 4096, 0);

                        page_specifier_t *tptl3 = (page_specifier_t *) (((uintptr_t) (((page_specifier_t *) ((((pfn_t *) 0xFC000000)[((uintptr_t) ((((page_specifier_t *) (start_info.ptl0))[((((tva) >> 22) & 0x3ff))].frame_address) << 12)) >> 12] << 12) + (((uintptr_t) ((((page_specifier_t *) (start_info.ptl0))[((((tva) >> 22) & 0x3ff))].frame_address) << 12)) & 0xfff))))) + 0x80000000);
                        set_pt_flags((page_specifier_t *) (tptl3), (int)((((tva) >> 12) & 0x3ff)), ((0<<1)));
                        { mmu_update_t update; update.ptr = ((start_info.pm_map[((uintptr_t) ((((uintptr_t) (&((page_specifier_t *) (start_info.ptl0))[((((va) >> 22) & 0x3ff))])) - 0x80000000))) >> 12] << 12) + (((uintptr_t) ((((uintptr_t) (&((page_specifier_t *) (start_info.ptl0))[((((va) >> 22) & 0x3ff))])) - 0x80000000))) & 0xfff)); update.val = ((start_info.pm_map[((uintptr_t) (tpa)) >> 12] << 12) + (((uintptr_t) (tpa)) & 0xfff)) | 0x0003; xen_mmu_update(&update, 1, ((void *)0), (0x7FF0U)); };

                        last_ptl0 = (((va) >> 22) & 0x3ff);
                        meminfo.reserved++;
                }

                page_specifier_t *ptl3 = (page_specifier_t *) (((uintptr_t) (((page_specifier_t *) ((((pfn_t *) 0xFC000000)[((uintptr_t) ((((page_specifier_t *) (start_info.ptl0))[((((va) >> 22) & 0x3ff))].frame_address) << 12)) >> 12] << 12) + (((uintptr_t) ((((page_specifier_t *) (start_info.ptl0))[((((va) >> 22) & 0x3ff))].frame_address) << 12)) & 0xfff))))) + 0x80000000);

                (((page_specifier_t *) (ptl3))[((((va) >> 12) & 0x3ff))].frame_address = ((start_info.pm_map[((uintptr_t) (pa)) >> 12] << 12) + (((uintptr_t) (pa)) & 0xfff)) >> 12);
                set_pt_flags((page_specifier_t *) (ptl3), (int)((((va) >> 12) & 0x3ff)), ((0<<1) | (1<<4)));
        }


        stack_safe = (((uintptr_t) (PFN2ADDR(meminfo.start + meminfo.reserved))) + 0x80000000);
}

void arch_pre_mm_init(void)
{
        pm_init();


}

void arch_post_mm_init(void)
{

                xen_console_init();
}

void arch_pre_smp_init(void)
{
                memory_print_map();




}


start_kernel()
{
        xen_puts("XEN : Load Jicama OS OK\n");
        while (1)
        {
        }
}


pm_init()
{
        xen_puts("XEN : Load...\n");
}
