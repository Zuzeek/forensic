// Copies a BMP file
#include <cs50.h>
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

// program resize.c scales bmp by a factor of 'n'
int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize n infile outfile\n");
        return 1;
    }

    // 'n' -> a user given number and scale factor
    int n = atoi(argv[1]);

    // 'n' must be a positive int <= 100
    if (n < 0 || n > 100)
    {
        printf("Usage: Resize value must be positive less than or equal to 100.");
        return 1;
    }

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf, bfOut;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);
    // set outfile BITMAPFILEHEADER for manipulation
    bfOut = bf;

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi, biOut;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);
    // set outfile BITMAPINFOHEADER for manipulation
    biOut = bi;

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // update header info for outfile resized by the value 'n'
    biOut.biWidth = bi.biWidth * n;
    biOut.biHeight = bi.biHeight * n;

    // determine padding for scanlines
    // infile
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    // outfile
    int outPadding = (4 - (biOut.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // update header info for outfile resized by the value 'n'
    biOut.biSizeImage = ((sizeof(RGBTRIPLE) * biOut.biWidth) + outPadding) * abs(biOut.biHeight);
    bfOut.bfSize = biOut.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // write those pixels into outfile's BITMAPFILEHEADER
    fwrite(&bfOut, sizeof(BITMAPFILEHEADER), 1, outptr);
    fwrite(&biOut, sizeof(BITMAPINFOHEADER), 1, outptr);

    // resize horizontally & vertically
    // iterate over infile's scanlines row
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // repeat 'n' times per infile's row
        for (int j = 0; j < n; j++)
        {
            // iterate over pixels in scanline
            for (int k = 0; k < bi.biWidth; k++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                // write RGB triple(pixels) to outfile 'n' times
                for (int l = 0; l < n; l++)
                {
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            }

            // write padding to outfile
            for (int m = 0; m < outPadding; m++)
            {
                fputc(0x00, outptr);
            }

            // resize vertically using re-copy method
            if (j < n - 1)
            {
                // send infile cursor back
                fseek(inptr, -bi.biWidth * sizeof(RGBTRIPLE), SEEK_CUR);
            }
        }

        // skip over infile's padding, if any
        fseek(inptr, padding, SEEK_CUR);
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
