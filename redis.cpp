#include<iostream>
#include<limits.h>
#include<string.h>
#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include "memory.cpp"
#define SIZE_HASH_TABLE INT_MAX>>16
#define SIZE_TREE INT_MAX>>16>>8 
#define MAX_PAIR_LEN 1024 
#define LINEAR_PROBE 20 
using namespace std;

typedef struct map{
	unsigned int size;
	time_t extime;
	char *value;
}map_t;
typedef struct node{
	int data;
	int size;
	char *addr_str;
	struct node *left;
	struct node *right;
	struct node *parent;
}node;
typedef struct map_tree{
	class tree *addr;
}map_tree_t;
typedef struct list{
	node *_node;
	struct list *next;
}list_t;
class tree: private memory{
	char name[1024];
	node* root;
	int count;
	static map_tree_t hash_table[SIZE_TREE];
	public:
	tree():memory(0){
	}
	tree(const char str[]):memory(50){
		int len = strlen(str);
		int probe = 0;
		int i;	
		if(len < 1024){
			for(i = 0; i < len;i++)
				name[i] = str[i];
		}
		name[i] = '\0';
		root = NULL;
		count = 0;
		i = MurmurHash1(str,len)%(SIZE_TREE);
		while(hash_table[i].addr){
			i++;
			probe++;
			if(probe == LINEAR_PROBE || i == SIZE_TREE)
				return;
		}
		hash_table[i].addr = this;
	}
	unsigned int MurmurHash1(const void *key, int len);
	node* addintotree(int, int,const char str[]);
	node* gettree(const char str[]);
	node* getfromtree(node*,const char str[]);
	node* deletefromtree(node*, node*);
	void countfromtree(node*, int min, int max, int *count);
	void getlistfromtree(node*,int min, int max,list_t **head);
	void savepreoder(node*, FILE *fp, const char *name);	
	int ZADD(const char key[], int data,const char str[]);
	int ZCARD(const char str[]);
	int ZCOUNT(const char str[], int min, int max);
	int GET(const char key[]);
	list_t* ZRANGE(const char key[], int min, int max);
	int save(FILE *fp);
};
map_tree_t tree:: hash_table[SIZE_TREE];
void tree::savepreoder(node* ptr, FILE *fp, const char *name){
	int len;
	if(!ptr)
		return;
	fwrite("3", 1, 1, fp);		
	len = strlen(name);	
	len++;	
	fwrite(&len, 1, 4, fp);		
	fwrite(name, 1, len, fp);
	fwrite(&ptr->size, 1, 4, fp);		
	fwrite(ptr->addr_str, 1, ptr->size, fp);
	fwrite(&ptr->data, 1, 4, fp);
	savepreoder(ptr->left, fp, name);	
	savepreoder(ptr->right, fp, name);	
}
int tree::save(FILE *fp){
	int i;
	for(i = 0; i < SIZE_TREE; i++){
		if(hash_table[i].addr){
			savepreoder(hash_table[i].addr->root, fp, hash_table[i].addr->name);	
		}
	} 
}
list_t* tree:: ZRANGE(const char key[],int min, int max){
	list_t *head = NULL;
	int index = GET(key);
	if(index != -1){
		getlistfromtree(hash_table[index].addr->root, min, max, &head);	
	}
	return head;
}
void tree:: getlistfromtree(node* ptr,int min, int max, list_t **head){
	static list_t *tail = NULL;
	if(!head) 
		tail = NULL;
	list_t *new_node;	
	if(ptr){
		if(ptr->left && ptr->left->data >= min)
			getlistfromtree(ptr->left, min, max, head);
		if((ptr->data >= min) && (ptr->data <= max)){
			new_node = (list_t*) malloc(sizeof(list_t));
			if(tail)			
				tail->next = new_node ;
			tail = new_node;			
			tail->next = NULL;		
			tail->_node = ptr;
			if(!*head)
				*head = tail;		
		}
		if(ptr->right && ptr->right->data <= max)
			getlistfromtree(ptr->right, min, max, head);
	}
}
void tree:: countfromtree(node *ptr, int min, int max, int *count){
	if(ptr){
		if((ptr->data >= min) && (ptr->data <= max))
			(*count)++;
		if(ptr->left && min <= ptr->left->data)		
			countfromtree(ptr->left, min, max, count);
		if(ptr->right && max >= ptr->right->data)		
			countfromtree(ptr->right, min, max, count);
	}
}
int tree:: ZCOUNT(const char key[], int min, int max){
	int index = GET(key);
	int count = 0;
	if(index != -1){
		countfromtree(hash_table[index].addr->root, min, max, &count);
	}
	return count;
}
node* tree:: deletefromtree(node *root, node *ptr){
	node *inords = NULL;
	if(ptr->left && ptr->right){
		inords = ptr->right;
		while(inords->left){
			inords = inords->left;
		}
		deletefromtree(root, inords);
		inords->left = ptr->left;
		if(ptr->left)
			ptr->left->parent = inords;
		inords->right = ptr->right;
		if(ptr->right)
			ptr->right->parent = inords;
		inords->parent = ptr->parent;
		if(ptr->parent){
			if(ptr == ptr->parent->left)
				ptr->parent->left = inords;
			else
				ptr->parent->right = inords;
		}
	}
	else if(ptr->left){
		if(ptr->parent){	
			if(ptr == ptr->parent->left){
				ptr->parent->left = ptr->left;
				ptr->left->parent = ptr->parent;
			}
			else{
				ptr->parent->right = ptr->left;
				ptr->left->parent = ptr->parent;
			}
		}
		else{
			inords = ptr->left;
			ptr->left->parent = NULL;
		}
	}
	else if(ptr->right){
		if(ptr->parent){
			if(ptr == ptr->parent->left){
				ptr->parent->left = ptr->right;
				ptr->right->parent = ptr->parent;
			}
			else{
				ptr->parent->right = ptr->right;
				ptr->right->parent = ptr->parent;
			}	
		}
		else{
			inords = ptr->right;
			ptr->right->parent = NULL;
		}
	}
	else{
		if(ptr->parent){
			if(ptr == ptr->parent->left){
				ptr->parent->left = NULL;
			}
			else{
				ptr->parent->right = NULL;
			}
		}
		else{	
			inords = ptr->parent;
			ptr->parent = NULL;
		}
	}
	if(ptr == root) 
		root = inords;
	return root;
}
int tree::ZADD(const char key[], int data,const char str[]){
	int index = GET(key);
	node *ptr;
	node *tmp;
	if(index != -1){
		if(tmp = getfromtree(hash_table[index].addr->root, str)){
			hash_table[index].addr->root = deletefromtree(hash_table[index].addr->root, tmp);		
			hash_table[index].addr->del(tmp->addr_str, tmp->size);	
			hash_table[index].addr->del((char*)tmp, sizeof(node));
			hash_table[index].addr->count--;	
		}
		tmp = addintotree(index, data, str);
		if(!tmp) 
			return 0;
		hash_table[index].addr->root = tmp;		
		return 1;	
	}
	else{
		tree *obj = new tree(key);
		index = GET(key);
		tmp = addintotree(index, data, str);
		if(!tmp) 
			return 0;
		obj->root = tmp;		
		return 1;	

	}
	return 0;
}
int tree:: GET(const char key[]){
	int index;
	int len1 = strlen(key);
	int probe = 0;
	index = MurmurHash1(key, len1) % (SIZE_TREE);
	while(probe < LINEAR_PROBE || index < SIZE_TREE){
		if(hash_table[index].addr){
			if(!strcmp(key, hash_table[index].addr->name))
				return index;
			index++;
			probe++;
		}
		else
			return -1;
	}
	return -1;
}
int tree:: ZCARD(const char key[]){
	int index = GET(key);
	if(index != -1)
		return hash_table[index].addr->count;
	return 0;
}
node* tree:: getfromtree(node* ptr,const char str[]){
	static node *rptr = NULL;
	rptr = NULL;
	if(ptr) {
		if(!strcmp(ptr->addr_str,str))
			return rptr = ptr;
		getfromtree(ptr->left, str);
		getfromtree(ptr->right, str);
	}
	return rptr;
}
node* tree:: addintotree(int index, int data,const char str[]){
	node *ptr = hash_table[index].addr->root;
	node *_ptr = ptr;
	node *new_node = (node*)hash_table[index].addr->getfreebytes(sizeof(node));
	if(!new_node) 
		return NULL;	
	hash_table[index].addr->count++;
	new_node->data = data;
	new_node->left = NULL;
	new_node->right = NULL;
	new_node->size = strlen(str)+1;
	new_node->addr_str = hash_table[index].addr->store(str, new_node->size);
	*(new_node->addr_str+new_node->size) = '\0';// NULL termination of string for strcmp
	if(NULL == ptr){
		new_node->parent = NULL;
		return new_node;
	}
	while(1){
		if(_ptr->data > data){
			if(NULL == _ptr->left){
				new_node->parent = _ptr;
				_ptr->left = new_node;
				break;
			}
			_ptr = _ptr->left;
		}
		else{
			if(NULL == _ptr->right){
				new_node->parent = _ptr;
				_ptr->right = new_node;
				break;
			}
			_ptr = _ptr->right;
		}

	}
	return ptr;
}
class table: private memory{
	map_t hash_table[SIZE_HASH_TABLE];
	char buff[MAX_PAIR_LEN];
	public:
	table():memory(300){
		int i;
		for(i = 0; i < SIZE_HASH_TABLE; i++){
			hash_table[i].extime = 0;
		} 
	}
	unsigned int MurmurHash2(const void *key, int len);
	int SET(const char key[], const char value[]);
	int SETEX(const char key[], int exsec);
	int SETPX(const char key[], int exsec);	
	int SETNX(const char key[], const char value[]);
	int SETXX(const char key[], const char value[]);
	int GET(const char key[]);
	void VALUE(const char key[], char buff[]);
	void COPY_VALUE(char *ptr,int size, char buff[]);
	void COPY_KEY(char *ptr,int size, char buff[]);
	int SETBIT(const char key[], int offset, int value);
	int GETBIT(const char key[], int offset);
	void TTL(const char key[], char *buff);
	void PTTL(const char key[], char *buff);
	int save(FILE *fp);
	void show_memory(){
		memory::show_memory();
	}
};
int table::save(FILE *fp){
	int i;
	for(i = 0; i < SIZE_HASH_TABLE; i++){
		if(hash_table[i].extime){
			fwrite("2",1,1,fp);		
			fwrite(&hash_table[i].size,1,4,fp);	
			fwrite(hash_table[i].value,1,hash_table[i].size,fp);
			hash_table[i].extime = hash_table[i].extime - time(NULL);	
			fwrite(&hash_table[i].extime,1,4,fp);
			hash_table[i].extime = hash_table[i].extime + time(NULL);	
		}
	} 
}
void table:: TTL(const char key[], char *buff){
	int index = GET(key);	
	if(index != -1){
		sprintf(buff,"(integer) %ld",(hash_table[index].extime - time(NULL)));
		return;
	}	
}
void table::PTTL(const char key[], char *buff){
	int index = GET(key);	
	if(index != -1){
		sprintf(buff,"(integere %ld",((hash_table[index].extime - time(NULL))*1000));
		return;
	}	
}
void table:: COPY_KEY(char *ptr, int size, char buff[]){
	while(size){
		if(*ptr == ',')
			break;
		*buff = *ptr;
		ptr++;
		buff++;		
		size--;
	}
	*buff = '\0';
}
void table:: COPY_VALUE(char *ptr, int size, char buff[]){
	while(size){
		if(*ptr == ',')
			break;
		ptr++;		
		size--;
	}
	ptr++;
	size--;
	while(size){
		*buff = *ptr;
		ptr++;
		buff++;
		size--;
	}
	*buff = '\0';
}
int table:: GETBIT(const char key[], int offset){
	if(!offset)return 0;	
	int index = GET(key);	
	char *ptr;
	int size;
	int big = offset/8;
	int small = offset % 8;
	int mask;
	if(small){	
		mask = 0x01 << (small - 1);
	}
	else if(big){
		mask = 0x80;
	}	
	if(index != -1){
		ptr = hash_table[index].value;
		size = hash_table[index].size;
		while(size){
			if(*ptr == ',')break;
			size--;
			ptr++;		
		}
		ptr++;
		size--;
		while(size){
			if(big){			
				if(big == 1 && mask == 0x80) 
					return(!!(*ptr & mask));
				ptr++;
				big--;
			}
			else{
				return(!!(*ptr & mask));
			}
			size--;
		}
	}
	else
		return 0;	

}
int table:: SETBIT(const char key[], int offset, int value){
	if(!offset)return 0;	
	int index = GET(key);	
	char *ptr;
	int size;
	int big = offset/8;
	int small = offset % 8;
	int mask;
	if(small){	
		mask = ~(0x01 << (small - 1));
	}
	else if(big){
		mask = ~(0x80);
	}	
	if(index != -1){
		ptr = hash_table[index].value;
		size = hash_table[index].size;
		while(size){
			if(*ptr == ',')break;
			ptr++;		
			size--;
		}
		ptr++;
		size--;
		while(size){
			if(big){			
				if(big == 1 && mask == ~0x80)
					if(value){ 
						*ptr |= ~mask;
						return 1;
					}
					else{ 
						*ptr &= mask; 
						return 1;
					}
				ptr++;
				big--;
			}
			else{
				if(value){ 
					*ptr |= ~mask;
					return 1;
				}
				else{ 
					*ptr &= mask; 
					return 1;
				}
			}
			size--;
		}
	}
	else
		return 0;	
}
void table:: VALUE(const char key[], char buff[]){
	int index = GET(key);	
	if(index != -1){
		COPY_VALUE(hash_table[index].value, hash_table[index].size, buff);
	}
	else
		*buff = '\0';	
}
int table:: GET(const char key[]){
	int index;
	int len1 = strlen(key);
	char buff[1024];
	int probe = 0;
	index = MurmurHash2(key, len1) % SIZE_HASH_TABLE;
	while(probe < LINEAR_PROBE && index < SIZE_HASH_TABLE){
		if(hash_table[index].extime){
			COPY_KEY(hash_table[index].value, hash_table[index].size, buff);
			//printf("copy key %s\n", buff);			
			if(!strcmp(key, buff))
				return index;
			probe++;
			index++;
		}
		else 
			return -1;
	}
	return -1;
}
int table:: SETEX(const char key[],int exsec){
	int index = GET(key);	
	if(index != -1){
		hash_table[index].extime = time(NULL) + exsec;
		return 1;
	}	
	return 0;
}
int table:: SETPX(const char key[],int exsec){
	int index = GET(key);
	if(index != -1) {
		hash_table[index].extime = time(NULL) + (exsec)/1000;	
		return 1;
	}	
	return 0;
}
int table:: SETXX(const char key[], const char value[]){
	int index = GET(key);
	if(index != -1) {
		del(hash_table[index].value, hash_table[index].size);
		hash_table[index].extime = 0;		
		return SET(key,value);
	}
	else	
		return 0;
}
int table:: SETNX(const char key[], const char value[]){
	int index = GET(key);
	if(index == -1) {
		return SET(key,value);
	}
	return 0;
}
int table:: SET(const char key[], const char value[]){
	int index;
	char *addr_memory;		
	int len1 = strlen(key);
	int len2 = strlen(value);
	int probe = 0;
	//printf("len1 %d len2 %d\n",len1 , len2);
	if((len1 + len2 ) >= (MAX_PAIR_LEN))
		return 0;
	index = GET(key);
	if(index != -1){
		return SETXX(key, value);
	}
	index = MurmurHash2(key, len1) % SIZE_HASH_TABLE;
	while(hash_table[index].extime){
		index++;
		probe++;
		if(probe == LINEAR_PROBE || index == SIZE_HASH_TABLE)
			return 0; 
	}
	len1=sprintf(buff,"%s,%s\0",key,value);
	buff[len1] = '\0';
	len1++;	
	printf("len1 %d %s", len1, buff);
	addr_memory = store(buff, len1);
//	printf("Index %d <key,value> %s  %u\n",index, buff,(unsigned int) addr_memory);		
	hash_table[index].size = len1; 
	hash_table[index].value = addr_memory;
	hash_table[index].extime = time(NULL);	
	return 1;
}
/*
int main(){
	const char str1[] = "jatinderpal";
	const char str2[] = "singh";
	const char str3[] = "abhishek";
	const char str4[] = "bathla";
	table obj;
	char buff[1024];
	obj.SETXX(str1, str4);
	obj.VALUE(str1, buff);
	//obj.show_memory();
	printf("key %s value %s\n", str1, buff);
	obj.VALUE(str3,buff);
	printf("key %s value %s\n", str3, buff);
	int off = 0,	val = 0;
	off = 8; val = 1;	
	obj.SETBIT(str1, off, val);
	printf("bit str1 off %d := %d\n",off, obj.GETBIT(str1,off));
	obj.VALUE(str1,buff);
	printf("key %s value %s\n", str1, buff);
	off = 8; val = 0;	
	obj.SETBIT(str1, off, val);
	printf("bit str1 off %d := %d\n",off, obj.GETBIT(str1,off));
	obj.VALUE(str1,buff);
	printf("key %s value %s\n", str1, buff);
	off = 3; val = 0;	
	obj.SETBIT(str1, off, val);
	printf("bit str1 off %d := %d\n",off, obj.GETBIT(str1,off));
	off = 4; val = 0;	
	obj.SETBIT(str1, off, val);
	printf("bit str1 off %d := %d\n",off, obj.GETBIT(str1,off));
	off = 5; val = 0;	
	obj.SETBIT(str1, off, val);
	printf("bit str1 off %d := %d\n",off, obj.GETBIT(str1,off));
	off = 6; val = 0;	
	obj.SETBIT(str1, off, val);
	printf("bit str1 off %d := %d\n",off, obj.GETBIT(str1,off));
	off = 7; val = 0;	
	obj.SETBIT(str1, off, val);
	printf("bit str1 off %d := %d\n",off, obj.GETBIT(str1,off));
	off = 8; val = 0;	
	obj.SETBIT(str1, off, val);
	printf("bit str1 off %d := %d\n",off, obj.GETBIT(str1,off));

	tree trees(str2);
	trees.ZADD(str2, 1 , str2);
	trees.ZADD(str2, 3 , str1);
	trees.ZADD(str2, 100 , str3);
	cout<<"ZCARD := "<<trees.ZCARD(str2)<<endl;
	cout<<"ZCOUNT := "<<trees.ZCOUNT(str2,0,10)<<endl;
	list_t *head = trees.ZRANGE(str2,1,5);
	while(head){
		printf("<%s,%d>\n", head->_node->addr_str, head->_node->data);
		head = head->next;
	}	
	return 0;
}
*/
unsigned int tree:: MurmurHash1 ( const void * key, int len)
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.
	const unsigned int m = 0x5bd1e995;
	const int r = 24;
	unsigned int h = len;
	// Mix 4 bytes at a time into the hash
	const unsigned char * data = (const unsigned char *)key;
	while(len >= 4)
	{
		unsigned int k = *(unsigned int *)data;
		k *= m; 
		k ^= k >> r; 
		k *= m; 
		h *= m; 
		h ^= k;
		data += 4;
		len -= 4;
	}
	// Handle the last few bytes of the input array
	switch(len)
	{
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0];
			h *= m;
	};
	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	return h;
}
unsigned int table:: MurmurHash2 ( const void * key, int len)
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.
	const unsigned int m = 0x5bd1e995;
	const int r = 24;
	unsigned int h = len;
	// Mix 4 bytes at a time into the hash
	const unsigned char * data = (const unsigned char *)key;
	while(len >= 4)
	{
		unsigned int k = *(unsigned int *)data;
		k *= m; 
		k ^= k >> r; 
		k *= m; 
		h *= m; 
		h ^= k;
		data += 4;
		len -= 4;
	}
	// Handle the last few bytes of the input array
	switch(len)
	{
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0];
			h *= m;
	};
	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	return h;
}
