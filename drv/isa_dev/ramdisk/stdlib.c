

#define isxdigit(C)							\
    ((isdigit(C) ||							\
      ((C) >= 'a' && (C) <= 'f') ||					\
      ((C) >= 'A' && (C) <= 'F')) ? 1 : 0)

#define isdigit(C) (((C) >= '0' && (C) <= '9') ? 1 : 0)


int tonumber(char c)
{
    if (c >= '0' && c <= '9') return(c - '0');
    else if (c >= 'A' && c <= 'F') return(c - 'A' + 10);
    else if (c >= 'a' && c <= 'f') return(c - 'a' + 10);
    else return(c);
}

int hex2num(char *str)
{
	int i, num;

	i=num=0;

	while (isxdigit(str[i]))
	{
		num+=num*16+tonumber(str[i]);
		i++;
	}
	
	return num;
}

