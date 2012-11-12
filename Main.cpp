/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Contains initialization and main loop of application.
 */

#include "AppWindow.h"
#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
  // Create and display the main window.
  AppWindow mainWindow;
  mainWindow.Create(hInstance);
  ShowWindow(mainWindow.GetHWnd(), iCmdShow);
  UpdateWindow(mainWindow.GetHWnd());

  // Stay in the message loop until the application quits.
  MSG message;
  while (GetMessage(&message, NULL, 0, 0)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  
  return (int)message.wParam;
}
