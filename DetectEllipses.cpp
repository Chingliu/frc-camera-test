/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Class representing an ellipse detection operation on an image.
 */

#include "DetectEllipses.h"

#include <iostream>

/*
 * Applies an ellipse detection operation to the source image.
 *
 * @param textOut Pointer to a 512-character buffer that is displayed beneath the processed image.
 */
Image* DetectEllipses::ProcessImage(Image* image, char* textOut) {
  Image* input = imaqCreateImage(IMAQ_IMAGE_U8, 3);

  // Extract the green plane only by setting the other two to NULL.
  imaqExtractColorPlanes(image, IMAQ_RGB, NULL, input, NULL);

  // Set up ellipse filter parameters.
  EllipseDescriptor descriptor;
  descriptor.minMajorRadius = 20;
  descriptor.maxMajorRadius = 300;
  descriptor.minMinorRadius = 20;
  descriptor.maxMinorRadius = 300;
  int numEllipses = 0;

  // Get the array of detected ellipses.
  EllipseMatch* match = imaqDetectEllipses(input, &descriptor, NULL, NULL, NULL, &numEllipses);
  
  int numChars = sprintf_s(textOut, 512, "# of ellipses: %d\r\n\r\n", numEllipses);
  float totalX = 0;
  for (int i = 0; i < numEllipses; i++)
  {
    // Print the location, size and score information for each detected ellipse.
    numChars += sprintf_s(textOut + numChars,
                          512 - numChars,
                          "Pos: (%.0f, %.0f)\tMaj: %.0f\tMin: %.0f\tScore: %.0f\r\n",
                          match[i].position.x,
                          match[i].position.y,
                          match[i].majorRadius,
                          match[i].minorRadius,
                          match[i].score);
    totalX += match[i].position.x;
  }
  if (numEllipses > 0)
  {
    // Print the average X-coordinate of the ellipse centers.
    numChars += sprintf_s(textOut + numChars,
                          512 - numChars,
                          "\r\nAverage X: %.0f\r\n",
                          totalX / numEllipses);
  }

  return input;
}
