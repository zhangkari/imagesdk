/***************************
 * file name:   imgsdk.c
 * description:
 * author:      kari.zhang
 * date:        2015-05-09
 *
 ***************************/

#include <png.h>
#include <stdio.h>
//#include <zlib.h>
#include "imgsdk.h"

int main(int argc, char **argv) {
    if (2 != argc) {
        printf("Usage:\n");
        printf("  %s path\n", argv[0]);
        return -1;
    }

    return read_png(argv[1]);
}

int read_png(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (NULL == fp) {
        return FILE_NOT_EXIST;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 
            0, 0, 0);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    setjmp(png_jmpbuf(png_ptr));
    png_init_io(png_ptr, fp);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
    int w = png_get_image_width(png_ptr, info_ptr);
    int h = png_get_image_height(png_ptr,info_ptr);
    int type = png_get_color_type(png_ptr, info_ptr);
    png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);

    int bpp = 0;
    switch (type) { 
        case PNG_COLOR_TYPE_RGB:
            bpp = 3;
            break;

        case PNG_COLOR_TYPE_RGB_ALPHA:
            bpp = 4;
            break;
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    fclose(fp);

    return OK;
}
