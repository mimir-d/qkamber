// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#   include <objidl.h>
#   include <gdiplus.h>
#endif

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <array>
#include <random>
#include <memory>

#include "logger.h"