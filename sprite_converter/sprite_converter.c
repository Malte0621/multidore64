#include <stdio.h>
#include <stdlib.h>

#define JPEG_MARKER_SOI 0xD8  // Start of image
#define JPEG_MARKER_SOF0 0xC0 // Start of frame (baseline DCT)
#define JPEG_MARKER_DHT 0xC4  // Define Huffman table(s)
#define JPEG_MARKER_SOS 0xDA  // Start of scan
#define JPEG_MARKER_EOI 0xD9  // End of image

int ntohs(int n)
{
    return (n >> 8) | (n << 8);
}

unsigned char colorlib_getpixel(unsigned char r, unsigned char g, unsigned char b)
{
    // Round the RGB values to the nearest color
    unsigned char color = 0;
    // use the color with the smallest difference
    unsigned char smallestDifference = 255;
    // black
    unsigned char difference = abs(r - 0) + abs(g - 0) + abs(b - 0);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x00;
    }
    // white
    difference = abs(r - 255) + abs(g - 255) + abs(b - 255);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x01;
    }
    // red
    difference = abs(r - 136) + abs(g - 0) + abs(b - 0);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x02;
    }
    // cyan
    difference = abs(r - 170) + abs(g - 255) + abs(b - 238);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x03;
    }
    // purple
    difference = abs(r - 204) + abs(g - 68) + abs(b - 204);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x04;
    }
    // green
    difference = abs(r - 0) + abs(g - 204) + abs(b - 85);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x05;
    }
    // blue
    difference = abs(r - 0) + abs(g - 0) + abs(b - 170);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x06;
    }
    // yellow
    difference = abs(r - 238) + abs(g - 238) + abs(b - 119);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x07;
    }
    // orange
    difference = abs(r - 221) + abs(g - 136) + abs(b - 85);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x08;
    }
    // brown
    difference = abs(r - 102) + abs(g - 68) + abs(b - 0);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x09;
    }
    // light red
    difference = abs(r - 255) + abs(g - 119) + abs(b - 119);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x0A;
    }
    // dark grey
    difference = abs(r - 51) + abs(g - 51) + abs(b - 51);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x0B;
    }
    // grey
    difference = abs(r - 119) + abs(g - 119) + abs(b - 119);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x0C;
    }
    // light green
    difference = abs(r - 170) + abs(g - 255) + abs(b - 102);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x0D;
    }
    // light blue
    difference = abs(r - 0) + abs(g - 136) + abs(b - 255);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x0E;
    }
    // light grey
    difference = abs(r - 187) + abs(g - 187) + abs(b - 187);
    if (difference < smallestDifference)
    {
        smallestDifference = difference;
        color = 0x0F;
    }

    return color;
}

int main(int argc, char **argv)
{
    // Check that a filename was provided
    if (argc != 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Open the file
    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
    {
        printf("Error opening file %s\n", argv[1]);
        return 1;
    }

    // Read the BMP file header
    unsigned char header[54];
    fread(header, 1, 54, fp);

    // Extract the image width and height from the header
    int width = *(int *)(header + 18);
    int height = *(int *)(header + 22);

    // Extract the pixel offset from the header
    int pixel_offset = *(int *)(header + 10);

    // Extract the image format from the header
    int bpp = *(short *)(header + 28);
    if (bpp != 24 && bpp != 4)
    {
        printf("Error: Unsupported BMP format\n");
        fclose(fp);
        return 1;
    }

    // Create the sprite
    unsigned char *sprite = malloc(width * height);

    if (bpp == 24)
    {
        // Extract the pixel data from the file
        int row_size = (bpp * width + 31) / 32 * 4; // Row size in bytes (rounded up to a multiple of 4)
        int padding = row_size - bpp * width / 8;   // Number of padding bytes per row
        unsigned char *pixels = (unsigned char *)malloc(row_size * height);
        fseek(fp, pixel_offset, SEEK_SET);
        fread(pixels, 1, row_size * height, fp);

        // Create a new buffer for the flipped image
        unsigned char *flipped_pixels = (unsigned char *)malloc(row_size * height);

        // Flip the image by copying the rows in reverse order
        for (int y = 0; y < height; y++) {
            memcpy(flipped_pixels + (height - y - 1) * row_size, pixels + y * row_size, row_size);
        }

        // Loop through the sprite
        unsigned int spriteIndex = 0;
        for (unsigned int y = 0; y < height; y++)
        {
            for (unsigned int x = 0; x < width; x++)
            {
                int offset = y * row_size + x * 3;
                int r = flipped_pixels[offset + 2];
                int g = flipped_pixels[offset + 1];
                int b = flipped_pixels[offset];

                unsigned char color = colorlib_getpixel(r, g, b);
                // Set the color of the pixel
                sprite[spriteIndex] = color;
                // Increase the sprite index
                spriteIndex++;
            }
        }

        // Free the jpg file data
        free(pixels);
        free(flipped_pixels);
    }
    else
    {
        // Extract the pixel data from the file
        // Read the color palette from the file
        int palette_size = *(int *)(header + 46);
        unsigned char *palette = (unsigned char *)malloc(palette_size * 4);
        fseek(fp, 14 + palette_size, SEEK_SET);
        fread(palette, 1, palette_size * 4, fp);

        // Extract the pixel data from the file
        int row_size = (bpp * width + 31) / 32 * 4; // Row size in bytes (rounded up to a multiple of 4)
        int padding = row_size - bpp * width / 8;   // Number of padding bytes per row
        unsigned char *pixels = (unsigned char *)malloc(row_size * height);
        fseek(fp, pixel_offset, SEEK_SET);
        fread(pixels, 1, row_size * height, fp);

        // Create a new buffer for the flipped image
        unsigned char *flipped_pixels = (unsigned char *)malloc(row_size * height);

        // Flip the image by copying the rows in reverse order
        for (int y = 0; y < height; y++) {
            memcpy(flipped_pixels + (height - y - 1) * row_size, pixels + y * row_size, row_size);
        }

        // Decode the pixel data
        unsigned char *rgb_pixels = (unsigned char *)malloc(width * height * 3);
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                // Extract the 4-bit pixel value
                int pixel = flipped_pixels[y * row_size + x / 2];
                if (x % 2 == 0)
                {
                    pixel = pixel >> 4;
                }
                else
                {
                    pixel = pixel & 0x0F;
                }

                // Look up the RGB values in the color palette
                int offset = pixel * 4;
                rgb_pixels[y * width * 3 + x * 3 + 0] = palette[offset + 2];
                rgb_pixels[y * width * 3 + x * 3 + 1] = palette[offset + 1];
                rgb_pixels[y * width * 3 + x * 3 + 2] = palette[offset + 0];
            }
        }

        // Loop through the sprite
        unsigned int spriteIndex = 0;
        for (unsigned int y = 0; y < height; y++)
        {
            for (unsigned int x = 0; x < width; x++)
            {
                int offset = y * width * 3 + x * 3;
                int r = rgb_pixels[offset + 0];
                int g = rgb_pixels[offset + 1];
                int b = rgb_pixels[offset + 2];

                unsigned char color = colorlib_getpixel(r, g, b);
                // Set the color of the pixel
                sprite[spriteIndex] = color;
                // Increase the sprite index
                spriteIndex++;
            }
        }

        // Free the jpg file data
        free(pixels);
        free(rgb_pixels);
        free(flipped_pixels);
    }

    // Close the file
    fclose(fp);

    // print the sprite code
    printf("unsigned char sprite[] = {");
    for (unsigned int i = 0; i < width * height; i++)
    {
        printf("%d", sprite[i]);
        if (i != width * height - 1)
        {
            printf(", ");
        }
    }
    printf("};\n");

    // print the function
    printf("renderlib_drawsprite(0, 0, %d, %d, sprite);\n", width, height);

    // Free the image data
    free(sprite);

    system("pause");

    return EXIT_SUCCESS;
}