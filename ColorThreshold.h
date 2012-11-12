/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Class representing a colour thresholding operation on an image.
 */

#ifndef _COLOR_THRESHOLD_H_
#define _COLOR_THRESHOLD_H_

#include "ImageProcessor.h"

class ColorThreshold : public ImageProcessor {
public:
  virtual Image* ProcessImage(Image* image, char* textOut);
};

#endif // _COLOR_THRESHOLD_H_
