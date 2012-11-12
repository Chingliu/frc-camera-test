/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Class representing a colour plane extraction operation on an image.
 */

#include "ColorPlaneExtraction.h"

/*
 * Applies a colour plane extraction operation to the source image.
 *
 * @param textOut Pointer to a 512-character buffer that is displayed beneath the processed image.
 */
Image* ColorPlaneExtraction::ProcessImage(Image* image, char* textOut) {
  Image* output = imaqCreateImage(IMAQ_IMAGE_U8, 3);

  // Extract the red plane only by setting the other two to NULL.
  imaqExtractColorPlanes(image, IMAQ_RGB, output, NULL, NULL);

  // No text to display.
  textOut[0] = 0;

  return output;
}
