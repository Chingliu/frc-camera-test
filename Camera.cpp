/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Class representing the Axis Camera.
 */

#include "Camera.h"

#include "BitmapImage.h"
#include "Camera.h"
#include "ColorPlaneExtraction.h"
#include "ColorThreshold.h"
#include "Constants.h"
#include "DetectEllipses.h"
#include "ImageProcessor.h"
#include <iostream>

Camera::Camera() {
  beforeBmp_ = NULL;
  afterBmp_ = NULL;

  // Create a 512k buffer on the heap for storing frames acquired from the camera.
  buffer_ = new char[524288];

  // The type of image processing to use is specified here.
  imageProcessor_ = new DetectEllipses();

  // Create a dummy image to be used to decode the JPEG images from the camera.
  // This is an ugly hack, necessary because NIVision doesn't provide a way to create an Image from
  // JPEG data already in memory. We create an Image from a "noisy" JPEG file which is almost
  // guaranteed to be bigger than the JPEG image from the camera, flatten it to a string, and
  // continuously replace its data with that from the camera and unflatten it into an Image.
  Image* flatImage = imaqCreateImage(IMAQ_IMAGE_RGB, 3);
  if (flatImage == NULL) {
    VisionError();
  }
  if (imaqReadFile(flatImage, "noise.jpg", NULL, NULL) == 0) {
    VisionError();
  }
  flatJpeg_ = imaqFlatten(flatImage, IMAQ_FLATTEN_IMAGE, IMAQ_COMPRESSION_JPEG, 1000, &flatJpegSize_);
  if (flatJpeg_ == NULL) {
    VisionError();
  }
  if (imaqDispose(flatImage) == 0) {
    VisionError();
  }
}

Camera::~Camera() {
  delete buffer_;
  closesocket(cameraSocket_);
  WSACleanup();
}

/*
 * Sets up a socket connection to the camera and begins retrieving JPEG image data from it.
 */
void Camera::Start() {
  // Acquire a handle for the mutex lock created by the main thread.
  mutex_ = OpenMutex(SYNCHRONIZE, FALSE, "CameraMutex");

  // Set up the socket library.
  WSADATA WsaDat;
  if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != NO_ERROR) {
    SocketError();
  }

  // Create a TCP socket to the camera and connect to it.
  cameraSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (cameraSocket_ == INVALID_SOCKET) {
    SocketError();
  }
  SOCKADDR_IN sockAddr;
  sockAddr.sin_family = AF_INET;
  sockAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
  sockAddr.sin_port = htons(PORT);
  if (connect(cameraSocket_, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR) {
    SocketError();
  }

  // Populate the settings string with user parameters defined in Constants.h
  char settingsString[512];
  sprintf_s(settingsString,
            512,
"GET /axis-cgi/admin/param.cgi?action=update\
&ImageSource.I0.Sensor.WhiteBalance=%s\
&ImageSource.I0.Sensor.Exposure=%s\
&ImageSource.I0.Sensor.ExposurePriority=%d\
&ImageSource.I0.Sensor.Brightness=%d\
&ImageSource.I0.Sensor.ColorLevel=%d HTTP/1.1\n\
Connection: Keep-Alive\n\
Authorization: Basic %s;\n\n",
            WHITE_BALANCE,
            EXPOSURE,
            EXPOSURE_PRIORITY,
            BRIGHTNESS,
            COLOR_LEVEL,
            AUTHENTICATION);

  // Send the settings string to the camera to ensure desired video settings are selected.
  if (send(cameraSocket_, settingsString, (int)strlen(settingsString), 0) == SOCKET_ERROR) {
    SocketError();
  }

  char buffer[256];
  if (recv(cameraSocket_, buffer, 256, 0) == SOCKET_ERROR) {
    SocketError();
  }

  // Reset the socket for another operation since the camera closed it on the other end.
  closesocket(cameraSocket_);
  cameraSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (connect(cameraSocket_, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR) {
    SocketError();
  }

  // Populate the request string with user parameters defined in Constants.h
  char requestString[256];
  sprintf_s(requestString,
            256,
"GET /axis-cgi/mjpg/video.cgi?\
des_fps=%i&compression=%i&resolution=%s&rotation=%i&color=1&colorlevel=100 HTTP/1.1\n\
Connection: Keep-Alive\n\
Authorization: Basic %s;\n\n",
            FRAMES_PER_SECOND,
            COMPRESSION,
            RESOLUTION,
            ROTATION,
            AUTHENTICATION);

  // Send the request string to the camera, prompting a continuous motion JPEG stream in reply.
  if (send(cameraSocket_, requestString, (int)strlen(requestString), 0) == SOCKET_ERROR) {
    SocketError();
  }

  // Shut down the sending half of the socket, since it is no longer needed.
  if (shutdown(cameraSocket_, SD_SEND) == SOCKET_ERROR) {
    SocketError();
  }

  Run();
}

/*
 * Receives images from the camera and performs image processing on them.
 */
void Camera::Run() {
  // Continue acquiring frames in an infinite loop.
  while(1) {
    int counter = 0;
    char* trailingPtr = buffer_;
    char* bufferPtr = buffer_;

    // Search for the double CRLF separating the HTTP headers from the content.
    while(1) {
      // Read one byte at a time into the buffer.
      if (recv(cameraSocket_, bufferPtr, 1, 0) == SOCKET_ERROR) {
        SocketError();
      }
      counter++;
      if (counter >= 4) {
        // Check for the double CRLF at the end of what has been read so far.
        if (strncmp(trailingPtr, "\r\n\r\n", 4) == 0) {
          // Introduce a null character so that the headers can safely be searched as a string.
          *(bufferPtr + 1) = 0;
          break;
        }
        trailingPtr++;
      }
      bufferPtr++;
    }

    // Determine the size in bytes of the current JPEG image.
    char* contentPtr = strstr(buffer_, "Content-Length: ");
    if (contentPtr == NULL) {
      continue;
    }
    int contentSize = atol(contentPtr + 16);

    // Read from the socket until the entire image has been received.
    int bufferSize = 0;
    do {
      int received = recv(cameraSocket_, buffer_ + bufferSize, contentSize - bufferSize, 0);
      if (received == 0) {
        continue;
      }
      else if (received == SOCKET_ERROR) {
        SocketError();
      }
      bufferSize += received;
    } while (bufferSize < contentSize);

    // Create an NIVision Image object to represent the new frame.
    Image* image = imaqCreateImage(IMAQ_IMAGE_RGB, 3);
    if (image == NULL) {
      VisionError();
    }

/*
    // Copy the JPEG data into the flattened JPEG and unflatten it into the new Image.
    memcpy((char*)flatJpeg_ + 84, buffer_, bufferSize);
    if (imaqUnflatten(image, (const unsigned char *)flatJpeg_, flatJpegSize_) == NULL) {
      VisionError();
    }
*/

    // This is a hack to convert the in-memory JPEG to an NIVision Image object. The library doesn't
    // have a way to do this, and the previous hack no longer seems to work, so we write the JPEG to
    // disk and load it back with NIVision.
    FILE* imageFile = fopen("temp.jpg", "wb");
    fwrite(buffer_, 1, bufferSize, imageFile);
    fclose(imageFile);
    if (imaqReadFile(image, "temp.jpg", NULL, NULL) == 0) {
      VisionError();
    }

    // Process the image using whatever image processing function was specified in the constructor.
    Image* processed = imageProcessor_->ProcessImage(image, textOutput_);

    // Convert the Image objects to Windows HBITMAP objects.
    BitmapImage* before = new BitmapImage(image);
    BitmapImage* after = new BitmapImage(processed);

    // Synchronize with the main thread to update the images.
    WaitForSingleObject(mutex_, INFINITE);
    if (beforeBmp_) {
      delete beforeBmp_;
    }
    beforeBmp_ = before;
    if (afterBmp_) {
      delete afterBmp_;
    }
    afterBmp_ = after;
    ReleaseMutex(mutex_);

    // Force the application window to redraw itself using the new images.
    InvalidateRect(appWindow_, NULL, false);

    if (imaqDispose(image) == 0) {
      VisionError();
    }
    if (processed != image) {
      // If the image processor is just returning the original image object, don't delete it twice.
      if (imaqDispose(processed) == 0) {
        VisionError();
      }
    }
  }
}

void Camera::SetWindow(HWND appWindow) {
  appWindow_ = appWindow;
}

BitmapImage* Camera::GetBeforeBmp() {
  return beforeBmp_;
}

BitmapImage* Camera::GetAfterBmp() {
  return afterBmp_;
}

char* Camera::GetText() {
  return textOutput_;
}

/*
 * Entry point for the camera thread.
 */
DWORD WINAPI Camera::StartCamera(LPVOID param) {
  Camera* camera = (Camera*)param;
  camera->Start();

  return 0;
}

/*
 * Handles socket errors. Displays an error message before exiting the application.
 */
void Camera::SocketError() {
  char errorText[64];
  int error = WSAGetLastError();

  // Provide text for the most common errors, or the error number otherwise.
  switch (error) {
    case 10054:
      sprintf_s(errorText, 64, "Socket error: Connection reset by peer. Camera disconnected.");
      break;
    case 10053:
    case 10060:
      sprintf_s(errorText, 64, "Socket error: Connection timed out.");
      break;
    case 10065:
      sprintf_s(errorText, 64, "Socket error: Destination unreachable.");
      break;
    default:
      sprintf_s(errorText, 64, "Socket error: %d", WSAGetLastError());
  }

  MessageBox(NULL, errorText, "Error", MB_OK | MB_ICONERROR);
  ExitProcess(1);
}

/*
 * Handles vision errors. Displays an error message before exiting the application.
 */
void Camera::VisionError() {
  char errorText[64];
  sprintf_s(errorText, 64, "Vision error: %s", imaqGetErrorText(imaqGetLastError()));
  MessageBox(NULL, errorText, "Error", MB_OK | MB_ICONERROR);
  ExitProcess(1);
}
