

/** Page Table Entry. */
struct  page_specifier{
	unsigned present : 1;
	unsigned writeable : 1;
	unsigned uaccessible : 1;
	unsigned page_write_through : 1;
	unsigned page_cache_disable : 1;
	unsigned accessed : 1;
	unsigned dirty : 1;
	unsigned pat : 1;
	unsigned global : 1;
	unsigned soft_valid : 1;	/**< Valid content even if the present bit is not set. */
	unsigned avl : 2;
	unsigned frame_address : 20;
} __attr_packet ;

typedef struct {
	u64_t ptr;      /**< Machine address of PTE */
	union {            /**< New contents of PTE */
		u64_t val;
		page_specifier_t pte;
	};
} mmu_update_t;


#define mp_map ((pfn_t *) XEN_VIRT_START)

#define SIF_PRIVILEGED	(1 << 0)  /**< Privileged domain */
#define SIF_INITDOMAIN	(1 << 1)  /**< Iinitial control domain */


//page
#define PAGE_SIZE			4096

#define GET_PTL1_FLAGS_ARCH(ptl0, i)		get_pt_flags((page_specifier_t *) (ptl0), (int)(i))
#define GET_PTL2_FLAGS_ARCH(ptl1, i)		PAGE_PRESENT
#define GET_PTL3_FLAGS_ARCH(ptl2, i)		PAGE_PRESENT
#define GET_FRAME_FLAGS_ARCH(ptl3, i)		get_pt_flags((page_specifier_t *) (ptl3), (int)(i))

#define SET_PTL1_FLAGS_ARCH(ptl0, i, x)		set_pt_flags((page_specifier_t *) (ptl0), (int)(i), (x))
#define SET_PTL2_FLAGS_ARCH(ptl1, i, x)
#define SET_PTL3_FLAGS_ARCH(ptl2, i, x)
#define SET_FRAME_FLAGS_ARCH(ptl3, i, x)		set_pt_flags((page_specifier_t *) (ptl3), (int)(i), (x))

#define PTE_VALID_ARCH(p)			(*((u32_t *) (p)) != 0)
#define PTE_PRESENT_ARCH(p)			((p)->present != 0)
#define PTE_GET_FRAME_ARCH(p)			((p)->frame_address << 12)
#define PTE_WRITABLE_ARCH(p)			((p)->writeable != 0)
#define PTE_EXECUTABLE_ARCH(p)			1
#define FRAME_SIZE	(1 << 12)
#define SET_FRAME_ADDRESS_ARCH(ptl3, i, a)	(((page_specifier_t *) (ptl3))[(i)].frame_address = PA2MA(a) >> 12)

#define PAGE_PRESENT_SHIFT		1
#define PAGE_NOT_CACHEABLE	(0<<PAGE_CACHEABLE_SHIFT)
#define PAGE_CACHEABLE		(1<<PAGE_CACHEABLE_SHIFT)

#define PAGE_PRESENT		(0<<PAGE_PRESENT_SHIFT)
#define PAGE_NOT_PRESENT	(1<<PAGE_PRESENT_SHIFT)
#define ALIGN_UP(s, a)		(((s) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(s, a)	((s) & ~((a) - 1))
#define SET_FRAME_ADDRESS_ARCH(ptl3, i, a)	(((page_specifier_t *) (ptl3))[(i)].frame_address = PA2MA(a) >> 12)

#	define KA2PA(x)	(((uintptr_t) (x)) - 0x80000000)
#	define PA2KA(x)	(((uintptr_t) (x)) + 0x80000000)


#define PTL0_INDEX_ARCH(vaddr)	(((vaddr) >> 22) & 0x3ff)
#define PTL1_INDEX_ARCH(vaddr)	0
#define PTL2_INDEX_ARCH(vaddr)	0
#define PTL3_INDEX_ARCH(vaddr)	(((vaddr) >> 12) & 0x3ff)

#define PTL0_INDEX(vaddr)		PTL0_INDEX_ARCH(vaddr)
#define PTL1_INDEX(vaddr)		PTL1_INDEX_ARCH(vaddr)
#define PTL2_INDEX(vaddr)		PTL2_INDEX_ARCH(vaddr)
#define PTL3_INDEX(vaddr)		PTL3_INDEX_ARCH(vaddr)

#define PA2MA(x)	((start_info.pm_map[((uintptr_t) (x)) >> 12] << 12) + (((uintptr_t) (x)) & 0xfff))
#define MA2PA(x)	((mp_map[((uintptr_t) (x)) >> 12] << 12) + (((uintptr_t) (x)) & 0xfff))

#define SET_FRAME_FLAGS(ptl3, i, x)		set_pt_flags((page_specifier_t *) (ptl3), (int)(i), (x))
#define GET_PTL1_ADDRESS(ptl0, i)		((page_specifier_t *) MA2PA((((page_specifier_t *) (ptl0))[(i)].frame_address) << 12))

#define SET_PTL0_ADDRESS_ARCH(ptl0) { \
	mmuext_op_t mmu_ext; \
	\
	mmu_ext.cmd = MMUEXT_NEW_BASEPTR; \
	mmu_ext.mfn = ADDR2PFN(PA2MA(ptl0)); \
	xen_mmuext_op(&mmu_ext, 1, NULL, DOMID_SELF); \
}

#define SET_PTL1_ADDRESS_ARCH(ptl0, i, a) { \
	mmu_update_t update; \
	\
	update.ptr = PA2MA(KA2PA(&((page_specifier_t *) (ptl0))[(i)])); \
	update.val = PA2MA(a) | 0x0003; \
	xen_mmu_update(&update, 1, NULL, DOMID_SELF); \
}


#define SET_PTL1_ADDRESS(ptl0, i, a)	SET_PTL1_ADDRESS_ARCH(ptl0, i, a)
#define SET_PTL2_ADDRESS(ptl1, i, a)	SET_PTL2_ADDRESS_ARCH(ptl1, i, a)
#define SET_PTL3_ADDRESS(ptl2, i, a)	SET_PTL3_ADDRESS_ARCH(ptl2, i, a)
#define SET_FRAME_ADDRESS(ptl3, i, a)	SET_FRAME_ADDRESS_ARCH(ptl3, i, a)


#define GET_PTL1_FLAGS(ptl0, i)		GET_PTL1_FLAGS_ARCH(ptl0, i)
#define GET_PTL2_FLAGS(ptl1, i)		GET_PTL2_FLAGS_ARCH(ptl1, i)
#define GET_PTL3_FLAGS(ptl2, i)		GET_PTL3_FLAGS_ARCH(ptl2, i)
#define GET_FRAME_FLAGS(ptl3, i)	GET_FRAME_FLAGS_ARCH(ptl3, i)


#define PAGE_READ		(1<<PAGE_READ_SHIFT)
#define PAGE_WRITE		(1<<PAGE_WRITE_SHIFT)
#define PAGE_EXEC		(1<<PAGE_EXEC_SHIFT)
#define PAGE_CACHEABLE_SHIFT		0
#define PAGE_NOT_CACHEABLE_SHIFT	PAGE_CACHEABLE_SHIFT
#define PAGE_PRESENT_SHIFT		1
#define PAGE_NOT_PRESENT_SHIFT		PAGE_PRESENT_SHIFT
#define PAGE_USER_SHIFT			2
#define PAGE_KERNEL_SHIFT		PAGE_USER_SHIFT
#define PAGE_READ_SHIFT			3
#define PAGE_WRITE_SHIFT		4
#define PAGE_EXEC_SHIFT			5
#define PAGE_GLOBAL_SHIFT		6


#define PAGE_USER		(1<<PAGE_USER_SHIFT)
#define PAGE_KERNEL		(0<<PAGE_USER_SHIFT)
#define PAGE_GLOBAL		(1<<PAGE_GLOBAL_SHIFT)


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
	return hypercall4(XEN_UPDATE_VA_MAPPING, va, pte, 0, flags);
}

static inline int xen_mmu_update(const mmu_update_t *req, const unsigned int count, unsigned int *success_count, domid_t domid)
{
	return hypercall4(XEN_MMU_UPDATE, req, count, success_count, domid);
}

static inline int xen_mmuext_op(const mmuext_op_t *op, const unsigned int count, unsigned int *success_count, domid_t domid)
{
	return hypercall4(XEN_MMUEXT_OP, op, count, success_count, domid);
}


extern uintptr_t stack_safe;
static inline int get_pt_flags(page_specifier_t *pt, int i)
{
	page_specifier_t *p = &pt[i];
	
	return (
		(!p->page_cache_disable)<<PAGE_CACHEABLE_SHIFT |
		(!p->present)<<PAGE_PRESENT_SHIFT |
		p->uaccessible<<PAGE_USER_SHIFT |
		1<<PAGE_READ_SHIFT |
		p->writeable<<PAGE_WRITE_SHIFT |
		1<<PAGE_EXEC_SHIFT |
		p->global<<PAGE_GLOBAL_SHIFT
	);
}

static inline void set_pt_flags(page_specifier_t *pt, int i, int flags)
{
	page_specifier_t *p = &pt[i];
	
	p->page_cache_disable = !(flags & PAGE_CACHEABLE);
	p->present = !(flags & PAGE_NOT_PRESENT);
	p->uaccessible = (flags & PAGE_USER) != 0;
	p->writeable = (flags & PAGE_WRITE) != 0;
	p->global = (flags & PAGE_GLOBAL) != 0;
	
	/*
	 * Ensure that there is at least one bit set even if the present bit is cleared.
	 */
	p->soft_valid = true;
}
