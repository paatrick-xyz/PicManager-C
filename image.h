#ifndef IMAGE_H
#define IMAGE_H

int    getXResolution(char *path);
int    getYResolution(char *path);
char  *getFileInfo(char *path);     // returns heap string — caller must free()

void   openImage(char *path);
void   renameImage(char *path);
void   deleteImage(char *path);
void   moveImage(char *path);

#endif