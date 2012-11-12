/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Class representing an ellipse detection operation on an image.
 */

#ifndef _DETECT_ELLIPSES_H_
#define _DETECT_ELLIPSES_H_

#include "ImageProcessor.h"

class DetectEllipses : public ImageProcessor {
public:
  virtual Image* ProcessImage(Image* image, char* textOut);
};

#endif // _DETECT_ELLIPSES_H_
