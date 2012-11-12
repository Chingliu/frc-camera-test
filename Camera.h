/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Class representing the Axis Camera.
 */

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <winsock2.h>
#include <Windows.h>

class BitmapImage;
class ImageProcessor;

class Camera {
public:
  Camera();
  ~Camera();
  void Start();
  void Run();
  void Shutdown();
  void SetWindow(HWND appWindow);
  BitmapImage* GetBeforeBmp();
  BitmapImage* GetAfterBmp();
  char* GetText();
  static DWORD WINAPI StartCamera(LPVOID param);

private:
  void SocketError();
  void VisionError();

  HANDLE mutex_;
  SOCKET cameraSocket_;
  char* buffer_;
  void* flatJpeg_;
  unsigned int flatJpegSize_;
  ImageProcessor* imageProcessor_;
  HWND appWindow_;
  BitmapImage* beforeBmp_;
  BitmapImage* afterBmp_;
  char textOutput_[512];
};

#endif // _CAMERA_H_
