#include <stdlib.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int
main(void)
{
    int x,y,n;
    uint8_t *data = stbi_load("data/screenshot.png", &x, &y, &n, 0);
    if (!data) {
        printf("Error: %s", stbi_failure_reason());
        exit(1);
    } else {
        printf("Loaded file: %i x %i, %i color channels\n", x, y, n);
    }

    // lets compress it to 3x3 because I know that's right for this image, later we can try to
    // figure out what to compress it too (but we need to crop to the right window first.)
    uint8_t *compressed_data = calloc(1, x/3 * y/3 * n);
    
    for (int pixel_y = 0; pixel_y < y/3; pixel_y++) {
        for (int pixel_x = 0; pixel_x < x/3; pixel_x++) {
            int data_index = pixel_y * 3 * (n*x) + pixel_x * 3 * n;
            int compressed_index = pixel_y * (n*x/3) + pixel_x * n;
            compressed_data[compressed_index] = data[data_index];
            compressed_data[compressed_index+1] = data[data_index+1];
            compressed_data[compressed_index+2] = data[data_index+2];
            compressed_data[compressed_index+3] = data[data_index+3];
        }
    }
    
    stbi_write_png("compressed.png", x/3, y/3, n, compressed_data, 0);
    printf("Wrote to file\n");

    stbi_image_free(data);
    free(compressed_data);
}
