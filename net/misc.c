#include <net/net.h>


void *memmove(void *dst, const void *src, size_t count)
{
  void * ret = dst;

  if (dst <= src || (char *) dst >= ((char *) src + count)) 
  {
    //
    // Non-Overlapping Buffers
    // copy from lower addresses to higher addresses
    //
    while (count--) 
    {
      *(char *) dst = *(char *) src;
      dst = (char *) dst + 1;
      src = (char *) src + 1;
    }
  }
  else 
  {
    //
    // Overlapping Buffers
    // copy from higher addresses to lower addresses
    //
    dst = (char *) dst + count - 1;
    src = (char *) src + count - 1;

    while (count--) 
    {
      *(char *) dst = *(char *) src;
      dst = (char *) dst - 1;
      src = (char *) src - 1;
    }
  }

  return ret;
}

__inline unsigned short htons(unsigned short n)
{
  return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
}

__inline unsigned short ntohs(unsigned short n)
{
  return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
}

__inline unsigned long htonl(unsigned long n)
{
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}

__inline unsigned long ntohl(unsigned long n)
{
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}

int check_iovec(struct iovec *iov, int iovlen)
{
  if (iov)
  {
    if (iovlen < 0) return EINVAL;
    //if (KERNELSPACE(iov)) return EFAULT;
    //if (!mem_mapped(iov, iovlen * sizeof(struct iovec))) return EFAULT;
    while (iovlen > 0)
    {
      if (iov->iov_len < 0) return EINVAL;
      if (iov->iov_base)
      {
	//if (KERNELSPACE(iov->iov_base)) return EFAULT;
	//if (!mem_mapped(iov->iov_base, iov->iov_len)) return EFAULT;
      }
      else if (iov->iov_len != 0) 
	return EFAULT;

      iov++;
      iovlen--;
    }
  }
  else
  {
    if (iovlen != 0) return EFAULT;
  }

  return 0;
}

size_t get_iovec_size(struct iovec *iov, int iovlen)
{
  size_t size = 0;

  while (iovlen > 0)
  {
    size += iov->iov_len;
    iov++;
    iovlen--;
  }

  return size;
}

struct iovec *dup_iovec(struct iovec *iov, int iovlen)
{
  struct iovec *newiov;

  newiov = (struct iovec *) kmalloc(iovlen * sizeof(struct iovec),0);
  if (!newiov) return NULL;
  memcpy(newiov, iov, iovlen * sizeof(struct iovec));

  return newiov;
}

int read_iovec(struct iovec *iov, int iovlen, char *buf, size_t count)
{
  size_t read = 0;
  size_t len;

  while (count > 0 && iovlen > 0)
  {
    if (iov->iov_len > 0)
    {
      len = iov->iov_len;
      if (count < iov->iov_len) len = count;

      memcpy(buf, iov->iov_base, len);
      read += len;

      iov->iov_base =  (char *)iov->iov_base + len;
      iov->iov_len -= len;

      buf += len;
      count -= len;
    }

    iov++;
    iovlen--;
  }

  return read;
}

int write_iovec(struct iovec *iov, int iovlen, char *buf, size_t count)
{
  size_t written = 0;
  size_t len;

  while (count > 0 && iovlen > 0)
  {
    if (iov->iov_len > 0)
    {
      len = iov->iov_len;
      if (count < iov->iov_len) len = count;

      memcpy(iov->iov_base, buf, len);
      written += len;

       iov->iov_base =(char *)iov->iov_base+ len;
      iov->iov_len -= len;

      buf += len;
      count -= len;
    }

    iov++;
    iovlen--;
  }

  return written;
}


