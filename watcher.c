/*This is the sample program to notify us for the file creation and file deletion takes place in “/tmp” directory*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <linux/inotify.h>
#include<sys/stat.h>
#include<sys/resource.h>
#include "memcacheconnector.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
#define BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )
#define LOCKON "/mnt/hackathon/feeddata"
#define fname "analyze.log"
#define f2name "feedfile.log"
#define cname "campaign.data"
#define mname "memcache.data"
#define FILEBUFFSIZE (1024*64 + 1)
#define FIELD17 16
#define FIELD16 16
int MAXF = 16;
#define MAXCAMPAIGN 1000

FILE* getcfile(char *campaign , int campaintcount);
int flushcfile(int campaintcount);
int pushtomemcache();

struct campaign{
	char name[1024];
	FILE *cfile;
}_campaign[MAXCAMPAIGN];
typedef struct campaign campaign;
unsigned int count = 0;
struct analyze{
	char *prodId;          
	char *prodName;
	char *landingPage;
	char *imageUrl; 
	char *catName;
	char *catId;
	char *discountPrice; 
	char *offerPrice;
	char *blank;     
	char *description;
	char *subCat1Id;
	char *subCat1Name;
	char *subCat2Id;
	char *subCat2Name;
	char *misc1; 
	char *misc2;
	char *campaign;
}_feed; 
typedef struct analyze analyze; 
#define ANALYZESIZE  sizeof(analyze)
off_t fsize(const char *filename); 
int main( )
{
	struct rlimit core_limits;
	core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
	setrlimit(RLIMIT_CORE, &core_limits);
	int length, i = 0;
	int fd;
	int wd;
	char buffer[EVENT_BUF_LEN];
	char FBUFF[FILEBUFFSIZE];
	size_t fsizeold = fsize(fname), fsizenew = 0;
	size_t f2sizeold = fsize(f2name), f2sizenew = 0;
	size_t cfsizeold = fsize(cname), cfsizenew = 0, campaintcount = 0;
	size_t mfsizeold = fsize(mname), mfsizenew = 0, mentery = 0;
	size_t datatoread = 0, dataread = 0;
	FILE **AF = NULL, *FAF = fopen(fname, "r"), *F2AF = fopen(f2name, "r"), *MAF = fopen(mname, "r"), *CAF =  fopen(cname, "r");
	if(FAF  == NULL || F2AF == NULL || MAF == NULL)
		printf("Failed to open some file\n");
	char *sp, *ep;
	char **feed = (char**)&_feed;
	fd = inotify_init();
	if ( fd < 0 ) {
		perror( "inotify_init" );
	}
	wd = inotify_add_watch( fd, LOCKON , IN_MODIFY | IN_CREATE | IN_DELETE );
	length = read( fd, buffer, EVENT_BUF_LEN ); 
	if ( length < 0 ) {
		perror( "read" );
	}  
	struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
	while(1){
		while ( i < length ) {    
			if ( event->len ) {
				memset(FBUFF, 0, FILEBUFFSIZE);
				if ( event->mask & IN_MODIFY ) {
					if ( !(event->mask & IN_ISDIR) ) {
						printf("evnet %s\n", event->name);
						// Reading analyze file start
						if(!strcmp(event->name , mname)){
							MAXF = FIELD16; 
							mfsizenew = fsize(mname);
							datatoread = mfsizenew - mfsizeold;
							AF = &MAF;
							fseek(*AF, mfsizeold, SEEK_SET);
						}
						else if(!strcmp(event->name , f2name)){
							MAXF = FIELD17; 
							f2sizenew = fsize(f2name);
							datatoread = f2sizenew - f2sizeold;
							AF = &F2AF;
							fseek(*AF, f2sizeold, SEEK_SET);
						}
						else{ 
							MAXF = FIELD17;
							fsizenew = fsize(fname);
							datatoread = fsizenew - fsizeold;
							AF = &FAF;
							fseek(*AF, fsizeold, SEEK_SET);
						}
						while(datatoread > 0){
							if(datatoread >= FILEBUFFSIZE){
								dataread = fread(FBUFF, 1, FILEBUFFSIZE, *AF);
							}
							else
								dataread = fread(FBUFF, 1, datatoread, *AF);
							datatoread -= dataread;
							sp = FBUFF; ep = sp+dataread;
							// printf("sp %u ep %u\n", sp, ep);
							int pos = 0;
							*(&feed[pos]) = sp;
							while(sp < ep){
								if(pos == MAXF ){
									while(*sp != '\n')
										sp++;
									*sp = '\0';
									count++;
									if(!strcmp(event->name , mname)){
										pushtomemcache();
									}
									else{
										FILE *cfile = getcfile(_feed.campaign, campaintcount);
										if(cfile == NULL){
											printf("No file for campaign %s\n",  _feed.campaign);
										}
										else{
											fprintf(cfile,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",_feed.prodId, _feed.prodName, _feed.landingPage, _feed.imageUrl, _feed.catName, _feed.catId, _feed.discountPrice, _feed.offerPrice, _feed.blank, _feed.description, _feed.subCat1Id, _feed.subCat1Name, _feed.subCat2Id, _feed.subCat2Name, _feed.misc1, _feed.misc2);
											printf("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",_feed.prodId, _feed.prodName, _feed.landingPage, _feed.imageUrl, _feed.catName, _feed.catId, _feed.discountPrice, _feed.offerPrice, _feed.blank, _feed.description, _feed.subCat1Id, _feed.subCat1Name, _feed.subCat2Id, _feed.subCat2Name, _feed.misc1, _feed.misc2);
										}
									}
									pos = 0;
									*(&feed[pos]) = sp+1;
								}
								if(*sp == '\t'){
									*sp = '\0';
									if(*(sp+1) != '\t'){

										pos++;
										feed[pos] = ++sp;
									}
									else{
										while(*(sp+1) == '\t'){
											*sp = '\0';
											pos++;
											feed[pos] = sp;
											sp++;
										}
										pos++;
										feed[pos] = sp;
										sp++;

									}
									while(*sp == '\t'){
										*sp = '\0';
										feed[pos] = sp;
										pos++;
										sp++;
									}
									if(*sp == '\0')
										feed[pos] = ++sp;
									else 
										feed[pos] = sp;
								}
								sp++;
							}
							//printf("new data :=\n%s\n", FBUFF);
							memset(FBUFF, 0, dataread);
						}
						if(!strcmp(event->name , mname)){
							mfsizeold = mfsizenew;
						}
						else if(!strcmp(event->name , f2name)){
							f2sizeold = f2sizenew;
						}
						else{ 
							fsizeold = fsizenew;
						}
						flushcfile(campaintcount);
					}
					//Reading analyze file end
					// Campaing list change handler start
					if(!strcmp(event->name, cname)){
						cfsizeold = 0;
						cfsizenew = fsize(cname);
						datatoread = cfsizenew - cfsizeold;
						AF = &CAF;
						fseek(*AF, cfsizeold, SEEK_SET);
						if(*AF == NULL)
							return 1;
						while(datatoread > 0){
							if(datatoread >= FILEBUFFSIZE){
								dataread = fread(FBUFF, 1, FILEBUFFSIZE, *AF);
							}
							else
								dataread = fread(FBUFF, 1, datatoread, *AF);
							sp = FBUFF; ep = sp+dataread;
							campaintcount  = 0;
							int ni = 0;
							while(sp < ep){
								if(campaintcount == MAXCAMPAIGN){
									break;
								}
								if(*sp == '\n'){
									_campaign[campaintcount].name[ni] = '\0';
									_campaign[campaintcount].cfile = fopen(_campaign[campaintcount].name, "w");
									if(_campaign[campaintcount].cfile == NULL){
										printf("Failed to open file for %s\n",_campaign[campaintcount].name);
									}
									printf("Campaign registered %s\n", _campaign[campaintcount].name);
									campaintcount++;
									ni = 0;
									sp++;
								}
								_campaign[campaintcount].name[ni] = *sp;
								ni++;
								sp++;
							}
							datatoread -= dataread;
						}
					}
					// Campaign list change handler end
				}
			}
			i += EVENT_SIZE + event->len;
		}
		length = read( fd, buffer, BUF_LEN );
		i = 0;
		event = ( struct inotify_event * ) &buffer[ i ];
		if ( length < 0 ) {
			perror( "read" );
		}
	}
	/*removing the “/tmp” directory from the watch list.*/
	inotify_rm_watch( fd, wd );
	/*closing the INOTIFY instance*/
	close( fd );
}
off_t fsize(const char *filename) {
	struct stat st; 
	if (stat(filename, &st) == 0)
		return st.st_size;
	return -1; 
}
FILE* getcfile(char *campaign , int campaintcount){
	int i = 0;
	for(i = 0; i < campaintcount; i++){
		// printf("%s %d  %s %d\n", _campaign[i].name, (int)strlen(_campaign[i].name), campaign, (int)strlen(campaign));
		if(!strcmp(_campaign[i].name, campaign))
			return _campaign[i].cfile;
	}
	return NULL;
}
int flushcfile(int campaintcount)
{
	int i = 0;
	for(i = 0; i < campaintcount; i++){
		fflush(_campaign[i].cfile);
	}
}
int pushtomemcache(){
	char key[1024];
	char value[1024*32];
	sprintf(key,"%s",_feed.prodId);
	sprintf(value,"[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]",_feed.prodName, _feed.landingPage, _feed.imageUrl, _feed.catName, _feed.catId, _feed.discountPrice, _feed.offerPrice, _feed.blank, _feed.description, _feed.subCat1Id, _feed.subCat1Name, _feed.subCat2Id, _feed.subCat2Name, _feed.misc1, _feed.misc2, _feed.campaign);
	printf("%s\t%s\n", key, value);
	update_cache(key, value);
	return 0;
}
