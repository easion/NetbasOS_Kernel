#ifndef _K_USER_H__
#define _K_USER_H__
#ifdef __cplusplus
extern "C" {
#endif

void write_user_dword(unsigned long val,unsigned long * addr);
void write_user_word(unsigned short val,unsigned short * addr);
void write_user_byte(char val,char *addr);
unsigned char read_user_byte(const char *addr);
unsigned short read_user_word(const unsigned short *addr);
unsigned long read_user_dword(const unsigned long *addr);

static inline void* memcpy_from_user( void *dst_ptr, const void *src_ptr, unsigned long count )
{
	void *ret_val = dst_ptr;
	const char *src = (const char *)src_ptr;
	char *dst = (char *)dst_ptr;

	for(; count != 0; count--){
		*dst++ = read_user_byte(src++);
	}

	return ret_val;
}

static inline  void* memcpy_to_user( void *dst_ptr, const void *src_ptr, unsigned long count )
{
	void *ret_val = dst_ptr;
	const char *src = (const char *)src_ptr;
	char *dst = (char *)dst_ptr;

	for(; count != 0; count--){
		write_user_byte(*dst++, (char *)src++);
	}

	return ret_val;
}

static inline  char *strncpy_from_user(char *src_char, const char *add_char, size_t nr)
{
	size_t i = 0;
	char *ret = src_char;

	while (*add_char != (char)'\0' && i<nr){
		*src_char++ = read_user_byte(add_char++);
		i++;
	}

	*src_char++ = '\0';

	return ret;
}

static inline  char *strncpy_to_user(char *src_char, const u8_t *add_char, size_t nr)
{
	size_t i = 0;
	char *ret = src_char;

	while (*add_char != '\0' && i<nr){
		//*src_char++ = *add_char++;
		write_user_byte(*src_char++, (char *)add_char++);
		i++;
	}

	*src_char++ = '\0';

	return ret;
}



#ifdef __cplusplus
}
#endif

#endif
