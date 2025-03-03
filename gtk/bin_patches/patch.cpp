#include <stdio.h>
#include <string.h>

#include "../include/patch.h"

static void CKACKME_Patch(FILE* file);

int binaryPatch(FILE* file ,const char* file_path){
    if (strcmp(file_path, "to_crack/CKACKME.COM") == 0){
        CKACKME_Patch(file);

        return SUCCESS;
    }

    return NO_CRACK_TO_THIS_FILE;
}

static void CKACKME_Patch(FILE* file){
    int offset = 0x4c;
    int password_address = 0x26;

    fseek(file, offset, SEEK_SET);
    fputc(password_address, file);

    fclose(file);
}
