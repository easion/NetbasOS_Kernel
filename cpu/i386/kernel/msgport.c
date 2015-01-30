
#include <jicama/process.h>
#include <jicama/trace.h>
#include <jicama/msgport.h>
#include <errno.h>

int SysCall( msgsend)(regs_t *reg)
{
	int ret;
	ret = msgport_send(reg->ebx,reg->edi, (void *)current_proc_vir2phys(reg->ecx), 
			reg->edx);

	return ret;
}

int SysCall( msgpend)(regs_t *reg)
{
	int ret;

	ret = msgport_pend(reg->ebx, (int *)current_proc_vir2phys(reg->edi),(void *)current_proc_vir2phys(reg->ecx), 
			reg->edx,reg->esi);
	return ret;
}

int SysCall( msgport)(regs_t *reg)
{
	int ret;
	int msg_type = (int)reg->edi;

	switch (msg_type)
	{
	case MSGPORT_CREATE_NO:
		{
		ret = create_msgport((const char *)current_proc_vir2phys(reg->ebx), current_thread());
		break;
		}

	case MSGPORT_CONNECT_NO:
		{
		ret = connect_msgport((const char *)current_proc_vir2phys(reg->ebx));
		break;
		}

	case MSGPORT_DESTROY_NO:
		{
		ret = msgport_destroy(reg->ebx);
		break;
		}

	case MSGPORT_WAT_NO:
		{
		ret = msgport_wait(reg->ebx, reg->edx);
		break;
		}
	
	case MSGPORT_PEND_NO:
		{
		ret = msgport_pend(reg->ebx,NULL, (void *)current_proc_vir2phys(reg->ecx), 
			reg->edx,reg->esi);
		break;
		}

	case MSGPORT_SEND_NO:
		{
		ret = msgport_send(reg->ebx, 0,(void *)current_proc_vir2phys(reg->ecx), 
			reg->edx);
		break;
		}

	case MSGPORT_NOTIFY_SEND_NO:
		{
		ret = msgport_send(reg->ebx, NULL,(void *)current_proc_vir2phys(reg->ecx), 
			reg->edx);
		break;
		}

		case MSGPORT_MDELAY:
		{
			ret = 0;
			thread_wait(current_thread(), reg->ebx);

			//schedule();
			break;
		}

		case MSGPORT_SLEEP:
		{
			ret = 0;
			thread_sleep_on((thread_wait_t *)current_proc_vir2phys(reg->ebx));

			schedule();
			break;
		}

		case MSGPORT_WAKEUP:
		{
			ret = 0;
			thread_wakeup((thread_wait_t *)current_proc_vir2phys(reg->ebx));

			schedule();
			break;
		}

		case MSGPORT_SET_PUBLIC:
		{
			int idx,port;
			idx = reg->ebx;
			port = reg->ecx;
			ret = msgport_set_public_port(idx,port);
			break;
		}

		case MSGPORT_GET_PUBLIC:
		{
			int idx,wait;
			idx = reg->ebx;
			wait = reg->ecx;
			ret = msgport_get_public_port(idx,wait,-1);
			break;
		}

		default:
			ret = ENOSYS;
			break;				
		}

		return ret;

}





