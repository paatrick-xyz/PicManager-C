#include "album.h"
#include "config.h"
#include "image.h"
#include "menu.h"   // Menu, Option, OptionType, clear_screen(), pause_screen()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <direct.h>
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>

//  Windows dialog helpers 

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg,
                                        LPARAM lParam, LPARAM lpData) {
    if (uMsg == BFFM_INITIALIZED && lpData) {
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}

char *openFileDialog() {
    OPENFILENAME ofn;
    char szFile[260];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = NULL;
    ofn.lpstrFile    = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile     = sizeof(szFile);
    ofn.lpstrFilter  = "Images\0*.png;*.jpg;*.jpeg\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileName(&ofn) == TRUE) {
        LOG_DEBUG("File selected: %s", ofn.lpstrFile);
        return strdup(ofn.lpstrFile);
    }
    LOG_DEBUG("No file selected");
    return NULL;
}

char *openFolderDialog() {
    static char path[MAX_PATH];
    BROWSEINFO bi  = {0};
    bi.lpszTitle   = "Select Folder";
    bi.ulFlags     = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn        = BrowseCallbackProc;
    bi.lParam      = (LPARAM)base_path;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl) {
        if (SHGetPathFromIDList(pidl, path)) {
            IMalloc *imalloc = NULL;
            if (SUCCEEDED(SHGetMalloc(&imalloc))) {
                imalloc->lpVtbl->Free(imalloc, pidl);
                imalloc->lpVtbl->Release(imalloc);
            }
            return strdup(path);
        }
    }
    LOG_DEBUG("No folder selected");
    return NULL;
}

//  Album operations 

void createAlbum(char *path) {
    if (path == NULL) return;

    char folder_name[150], full_path[512];
    clear_screen();
    printf("===CREATE NEW FOLDER===\n");
    printf("Enter folder name: ");
    scanf("%s", folder_name);

    snprintf(full_path, sizeof(full_path), "%s/%s", path, folder_name);
    if (_mkdir(full_path) == 0) {
        LOG_DEBUG("Album created: %s", full_path);
        printf("Album '%s' created successfully.", folder_name);
    } else {
        LOG_DEBUG("Error creating album: %s", full_path);
        printf("Error creating album.");
    }
    pause_screen();
}

void addImageToAlbum(char *path) {
    clear_screen();
    printf("===ADD IMAGE TO ALBUM===\n");
    printf("The file selection window is opening...\n");

    char *image_path = openFileDialog();
    if (image_path == NULL) {
        LOG_DEBUG("No image selected");
        printf("No image selected.");
    } else {
        char command[1024];
        snprintf(command, sizeof(command), "copy \"%s\" \"%s\"", image_path, path);
        LOG_DEBUG("Copying image: %s", command);

        if (system(command) == 0) {
            LOG_DEBUG("Image copied successfully");
            printf("Image added to album.");
        } else {
            LOG_DEBUG("Error copying image");
            printf("Error adding image.");
        }
        free(image_path);
    }
    pause_screen();
}

//  Menu builders 

Menu *createPhotoMenu(char *path, char *name) {
    Menu *m   = (Menu *)malloc(sizeof(Menu));
    m->title  = strdup(name);
    m->path   = strdup(path);
    m->n      = 0;
    m->options = NULL;

    typedef struct { char *name; OptionType type; } PhotoOp;
    PhotoOp ops[] = {
        {"[OPEN IMAGE]",   OPTION_OPEN_IMAGE},
        {"[RENAME IMAGE]", OPTION_RENAME_IMAGE},
        {"[DELETE IMAGE]", OPTION_DELETE_IMAGE},
        {"[MOVE IMAGE]",   OPTION_MOVE_IMAGE},
    };
    int count = sizeof(ops) / sizeof(ops[0]);

    m->options = (Option *)malloc(count * sizeof(Option));
    for (int i = 0; i < count; i++) {
        m->options[i].name    = strdup(ops[i].name);
        m->options[i].details = strdup("");
        m->options[i].path    = strdup(path);
        m->options[i].submenu = NULL;
        m->options[i].action  = NULL;
        m->options[i].type    = ops[i].type;
    }
    m->n = count;
    return m;
}

Menu *createAlbumMenu(char *path, char *name) {
    DIR *dir = opendir(path);
    if (!dir) {
        LOG_DEBUG("Couldn't open directory: %s", path);
        return NULL;
    }

    Menu *m   = (Menu *)malloc(sizeof(Menu));
    m->title  = strdup(name);
    m->path   = strdup(path);
    m->n      = 0;
    m->options = NULL;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') continue;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, ent->d_name);

        struct stat st;
        stat(full_path, &st);
        LOG_DEBUG("Found: %s (%s)", ent->d_name, S_ISDIR(st.st_mode) ? "DIR" : "FILE");

        m->options = (Option *)realloc(m->options, (m->n + 1) * sizeof(Option));
        Option *o  = &m->options[m->n];
        o->name    = strdup(ent->d_name);
        o->details = getFileInfo(full_path);
        o->path    = strdup(full_path);
        o->action  = NULL;
        o->type    = OPTION_NORMAL;
        o->submenu = S_ISDIR(st.st_mode)
                     ? createAlbumMenu(full_path, ent->d_name)
                     : createPhotoMenu(full_path, ent->d_name);
        m->n++;
    }
    closedir(dir);

    // Append the two special action entries
    typedef struct { char *name; OptionType type; } SpecialOp;
    SpecialOp specials[] = {
        {"[NEW ALBUM]", OPTION_NEW_ALBUM},
        {"[ADD IMAGE]", OPTION_ADD_IMAGE},
    };
    int count = sizeof(specials) / sizeof(specials[0]);

    m->options = (Option *)realloc(m->options, (m->n + count) * sizeof(Option));
    for (int i = 0; i < count; i++) {
        Option *o  = &m->options[m->n];
        o->name    = strdup(specials[i].name);
        o->details = strdup("");
        o->path    = NULL;
        o->submenu = NULL;
        o->action  = NULL;
        o->type    = specials[i].type;
        m->n++;
    }

    LOG_DEBUG("Menu '%s' built with %d options", name, m->n);
    return m;
}