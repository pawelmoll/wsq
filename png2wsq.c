#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <png.h>

#include "wsq.h"

int debug;

static int read_grey_8bit_png(FILE *fl, void **image, int *width, int *height)
{
	unsigned char sig[8];
	png_structp png;
	png_infop info;
	png_uint_32 w, h;
	int bit_depth, color_type;
	int interlace_method, compression_method, filter_method;
	png_bytep *rows;
	int y;

	fread(sig, 1, 8, fl);
	if (!png_check_sig(sig, 8))
		return -__LINE__;

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
		return -__LINE__;

	info = png_create_info_struct(png);
	if (!info) {
		png_destroy_read_struct(&png, NULL, NULL);
		return -__LINE__;
	}

	png_init_io(png, fl);
	png_set_sig_bytes(png, 8);
	png_read_info(png, info);

	png_get_IHDR(png, info, &w, &h, &bit_depth, &color_type,
			&interlace_method, &compression_method, &filter_method);
	if (bit_depth != 8 || color_type != PNG_COLOR_TYPE_GRAY ||
			interlace_method != PNG_INTERLACE_NONE ||
			compression_method != PNG_COMPRESSION_TYPE_BASE ||
			filter_method != PNG_FILTER_TYPE_BASE) {
		png_destroy_read_struct(&png, NULL, NULL);
		return -__LINE__;
	}
	*width = w;
	*height = h;

	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, NULL, NULL);
		return -__LINE__;
	}

	rows = malloc(sizeof(png_bytep) * h);
	if (!rows) {
		png_destroy_read_struct(&png, NULL, NULL);
		return -__LINE__;
	}
	for (y = 0; y < h; y++)
		rows[y] = malloc(png_get_rowbytes(png, info));

	png_read_image(png, rows);

	png_destroy_read_struct(&png, NULL, NULL);

	*image = malloc(w * h);
	for (y = 0; y < h; y++)
		memcpy(((char *)(*image)) + (w * y), rows[y], w);

	for (y = 0; y < h; y++)
		free(rows[y]);
	free(rows);

	return 0;
}

static void usage(const char *comm)
{
	fprintf(stderr, "Usage: %s [-h] [-b BITRATE] <in.png> <out.wsq>\n", comm);
	fprintf(stderr, "where:\n");
	fprintf(stderr, "\t-h\tusage syntax (this message)\n");
	fprintf(stderr, "\t-b\tcompression bitrate (0.75 by default)\n");
	fprintf(stderr, "\t\tuse - instead of file name to use stdin/stdout\n");
}

int main(int argc, char *argv[])
{
	int opt;
	FILE *in = NULL;
	FILE *out = NULL;
	int err;
	int width, height;
	float bitrate = 0.75;
	unsigned char *image;
	unsigned char *wsq;
	int size;

	while ((opt = getopt(argc, argv, "hb:")) != -1) {
		char *t;
		switch (opt) {
		case 'b':
			bitrate = strtof(optarg, &t);
			if (bitrate <= 0.0 || bitrate > 1.0 || *t) {
				fprintf(stderr, "Invalid bitrate '%s'\n",
						optarg);
				return 1;
			}
			break;
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

	err = read_grey_8bit_png(in, (void **)&image, &width, &height);
	if (err) {
		fprintf(stderr, "Failed to read PNG image! (%d)\n", err);
		return 1;
	}

	err = wsq_encode_mem(&wsq, &size, bitrate,
			image, width, height, 8, -1, NULL);
	if (err) {
		fprintf(stderr, "Failed to compress image! (%d)\n", err);
		return 1;
	}
	if (fwrite(wsq, size, 1, out) != 1) {
		perror("fwrite");
		return -1;
	}

	if (in != stdin)
		fclose(in);

	if (out != stdout)
		fclose(out);

	free(image);

	return 0;
}
