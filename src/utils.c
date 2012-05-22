#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
#include <iostream>
#include <stdarg.h>

/* 创建多层目录的函数 */
void create_multilayer_dir( char *muldir ) 
{
    int    i,len;
    char    str[512];    
    strncpy( str, muldir, 512 );
    len=strlen(str);
    for( i=0; i<len; i++ )
    {
        if( str[i]=='/' )
        {
            str[i] = '\0';
            //判断此目录是否存在,不存在则创建
            if( access(str, F_OK)!=0 )
                mkdir( str, 0777 );

            str[i]='/';
        }
    }
    if( len>0 && access(str, F_OK)!=0 )
        mkdir( str, 0777 );

    return;
}

char *urldecode(char *input_str) 
{
	int len = strlen(input_str);
	char *str = strdup(input_str);
	
    char *dest = str; 
    char *data = str; 

    int value; 
    int c; 

    while (len--) { 
        if (*data == '+') 
            *dest = ' '; 
 
        else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))) 
        { 
            c = ((unsigned char *)(data+1))[0]; 
            if (isupper(c)) 
                c = tolower(c); 
            value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16; 
            c = ((unsigned char *)(data+1))[1]; 
            if (isupper(c)) 
               c = tolower(c); 
            value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10; 

            *dest = (char)value ; 
            data += 2; 
            len -= 2; 
		} else {
            *dest = *data; 
	    }
        data++; 
        dest++; 
    } 
    *dest = '\0'; 
    return str; 
}


/* Count the number of characters in a string of UTF-8. */
int utf8count(const char *str){
  assert(str);
  const unsigned char *rp = (unsigned char *)str;
  int cnt = 0;
  while(*rp != '\0'){
    if((*rp & 0x80) == 0x00 || (*rp & 0xe0) == 0xc0 ||
       (*rp & 0xf0) == 0xe0 || (*rp & 0xf8) == 0xf0) cnt++;
    rp++;
  }
  return cnt;
}

/* tokenize a string */
char **split(char *str, int *length, char delim){
	int anum = 256;
	char **tokens = (char**) malloc(sizeof(*tokens) * anum);
	int tnum = 0;
	while(*str == delim || *str == '\t'){
	  str++;
	}
	while(*str != '\0'){
	  if(tnum >= anum){
		anum *= 2;
		tokens = (char**)realloc(tokens, sizeof(*tokens) * anum);
	  }
	  tokens[tnum++] = str;
	  while(*str != '\0' && *str != delim && *str != '\t' && *str != '\r'){
		str++;
	  }
	  while(*str == delim || *str == '\t' || *str == '\r'){
		*(str++) = '\0';
	  }
	}
	*length = tnum;
	return tokens;
}

# if __WORDSIZE == 64
typedef long int  int64_t;
# else
typedef long long int  int64_t;
# endif

/* Convert a string to an integer. */
int64_t strtoi(const char *str){
	assert(str);
	while(*str > '\0' && *str <= ' '){
		str++;
	}
	int sign = 1;
	int64_t num = 0;
	if(*str == '-'){
		str++;
		sign = -1;
	} else if(*str == '+'){
		str++;
	}
	while(*str != '\0'){
		if(*str < '0' || *str > '9') break;
		num = num * 10 + *str - '0';
		str++;
	}
	return num * sign;
}

