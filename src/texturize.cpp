#include "texturize.h"
#include <opencv2/opencv.hpp>
#include <cmath>
#include <cstdlib>
#include <iostream>

int modulo(int a, int b) {
    return (a % b + b) % b;
}

// Computes the distance between image_tab and patch_tab for the zone that's been filled in:
// (x_min,y_min) -> (x_max,y_max) in image_tab
// (x_min,y_min)-posn -> (x_max,y_max) - posn in patch_tab
float difference(int width_i, int height_i, int width_p, int height_p,
                 unsigned char *image, unsigned char *patch, int posn_x, int posn_y,
                 int x_min, int y_min, int x_max, int y_max,
                 int channels, unsigned char **filled) {
    int somme = 0, zone = 0;
    int x_i, y_i, k;
    unsigned char *image_ptr, *patch_ptr;
    int x_p, y_p;
    int x_i_start, x_p_start;
    int xcount, ycount;
    int iy, ix;
    unsigned char *image_ptr_x, *patch_ptr_x;
    int image_add_y, patch_add_y;

    // Debugging output
    std::cout << "difference function called with parameters:" << std::endl;
    std::cout << "posn_x: " << posn_x << ", posn_y: " << posn_y << std::endl;
    std::cout << "x_min: " << x_min << ", y_min: " << y_min << std::endl;
    std::cout << "x_max: " << x_max << ", y_max: " << y_max << std::endl;

    ycount = y_max - y_min;
    xcount = x_max - x_min;
    y_i = modulo(y_min, height_i);
    x_i_start = modulo(x_min, width_i);
    y_p = modulo(y_i - posn_y, height_p);
    x_p_start = modulo(x_i_start - posn_x, width_p);
    image_add_y = width_i * channels;
    patch_add_y = width_p * channels;
    image_ptr_x = image + y_i * image_add_y;
    patch_ptr_x = patch + y_p * patch_add_y;

    for (iy = 0; iy < ycount; iy++) {
        x_i = x_i_start;
        x_p = x_p_start;
        image_ptr = image_ptr_x + x_i * channels;
        patch_ptr = patch_ptr_x + x_p * channels;

        for (ix = 0; ix < xcount; ix++) {
            if (filled[x_i][y_i]) {
                for (k = 0; k < channels; k++) {
                    somme += abs(*image_ptr - *patch_ptr);
                    image_ptr++;
                    patch_ptr++;
                    zone++;
                }
            } else {
                image_ptr += channels;
                patch_ptr += channels;
            }
            if (++x_i >= width_i) { x_i = 0; image_ptr = image_ptr_x; }
            if (++x_p >= width_p) { x_p = 0; patch_ptr = patch_ptr_x; }
        }
        image_ptr_x += image_add_y;
        patch_ptr_x += patch_add_y;
        if (++y_i >= height_i) { y_i = 0; image_ptr_x = image; }
        if (++y_p >= height_p) { y_p = 0; patch_ptr_x = patch; }
    }

    if (zone == 0) {
        std::cerr << "Bug: Zone = 0" << std::endl;
        std::cerr << "No filled pixels in the overlapping region." << std::endl;
        std::cerr << "posn_x: " << posn_x << ", posn_y: " << posn_y << std::endl;
        std::cerr << "x_min: " << x_min << ", y_min: " << y_min << std::endl;
        std::cerr << "x_max: " << x_max << ", y_max: " << y_max << std::endl;
        std::cerr << "filled state at (x_i, y_i): " << (int)filled[x_i][y_i] << std::endl;
        exit(-1);
    }
    return (((float) somme) / ((float) zone));
}

void offset_optimal(int *resultat, unsigned char *image, unsigned char *patch,
                    int width_p, int height_p, int width_i, int height_i,
                    int x_patch_posn_min, int y_patch_posn_min, int x_patch_posn_max, int y_patch_posn_max,
                    int channels, unsigned char **filled, bool tileable) {
    int x_i, y_i;
    float best_difference = INFINITY, tmp_difference;
    if (tileable) {
        for (x_i = x_patch_posn_min; x_i < x_patch_posn_max; x_i++) {
            for (y_i = y_patch_posn_min; y_i < y_patch_posn_max; y_i++) {
                // Ensure valid patch position
                if (x_i < 0 || y_i < 0 || x_i >= width_i || y_i >= height_i) continue;
                tmp_difference = difference(width_i, height_i, width_p, height_p, image, patch,
                                            x_i, y_i, std::max(0, x_i), std::max(0, y_i),
                                            x_i + width_p, y_i + height_p, channels, filled);
                std::cout << "x_i: " << x_i << ", y_i: " << y_i << ", tmp_difference: " << tmp_difference << std::endl;
                if (tmp_difference < best_difference) {
                    best_difference = tmp_difference;
                    resultat[0] = x_i; resultat[1] = y_i;
                }
            }
        }
    } else {
        for (x_i = x_patch_posn_min; x_i < x_patch_posn_max; x_i++) {
            for (y_i = y_patch_posn_min; y_i < y_patch_posn_max; y_i++) {
                // Ensure valid patch position
                if (x_i < 0 || y_i < 0 || x_i >= width_i || y_i >= height_i) continue;
                tmp_difference = difference(width_i, height_i, width_p, height_p, image, patch,
                                            x_i, y_i, std::max(0, x_i), std::max(0, y_i),
                                            std::min(x_i + width_p, width_i), std::min(y_i + height_p, height_i),
                                            channels, filled);
                std::cout << "x_i: " << x_i << ", y_i: " << y_i << ", tmp_difference: " << tmp_difference << std::endl;
                if (tmp_difference < best_difference) {
                    best_difference = tmp_difference;
                    resultat[0] = x_i; resultat[1] = y_i;
                }
            }
        }
    }
}

// Finds the next pixel that needs to be filled.
int* pixel_to_fill(unsigned char **filled, int width_i, int height_i, int *resultat) {
    int x_i, y_i;
    for (y_i = 0; y_i < height_i; y_i++) {
        for (x_i = 0; x_i < width_i; x_i++) {
            if (!filled[x_i][y_i]) {
                resultat[0] = x_i;
                resultat[1] = y_i;
                return resultat;
            }
        }
    }
    return NULL;
}

// Allocates enough memory for a 2-dimensional table of unsigned chars and
// initializes all elements to zero.
unsigned char ** init_guchar_tab_2d(int x, int y) {
    unsigned char ** tab;
    int i, j;
    tab = (unsigned char**) malloc (x * sizeof (unsigned char*));
    for (i = 0; i < x; i++) {
        tab[i] = (unsigned char*) malloc (y * sizeof (unsigned char));
    }
    for (i = 0; i < x; i++) {
        for (j = 0; j < y; j++) tab[i][j] = 0;
    }
    return tab;
}

// Counts the number of pixels that are already filled.
int count_filled_pixels(unsigned char **filled, int width_i, int height_i) {
    int x_i, y_i;
    int somme = 0;
    for (x_i = 0; x_i < width_i; x_i++) {
        for (y_i = 0; y_i < height_i; y_i++) {
            if (filled[x_i][y_i]) somme++;
        }
    }
    return somme;
}