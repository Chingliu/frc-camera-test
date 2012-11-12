/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Abstract class representing an image processing operation.
 */

#ifndef _IMAGE_PROCESSOR_H_
#define _IMAGE_PROCESSOR_H_

#include <nivision.h>

class ImageProcessor {
public:
  virtual Image* ProcessImage(Image* image, char* textOut) = 0;
};

#endif // _IMAGE_PROCESSOR_H_
