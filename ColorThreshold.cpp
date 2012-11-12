/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Class representing a colour thresholding operation on an image.
 */

#include "ColorThreshold.h"

#include <iostream>

/*
 * Applies a colour thresholding operation to the source image, and analyzes the largest particle.
 *
 * @param textOut Pointer to a 512-character buffer that is displayed beneath the processed image.
 */
Image* ColorThreshold::ProcessImage(Image* image, char* textOut) {
  Range H;
  Range S;
  Range L;
  H.minValue = 250;
  H.maxValue = 255;
  S.minValue = 90;
  S.maxValue = 150;
  L.minValue = 70;
  L.maxValue = 130;

  Image* output = imaqCreateImage(IMAQ_IMAGE_U8, 3);
  imaqColorThreshold(output, image, 150, IMAQ_HSL, &H, &S, &L);

  // Find the largest particle in the thresholded image.
  int numParticles;
  imaqCountParticles(output, false, &numParticles);
  int biggestParticle = 0;
  double biggestArea = 0;
  for (int i = 0; i < numParticles; i++) {
    double area;
    imaqMeasureParticle(output, i, FALSE, IMAQ_MT_AREA, &area);
    if (area > biggestArea) {
      biggestParticle = i;
      biggestArea = area;
    }
  }

  // Determine the x and y coordinates of the largest particle.
  double x, y;
  imaqMeasureParticle(output, biggestParticle, FALSE, IMAQ_MT_CENTER_OF_MASS_X, &x);
  imaqMeasureParticle(output, biggestParticle, FALSE, IMAQ_MT_CENTER_OF_MASS_Y, &y);

  // Format the particle information for display under the processed image.
  if (numParticles > 0) {
    sprintf_s(textOut, 512, "Position: (%3.1f, %3.1f)\r\nArea: %.0f", x, y, biggestArea);
  }
  else {
    sprintf_s(textOut, 512, "No particles found.");
  }

  return output;
}
