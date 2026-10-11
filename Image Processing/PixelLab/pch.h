// pch.h - Pixel Lab Precompiled Header
// This header is compiled ONCE into the PCH (pch.obj) by pch.cpp.
// All PixelLab translation units use /Yu"pch.h" so that Image_Class.h
// (and its STB_IMAGE_IMPLEMENTATION) is compiled exactly once.
// main.cpp uses /FI pch.h to receive the PCH without modification.
#pragma once

// Standard Library
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <string>
#include <sstream>
#include <cassert>
#include <cstring>
#include <cstdio>

// Image class + STB implementations
// We define STB_IMAGE_STATIC and STB_IMAGE_WRITE_STATIC to prevent LNK2005 
// (multiply defined symbols) across TUs, as Image_Class.h forces implementations.
#define STB_IMAGE_STATIC
#define STB_IMAGE_WRITE_STATIC
#include "Image_Class.h"
