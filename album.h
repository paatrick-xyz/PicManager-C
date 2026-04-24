#ifndef ALBUM_H
#define ALBUM_H

typedef struct Menu Menu;

char  *openFileDialog();
char  *openFolderDialog();

void   createAlbum(char *path);
void   addImageToAlbum(char *path);

Menu  *createAlbumMenu(char *path, char *name);
Menu  *createPhotoMenu(char *path, char *name);

#endif