#pragma once

// enable C++ AMP GPU support on Windows only
#if defined(_WIN32)
#define GPU_SUPPORT 1
#elif defined(_WIN64)
#define GPU_SUPPORT 1
#endif