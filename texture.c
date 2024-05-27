#include "texture.h"
#include <stdio.h>
#include <stdlib.h>

unsigned char* lade_textur(char name[], unsigned int pixel) {
    //lese (r) die Datei in Binär (b)
    FILE* textur = fopen(name, "rb");
    if (!textur) {
        perror("keine datei :(");
        return NULL;
    }

    /*Die Texturen sind alle pixel*pixel Pixel groß. Ein Pixel hat 4 byte(RGBA). Also weise ich dem pointer diese größe zu.*/
    unsigned char *buffer = malloc(pixel * 4);
    // for (int i = 0; fread(&byte, sizeof(unsigned char), 1, textur) == 1; i++) {
    //     buffer[i] = byte;
    // }
    fread(buffer, pixel * 4, 1, textur);

    // //pixel in hex ausgeben
    // for (int i = 0; i < pixel * pixel * 4; ++i) {
    //     printf("%02X ", buffer[i]);
    // }
    // printf("\n");

    // Close the file
    fclose(textur);

    return buffer;
}