// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define _CRT_SECURE_NO_WARNINGS
#ifdef NDEBUG
#define _SECURE_SCL 0
#endif
#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
#define NOMINMAX
#include <algorithm>
using std::min;
using std::max;

// Windows ヘッダー ファイル:
#include <windows.h>
#include <objbase.h>
#include <MMSystem.h>
#pragma comment(lib, "winmm.lib")
#include <gl/GL.h>
#pragma comment(lib, "opengl32.lib")
#include <gl/GLU.h>
#pragma comment(lib, "glu32.lib")
#include <CommCtrl.h>

// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <cstdint>
#include <numeric>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <deque>
