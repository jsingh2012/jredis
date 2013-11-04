#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include "redis.cpp"
class redis{
	table set;
	tree  range; 
	public:
	int call_redis(char *indata, char *outdata);
	int get_command(char *ptr);
	int save(const char *file);
	int load(const char *file);
}database;
int main(int argc, char *argv[])
{
	int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char send_buff[1025];
    char receive_buff[1025];
    time_t ticks; 
    char buff[1024];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(send_buff, '0', sizeof(send_buff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(15000); 
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    listen(listenfd, 10); 

    while(1)
    {
	    printf("Server is listenning on port 15000\n");
	    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
	    { 
		    while(1){		  
			    sprintf(send_buff,"$: \r\n");		    
			    write(connfd, send_buff, strlen(send_buff)); 
			    memset(receive_buff,'\0',sizeof(receive_buff));			    
			    read(connfd, receive_buff, 1025);
			    printf("Received data %s", receive_buff);		  
			    if(!database.call_redis(receive_buff,send_buff)){
				    sprintf(send_buff,"%s\r\n",send_buff);		    
				    write(connfd, send_buff, strlen(send_buff));
			    }
			    else{
				    close(connfd);
				    break;
			    }
		    }  
	    }
	    sleep(1);
    }
}
#define COMMANDS \
CMD(GET,0,"GET")\
CMD(SET, 1,"SET")\
CMD(SETEX,2,"SETEX")\
CMD(SETPX,3,"SETPX")\
CMD(SETNX,4,"SETNX")\
CMD(SETXX,5,"SETXX")\
CMD(SETBIT,6,"SETBIT")\
CMD(GETBIT,7,"GETBIT")\
CMD(ZADD,8,"ZADD")\
CMD(ZCARD,9,"ZCARD")\
CMD(ZCOUNT,10,"ZCOUNT")\
CMD(ZRANGE,11,"ZRANGE")\
CMD(EXPIRE,12,"EXPIRE")\
CMD(TTL,13,"TTL")\
CMD(PTTL,14,"PTTL")\
CMD(SAVE,15,"SAVE")\
CMD(LOAD,16,"LOAD")

int redis:: get_command(char *ptr){
#define CMD(val1,val2, val3)\
if(!strcmp(ptr,val3))\
	return val2;
	COMMANDS
#undef CMD
return INT_MAX;
}
int redis::call_redis(char *indata, char *outdata){
	char *str1;
	char *str2;
	int num, num2;
	int negflag = 1;
#define SKIP_PADDING(ptr)\
	if(*indata == '\r')indata++;\
	if(*indata == '\n')indata++;
	SKIP_PADDING(indata);
#define CMD(val1,val2,val3) val1 = val2,
	enum{
		COMMANDS
	};
#undef CMD 
#define STRTOKEN(ptr)\
	ptr = indata;\
	while(*indata != ' ' && *indata != '\r'){\
		if((*indata >= 'a' && *indata <= 'z') || (*indata >= 'A' && *indata <= 'Z') || (*indata >= '0' && *indata <= '9') || *indata == '/' || *indata == '.' )\
		indata++;\
		else{\
			*outdata = '0';\
			return 1;\
		}\
	}\
	*indata = '\0';\
	indata++;\
	if(*indata == '\r')\
	indata++;\
	if(*indata == '\n')\
	indata++;
#define NUMTOKEN(num)\
	num = 0;\
	negflag = 1;\
	while(*indata != ' ' && *indata != '\r'){\
		if((*indata >= '0' && *indata <= '9') || *indata == '-'){\
			num = 10*num + (*indata - '0');\
			if(*indata == '-'){\
				negflag = -1;\
				num = 0;\
			}\
			indata++;\
		}\
		else{\
			*outdata = '0';\
				return 1;\
		}\
	}\
	indata++;\
	num *= negflag;\
	if(*indata == '\r')\
	indata++;\
	if(*indata == '\n')\
	indata++;

	STRTOKEN(str1);
	switch(database.get_command(str1)){
		case GET:
			STRTOKEN(str1);
			printf("GET %s\n", str1);
			database.set.VALUE(str1, outdata);
			break;
		case SET:
			STRTOKEN(str1);
			STRTOKEN(str2);
			printf("SET %s %s\n", str1, str2);
			if(database.set.SET(str1,str2)){
				sprintf(outdata,"1");
			}
			else
				sprintf(outdata,"0");
			break;
		case EXPIRE:
			STRTOKEN(str1);
			NUMTOKEN(num);
			printf("EXPIRE %s %d\n", str1, num);
			if(database.set.SETEX(str1,num)){
				sprintf(outdata,"1");
			}
			else
				sprintf(outdata,"0");
			break;
		case TTL:
			STRTOKEN(str1);
			printf("TTL %s\n", str1);
			database.set.TTL(str1,outdata);
			break;
		case PTTL:
			STRTOKEN(str1);
			printf("PTTL %s\n", str1);
			database.set.PTTL(str1,outdata);
			break;
		case SETEX:
			STRTOKEN(str1);
			NUMTOKEN(num);
			STRTOKEN(str2);
			printf("SETEX %s %d %s\n", str1, num, str2);
			if(database.set.SET(str1,str2)){
				if(database.set.SETEX(str1,num)){
					sprintf(outdata,"1");
				}
			}
			else
				sprintf(outdata,"0");
			break;
		case SETPX:
			STRTOKEN(str1);
			NUMTOKEN(num);
			STRTOKEN(str2);
			printf("PSETEX %s %d %s\n", str1, num, str2);
			if(database.set.SET(str1,str2)){
				if(database.set.SETPX(str1,num)){
					sprintf(outdata,"1");
				}
			}
			else
				sprintf(outdata,"0");
			break;
		case SETNX:
			STRTOKEN(str1);
			STRTOKEN(str2);
			printf("SETNX %s %s\n", str1, str2);
			if(database.set.SETNX(str1,str2)){
				sprintf(outdata,"1");
			}
			else
				sprintf(outdata,"0");
			break;
		case SETXX:
			STRTOKEN(str1);
			STRTOKEN(str2);
			printf("SETXX %s %s\n", str1, str2);
			if(database.set.SETXX(str1,str2)){
				sprintf(outdata,"1");
			}
			else
				sprintf(outdata,"0");
			break;
		case SETBIT:
			STRTOKEN(str1);
			NUMTOKEN(num);
			NUMTOKEN(num2);
			printf("SETBIT %s %d %d\n", str1, num, num2);
			if(database.set.SETBIT(str1,num, num2)){
				sprintf(outdata,"1");
			}
			else
				sprintf(outdata,"0");
			break;
		case GETBIT:
			STRTOKEN(str1);
			NUMTOKEN(num);
			printf("GETBIT %s %d \n", str1, num);
			if(database.set.GETBIT(str1,num)){
				sprintf(outdata,"1");
			}
			else
				sprintf(outdata,"0");
			break;
		case ZADD:
			STRTOKEN(str1);
			NUMTOKEN(num);
			STRTOKEN(str2);
			printf("ZADD %s %d %s\n", str1,num, str2);
			if(database.range.ZADD(str1,num, str2)){
				sprintf(outdata,"1");
			}
			else
				sprintf(outdata,"0");
			break;
		case ZCARD:
			STRTOKEN(str1);
			printf("ZCARD %s\n", str1);
			if(num = database.range.ZCARD(str1)){
				sprintf(outdata,"%d", num);
			}
			else
				sprintf(outdata,"%d",num);
			break;
		case ZCOUNT:
			STRTOKEN(str1);
			NUMTOKEN(num);
			NUMTOKEN(num2);
			printf("ZCOUNT %s %d %d\n", str1, num, num2);
			if(num = database.range.ZCOUNT(str1, num, num2)){
				sprintf(outdata,"%d",num);
			}
			else
				sprintf(outdata,"%d",num);
			break;
		case ZRANGE:
			list_t *node;
			list_t *tmp;
			STRTOKEN(str1);
			NUMTOKEN(num);
			NUMTOKEN(num2);
			printf("ZRANGE %s %d %d\n", str1, num, num2);
			node = database.range.ZRANGE(str1, num, num2);	
			while(node){
				outdata += sprintf(outdata,"%s\n",node->_node->addr_str);
				tmp = node;			
				node = node->next;
				free(tmp);	
			}
			*(outdata-1) = '\0';			
			break;
		case SAVE:
			STRTOKEN(str1);
			if(database.save(str1)){
				sprintf(outdata,"1");
			}
			else
				sprintf(outdata,"0");
			break;
		case LOAD:
			STRTOKEN(str1);
			if(database.load(str1)){
				sprintf(outdata,"1");
			}
			else
				sprintf(outdata,"0");
			break;
	}
	return 0;
}
int redis::load(const char *file){
	char data_b[1024];
	char name1[1024];
	char name2[1024];
	char cmd[1024];
	int data1, data2;
	int len1,len2;
	char *ptr1, *ptr2;
	FILE *fp = fopen(file,"rb");
	if(!fp)
		return 0;
	fread(data_b,1,5, fp);
	data_b[6] = '\0';
	if(strcmp(data_b, "REDIS"))
		return 0;
	fread(data_b,1,1,fp);
	data1 = (int)data_b[0];
	while(data1){
		switch(data1){
			case 50:
				if(fread(&len1,1,4,fp) !=4)
					return 1;
				if(fread(name1,1,len1,fp) != len1)
					return 0;
				ptr2 = name1;
				while(*ptr2 != ',')
					ptr2++;
				*ptr2 = '\0';
				ptr2++;
				if(fread(&data1, 1, 4, fp) != 4)
					return 0;
				sprintf(cmd,"SET %s %s\r\n",name1, ptr2);				
				this->call_redis(cmd,cmd);
				sprintf(cmd,"EXPIRE %s %d\r\n", name1, data1);	
				this->call_redis(cmd,cmd);
				fread(data_b,1,1,fp);
				data1 = (int)data_b[0];
				break;
			case 51:
				if(fread(&len1,1,4,fp) !=4)
					return 1;
				if(fread(name1,1,len1,fp) != len1)
					return 0;
				if(fread(&len1,1,4,fp) !=4)
					return 0;
				if(fread(name2,1,len1,fp) != len1)
					return 0;
				if(fread(&data1, 1, 4, fp) != 4)
					return 0;
				sprintf(cmd,"ZADD %s %d %s\r\n", name1, data1, name2);
				this->call_redis(cmd,cmd);	
				fread(data_b,1,1,fp);
				data1 = (int)data_b[0];
				break;
			default:
				data1 = 0; 
				break;
		}
	}
return 1;
}
int redis::save(const char *file){
	FILE *fp = fopen(file,"wb");
	if(!fp)
		return 0;
	fwrite("REDIS",1,5,fp);
	range.save(fp);
	set.save(fp);
	fclose(fp);
	return 1;
}
