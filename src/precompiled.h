
#pragma once

#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#   include <ntddkbd.h>
// NOTE: these are defined by winapi and shadow std::min/max
#   undef min
#   undef max
#else
    #include "SDL.h"
    #include "SDL_ttf.h"
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <array>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include <random>
#include <new>
#include <memory>
#include <chrono>
#include <numeric>

#include <cstdint>

#include "misc.h"
#include "logger.h"
