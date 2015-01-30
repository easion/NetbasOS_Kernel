
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/elf.h>
#include <jicama/module.h>
#include <string.h>
#include <assert.h>

#include "rld_priv.h"


elf_vmid _kern_vm_create_anonymous_region(proc_t *rp, void **address, int addr_type,
	addr_t size, int attr)
	{
		unsigned int *p = NULL;
		if (addr_type == REGION_ADDR_ANY_ADDRESS)
		{
			p= user_malloc(rp,size,0);
			*address = p;
			kprintf("create_anonymous_region error static addr %x \n",*address);
		}
#if 0

		if (addr_type == REGION_ADDR_EXACT_ADDRESS)
		{
		//kprintf("_kern_vm_create_anonymous_region: , type%d, size%d\n", addr_type,size );
		p= user_specifier_malloc(rp,p,size);
		if (p!=*address)
			{
				kprintf("_kern_vm_create_anonymous_region:alloc error size  %d, %p,%p\n",size,p,*address);
			}
		}
		else{
		kprintf("create_anonymous_region error static addr %x \n",*address);
		p= user_malloc(rp,size,0);
		*address = p;
		}

#endif

		if (!p)
		{
			return -1;
		}
		return 0;
	}

static int _kern_vm_delete_region(proc_t *rp,elf_vmid id)
{
	if (!id||id==-1)
	{
		return -1;
	}

	//kprintf("user free %p\n",id);
		user_free(rp, id);
		return 0;
}

elf_vmid _kern_vm_map_file(struct filp*fp, void **address, int addr_type,
	addr_t size,  proc_t *rp, off_t offset)
	{
		//int fd;
		char *ptr=NULL;
		elf_vmid *ret=-1;
	addr_t base=0;


	base = proc_phy_addr(rp);

		//printf("_kern_vm_map_file name=%s, %s offset = %d size %d",name, path, offset,size);

		if (addr_type == REGION_ADDR_EXACT_ADDRESS)
		{
			ret = -1;
			//ret = user_specifier_malloc(rp,ptr,size);
			//if (ret!=*address)
			{
			//	kprintf("_kern_vm_map_file:alloc error size  %d, %p,%p\n",size,ret,*address);
			}
			ptr = (void*)(*address);
			//printf("static  addr 0x%x\n",(long int)ptr);
			//return 1;
		}
		else{
			ptr = user_malloc(rp,size,0);
			if (!ptr)
			{
				return -1;
			}
			ret=ptr;
		}
			//kprintf("_kern_vm_map_file:  ptr %p,%p,size %x\n", 		ptr, ptr+size,size );

		*address = ptr;
		

		//kprintf("load file %x size=%d offset=%d succ\n",ptr, size,offset);
		ptr+=base;

		fseek(fp,offset,SEEK_SET);
		fread( ptr,size,1,fp);

		return ret;
}




void enqueue_image(image_queue_t *queue, image_t *img)
{
	img->next= 0;

	img->prev= queue->tail;
	if(queue->tail) {
		queue->tail->next= img;
	}
	queue->tail= img;
	if(!queue->head) {
		queue->head= img;
	}
}

void
dequeue_image(image_queue_t *queue, image_t *img)
{
	if(img->next) {
		img->next->prev= img->prev;
	} else {
		queue->tail= img->prev;
	}

	if(img->prev) {
		img->prev->next= img->next;
	} else {
		queue->head= img->next;
	}

	img->prev= 0;
	img->next= 0;
}

image_t *
find_image(proc_t *rp,char const *name)
{
	image_t *iter;

	iter= rp->dynamic_module_list.head;
	while(iter) {
		if(strncmp(iter->name, name, sizeof(iter->name)) == 0) {
			return iter;
		}
		iter= iter->next;
	}

	
	return 0;
}



/*
 * create_image() & destroy_image()
 *
 * 	Create and destroy image_t structures. The destroyer makes sure that the
 * 	memory buffers are full of garbage before freeing.
 */
image_t *
create_image(proc_t *rp, char const *name, int num_regions)
{
	image_t *retval;
	size_t   alloc_size;

	alloc_size= sizeof(image_t)+(num_regions-1)*sizeof(elf_region_t);

	retval= kmalloc(alloc_size,0);
	if (!retval)
	{
		return NULL;
	}

	memset(retval, 0, alloc_size);

	strncpy(retval->name, name, sizeof(retval->name));
	retval->imageid= rp->imageid_count;
	retval->refcount= 1;
	retval->num_regions= num_regions;

	rp->imageid_count+= 1;

	return retval;
}

static
void
destroy_image(image_t *image)
{
	size_t alloc_size;

	alloc_size= sizeof(image_t)+(image->num_regions-1)*sizeof(elf_region_t);

	memset(image->needed, 0xa5, sizeof(image->needed[0])*image->num_needed);
	kfree(image->needed);

	memset(image, 0xa5, alloc_size);
	kfree(image);
}


void
put_image(proc_t *rp, image_t *img)
{
	img->refcount-= 1;
	if(img->refcount== 0) {
		size_t i;

		dequeue_image(&rp->dynamic_module_list, img);
		enqueue_image(&rp->disposable_module_list, img);

		for(i= 0; i< img->num_needed; i++) {
			put_image(rp,img->needed[i]);
		}
	}
}


dynmodule_id
load_program(proc_t *rp, struct filp*fp,char const *path, void **entry)
{
	image_t *image;
	image_t *iter;
	long bss=0;
	char dirtmp[255];

	strncpy(dirtmp,path,255);

	rp->p_bss_code=0;


	image = load_container(rp,fp, path, MAGIC_APPNAME, true,&bss);

	if (!image)
	{
		kprintf("enqueue_image:%p error\n",image );
		return -1;
	}


	iter= rp->dynamic_module_list.head;
	while(iter) {

		//kprintf("load_dependencies:%p\n",iter );
		load_dependencies(rp,iter,dirname(dirtmp));

		iter= iter->next;
	};

	iter= rp->dynamic_module_list.head;
	while(iter) {
		bool relocate_success;

		relocate_success= relocate_image(rp,iter);
		if(relocate_success==false){
			kprintf( "troubles relocating\n");
			goto error;
		}

		iter= iter->next;
	};
	
	//rp->p_bss_code = ROUNDUP(rp->p_bss_code,PAGE_SIZE);
	rp->p_bss_code = image->regions[0].vmstart+image->regions[0].vmsize;


	 if (image->num_regions>1)
	{
	rp->p_bss_code = MAX(rp->p_bss_code, image->regions[1].vmstart+image->regions[1].vmsize);
	}

	rp->p_bss_code+=(bss+0x10000);
	//rp->p_bss_code=0x10000;

	//init_dependencies(rp->dynamic_module_list.head, false);
	//kprintf("rp->p_bss_code = %x\n", rp->p_bss_code);

	*entry= (void*)(image->entry_point);
	//kprintf("load_program rp->p_bss_code = %p,%p, %p\n", rp->p_bss_code,*entry,bss);
	return image->imageid;

error:
	return -1;
}

dynmodule_id load_library(proc_t *rp, char const *path,long flags)
{
	image_t *image;
	image_t *iter;
	struct filp*fp;
	long bss=0;
	char dirtmp[255]="/system/";

	//LOCK();


	/*try open if it loaded into mem*/
	//fp = modfs_open(path, "r");

	//if (fp == NIL_FLP)	{
		/*not in mem, try open it on disk*/
		fp = server_open(path, "r");	
	//}
	//else{
	//	trace("exec_file(): %s load on memory\n", path);
	//}

	if(fp==NIL_FLP){
		/*file not exist*/
		//kprintf("exec_file(): file %s not found", path);
		return ENOENT;
	}


	image = find_image(rp,path);
	if(image) {
		image->refcount+= 1;
		return image->imageid;
	}

	image = load_container(rp,fp, path, path,false,&bss);
	if (!image)
	{
		kprintf("load_container error ..\n");
		return -1;
	}


	iter= rp->dynamic_module_list.head;
	while(iter) {
		load_dependencies(rp,iter,dirtmp);

		iter= iter->next;
	};


	iter= rp->dynamic_module_list.head;
	while(iter) {
		bool relocate_success;

		relocate_success= relocate_image(rp,iter);
		if(!relocate_success){
			kprintf("troubles relocating\n");
			return -1;
		}

		iter= iter->next;
	};

	fclose(fp);
	user_malloc(rp,bss,0);

	//UNLOCK();

	//init_dependencies(image, true);

	//kprintf("load_library image->imageid = %d\n", image->imageid);

	return image->imageid;
}

static void unmap_image(proc_t *rp,image_t *image)
{
	unsigned i;

	for(i= 0; i< image->num_regions; i++) {
		_kern_vm_delete_region(rp,image->regions[i].id);

		image->regions[i].id= 0;
	}
}


dynmodule_id unload_library(proc_t *rp, dynmodule_id imid)
{
	int retval;
	image_t *iter;


	/*
	 * we only check images that have been already initialized
	 */
	iter= rp->dynamic_module_list.head;
	while(iter) {
		if(iter->imageid== imid) {
			/*
			 * do the unloading
			 */
			put_image(rp,iter);


			break;
		}

		iter= iter->next;
	}

	if(iter) {
		retval= 0;
	} else {
		retval= -1;
	}


	iter= rp->disposable_module_list.head;
	while(iter) {
		// call image fini here...

		dequeue_image(&rp->disposable_module_list, iter);
		//kprintf("unload_library called %d line %d\n",imid,__LINE__);
		unmap_image(rp,iter);
		//kprintf("unload_library called %d line %d\n",imid,__LINE__);

		destroy_image(iter);
		iter= rp->disposable_module_list.head;
	}

	kprintf("unload_library called %d line %d\n",imid,__LINE__);


	return retval;
}

image_t *
dynamic_find_id(proc_t *rp, dynmodule_id imid)
{
	image_t *iter;

	/*
	 * we only check images that have been already initialized
	 */
	iter= rp->dynamic_module_list.head;
	while(iter) {

		//kprintf("we found %d, %s \n", iter->imageid,iter->name);
		if(iter->imageid== imid) {
			return iter;
		}

		iter= iter->next;
	}

	return NULL;
}


void *dynamic_get_symbol(image_t *iter, char const *symname)
{
	/*
	 * we only check images that have been already initialized
	 */
	
	if(iter) {
		elf_sym_t *sym= find_symbol_xxx(iter, symname);

		if(sym) {
			return (void*)(sym->value + iter->regions[0].delta);
		}
	}


	return NULL;
}

void *dynamic_symbol(proc_t *rp, dynmodule_id imid, char const *symname)
{
	image_t *iter;

	iter = dynamic_find_id(rp,imid);

	if (!iter)
	{
		return NULL;
	}	

	//kprintf("dynamic_get_symbol = %s\n", symname);

	return dynamic_get_symbol(iter, symname);
}


/*
 if ( strcmp( psImage->ei_psSymbols[ i ].es_pcName, "start_ctors" ) == 0 ) {
        pCtorStart = ( void* )psImage->ei_psSymbols[ i ].es_nAddress;
      } else if ( strcmp( psImage->ei_psSymbols[ i ].es_pcName, "end_ctors" ) == 0 ) {
        pCtorEnd = ( void* )psImage->ei_psSymbols[ i ].es_nAddress;
      }

 if ( ( pCtorStart != NULL ) && ( pCtorEnd != NULL ) ) {
      psImage->ei_nCtors = ( uint32 )pCtorStart;
      psImage->ei_nCtorCount = ( int )( pCtorEnd - pCtorStart ) / sizeof( void* );
    }
	  */

int read_elf_dynamic(struct filp*  fp,  thread_t *pthread, void**entry)
{
	dynmodule_id id;
	proc_t *rp = THREAD_SPACE(pthread);

	id = load_program(rp, fp, "",entry);
	if (id<0)
	{
		return -1;
	}

	return 0;
}

int get_dynamic_module_info(proc_t *rp,dynmodule_id imid, dyinfo_t *info)
{
	image_t *img;

	img = dynamic_find_id(rp,imid);
	if (!img)
	{
		//kprintf("get_dynamic_module_info %d not found\n", imid);
		return -1;
	}

	//info-> = start_ctors;
    info->dy_image_id = img->imageid;  
     strncpy(info-> dy_name[ 256 ],  img->name,256);    
     info->dy_num_count = rp->imageid_count; 
     info->dy_entry_point = img->entry_point;  
     info-> dy_init = img->e_init;   
     info-> dy_fini = img->e_finit;  
     info-> dy_text_addr = img->regions[0].vmstart;   
     info->dy_text_size = img->regions[0].vmsize;  

	 if (img->num_regions>1)
	 {
		 info-> dy_data_addr = img->regions[1].vmstart;   
		 info->dy_data_size = img->regions[1].vmsize;   
	 }

     info->dy_ctor_addr = img->start_ctors;
     info->dy_ctor_count=(long)(img->end_ctors-img->start_ctors)/sizeof(void*);
	//	kprintf("get_dynamic_module_info %d begin,%p,%p\n", imid,info->dy_ctor_addr,info-> dy_init);
	return 0;
}


int get_dynamic_dependencies(proc_t *rp,dynmodule_id imid, void **info,int init_head)
{
	image_t *img;

	img = dynamic_find_id(rp,imid);
	if (!img)
	{
		//kprintf("get_dynamic_dependencies %d not found\n", imid);
		return -1;
	}

	//	kprintf("get_dynamic_dependencies for %d \n", imid);
	return get_dependencies(img,init_head,info);
}
