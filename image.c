#include "image.h"
#include "config.h"
#include "album.h"  // openFolderDialog()
#include "menu.h"   // clear_screen(), pause_screen()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <windows.h>
#include <shlobj.h>

//  Resolution helpers 

int getXResolution(char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        LOG_DEBUG("Image couldn't be opened: %s", path);
        return 0;
    }
    unsigned char buf[32];
    if (fread(buf, 1, 32, f) < 24) { fclose(f); return 0; }

    int width = 0;
    if (buf[0] == 0x89 && buf[1] == 'P' && buf[2] == 'N' && buf[3] == 'G') {
        width = (buf[16] << 24) | (buf[17] << 16) | (buf[18] << 8) | buf[19];
    } else if (buf[0] == 0xFF && buf[1] == 0xD8) {
        int pos = 2;
        while (pos < 32) {
            if (buf[pos] == 0xFF && (buf[pos + 1] & 0xF0) == 0xC0) {
                width = (buf[pos + 4] << 8) | buf[pos + 5];
                break;
            }
            pos += 2 + ((buf[pos + 2] << 8) | buf[pos + 3]);
        }
    }
    fclose(f);
    LOG_DEBUG("Width:%d path:%s", width, path);
    return width;
}

int getYResolution(char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        LOG_DEBUG("Image couldn't be opened: %s", path);
        return 0;
    }
    unsigned char buf[32];
    if (fread(buf, 1, 32, f) < 24) { fclose(f); return 0; }

    int height = 0;
    if (buf[0] == 0x89 && buf[1] == 'P' && buf[2] == 'N' && buf[3] == 'G') {
        height = (buf[20] << 24) | (buf[21] << 16) | (buf[22] << 8) | buf[23];
    }
    fclose(f);
    LOG_DEBUG("Height:%d path:%s", height, path);
    return height;
}

typedef struct {
    int w;
    int h;
} ImageSize;

ImageSize getImageSize(char *path){
    ImageSize size = {0,0};
    FILE *f = fopen(path, "rb");
    if(!f){
        LOG_DEBUG("Image couldn't be opened: %s", path);
        return size;
    }
    unsigned char buf[64];
    if(fread(buf,1,64,f)<24){
        fclose(f);
        LOG_DEBUG("Error reading image: %s", path);
        return size;
    }
    if (buf[0] == 0x89 && buf[1] == 'P' && buf[2] == 'N' && buf[3] == 'G') {
        size.w = (buf[16] << 24) | (buf[17] << 16) | (buf[18] << 8) | buf[19];
        size.h = (buf[20] << 24) | (buf[21] << 16) | (buf[22] << 8) | buf[23];
    }
    else if (buf[0] == 0xFF && buf[1] == 0xD8) {
        int pos = 2;
        while (pos + 8 < 64) {
            while (buf[pos] == 0xFF && pos + 4 < 64) pos++;

            unsigned char marker = buf[pos + 1];
            int length = (buf[pos + 2] << 8) | buf[pos + 3];

            if (marker >= 0xC0 && marker <= 0xC2) {
                size.h = (buf[pos + 5] << 8) | buf[pos + 6];
                size.w = (buf[pos + 7] << 8) | buf[pos + 8];
                break;
            }
            
            // Move to the next marker
            pos += 2 + length;
        }
    }
    fclose(f);
    LOG_DEBUG("Image size: %dx%d path:%s", size.w, size.h, path);
    return size;
}


char *getFileInfo(char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return strdup("---");

    if (S_ISDIR(st.st_mode)) return strdup("<DIR>");

    ImageSize dim = getImageSize(path);

    char info[128];
    double size_kb = (double)st.st_size / 1024.0;
    snprintf(info, sizeof(info), "%.1f KB | %dx%d px",
             size_kb, dim.w, dim.h);
    return strdup(info);
}

//  Path helper 

static void fix_path_separators(char *path) {
    for (int i = 0; path[i] != '\0'; i++) {
        if (path[i] == '/') path[i] = '\\';
    }
}

//  Image operations 

void openImage(char *path) {
    char fixed_path[512];
    strncpy(fixed_path, path, sizeof(fixed_path) - 1);
    fixed_path[sizeof(fixed_path) - 1] = '\0';
    fix_path_separators(fixed_path);

    char command[512];
    snprintf(command, sizeof(command), "start \"\" \"%s\"", fixed_path);
    LOG_DEBUG("Opening image: %s", fixed_path);
    system(command);
}

void renameImage(char *path) {
    char new_name[256], new_path[512];
    clear_screen();
    printf("===RENAME IMAGE===\n");
    printf("Enter new name (without extension): ");
    scanf("%s", new_name);

    strcpy(new_path, path);
    char *last_bslash = strrchr(new_path, '\\');
    char *last_fslash = strrchr(new_path, '/');
    char *last_sep    = (last_bslash > last_fslash) ? last_bslash : last_fslash;

    if (last_sep) {
        strcpy(last_sep + 1, new_name);
        char *dot = strrchr(path, '.');
        if (dot) strcat(new_path, dot);
        // no dot = no extension — proceed as-is
    }

    if (rename(path, new_path) == 0) {
        LOG_DEBUG("Image renamed to %s", new_path);
        printf("Image renamed successfully.");
    } else {
        LOG_DEBUG("Error renaming image: %s", strerror(errno));
        printf("Error renaming image.");
    }
    pause_screen();
}

void deleteImage(char *path) {
    clear_screen();
    printf("===DELETE IMAGE===\n");
    printf("Are you sure you want to delete this image? (y/n): ");
    char confirm;
    scanf(" %c", &confirm);

    if (confirm == 'y' || confirm == 'Y') {
        if (remove(path) == 0) {
            LOG_DEBUG("Image deleted: %s", path);
            printf("Image deleted successfully.");
        } else {
            LOG_DEBUG("Error deleting image: %s", strerror(errno));
            printf("Error deleting image.");
        }
    } else {
        LOG_DEBUG("Image deletion cancelled");
        printf("Deletion cancelled.");
    }
    pause_screen();
}

void moveImage(char *path) {
    char fixed_path[512];
    strncpy(fixed_path, path, sizeof(fixed_path) - 1);
    fixed_path[sizeof(fixed_path) - 1] = '\0';
    fix_path_separators(fixed_path);

    clear_screen();
    printf("===MOVE IMAGE===\n");
    printf("The folder selection window is opening...\n");

    char *new_location = openFolderDialog();
    if (new_location == NULL) {
        LOG_DEBUG("No destination selected for move");
        printf("No destination selected.");
    } else {
        char command[1024];
        snprintf(command, sizeof(command), "move \"%s\" \"%s\"", fixed_path, new_location);
        LOG_DEBUG("Moving image: %s", command);

        if (system(command) == 0) {
            LOG_DEBUG("Image moved to %s", new_location);
            printf("Image moved successfully.");
        } else {
            LOG_DEBUG("Error moving image");
            printf("Error moving image. Make sure the image is not currently open.");
        }
        free(new_location);
    }
    pause_screen();
}