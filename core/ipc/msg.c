
#include <jicama/system.h>
#include <jicama/message.h>
#include <jicama/process.h>
#include <jicama/syscall.h>
#include <jicama/message.h>

#define IPC_UNUSED	((void *) -1)

extern int ipcperms (struct ipc_perm *ipcp, short msgflg);

__local struct msqid_ds *msgque[MSGMNI];
__local int msgbytes = 0;
__local int msghdrs = 0;
__local unsigned short msg_seq = 0;
__local int used_queues = 0;
__local int max_msqid = 0;
__local struct wait_queue *msg_lock = (struct wait_queue *)0;

__public void ipc_init (void)
{
	int id;
	
	for (id=0; id < MSGMNI; id++) 
		msgque[id] = (struct msqid_ds *) IPC_UNUSED;
	msgbytes = msghdrs = msg_seq = max_msqid = used_queues = 0;
	msg_lock = (struct wait_queue *)0;
	return;
}

__public int msgsnd (int msqid, struct msgbuf *msgp, int msgsz, int msgflg)
{
	if (msgsz > MSGMAX || msgsz < 0 || msqid < 0)
		return EINVAL;

	return msgsz;
}

__public int msgrcv (int msqid, struct msgbuf *msgp, int msgsz, long msgtyp, 
		int msgflg)
{

	if (msqid < 0 || msgsz < 0)
		return EINVAL;

	return -1;
}

__public int msgget (key_t key, int msgflg)
{
	int id;
	struct msqid_ds *msq;

	id=0;
	return id;
} 



__public int msgctl (int msqid, int cmd, struct msqid_ds *buf)
{
	int id, err;
	struct msqid_ds *msq, tbuf;
	struct ipc_perm *ipcp;
	
	if (msqid < 0 || cmd < 0)
		return EINVAL;

	switch (cmd) {
	case IPC_STAT:
		break;
	case IPC_RMID: 
	case IPC_SET:
		break;
	default:
		return EINVAL;
		break;
	}
	return 0;
}

