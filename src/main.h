#ifndef __MAIN_H__
#define __MAIN_H__

#include <glib.h>

typedef struct {
  int     width_i;
  int     height_i;
  int     overlap;
  bool make_tileable;
} PlugInVals;

typedef struct {
  int32 image_id;
  int width_p;
  int height_p;
} PlugInImageVals;

typedef struct {
  int32    drawable_id;
} PlugInDrawableVals;

typedef struct {
  bool  chain_active;
} PlugInUIVals;


/*  Default values  */

extern const PlugInVals         default_vals;
extern const PlugInImageVals    default_image_vals;
extern const PlugInDrawableVals default_drawable_vals;
extern const PlugInUIVals       default_ui_vals;


#endif /* __MAIN_H__ */
