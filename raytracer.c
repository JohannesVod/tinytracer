#include <stdio.h>
#include <stdlib.h>
#include <png.h>

void write_png_file(char *filename, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Unable to open file %s for writing.\n", filename);
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Error: Unable to create PNG write structure.\n");
        fclose(fp);
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Error: Unable to create PNG info structure.\n");
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error: Error during PNG creation.\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    png_bytep row = (png_bytep)malloc(3 * width * sizeof(png_byte));
    if (!row) {
        fprintf(stderr, "Error: Unable to allocate memory for PNG row.\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            row[x * 3] = (unsigned char)((y + x) * 255 / (height + width)); // Blue
            row[x * 3 + 1] = (unsigned char)(x * 255 / width);               // Green
            row[x * 3 + 2] = (unsigned char)(y * 255 / height);              // Red
        }
        png_write_row(png_ptr, row);
    }

    png_write_end(png_ptr, NULL);

    free(row);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

int main() {
    char *filename = "output.png";
    int width = 800;
    int height = 600;

    write_png_file(filename, width, height);

    printf("Image created successfully: %s\n", filename);

    return 0;
}
