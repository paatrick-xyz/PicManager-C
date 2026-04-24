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
#include <direct.h>
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
char base_path[256];

int getXResolution (char* path){
    FILE *f = fopen(path,"rb");
    if(!f){
        LOG_DEBUG("Image couldnt be opened");
        return 0;
    }
    unsigned char buf[32];
    if(fread(buf,1,32,f)<24){
        fclose(f);
        return 0;
    }
    int width=0;
    if(buf[0]==0x89 && buf[1]=='P' && buf[2]=='N' && buf[3]=='G'){
        width=(buf[16] <<24) | (buf[17] << 16) | (buf[18] << 8) | (buf[19]);
    }
    if(buf[0]==0xFF && buf[1]==0xD8){
        int pos=2;
        while(pos<32){
            if(buf[pos]==0xFF && (buf[pos+1] & 0xF0)==0xC0){
                width=(buf[pos+4] <<8) | (buf[pos+5]);
                break;
            }
            pos+=2+(buf[pos+2]<<8) | buf[pos+3];
        }
    }
    fclose(f);
    LOG_DEBUG("Width:%d, path:%s , %s",width, path, buf);
    return width;
}

int getYResolution (char* path){
    FILE *f = fopen(path,"rb");
    if(!f){
        LOG_DEBUG("Image couldnt be opened");
        return 0;
    }
    unsigned char buf[32];
    if(fread(buf,1,32,f)<24){
        fclose(f);
        return 0;
    }
    int height=0;
    if(buf[0]==0x89 && buf[1]=='P' && buf[2]=='N' && buf[3]=='G'){
        height=(buf[20] <<24) | (buf[21] << 16) | (buf[22] << 8) | (buf[23]);
    }
    fclose(f);
    LOG_DEBUG("Height:%d, path:%s",height, path);
    return height;
}

char* getFileInfo(char* path){
    struct stat st;
    char info[100];
    if(stat(path,&st) ==0){
        if(S_ISDIR(st.st_mode)){
            return strdup("<DIR>");
        }else{
            double size_kb=(double)st.st_size/1024;
            sprintf(info,"%.01f KB | %dx%d px",size_kb,getXResolution(path),getYResolution(path));
        }
        return strdup(info);
    }
    return strdup("---");  
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
    // Dacă fereastra abia s-a deschis (BFFM_INITIALIZED)
    if (uMsg == BFFM_INITIALIZED) {
        if (lpData) {
            // Îi spunem ferestrei să sară la calea trimisă prin lpData
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        }
    }
    return 0;
}


void openImage(char* path){
    char command[512];
    //Make path correct
    char fixed_path[512];
    strncpy(fixed_path, path, sizeof(fixed_path) - 1);
    fixed_path[sizeof(fixed_path) - 1] = '\0';

   for(int i = 0; fixed_path[i] != '\0'; i++){
        if(fixed_path[i] == '/') {
            fixed_path[i] = '\\';
        }
    }
    sprintf(command, "start \"\" \"%s\"", fixed_path);
    //printf("%s", command);
    LOG_DEBUG("Try to open image: %s, with command: %s", fixed_path,command);
    system(command);
    
}

void createAlbum(char* path){
    if(path==NULL) return;
    char folder_name[150];
    char full_path_w_folder[512];

    clear_screen();
    printf("===CREATE NEW FOLDER===\n");
    printf("Enter folder name:");
    scanf("%s",folder_name);
    /*
    strcpy(full_path_w_folder,path);
    strcat(full_path_w_folder,"\\");
    strcat(full_path_w_folder,folder_name);
    */
    snprintf(full_path_w_folder, sizeof(full_path_w_folder), "%s/%s", path, folder_name);
    if(_mkdir(full_path_w_folder) ==0){
        printf("Album created with name %s", folder_name);
        LOG_DEBUG("Album created with path %s",full_path_w_folder);
    }
    else{
        LOG_DEBUG("error creating album");
    }
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
    m->path=strdup(path);
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
            o->type=OPTION_NORMAL;
        }
        else{
            o->submenu=createPhotoMenu(full_path,ent->d_name);
            o->action=NULL;
            o->type=OPTION_NORMAL;
        }
        m->n++;
    }
    
    m->options = (Option*)realloc(m->options,(m->n+1)*sizeof(Option));
    m->options[m->n].name= strdup("[NEW ALBUM]");
    m->options[m->n].details=strdup("");
    m->options[m->n].submenu=NULL;
    m->options[m->n].action=NULL;
    m->options[m->n].path=NULL;
    m->options[m->n].type=OPTION_NEW_ALBUM;
    m->n++;
    
    m->options = (Option*)realloc(m->options,(m->n+1)*sizeof(Option));
    m->options[m->n].name= strdup("[ADD IMAGE]");
    m->options[m->n].details=strdup("");
    m->options[m->n].submenu=NULL;
    m->options[m->n].action=NULL;
    m->options[m->n].path=NULL;
    m->options[m->n].type=OPTION_ADD_IMAGE;
    m->n++;

    closedir(dir);
    LOG_DEBUG("Meniu '%s' created with %d options", name, m->n);
    return m;
}

char* openFileDialog(){
    OPENFILENAME ofn;
    char szFile[260];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Images\0*.png;*.jpg;*.jpeg;\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if(GetOpenFileName(&ofn) == TRUE){
        LOG_DEBUG("File selected: %s", ofn.lpstrFile);
        return strdup(ofn.lpstrFile);
    }
    LOG_DEBUG("No file selected or error occurred");
    return NULL;
}

char* openFolderDialog(){
    static char path[MAX_PATH];
    BROWSEINFO bi = {0};
    bi.lpszTitle = "Select Foler";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)base_path;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

    if (pidl != 0) {
        if (SHGetPathFromIDList(pidl, path)) {
            IMalloc * imalloc = 0;
            if (SUCCEEDED(SHGetMalloc(&imalloc))) {
                imalloc->lpVtbl->Free(imalloc, pidl);
                imalloc->lpVtbl->Release(imalloc);
            }
            return strdup(path);
        }
    }
    LOG_DEBUG("No folder selected or error occurred");
    return NULL;
}



void addImageToAlbum(char* path){
    clear_screen();
    printf("===ADD IMAGE TO ALBUM===\n");
    printf("The selecting window is opening...\n");
    char* image_path = openFileDialog();

    if(image_path==NULL){
        LOG_DEBUG("No image selected by user");
        printf("No image selected");
    }else{
        char command[1024];
        sprintf(command, "copy \"%s\" \"%s\"", image_path, path);
        LOG_DEBUG("Copying image with command: %s", command);
        int ok = system(command);
        if(ok != 0){
            LOG_DEBUG("Error occurred while copying image");
        }
        else{
            LOG_DEBUG("Image copied successfully to album");
        }
        free(image_path);
    }
    pause_screen();

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

Menu* createPhotoMenu(char* path, char* name){
    Menu *m =(Menu*)malloc(sizeof(Menu));
    m->title= strdup(name);
    m->options=NULL;
    m->path=strdup(path);
    m->n=0;
    typedef struct { char *name; OptionType type; } PhotoOp;
    PhotoOp op_info[] = {
        {"[OPEN IMAGE]",   OPTION_OPEN_IMAGE},
        {"[RENAME IMAGE]", OPTION_RENAME_IMAGE},
        {"[DELETE IMAGE]", OPTION_DELETE_IMAGE},
        {"[MOVE IMAGE]",   OPTION_MOVE_IMAGE},
    };
    m->options = (Option*)realloc(m->options,(m->n+4)*sizeof(Option));
    for(int i=0;i<4;i++){
        m->options[i].name=strdup(op_info[i].name);
        m->options[i].details=strdup("");
        m->options[i].submenu=NULL;
        m->options[i].action=NULL;
        m->options[i].path=strdup(path);
        m->options[i].type=op_info[i].type;
    }
    m->n=4;
    return m;
}

void renameImage(char* path){
    char new_name[256], new_path[512];
    clear_screen();
    printf("===RENAME IMAGE===\n");
    printf("Enter new name (without extension): ");
    scanf("%s", new_name);
    strcpy(new_path, path);
    char *last_bslash = strrchr(new_path, '\\');
    char *last_fslash = strrchr(new_path, '/');
    char *last_sep = (last_bslash > last_fslash) ? last_bslash : last_fslash;
    if (last_sep) {
        strcpy(last_sep + 1, new_name);
        char *dot = strrchr(path, '.');
        if (dot) {
            strcat(new_path, dot);
        }
    }
    if(rename(path,new_path) == 0){
        LOG_DEBUG("Image renamed successfully to %s", new_path);
        printf("Image renamed successfully");
    }
    else{
        LOG_DEBUG("Error renaming image: %s", strerror(errno));
        printf("Error renaming image");
    }
}

void deleteImage(char* path){
    clear_screen();
    printf("===DELETE IMAGE===\n");
    printf("Are you sure you want to delete this image? (y/n): ");
    char confirm;
    scanf(" %c", &confirm);
    if (confirm == 'y' || confirm == 'Y') {
        if (remove(path) == 0) {
            LOG_DEBUG("Image deleted successfully: %s", path);
            printf("Image deleted successfully");
        } else {
            LOG_DEBUG("Error deleting image: %s", strerror(errno));
            printf("Error deleting image");
        }
    } else {
        LOG_DEBUG("Image deletion cancelled by user");
        printf("Image deletion cancelled");
    }
}
void moveImage(char* path){

    char fixed_path[512];
    strncpy(fixed_path, path, sizeof(fixed_path) - 1);
    fixed_path[sizeof(fixed_path) - 1] = '\0';
    
    for (int i = 0; fixed_path[i] != '\0'; i++) {
        if (fixed_path[i] == '/') {
            fixed_path[i] = '\\';
        }
    }

    clear_screen();
    printf("===MOVE IMAGE===\n");
    printf("The selecting window is opening...\n");
    char* new_location = openFolderDialog();

    if(new_location==NULL){
        LOG_DEBUG("No destination selected by user for moving image");
        printf("No destination selected");
    }else{
        char command[1024];
        
        sprintf(command, "move \"%s\" \"%s\"", fixed_path, new_location);
        LOG_DEBUG("Moving image with command: %s", command);
        
        int ok = system(command);
        if(ok == 0){
            LOG_DEBUG("Image moved successfully to %s", new_location);
            printf("Image moved successfully\n");
        }
        else{
            LOG_DEBUG("Error occurred while moving image");
            printf("Error moving image. Asigura-te ca imaginea nu este deja deschisa.\n");
        }
        free(new_location);
    }
    pause_screen();
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