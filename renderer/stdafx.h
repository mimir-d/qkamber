// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#   include <objidl.h>
#   include <gdiplus.h>
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <random>
#include <memory>
#include <cstdint>

#include "logger.h"