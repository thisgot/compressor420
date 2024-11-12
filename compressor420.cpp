#define STBI_MAX_DIMENSIONS 1000000000
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>

using namespace std;

double to_fixed(double number, int decimals){
	double divider = pow(10,decimals);
	int n_number = number * divider;
	return double(n_number) / divider;
}

double bytes_to_mb(long double bytes_value) {
    return bytes_value / (1024 * 1024);
}

template <typename T>
void create_array(T** &array, int width, int height) {
    array = new T*[width];
    for (int i = 0; i < width; i++) {
        array[i] = new T[height];
    }
}

template <typename T>
void free_array(T** &array, int width) {
    for (int i = 0; i < width; i++) {
        delete[] array[i];
    }
    delete[] array;
}

void compress420(unsigned char **pixels, int width, int height, unsigned char *&y_channel, unsigned char *&cb_420, unsigned char *&cr_420, int &output_size) {
    output_size = 0;
    y_channel = new unsigned char[width * height];
    unsigned char *cb_channel = new unsigned char[width * height];
    unsigned char *cr_channel = new unsigned char[width * height];

    for (int i = 0; i < width * height; i++) {
        int r = pixels[i][0];
        int g = pixels[i][1];
        int b = pixels[i][2];
        y_channel[i] = int(0.299 * r + 0.587 * g + 0.114 * b);
        cb_channel[i] = int((b - int(y_channel[i])) * 0.564 + 128);
        cr_channel[i] = int((r - int(y_channel[i])) * 0.713 + 128);
        output_size += 1;
    }

    cb_420 = new unsigned char[width * height / 4];
    cr_420 = new unsigned char[width * height / 4];
    int chroma_index = 0;

    for (int i = 0; i < height; i += 2) {
        for (int j = 0; j < width; j += 2) {
            int index1 = i * width + j;
            int index2 = i * width + (j + 1);
            int index3 = (i + 1) * width + j;
            int index4 = (i + 1) * width + (j + 1);

            int avg_cr = (cr_channel[index1] + cr_channel[index2] + cr_channel[index3] + cr_channel[index4]) / 4;
            int avg_cb = (cb_channel[index1] + cb_channel[index2] + cb_channel[index3] + cb_channel[index4]) / 4;
            cb_420[chroma_index] = avg_cb;
            cr_420[chroma_index] = avg_cr;

            chroma_index++;
            output_size += 2;
        }
    }

    delete[] cb_channel;
    delete[] cr_channel;
}

void decompress420(unsigned char **output, int width, int height, unsigned char *y_channel, unsigned char *cb_420, unsigned char *cr_420) {
    int chroma_index = 0;

    for (int i = 0; i < height; i += 2) {
        for (int j = 0; j < width; j += 2) {
			unsigned char cr = cr_420[chroma_index];
			unsigned char cb = cb_420[chroma_index];
			chroma_index++;

			for (int dy = 0; dy < 2; dy++) {
				for (int dx = 0; dx < 2; dx++) {
					int y_index = (i + dy) * width + (j + dx);
					if (y_index >= width * height) continue;

					int y = y_channel[y_index];
					int r = int(y + 1.402 * (cr - 128));
					int g = int(y - 0.344136 * (cb - 128) - 0.714136 * (cr - 128));
					int b = int(y + 1.772 * (cb - 128));

					r = max(0, min(255, r));
					g = max(0, min(255, g));
					b = max(0, min(255, b));

					output[y_index][0] = r;
					output[y_index][1] = g;
					output[y_index][2] = b;
				}
			}
		}
	}
}

void image_to_pixels(unsigned char* image, unsigned char** pixels, int width, int height) {
    for (int i = 0; i < width * height; ++i) {
        int pixel_start_index = i * 3;
        pixels[i][0] = image[pixel_start_index];
        pixels[i][1] = image[pixel_start_index + 1];
        pixels[i][2] = image[pixel_start_index + 2];
    }
}

void pixels_to_image(unsigned char** pixels, unsigned char* image, int width, int height) {
    for (int i = 0; i < width * height; ++i) {
        int pixel_start_index = i * 3;
        image[pixel_start_index] = pixels[i][0];
        image[pixel_start_index + 1] = pixels[i][1];
        image[pixel_start_index + 2] = pixels[i][2];
    }
}

int main() {
    int width, height, channels;
    unsigned char* image = stbi_load("image.jpg", &width, &height, &channels, 0);
    
    if (image == nullptr) {
        cerr << "Error loading image!" << endl;
        return -1;
    }

    cout << "Total raw size: " << to_fixed(bytes_to_mb(width*height*3),2) << " MB" << endl;
	
	unsigned char **pixels;
	unsigned char **decompressed_pixels;
	create_array(pixels,width*height,3);
	create_array(decompressed_pixels,width*height,3);
	
	//converte para matriz de pixels, junta cada 3 itens (cores) em 1 vetor
	image_to_pixels(image,pixels,width,height);

	//converte de vetor rgb para YCrCb
	unsigned char *y_channel;
	unsigned char *cb_420;
	unsigned char *cr_420;
	int output_size = 0;
	compress420(pixels,width,height,y_channel,cb_420,cr_420,output_size);
	cout << "Total compressed raw size: " << to_fixed(bytes_to_mb(output_size),2) << " MB" << endl;
	
	//reconverte de YCrCb para vetor de pixels rgb
	decompress420(decompressed_pixels,width,height,y_channel,cb_420,cr_420);
	pixels_to_image(decompressed_pixels,image,width,height);
	
	//salva a imagem que foi comprimida e descomprimida
	stbi_write_jpg("image_compressed_n_decompressed.jpg", width, height, 3, image, 70);
	
    stbi_image_free(image);
    free_array(pixels,width*height);
	free_array(decompressed_pixels,width*height);
	delete[] y_channel;
	delete[] cb_420;
	delete[] cr_420;
	system("pause");
	return 0;
}