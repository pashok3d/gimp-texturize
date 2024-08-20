#ifndef __INTERFACE_H__
#define __INTERFACE_H__


/*  Public functions  */

bool dialog (int32 image_ID,
                 GimpDrawable       *drawable,
                 PlugInVals         *vals,
                 PlugInImageVals    *image_vals,
                 PlugInDrawableVals *drawable_vals,
                 PlugInUIVals       *ui_vals);

#endif /* __INTERFACE_H__ */
