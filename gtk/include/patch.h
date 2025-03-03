#ifndef PATCH_H
#define PATCH_H

int binaryPatch(FILE* file, const char* filename);

enum PATCH_RETURN{
    SUCCESS                 = 0,
    NO_CRACK_TO_THIS_FILE   = 1,
};

#endif
