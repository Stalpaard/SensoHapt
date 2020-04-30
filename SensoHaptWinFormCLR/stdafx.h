//Header containing header files that don't change often (this file only has to be passed 1 time through the preprocessor in order to speedup future builds)
#pragma once
#define _USE_MATH_DEFINES
//#define NO_HARDWARE_DEBUG
//#define LOG_DEBUG

#include <Windows.h>
#include <msclr\marshal_cppstd.h>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctime>
#include <string>
#include <algorithm>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <array>
#include <assert.h>
#include <limits.h>
#include "HDU\hduMatrix.h"
#include "HDU\hduError.h"
#include "HDU\hduVector.h"
#include "HD\hdDefines.h"
#include "DSBeeper\DSBeeper.h"
#include "pugixml\pugixml.hpp"
#include "GL\glut.h"