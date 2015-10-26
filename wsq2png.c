#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <png.h>

#include "wsq.h"

int debug;

static int write_grey_8bit_png(FILE *fl, void *image, int width, int height)
{
	unsigned char *pixels = image;
	png_structp png;
	png_infop info;
	int row;

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
			NULL, NULL, NULL);
	if (!png)
		return -__LINE__;

	info = png_create_info_struct(png);
	if (!info)
		return -__LINE__;

	if (setjmp(png_jmpbuf(png))) {
		png_destroy_write_struct(&png, &info);
		return -__LINE__;
	}

	png_init_io(png, fl);

	png_set_IHDR(png, info, width, height,
			8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png, info);

	for (row = 0; row < height; row++)
		png_write_row(png, pixels + (width * row));

	png_write_end(png, NULL);

	png_destroy_write_struct(&png, &info);

	return 0;
}

static void usage(const char *comm)
{
	fprintf(stderr, "Usage: %s [-h] <in.wsq> <out.png>\n", comm);
	fprintf(stderr, "where:\n");
	fprintf(stderr, "\t-h\tusage syntax (this message)\n");
	fprintf(stderr, "\t\tuse - instead of file name to use stdin/stdout\n");
}

int main(int argc, char *argv[])
{
	int opt;
	FILE *in = NULL;
	FILE *out = NULL;
	int err;
	int width, height, depth, ppi, lossy;
	unsigned char *image;

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (argc - optind != 2) {
		usage(argv[0]);
		return 1;
	}

	if (strcmp(argv[optind], "-") == 0)
		in = stdin;
	else
		in = fopen(argv[optind], "rb");
	if (!in) {
		perror(argv[optind]);
		return 1;
	}

	if (strcmp(argv[optind + 1], "-") == 0)
		out = stdout;
	else
		out = fopen(argv[optind + 1], "wb");
	if (!out) {
		perror(argv[optind + 1]);
		return 1;
	}

	err = wsq_decode_file(&image, &width, &height, &depth, &ppi,
			&lossy, in);
	if (err) {
		fprintf(stderr, "Failed to compress image! (%d)\n", err);
		return 1;
	}
	if (depth != 8)
		fprintf(stderr, "Warning: expected 8-bit image, got %d...\n",
				depth);

	err = write_grey_8bit_png(out, image, width, height);
	if (err) {
		fprintf(stderr, "Failed to read PNG image! (%d)\n", err);
		return 1;
	}

	if (in != stdin)
		fclose(in);

	if (out != stdout)
		fclose(out);

	free(image);

	return 0;
}
