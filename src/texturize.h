#ifndef TEXTURIZE_H
#define TEXTURIZE_H

#include <opencv2/opencv.hpp>

// Counts number of cells != 0 in filled.
int count_filled_pixels(unsigned char **filled, int width_i, int height_i);

// Compute the graph, cuts it and updates the image.
void decoupe_graphe(
    int* patch_posn, // Where to put the patch.
    int width_i, int height_i, int width_p, int height_p,
    int channels,
    unsigned char **filled, // Tells whether the pixel is filled and if there is a cut here.
    unsigned char *image, unsigned char *patch,
    unsigned char *coupe_h_here, unsigned char *coupe_h_west,   // Pixels lost along an old horizontal cut
    unsigned char *coupe_v_here, unsigned char *coupe_v_north,  // idem for vertical cuts
    bool make_tileable, bool invert);

// Allocates the memory (with malloc) and fills with 0.
unsigned char **init_guchar_tab_2d(int x, int y);

/* Compute the best position to put the patch,
 * between (x_patch_posn_min, y_patch_posn_min)
 * and     (x_patch_posn_max, y_patch_posn_max).
 */
void offset_optimal(
    int *resultat, // The position where the patch will have to be put.
    unsigned char *image, unsigned char *patch,
    int width_p, int height_p, int width_i, int height_i,
    int x_patch_posn_min, int y_patch_posn_min, int x_patch_posn_max, int y_patch_posn_max,
    // Admissible positions for the patch, this function determines the best one.
    int channels,
    unsigned char **filled,
    bool make_tileable);

// Returns the minimal unfilled pixel under lexicographical order (y,x).
int *pixel_to_fill(unsigned char **filled, int width_i, int height_i, int *resultat);

int modulo(int x, int m);

#endif // TEXTURIZE_H