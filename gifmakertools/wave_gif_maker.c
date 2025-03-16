#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 30000
#define HEIGHT 30000
#define CHANNELS 3  // RGB

// Color structure
typedef struct {
    unsigned char r, g, b;
} Color;

// Function to create a frame
unsigned char* create_frame(Color color) {
    size_t total_size = (size_t)WIDTH * (size_t)HEIGHT * (size_t)CHANNELS;
    unsigned char* frame = (unsigned char*)malloc(total_size);
    if (!frame) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    // Fill the frame with the color
    #pragma omp parallel for
    for (size_t i = 0; i < total_size; i += CHANNELS) {
        frame[i] = color.r;
        frame[i + 1] = color.g;
        frame[i + 2] = color.b;
    }

    return frame;
}

int main() {
    // Create output directory if it doesn't exist
    system("mkdir -p frames");

    // Create a single red frame
    Color color = {255, 0, 0};  // Red
    unsigned char* frame = create_frame(color);
    
    // Save frame as PNG
    printf("Creating frame...\n");
    stbi_write_png("frames/frame.png", WIDTH, HEIGHT, CHANNELS, frame, WIDTH * CHANNELS);
    free(frame);
    
    // Create GIF from frame using ImageMagick with increased memory limits
    printf("Creating GIF...\n");
    system("convert -limit memory 8GB -limit map 8GB -size 30000x30000 frames/frame.png -define registry:temporary-path=/tmp color_wave.gif");
    
    printf("Cleaning up...\n");
    system("rm -rf frames");
    
    printf("Done!\n");
    return 0;
} 