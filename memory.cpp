#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include<limits.h>
#include<string.h>
#include<iostream>
#include<stdlib.h>
using namespace std;
class memory{
	char *bitmap;
	int segment_id;
	char* shared_memory;
	char *sp, *ep;
	struct shmid_ds shmbuffer;
	int page_size;
	int bitmap_size;
	int size;
	public:
	memory(){
	}
	memory(int size = 1){ 
		page_size = 0;	
		if(size <= 0)
			return;
		segment_id = shmget (IPC_PRIVATE, size * getpagesize(),IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
		page_size = size * getpagesize();	
		bitmap_size = page_size / CHAR_BIT; 
		bitmap = (char *)malloc(bitmap_size);	
		if(!(segment_id < 0)){
			shared_memory = (char*) shmat (segment_id, 0, 0);
			page_size = size * getpagesize();
			sp = bitmap; 
			for(int i = 0; i < bitmap_size;i++){
				*sp &= 0x00 ;
				sp++;
			}
			sp = shared_memory;
			ep = sp + page_size; 
		}
	}
	int check_status(){
	if(!page_size)
		return 0;
		if(segment_id < 0) return 1;
		printf("page_size %d size of bitmap := %d \n",page_size,
				bitmap_size);
		return 0;
	}
	char* store(const char *ptr, int size);
	int del(char *ptr, int size);
	char* getfreebytes(int size); 
	~memory(){
		if(!page_size)
			return;
		shmdt(shared_memory);
		shmctl(segment_id, IPC_RMID, 0);
		cout<<"Dellocate"<<endl;
	}
	/* Debugging functions */
	void show_bitmap();
	void show_memory();	
};
void memory:: show_bitmap(){
	if(!page_size)
		return;
	int *sp = (int*)bitmap, *ep = (int*)(bitmap + bitmap_size);
	int count = 0;
	while(sp < ep){
		printf("%0x ", *sp);
		sp++;
		count++;
		if(!(count % 8))
			cout<<endl;
	}
	cout<<"count := "<<count<<endl;
}
void memory:: show_memory(){
	if(!page_size)
		return;
	char *sp = shared_memory, *ep = shared_memory + page_size;
	int count = 0;
	while(sp < ep){
		printf("%c", *sp);
		sp++;
		count++;
		if(!(count % 256))
			cout<<endl;
	}
	cout<<"count := "<<count<<endl;
}
char* memory:: getfreebytes(int size){
	if(size < 0) 
		return NULL;
	char *lsp = bitmap, *lep ;
	int big = size >> 3;
	if(size % 8)
		big++;
	int init_big = big;
	lep = bitmap + bitmap_size;
	while(lsp < lep && big){
		if(*lsp == 0){ 
			big--;
		}
		else{
			big = init_big;		
		}
		lsp++; 
	}
	if(big)
		return NULL;
	big = init_big;
	while(big){
		lsp--;
		*lsp = 0xFF;  
		big--;	
	}
	return (shared_memory + ((lsp - bitmap) << 3));
}
char* memory:: store(const char *ptr, int size){
	if(!page_size)
		return NULL;
	char *lsp, *rptr;
	if(rptr = (lsp = getfreebytes(size))){
		while(size){
			*lsp = *ptr;
			lsp++;
			ptr++;
			size--;		
		}
	}
	return rptr;
}
int memory:: del(char *ptr, int size){
	if(!page_size)
		return 1;
	char *lsp;
	int big = size >> 3;
	if(size % 8)
		big++;
	lsp = bitmap + ((shared_memory - ptr) >> 3);
	while(big){
		*lsp = 0x00;
		lsp++;
		big--;
	}
	return 0;
}
/*
int main()
{
	memory obj(5);
	obj.check_status();
	char *ptr;
	char str[] = "Jatinderpal singh\n";
	char str2[] = "balle balle\n";
	ptr =	obj.store(str, 20);
	obj.show_bitmap();
	obj.show_memory();
	ptr = obj.store(str2, 16);
	obj.del(ptr ,16);
	return 0;
}
*/


