/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Class representing the main application window.
 */

#include "AppWindow.h"

#include "BitmapImage.h"
#include <cmath>
#include <iostream>

AppWindow::AppWindow() {
  // Create a mutex lock to be shared with the camera thread.
  mutex_ = CreateMutex(NULL, FALSE, "CameraMutex");
}

/*
 * Registers the window Win32 class and creates the window instance.
 */
void AppWindow::Create(HINSTANCE hInstance) {
  // Register the window class.
  WNDCLASS wndclass;
  wndclass.style = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc = _WndProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = hInstance;
  wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
  wndclass.lpszMenuName = NULL;
  wndclass.lpszClassName = CLASSNAME;
  if (!RegisterClass(&wndclass)) {
    DebugBreak();
  }

  // Create the window.
  RECT desktopRect;
  GetWindowRect(GetDesktopWindow(), &desktopRect);
  hWnd_ = CreateWindow(CLASSNAME,
                       APPNAME,
                       WS_POPUPWINDOW | WS_CAPTION,
                       (desktopRect.right - WIDTH) / 2,
                       (desktopRect.bottom - HEIGHT) / 2,
                       WIDTH,
                       HEIGHT,
                       NULL,
                       NULL,
                       hInstance,
                       NULL);

  // Store the pointer to this AppWindow instance so that it can be retrieved in _WndProc.
  SetWindowLongPtr(hWnd_, GWLP_USERDATA, (LONG)(LONG_PTR)this);

  // Give the camera a reference to the application window so it can force it to be redrawn.
  camera_.SetWindow(hWnd_);

  // Start a separate thread for acquisition and processing of images from the camera.
  CreateThread(NULL, 0, Camera::StartCamera, &camera_, 0, NULL);

  leftTextWnd_ = CreateWindow("static",
                              NULL,
                              WS_CHILD | WS_VISIBLE,
                              0,
                              480,
                              640,
                              170,
                              hWnd_,
                              NULL,
                              hInstance,
                              NULL);
  rightTextWnd_ = CreateWindow("static",
                               NULL,
                               WS_CHILD | WS_VISIBLE,
                               640,
                               480,
                               640,
                               170,
                               hWnd_,
                               NULL,
                               hInstance,
                               NULL);
}

HWND AppWindow::GetHWnd() {
  return hWnd_;
}

/*
 * Redraws the contents of the window.
 */
void AppWindow::Paint() {
  // Synchronize with the camera thread to access the current images.
  WaitForSingleObject(mutex_, INFINITE);

  BitmapImage* beforeBmp = camera_.GetBeforeBmp();
  BitmapImage* afterBmp = camera_.GetAfterBmp();
  if (!(beforeBmp && afterBmp)) {
    // Fill the window background with a solid color if no images have yet to be acquired.
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd_, &ps);
    RECT rect;
    GetClientRect(hWnd_, &rect);
    FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
    EndPaint(hWnd_, &ps);

    ReleaseMutex(mutex_);
    return;
  }

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(hWnd_, &ps);
  HDC hdcMem = CreateCompatibleDC(hdc);

  // Fill the window background with a solid color.
  RECT rect;
  GetClientRect(hWnd_, &rect);
  FillRect(hdcMem, &rect, GetSysColorBrush(COLOR_BACKGROUND));

  // Display the original image on the left side.
  HBITMAP hbmpBefore = beforeBmp->GetBitmap();
  BITMAP bmpBefore;
  GetObject(hbmpBefore, sizeof(BITMAP), &bmpBefore);
  SelectObject(hdcMem, hbmpBefore);
  BitBlt(hdc, 0, 0, bmpBefore.bmWidth, bmpBefore.bmHeight, hdcMem, 0, 0, SRCCOPY);

  // Display the processed image on the right side.
  HBITMAP hbmpAfter = afterBmp->GetBitmap();
  BITMAP bmpAfter;
  GetObject(hbmpAfter, sizeof(BITMAP), &bmpAfter);
  SelectObject(hdcMem, hbmpAfter);
  BitBlt(hdc, bmpBefore.bmWidth, 0, bmpAfter.bmWidth, bmpAfter.bmHeight, hdcMem, 0, 0, SRCCOPY);

  // Release the lock on the bitmaps once they are no longer needed.
  ReleaseMutex(mutex_);

  DeleteDC(hdcMem);

  // Get the colour of the pixel currently beneath the mouse cursor.
  POINT pt;
  GetCursorPos(&pt);
  ScreenToClient(hWnd_, &pt);
  COLORREF color = GetPixel(hdc, pt.x, pt.y);
  HslStruct hsl = RgbToHsl(color);

  EndPaint(hWnd_, &ps);

  // Display the colour information text on the left side.
  char colorText[128];
  sprintf_s(colorText,
            128,
            "Pixel colour at (%d, %d):\r\nR: %d\tH: %d\r\nG: %d\tS: %d\r\nB: %d\tL: %d",
            pt.x,
            pt.y,
            GetRValue(color),
            hsl.h,
            GetGValue(color),
            hsl.s,
            GetBValue(color),
            hsl.l);
  SetWindowText(leftTextWnd_, colorText);

  // Display the text defined in the image processing function on the right side.
  SetWindowText(rightTextWnd_, camera_.GetText());
}

/*
 * Handles all messages passed to the window.
 */
LRESULT AppWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  LRESULT result = FALSE;
  switch (uMsg) {
    case WM_ERASEBKGND:
      result = TRUE;
      break;
    case WM_PAINT:
      Paint();
      result = TRUE;
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
  }
  return result;
}

/*
 * Static message handler. Determines the window instance and calls its WndProc.
 */
LRESULT CALLBACK AppWindow::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  AppWindow* window = (AppWindow*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if (window) {
    LRESULT lResult = window->WndProc(hWnd, uMsg, wParam, lParam);
    if (lResult) {
      return lResult;
    }
  }
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/*
 * Converts pixel colour values from the RGB colour space to the HSL colour space.
 */
HslStruct AppWindow::RgbToHsl(COLORREF color) {
    // Scale the RGB values down to floating point numbers between 0 and 1.
    double r = (double)GetRValue(color) / 255;
    double g = (double)GetGValue(color) / 255;
    double b = (double)GetBValue(color) / 255;
    double rgbMin = min(min(r, g), min(g, b));
    double rgbMax = max(max(r, g), max(g, b));
    double h, s, l;

    l = (rgbMin + rgbMax) / 2;
    if (rgbMin == rgbMax) {
      h = 0;
      s = 0;
    }
    else {
      if (l <= 0.5) {
        s = (rgbMax - rgbMin) / (rgbMax + rgbMin);
      }
      else {
        s = (rgbMax - rgbMin) / (2.0 - rgbMax - rgbMin);
      }

      if (r == rgbMax) {
        h = (g - b) / (rgbMax - rgbMin);
      }
      else if (g == rgbMax) {
        h = 2.0 + (b - r) / (rgbMax - rgbMin);
      }
      else {
        h = 4.0 + (r - g) / (rgbMax - rgbMin);
      }
    }
    if (h < 0) {
      h += 6;
    }

    // Scale the HSL values up to the 0-255 range used by NI.
    HslStruct hsl;
    hsl.h = (unsigned char)(h * 42.5);
    hsl.s = (unsigned char)(s * 255);
    hsl.l = (unsigned char)(l * 255);

    return hsl;
}
