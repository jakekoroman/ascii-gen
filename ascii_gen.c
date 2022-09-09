#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>
#include <stdbool.h>

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "libs/stb_image_resize.h"

#define BUF_CAP 255

#define internal static

typedef uint8_t uint8;

#define SET_LONG "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "
#define SET_SIZE_LONG 70
#define SET_SHORT "@%#*+=-:. "
#define SET_SIZE_SHORT 10

typedef enum {
	SET_TYPE_SHORT,
	SET_TYPE_LONG,
	SET_TYPE_CUSTOM,
} SetType;

typedef struct {
	char in_file[BUF_CAP];
	char out_file[BUF_CAP];
	char set[BUF_CAP];
	SetType set_type; // defaults to short
	int set_size;
	int scale;
	bool debug;
} Options;

internal inline int
map(int x, int a, int b, int c, int d)
{
	return (x - a) * (d - c) / (b - a);
}

internal void
print_docs(const char* fname)
{
	printf("Usage: %s img [-o file] [-s scale] [-S set] [-hltv]\n", fname); 
	printf("  -h: Shows help\n");
	printf("  -o: Output to file, if no file is given it will output to stdout\n");
	printf("  -s: The scale of the output image, defaults to 1\n");
	printf("  -S: The character set that will be used to generate the output, defaults to the builtin 'short' character set\n");
	printf("  -l: Uses the builtin 'long' character set for generating the output\n");
	printf("  -t: Uses the builtin 'short' character set for generating the output\n");
	printf("  -v: Shows debug info\n");
}

// Parses args with getopt and fills an Options struct of all the options passed
// Resource: https://azrael.digipen.edu/~mmead/www/Courses/CS180/getopt.html
internal void
parse_args(int argc, char **argv, Options *options)
{
	int opt;
	options->scale = 1;
	
	while ((opt = getopt(argc, argv, ":o:hls:S:vc")) != -1)
	{
		switch (opt)
		{
			case 'o':
			{
				strncpy(options->out_file, optarg, BUF_CAP);
			} break; 

			case 'h':
			{
				print_docs(argv[0]);
				exit(0);
			} break;

			case 's':
			{
				options->scale = atoi(optarg);
				if (options->scale == 0) {
					fprintf(stderr, "Invalid scale %s\n", optarg);
					exit(69);
				}
			} break;	

			case 'v':
			{
				options->debug = true;
			} break;	

			case 'l':
			{
				options->set_type = SET_TYPE_LONG;
				options->set_size = SET_SIZE_LONG;
				strncpy(options->set, SET_LONG, BUF_CAP);
				break;
			} break;

			case 't':  // 't' for tiny I guess? wanted to keep 's' for scale
			{
				// Redundant because it will default to short but sometimes
				// it's nice to request it even though its default.
				options->set_type = SET_TYPE_SHORT;
				break;
			} break;

			case 'S':
			{
				int size = strlen(optarg);
				options->set_size = size > 255 ? BUF_CAP : size;
				options->set_type = SET_TYPE_CUSTOM;
				strncpy(options->set, optarg, BUF_CAP);
			} break;

			case '?':
			{
				printf("Unknown Option: %c\n", optopt);
				exit(69);
			} break;

			case ':':
			{
				printf("Missing arg for %c\n", optopt);
				exit(69);
			} break;

			default:
			{
				printf("Something broke\n");
				break;
			}
		}
	}

	if (options->set_type == SET_TYPE_SHORT) {
		options->set_size = SET_SIZE_SHORT;
		strncpy(options->set, SET_SHORT, BUF_CAP);
	}

	if (optind < argc) {
		strncpy(options->in_file, argv[optind], BUF_CAP);
	}

}

int
main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Basic Usage: %s img [-o file]\n", argv[0]);
		exit(69);
	}

	Options options = {0};
	parse_args(argc, argv, &options);

	if (options.debug) {
		printf("In File  : %s\n", options.in_file);
		printf("Out File : %s\n", options.out_file);
		printf("Scale    : %d\n", options.scale);
		printf("Set size : %d\n", options.set_size);
		printf("Set type : %d\n", options.set_type);
		printf("Set      : %s\n", options.set);
	}

	// TODO: Maybe bundle this stuff up? including the resized stuff?
	int width, height;
	int channels = 1;
	uint8 *image_bytes = stbi_load(options.in_file, &width, &height, NULL, channels);

	if (!image_bytes) {
		fprintf(stderr, "Couldn't Open file %s\n", options.in_file);
		exit(69);
	}

	int nw = width / options.scale;
	int nh = height / options.scale;
	bool resized = false;
	uint8 *resized_bytes;

	if (options.scale > 1) {
		int stride = 0;
		resized = true;
		resized_bytes = malloc(sizeof(uint8) * (nw * nh));
		if (!stbir_resize_uint8(image_bytes, width, height, stride,
								resized_bytes, nw, nh, stride, channels)) // only resizes if the scale is greater than 1
		{
			printf("Couldn't Resize Image\n");
			exit(69);
		}
	} else {
		resized_bytes = image_bytes;
	}

	FILE *f;
	if (strncmp(options.out_file, "", BUF_CAP)) {
		f = fopen(options.out_file, "w");
	} else {
		f = stdout;
	}

	for (size_t i = 0;
		 i < nw * nh;
		 ++i)
	{
		int map_result = map(resized_bytes[i], 0, 255, 0, options.set_size - 1);
		int idx = (options.set_size -  1) - map_result;

		if (i % nw == 0) {
			fprintf(f, "\n");
		}

		fprintf(f, "%c", options.set[idx]);
	}

    fprintf(f, "\n");

	if (!(f == NULL || f == stdout)) {
		fclose(f);
	}
	
	if (image_bytes) {
		stbi_image_free(image_bytes);
    }

	if (resized_bytes && resized) {
		stbi_image_free(resized_bytes);
	}

	return 0;
}
