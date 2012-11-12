/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Class representing the main application window.
 */

#ifndef _APP_WINDOW_H_
#define _APP_WINDOW_H_

#include "Camera.h"
#include "Constants.h"
#include <Windows.h>

// Represents a pixel value in the HSL colour space.
typedef struct {
  unsigned char h;
  unsigned char s;
  unsigned char l;
} HslStruct;

class AppWindow {
public:
  AppWindow();
  void Create(HINSTANCE hInstance);
  HWND GetHWnd();

private:
  void Paint();
  LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  HslStruct RgbToHsl(COLORREF color);

  HWND hWnd_;
  HWND leftTextWnd_;
  HWND rightTextWnd_;
  HANDLE mutex_;
  Camera camera_;
};

#endif // _APP_WINDOW_H_
