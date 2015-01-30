
#include <ansi.h>
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/grub.h>
#include <jicama/console.h>
/*
**
*/


__public int get_lfb( unsigned int *width, unsigned int *height,
             unsigned int *depth,  unsigned int *scanline,  unsigned long *ptr )
{
#if 0
	*width  = vin.lfb_width;
	*height = vin.lfb_height;
	*scanline  =  vin.lfb_scanline;
	*depth  = vin.lfb_bpp;
	*ptr    = vin.lfb_ptr;
#endif
	return 0;
}

int grub_read_lfb(grub_info_t* info)
{
#if 0
	struct vbe_mode_t *mode_info = (struct vbe_mode_t*) info->vbe_mode_info;
    struct vbe_controller_t *control= (struct vbe_controller_t*) info->vbe_control_info;

    if (CHECK_GRUB(info->flags,11)){

	    boot_parameters.lfb_width  = mode_info->x_resolution;
	    boot_parameters.lfb_height = mode_info->y_resolution;
	    boot_parameters.lfb_bpp = mode_info->bits_per_pixel;
	    lfb_type = 0;

        if(control->version >= 0x0300)
                lfb_scanline = mode_info->linear_bytes_per_scanline;
		else
		        lfb_scanline = mode_info->bytes_per_scanline;

	    boot_parameters.lfb_ptr = mode_info->phys_base;
		VESA_PTR = (unsigned long *)boot_parameters.lfb_ptr;

		switch (mode_info->bits_per_pixel)
		{
		case 8:
			vesa_put_pixel = &vesa_put_pixel8;
		     vesa_get_pixel = &vesa_get_pixel8;
		    VESA8_PTR = (unsigned char *)boot_parameters.lfb_ptr;
		break;
		case 16:
			vesa_put_pixel = &vesa_put_pixel16;	
		    vesa_get_pixel = &vesa_get_pixel16;
		    VESA16_PTR = (unsigned short *)boot_parameters.lfb_ptr;
		break;
		case 32:
			vesa_put_pixel = &vesa_put_pixel32;
		    vesa_get_pixel = &vesa_get_pixel32;
		break;
		default:
			return -1;
		}
	
		vesa_init();
		#ifndef __AMD64
		//set_video_seg((u32_t)boot_parameters.lfb_ptr, (u32_t)(lfb_scanline*boot_parameters.lfb_height));
		#endif
         kprintf("Linear Frame Buffer information:\n %d %d depth:%d,  lfb_scanline:%d at lfb base: %x\n",
			boot_parameters.lfb_width, boot_parameters.lfb_height,boot_parameters.lfb_bpp,	 lfb_scanline, boot_parameters.lfb_ptr );  
		      kprintf (" VBE version %d.%d \n",
		   (int) (control->version >> 8), (int) (control->version & 0xFF));
			  kprintf("mode :%X, vbe_interface_seg: %X -OFF:%X\n", info->vbe_mode, info->vbe_interface_seg, info->vbe_interface_off);
 		get_vesa_info(control);

  }
   else	{
	    kprintf("Can not get VBE LFB information from GRUB\n");
	    boot_parameters.lfb_width  = 640;
		boot_parameters.lfb_height = 480;
		boot_parameters.lfb_bpp = 4;
		lfb_type = 0;
	    boot_parameters.lfb_ptr = 0xa0000;
		return 0;
	}

	    if (control->version < 0x0200)
    {
      kprintf (" VBE version %d.%d is not supported.\n",
		   (int) (control->version >> 8), (int) (control->version & 0xFF));
      return 0;
    }
#endif
	return 1;
}

