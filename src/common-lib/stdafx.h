#pragma once

#define WIN32_LEAN_AND_MEAN
#define _SILENCE_PARALLEL_ALGORITHMS_EXPERIMENTAL_WARNING

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <execution>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include <windows.h>
#include <d2d1.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <DirectXPackedVector.h>
#include <wrl.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
