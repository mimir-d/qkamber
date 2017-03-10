// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#   include <windowsx.h>
#   include <ntddkbd.h>
#   include <objidl.h>
// NOTE: these are defined by winapi and shadow std::min/max
#   undef min
#   undef max
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <array>
#include <string>
#include <vector>
#include <unordered_map>

#include <random>
#include <memory>
#include <chrono>

#include <cstdint>

#include "misc.h"
#include "logger.h"