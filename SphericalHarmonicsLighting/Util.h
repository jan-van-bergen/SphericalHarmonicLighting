#pragma once

#define INVALID -1

#define DATA_PATH(filename) "../SphericalHarmonicsLighting/Data/" filename

#define PI          3.14159265359f
#define ONE_OVER_PI 0.31830988618f

#define DEG_TO_RAD(angle) ((angle) * PI / 180.0f)
#define RAD_TO_DEG(angle) ((angle) / PI * 180.0f)

#define TRAP __debugbreak()

#define KILO_BYTE(value) (value) * 1024
#define MEGA_BYTE(value) (value) * 1024 * 1024
#define GIGA_BYTE(value) (value) * 1024 * 1024 * 1024

// Macro indirection to allow #defines to be converted into strings
#define _TO_STRING(value) #value
#define TO_STRING(value) _TO_STRING(value)
