#include "logic.h"
#include "menu.h"
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <locale.h>
#include <time.h>
#include <sys/stat.h>


char base_path[256];


int getXResolution (char* path){
    FILE *f = fopen(path,"rb");
    if(!f){
        LOG_DEBUG("Image couldnt be opened");
        return 0;
    }
    unsigned char buf[24];
    if(fread(buf,1,24,f)<24){
        fclose(f);
        return 0;
    }
    int width=0;
    if(buf[0]==0x89 && buf[1]=='P' && buf[2]=='N' && buf[3]=='G'){
        width=(buf[16] <<24) | (buf[17] << 16) | (buf[18] << 8) | (buf[19]);
    }
    fclose(f);
    LOG_DEBUG("Width:%d, path:%s",width, path);
    return width;
}

int getYResolution (char* path){
    FILE *f = fopen(path,"rb");
    if(!f){
        LOG_DEBUG("Image couldnt be opened");
        return 0;
    }
    unsigned char buf[24];
    if(fread(buf,1,24,f)<24){
        fclose(f);
        return 0;
    }
    int height=0;
    if(buf[0]==0x89 && buf[1]=='P' && buf[2]=='N' && buf[3]=='G'){
        height=(buf[20] <<24) | (buf[21] << 16) | (buf[22] << 8) | (buf[23]);
    }
    fclose(f);
    LOG_DEBUG("Height:%d, path:%s",height, path);
}

char* getFileInfo(char* path){
    struct stat st;
    char info[100];
    if(stat(path,&st) ==0){
        if(S_ISDIR(st.st_mode)){
            return strdup("<DIR>");
        }else{
            double size_kb=(double)st.st_size/1024;
            sprintf(info,"%.01f KB | W: %d px H: %d px",size_kb,getXResolution(path),getYResolution(path));
        }
        return strdup(info);
    }
    return strdup("---");  
}




void openImage(char* path){
    char command[512]="start ";
    //Make path correct
    for(int i=strlen(path);i>0;i--){
        if(path[i]=='/')path[i]='\\';
        if(path[i]=='\\')break;
    }
    sprintf(command, "start \"\" \"%s\"", path);
    //printf("%s", command);
    LOG_DEBUG("Try to open image: %s, with command: %s", path,command);
    system(command);
    
}

Menu* createAlbumMenu(char* path,char* name){
    DIR *dir=opendir(path);
    if(!dir) {
        return NULL;
        LOG_DEBUG("The path is not correct, dir couldnt be opened");
    }
    Menu *m =(Menu*)malloc(sizeof(Menu));
    m->title= strdup(name);
    m->options=NULL;
    m->n=0;

    struct dirent *ent;
    while((ent=readdir(dir))!=NULL){
        if(ent->d_name[0]== '.') continue;

        char full_path[512];
        snprintf(full_path,sizeof(full_path), "%s/%s", path,ent->d_name);

        struct stat st;
        stat(full_path,&st);

        LOG_DEBUG("Element found: %s (%s)", ent->d_name, S_ISDIR(st.st_mode) ? "DIR" : "FILE");

        m->options=realloc(m->options,(m->n+1)*sizeof(Option));
        Option *o= &m->options[m->n];
        o->name= strdup(ent->d_name);
        o->details=getFileInfo(full_path);
        o->path= strdup(full_path);
        if(S_ISDIR(st.st_mode)){
            o->submenu=createAlbumMenu(full_path,ent->d_name);
            o->action=NULL;
        }
        else{
            o->submenu=NULL;
            o->action = NULL;
        }
        m->n++;
    }
    /*
    m->options = (Option*)realloc(m->options,(m->n+1)*sizeof(Option));
    m->options[m->n].name= strdup("[BACK]");
    m->options[m->n].details=strdup("");
    m->options[m->n].submenu=NULL;
    m->options[m->n].action=NULL;
    m->n++;
    */
    closedir(dir);
    LOG_DEBUG("Meniu '%s' created with %d options", name, m->n);
    return m;
}

int debug_mode=1;

void loadConfig(){
    FILE *f=fopen("config.cfg","r");
    if(!f){
        printf("Fisierul de configurare nu a fost gasit te rog sa introduci calea: ");
        scanf("%s",base_path);
        LOG_DEBUG("config file not found, creating one");
        //create cfg file
        FILE *f_new=fopen("config.cfg","w");
        fprintf(f_new,"ALBUMS_PATH=%s\n",base_path);
        fprintf(f_new,"DEBUG_MODE=%d",debug_mode);
        fclose(f_new);
        printf("Fisierul de configurarea a fost creat si calea adaugata");
        pause_screen();
        return ;
    }
    char line[300];
    while(fgets(line,sizeof(line),f)){
        if(strncmp(line,"ALBUMS_PATH=",12)==0){
            sscanf(line,"ALBUMS_PATH=%s",base_path);
        }
        if(strncmp(line,"DEBUG_MODE=",13)==0){
            sscanf(line,"DEBUG_MODE=%d",debug_mode);
        }
        LOG_DEBUG("Path set as: %s",base_path);

    }
    fclose(f);
}

/*
void action_list_directory() {
    list_directory("C:/Users/HP/Documents/PicManager-C");
    pause_screen();
}

void list_directory(const char* dirname){
    DIR *dir = opendir(dirname);
    if (!dir) {
        fprintf(stderr, "Cannot open %s (%s)\n", dirname, strerror(errno));
        return;
    }

    printf("Directory of %s\n\n", dirname);
    int filecount = 0;
    int dircount = 0;
    long long bytecount = 0;
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        char path[PATH_MAX + 2];
        snprintf(path, sizeof(path), "%s/%s", dirname, ent->d_name);

        struct stat stbuf;
        if (stat(path, &stbuf) == -1) {
            continue; // skip if can't stat
        }

        char *type;
        if (S_ISDIR(stbuf.st_mode)) {
            type = "<DIR>";
            list_directory(strcat(dirname, ent->d_name));
            dircount++;
        } else if (S_ISREG(stbuf.st_mode)) {
            type = "";
            filecount++;
            bytecount += (long long)stbuf.st_size;
        } else {
            type = "<UNK>";
        }

        struct tm *tp = localtime(&stbuf.st_mtime);
        char mtime[40];
        sprintf(mtime, "%04d-%02d-%02d %02d:%02d",
            tp->tm_year + 1900,
            tp->tm_mon + 1,
            tp->tm_mday,
            tp->tm_hour,
            tp->tm_min);

        char s[40];
        if (S_ISREG(stbuf.st_mode)) {
            sprintf(s, "%lld", (long long)stbuf.st_size);
        } else {
            s[0] = '\0';
        }

        printf("%-20s      %-5s  %12s %s\n", mtime, type, s, ent->d_name);
    }
    
    closedir(dir);

    printf("%20d Files %12lld bytes\n", filecount, bytecount);
    printf("%20d Dirs\n", dircount);
    
}
*/