#include <stdio.h>
#include <string.h>

#include "../include/patch.h"

static void CKACKME_Patch();

void binaryPatch(const char* file_path){
    if (strcmp(file_path, "to_crack/CKACKME.COM") == 0){
        CKACKME_Patch();
    }
}

static void CKACKME_Patch(){
    FILE* file = fopen("to_crack/CKACKME.COM", "r+b");
    int offset = 0x4c;
    int password_address = 0x26;

    fseek(file, offset, SEEK_SET);
    fputc(password_address, file);

    fclose(file);
}
