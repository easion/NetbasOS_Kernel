#include "lwip/debug.h"
#include "lwip/stats.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"

struct http_state {
  char *file; //�ļ���
  u32_t left; //ʣ���ֽ���
};

/*-----------------------------------------------------------------------------------*/
static void
conn_err(void *arg, err_t err)
{
  struct http_state *hs;

  hs = arg; //�ͷ��ڴ�
  mem_free(hs);
}
/*-----------------------------------------------------------------------------------*/
static void
close_conn(struct tcp_pcb *pcb, struct http_state *hs)
{
  tcp_arg(pcb, NULL); //���ò���Ϊ��
  tcp_sent(pcb, NULL); //�رշ��͹���
  tcp_recv(pcb, NULL); //�رս��ܹ���
  mem_free(hs); //�ͷ��ļ���Ϣ�ڴ�
  tcp_close(pcb);
}
/*-----------------------------------------------------------------------------------*/
static void
send_data(struct tcp_pcb *pcb, struct http_state *hs)
{
  err_t err;
  u16_t len;

  /* We cannot send more data than space avaliable in the send
     buffer. */     
  if(tcp_sndbuf(pcb) < hs->left) {
		 //���������ݴ��ڻ����
    len = tcp_sndbuf(pcb);
  } else {
	  //���ڣ����������һ��
    len = hs->left;
  }
	//����
  err = tcp_write(pcb, hs->file, len, 0);
  
  if(err == ERR_OK) {
	  //�ı䷢�͵��ļ�ָ��
    hs->file += len;
    hs->left -= len;
    /*  } else {
	printf("send_data: error %s len %d %d\n", lwip_strerr(err), len, tcp_sndbuf(pcb));*/
  }
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_poll(void *arg, struct tcp_pcb *pcb)
{
	  //printk("call http_pool\n");

  /*  printf("Polll\n");*/
  if(arg == NULL) {
	  //����Ϊ�գ����ļ��Ѿ�ִ�йر���
    /*    printf("Null, close\n");*/
    tcp_close(pcb);
  } else {
	  //��������ʣ������
    send_data(pcb, (struct http_state *)arg);
  }

  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
  struct http_state *hs;

    //printk("http_sent call\n");


  hs = arg;

  if(hs->left > 0) {    
	  //�����ݿɷ���
    send_data(pcb, hs);
  } else {
	  //�ر�����
    close_conn(pcb, hs);
  }

  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  int i, j;
  char *data;
  char fname[40];
  struct fs_file file;
  struct http_state *hs;

  hs = arg;
  if(err == ERR_OK && p != NULL) {

    /* Inform TCP that we have taken the data. */
    tcp_recved(pcb, p->tot_len);
    
    if(hs->file == NULL) {
      data = p->payload;
      
      if(strncmp(data, "GET ", 4) == 0) {
		for(i = 0; i < 40; i++) {
		  if(((char *)data + 4)[i] == ' ' ||
			 ((char *)data + 4)[i] == '\r' ||
			 ((char *)data + 4)[i] == '\n') {
			((char *)data + 4)[i] = 0;
		  }
	}
	i = 0;
	do {
		//�����ļ���·��
	  fname[i] = "/http"[i];
	  i++;
	} 
	while(fname[i - 1] != 0 && i < 40);

	i--;
	j = 0;
	do {
	  fname[i] = ((char *)data + 4)[j];
	  j++;
	  i++;
	} while(fname[i - 1] != 0 && i < 40);
	pbuf_free(p);

	//��HTML�ļ�
	
	if(!fs_open(fname, &file)) {
	  fs_open("/http/index.html", &file);
	}
	hs->file = file.data;
	hs->left = file.len;
	/*	printf("data %p len %ld\n", hs->file, hs->left);*/
	//�����ײ�������
	send_data(pcb, hs);

	/* Tell TCP that we wish be to informed of data that has been
	   successfully sent by a call to the http_sent() function. */
	   //���÷��͹���
	   //printk("set http_send\n");
	tcp_sent(pcb, http_sent);
      }
	  else {
	close_conn(pcb, hs);
      }
    } 
	else {
		//�ͷŻ���
      pbuf_free(p);
    }
  }

  if(err == ERR_OK && p == NULL) {
	  //�ر�����
    close_conn(pcb, hs);
  }
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
  struct http_state *hs;

  /* Allocate memory for the structure that holds the state of the
     connection. */
  hs = mem_malloc(sizeof(struct http_state));

  //�յ�һ������

  if(hs == NULL) {
    printf("http_accept: Out of memory\n");
    return ERR_MEM;
  }
  
  /* Initialize the structure. */
  hs->file = NULL;
  hs->left = 0;
  //�ļ�ָ�����ݽ��г�ʼ��

  /* Tell TCP that this is the structure we wish to be passed for our
     callbacks. */
  tcp_arg(pcb, hs);
  //���ô���Ĳ���

  /* Tell TCP that we wish to be informed of incoming data by a call
     to the http_recv() function. */
  tcp_recv(pcb, http_recv);
  //���ý��չ���

  tcp_err(pcb, conn_err);
  //���ô�������
  tcp_poll(pcb, http_poll, 10);
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
void
httpd_init(void)
{
  struct tcp_pcb *pcb;

  pcb = tcp_new(); //����һ���µ�TCP����
  tcp_bind(pcb, IP_ADDR_ANY, 80); //�󶨶˿ں�
  pcb = tcp_listen(pcb); //��������
  tcp_accept(pcb, http_accept); //�������ӽ��ܹ���
}
/*-----------------------------------------------------------------------------------*/

