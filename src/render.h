#ifndef __RENDER_H__
#define __RENDER_H__

/*  Public functions  */

int32 render(int32     image_ID,
              GimpDrawable       *drawable,
              PlugInVals         *vals,
              PlugInImageVals    *image_vals,
              PlugInDrawableVals *drawable_vals);

#endif /* __RENDER_H__ */
