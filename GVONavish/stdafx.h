// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B
//

#pragma once

#include "targetver.h"

#define _CRT_SECURE_NO_WARNINGS
#ifdef NDEBUG
#define _SECURE_SCL 0
#endif
#define WIN32_LEAN_AND_MEAN             // Windows �w�b�_�[����g�p����Ă��Ȃ����������O���܂��B
#define NOMINMAX
#include <algorithm>
using std::min;
using std::max;

// Windows �w�b�_�[ �t�@�C��:
#include <windows.h>
#include <objbase.h>
#include <MMSystem.h>
#pragma comment(lib, "winmm.lib")
#include <gl/GL.h>
#pragma comment(lib, "opengl32.lib")
#include <gl/GLU.h>
#pragma comment(lib, "glu32.lib")
#include <CommCtrl.h>

// C �����^�C�� �w�b�_�[ �t�@�C��
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
