#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "texturize.h"
#include "graph.h"

#define MAX_CAPACITY 16383 // Half of the largest short, (captype is short in graph.h)
#define REMPLI    1
#define CUT_NORTH 2
#define CUT_WEST  4
#define HAS_CUT_NORTH(r) (r) & CUT_NORTH
#define HAS_CUT_WEST(r)  (r) & CUT_WEST

// ||pixel1 - pixel2||^2
// From experience, squares seem to work better than another type of norm.
inline Graph::captype cost (unsigned char * pixel1, unsigned char * pixel2, int channels) {
  int diff, result = 0;
  for (int c = 0; c < channels; c++){
    diff = pixel1[c] - pixel2[c];
    result += diff*diff;
  }
  return (result/24);
  // We need to divide at least by 24, or we might return more than
  // MAX_CAPACITY.
}

inline Graph::captype gradient (unsigned char * pixel1, unsigned char * pixel2, int channels) {
  int diff, result = 0;
  for (int c = 0; c < channels; c++){
    diff = pixel1[c] - pixel2[c];
    result += diff*diff;
  }
  return ((Graph::captype) sqrt(result));
}

// When we write the four arguments to edge_weight on two lines of code,
// we try to always align things (pixel VS image) so that it makes sense.
inline Graph::captype edge_weight (int channels,
                                   unsigned char * im1_pix1, unsigned char * im2_pix1,
                                   unsigned char * im1_pix2, unsigned char * im2_pix2) {
  return ((cost(im1_pix1,im2_pix1,channels) + (cost(im1_pix2,im2_pix2,channels)))
          / (gradient(im1_pix1,im1_pix2,channels) + gradient(im2_pix1,im2_pix2,channels) +1));
}

inline void paste_patch_pixel_to_image(int width_i, int height_i, int width_p, int height_p,
                                       int x_i, int y_i, int x_p, int y_p,
                                       int channels,
                                       unsigned char * image, unsigned char * patch) {
  int k;
  for (k = 0; k < channels; k++) {
    image[(y_i * width_i + x_i) * channels + k] = patch[(y_p * width_p + x_p) * channels + k];
  }
}

void decoupe_graphe (int* patch_posn,
                     int width_i, int height_i, int width_p, int height_p,
                     int channels,
                     unsigned char  **rempli,
                     unsigned char   *image, unsigned char * patch,
                     unsigned char   *coupe_h_here, unsigned char * coupe_h_west,
                     unsigned char   *coupe_v_here, unsigned char * coupe_v_north,
                     bool  make_tileable, bool invert) {

  int k, x_p, y_p, x_i, y_i;// nb_sommets, sommet_courant; // Compteurs
  int real_x_i, real_y_i;
  int x_inf, y_inf, x_sup, y_sup;
  Graph * graphe = new Graph(); // Le graphe à couper
  Graph::node_id *node_of_pixel = (void **) calloc (width_p * height_p, sizeof (Graph::node_id)); // Le noeud du graph auquel correspond un pointeur.
  for (k=0; k<width_p * height_p; k++) node_of_pixel[k] = NULL;

  Graph::captype poids; // Pour calculer le poids d'un arc avant de le déclarer à Graph:add_edge
  Graph::node_id first_node = NULL, node_sommet_courant;
  unsigned char r;

  if (make_tileable) {
    x_inf = patch_posn[0];
    y_inf = patch_posn[1];
    x_sup = patch_posn[0] + width_p;
    y_sup = patch_posn[1] + height_p;
  } else {
    x_inf = MAX (0, patch_posn[0]);
    y_inf = MAX (0, patch_posn[1]);
    x_sup = MIN (width_i,  patch_posn[0] + width_p);
    y_sup = MIN (height_i, patch_posn[1] + height_p);
  }

  for (real_x_i = x_inf;
       real_x_i < x_sup;
       real_x_i++) {
    x_p = real_x_i - patch_posn[0];
    x_i = modulo (real_x_i, width_i);
    for (real_y_i = y_inf;
         real_y_i < y_sup;
         real_y_i++) {
      y_p = real_y_i - patch_posn[1];
      y_i = modulo (real_y_i, height_i);

      // Si le pixel de l'image n'est pas rempli, on ne fait rien et on passe au suivant
      if (rempli[x_i][y_i]) {
        node_of_pixel[x_p * height_p + y_p] = graphe->add_node ();
        if (first_node == NULL) first_node = node_of_pixel[x_p * height_p + y_p];
      }
    }
  }

  for (real_x_i = x_inf;
       real_x_i < x_sup;
       real_x_i++) {
    x_p = real_x_i - patch_posn[0];
    x_i = modulo (real_x_i, width_i);

    for (real_y_i = y_inf;
         real_y_i < y_sup;
         real_y_i++) {
      y_p = real_y_i - patch_posn[1];
      y_i = modulo (real_y_i, height_i);

      // If the pixel in the image hasn't been filled, we do nothing and skip
      // to the next one.
      if (!rempli[x_i][y_i]) {
        continue;
      } else {
        // Create the nodes and edges.
        node_sommet_courant = node_of_pixel[x_p * height_p + y_p];

        // If the neighbord exists in the patch and if the pixel to the North
        // is filled in the image, create a link to it.
        if ((!make_tileable && y_p != 0 && y_i != 0 && rempli[x_i][y_i - 1])
          || (make_tileable && y_p != 0 && rempli[x_i][modulo (y_i - 1, height_i)])) {
          poids = edge_weight (channels,
                               image + ((y_i * width_i + x_i) * channels),
                               patch + ((y_p * width_p + x_p) * channels),
                               image + (((modulo (y_i - 1, height_i)) * width_i + x_i) * channels),
                               patch + (((y_p - 1) * width_p + x_p) * channels));
          graphe->add_edge (node_sommet_courant,
                            node_of_pixel[x_p * height_p + y_p - 1],
                            poids, poids);
        }

        // If the West neighbor exists in the patch and if the West pixel is
        // filled in the image, we create a link to it.
        if ((!make_tileable && x_p != 0 && x_i != 0 && rempli[x_i - 1][y_i])
          || (make_tileable && x_p != 0 && rempli[modulo (x_i - 1, width_i)][y_i])) {
          poids = edge_weight (channels,
                               image + ((y_i * width_i + x_i) * channels),
                               patch + ((y_p * width_p + x_p) * channels),
                               image + ((y_i * width_i + (modulo (x_i, width_i) - 1)) * channels),
                               patch + ((y_p * width_p + (x_p - 1)) * channels));
          graphe->add_edge (node_sommet_courant,
                            node_of_pixel[(x_p - 1) * height_p + y_p],
                            poids, poids);
        }

        // If I am on the edge of the patch and, if !make_tileable, I am not on
        // the edge of the image, link me to the source.
       if (    (make_tileable && (x_p == 0 || y_p == 0 || x_p == width_p - 1 || y_p == height_p - 1))
            || (!make_tileable && (x_p == 0 || y_p == 0 || x_p == width_p - 1 || y_p == height_p - 1)
		               &&  x_i != 0 && y_i != 0 && x_i != width_i - 1 && y_i != height_i - 1)) {
          graphe->add_tweights (node_sommet_courant, MAX_CAPACITY, 0);
	}

        // If one of my neighbords exists and isn't filled, link me to the sink.
        if (((!make_tileable)
              && (  (y_p != 0            && y_i != 0            && !rempli[x_i][y_i - 1])      // North
                 || (y_p != height_p - 1 && y_i != height_i - 1 && !rempli[x_i][y_i + 1])      // South
                 || (x_p != width_p - 1  && x_i != width_i - 1  && !rempli[x_i + 1][y_i])      // East
                 || (x_p != 0            && x_i != 0            && !rempli[x_i - 1][y_i])))    // West
            || ((make_tileable)
              && (  (y_p != 0            && !rempli[x_i][modulo (y_i - 1, height_i)])          // North
                 || (y_p != height_p - 1 && !rempli[x_i][modulo (y_i + 1, height_i)])          // South
                 || (x_p != width_p - 1  && !rempli[modulo (x_i + 1, width_i)][y_i])           // East
                 || (x_p != 0            && !rempli[modulo (x_i - 1, width_i)][y_i])))) {      // West
          graphe->add_tweights (node_sommet_courant, 0, MAX_CAPACITY);
	}
      }
    }
  }

  // If !make_tileable, link the top left pixel in patch \cap image to the
  // source.
  if (!make_tileable) {
    graphe->add_tweights (first_node, MAX_CAPACITY, 0);
  }

  graphe->maxflow();

  for (real_x_i = x_inf; real_x_i < x_sup; real_x_i++) {
    x_p = real_x_i - patch_posn[0];
    x_i = modulo (real_x_i, width_i);
    for (real_y_i = y_inf; real_y_i < y_sup; real_y_i++) {
      y_p = real_y_i - patch_posn[1];
      y_i = modulo (real_y_i, height_i);
      r = rempli[x_i][y_i];
      if (r) {
        if (graphe->what_segment(node_of_pixel[x_p * height_p + y_p]) == Graph::SINK) {
          paste_patch_pixel_to_image (width_i, height_i, width_p, height_p, x_i, y_i, x_p, y_p,
                                      channels, image, patch);
	}
      } else {
        paste_patch_pixel_to_image (width_i, height_i, width_p, height_p, x_i, y_i, x_p, y_p,
                                    channels, image, patch);
        rempli[x_i][y_i] = REMPLI;
      }
    }
  }

  delete graphe;
  free (node_of_pixel);

  return;
}