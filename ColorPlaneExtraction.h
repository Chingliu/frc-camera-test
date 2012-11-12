/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Class representing a colour plane extraction operation on an image.
 */

#ifndef _COLOR_PLANE_EXTRACTION_H_
#define _COLOR_PLANE_EXTRACTION_H_

#include "ImageProcessor.h"

class ColorPlaneExtraction : public ImageProcessor {
public:
  virtual Image* ProcessImage(Image* image, char* textOut);
};

#endif // _COLOR_PLANE_EXTRACTION_H_
