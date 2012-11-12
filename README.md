FRC Camera Test
===============

## Overview

FRC Camera Test is a C++ Windows framework for robotless testing of the NI Vision library with the Axis
camera. It makes it possible to develop and test camera code for FRC robots without access to the control
system.

## Compiling

This application was written using Microsoft Visual C++ 2005 Express Edition,
but any Windows-capable C++ build platform should work.

In order to build this project, the directory containing nivision.h (usually
C:\Program Files\National Instruments\Vision\Include) must be added to the list
of include directories. Similarly, the directory containing nivision.lib
(usually C:\Program Files\National Instruments\Vision\Lib\MSVC) must be added to
the list of library directories, and wsock32.lib and nivision.lib must be
specified as dependencies.

## Usage

The application can be customized by changing the parameters in Constants.h and
the class used to process images in Camera.cpp. Users can write their own image
processing functions by deriving from the ImageProcessor class, or by modifying
the existing ones (ColorThreshold and ColorPlaneExtraction).
