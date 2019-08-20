#pragma once

#define INVALID -1

#define DATA_PATH(filename) "../SphericalHarmonicsLighting/Data/" filename

#define BOX_COUNT 100

#define PI          3.14159265359f
#define ONE_OVER_PI 0.31830988618f

#define DEG_TO_RAD(angle) ((angle) * PI / 180.0f)
#define RAD_TO_DEG(angle) ((angle) / PI * 180.0f)

#define TRAP __debugbreak()
