/*
 * Copyright 2009-2010 Patrick Fairbank. All Rights Reserved.
 * See LICENSE.TXT for licensing information.
 *
 * Contains configuration parameters for the entire project.
 */

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

// Parameters for the Axis Camera.
#define FRAMES_PER_SECOND 5
#define COMPRESSION 20
#define RESOLUTION "640x280"
#define ROTATION 0
#define IP_ADDRESS "192.168.0.90"
#define PORT 80
#define AUTHENTICATION "RlJDOkZSQw==" // Username 'FRC', password 'FRC'.

// Camera white balance (auto, fixed_fluor2, fixed_indoor, fixed_outdoor1, fixed_outdoor2,
// fixed_fluor1, fixed_fluor2, or hold).
#define WHITE_BALANCE "fixed_fluor2"

// Camera exposure level (auto, flickerfree50, flickerfree60, hold).
#define EXPOSURE "hold"

// Camera exposure priority (0 - prioritize quality, 50 - none, 100 - prioritize framerate).
#define EXPOSURE_PRIORITY 0

#define BRIGHTNESS 50
#define COLOR_LEVEL 50

// Parameters for the application window.
#define CLASSNAME "AppWindow"
#define APPNAME "FRC Camera Test v1.0"
#define WIDTH 1280
#define HEIGHT 680

#endif // _CONSTANTS_H_
