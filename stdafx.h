#pragma once

#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#pragma comment(lib, "d3dcompiler.lib")

#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
#include <DirectXColors.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <unordered_map>
#include <WindowsX.h>
#include <process.h>
#include <Xaudio2.h>
#include <windows.h>
#include <dxgi1_4.h>
#include <algorithm>
#include <comdef.h>
#include <stdlib.h>
#include <malloc.h>
#include <d3d12.h>
#include <tchar.h>
#include <float.h>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <time.h>
#include <array>
#include <wrl.h>
#include <cmath>

#include "d3dx12.h"

#include <d3d11on12.h>
#include <shellapi.h>
#include <d2d1_3.h>
#include <dwrite.h>
#include <pix.h>

static const UINT FRAME = 2;

static const UINT THREADNUM = 3;

static const UINT FINAL_LEVEL = 9;

static const UINT X_SIZE = 5;
static const UINT Y_SIZE = 5;
static const UINT Z_SIZE = 12;