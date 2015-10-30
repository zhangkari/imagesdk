/***************************
 * file name:   imgsdk.c
 * description:
 * author:      kari.zhang
 * date:        2015-05-09
 *
 ***************************/

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <png.h>
#include "imgsdk.h"

#define TAG "ImgSdk"

int main(int argc, char **argv) {
	if (2 != argc) {
		printf("Usage:\n");
		printf("  %s path\n", argv[0]);
		return -1;
	}

	Bitmap_t img;
	if(read_png(argv[1], &img) < 0) {
		LogE(TAG, "Failed read png\n");
		return -1;
	}

	char *vertexSource = NULL;
	if (readFile("vertex.shdr", &vertexSource) < 0) {
		LogE(TAG, "Failed read vertext.shdr");
	}

	int buffers[1];
	glGenBuffers(1, buffers);
	if (!glIsBuffer(buffers[0])) {
		LogE(TAG, "Failed glGenBuffer at 0");
	}

	// copy pixels from CPU memory to GPU memory
	int size = img.form * img.width * img.height;
//	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffers[0]);
//	glBufferData(GL_PIXEL_UNPACK_BUFFER, size, img.base, GL_STREAM_DRAW);

	// execute shader
	int vertex_shader = executeVertexShader(vertexSource);

	// copy pixels from GPU memory to CPU memory
	glReadPixels(0, 0, img.width, img.height, GL_RGB, GL_UNSIGNED_BYTE, img.base);

	if (write_png("2.png", &img) < 0) {
		LogE(TAG, "Failed write png\n");
		return -1;
	}

	freeBitmap(&img);
	if (vertexSource != NULL) {
		free (vertexSource);
		vertexSource = NULL;
	}
}

int read_png(const char *path, Bitmap_t *mem)
{
	VALIDATE_NOT_NULL2(path, mem);
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
	int width = png_get_image_width(png_ptr, info_ptr);
	int height = png_get_image_height(png_ptr,info_ptr);
	int type = png_get_color_type(png_ptr, info_ptr);
	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);

	int bpp = 0;
	switch (type) { 
		case PNG_COLOR_TYPE_GRAY:
			bpp = 1;
			break;

		case PNG_COLOR_TYPE_RGB:
			bpp = 3;
			break;

		case PNG_COLOR_TYPE_RGB_ALPHA:
			bpp = 4;
			break;
	}

	if (bpp == 0) {
		LogE(TAG, "Failed get pixel format");
	}

	mem->form = bpp;
	mem->width = width;
	mem->height = height;

	printf("%s: bpp=%d\n", TAG, bpp);

	int size = width * height * bpp;
	if (mem->base == NULL) {
		mem->base = (char *)calloc(size, 1);
		if (mem->base == NULL) {
			LogE(TAG, "Failed calloc mem\n");
		}
	}
	memset(mem->base, 0, size);
	char *start = mem->base;
	int i, j;
	for (i = 0; i < height; ++i) {
		/*
		   for (j = 0; j < width; ++j) {
		   memcpy(start, row_pointers[i] + j, bpp);
		   start += bpp;
		   }
		 */
		memcpy(start, row_pointers[i], width * bpp);
		start += width * bpp;
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(fp);
	return size;
}

int write_png(const char *path, Bitmap_t *mem)
{
	VALIDATE_NOT_NULL2(path, mem);
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp palette;

	fp = fopen(path, "wb");
	if (NULL == fp) {
		LogE(TAG, "Failed open file");
		return -1;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (NULL == png_ptr) {
		fclose (fp);
		return -1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (NULL == info_ptr) {
		fclose (fp);
		png_destroy_write_struct(&png_ptr, NULL);
		return -1;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		LogE(TAG, "write file occur error");
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return -1;
	}

	int form = PNG_COLOR_TYPE_RGB_ALPHA;
	int depth = 8;
	switch (mem->form) {
		case RGBA32:
			form = PNG_COLOR_TYPE_RGBA;
			break;

		case RGB24:
			form = PNG_COLOR_TYPE_RGB;
			break;

		case GRAY:
			form = PNG_COLOR_TYPE_GRAY;
			depth = 1;
			break;
	}

	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, mem->width, mem->height, depth, form, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);
	png_bytep row_pointers[mem->height];
	int k;
	for (k = 0; k < mem->height; ++k) {
		row_pointers[k] = mem->base + k * mem->width * mem->form;
	}
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose (fp);

	return 0;
}

int readFile(const char* path, char** mem)
{
	VALIDATE_NOT_NULL2(path, mem);
	FILE *fp = fopen(path, "r");
	if (NULL == fp) {
		LogE(TAG, "Failed open shader file\n");
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (NULL == *mem) {
		(*mem) = (char *)calloc(1, size);
		if (NULL == *mem) {
			LogE(TAG, "Failed calloc mem for shader file");
			close (fp);
			return -1;
		}
	}

	if (fread(*mem, 1, size, fp) != size) {
		LogE(TAG, "Failed read shader to mem");
		close(fp);
		free (*mem);
		*mem = NULL;
		return -1;
	}

	return 0;
}

static int createShader(int type) {
	int shader = glCreateShader(GL_VERTEX_SHADER);
	if (!shader) {
		LogE(TAG, "Failed glCreateShader");
		return -1;
	}
	return shader;
}

#define LOG_BUFF_SIZE 1024
static char gLogBuff[LOG_BUFF_SIZE];

static int executeShader(int shader, const char* source) {
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	int compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		int len;
		glGetShaderInfoLog(shader, LOG_BUFF_SIZE, &len, 0);
		if (len > 0) {
			gLogBuff[len] = 0;
			LogE(TAG, gLogBuff);
		} else {
			LogE(TAG, "Failed get compile log");
		}
		glDeleteShader(shader);
		return -1;
	}

	// Mark to delete
	glDeleteShader(shader);

	int program = glCreateProgram();
	if (!program) {
		LogE(TAG, "Failed glCreateProgram");
		glDeleteShader(shader);
		return -1;
	}
	glAttachShader(program, shader);
	glLinkProgram(program);
	glUseProgram(program);
	glDetachShader(program, shader);
	glUseProgram(0);
	return 0;
}

int executeVertexShader(const char* source)
{
	VALIDATE_NOT_NULL(source);
	int shader = glCreateShader(GL_VERTEX_SHADER);
	if (!shader) {
		LogE(TAG, "Failed glCreateShader");
		return -1;
	}
	return executeShader(shader, source);
}

int executeFragmentShader(const char* source)
{
	VALIDATE_NOT_NULL(source);
	int shader = glCreateShader(GL_FRAGMENT_SHADER);
	if (!shader) {
		LogE(TAG, "Failed glCreateShader");
		return -1;
	}
	return executeShader(shader, source);
}

void freeBitmap(Bitmap_t *mem)
{
	if (NULL != mem) {
		if (NULL != mem->base) {
			free (mem->base);
			mem->base = NULL;
		}
	}
}
