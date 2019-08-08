#pragma once

#define INVALID -1

#define DATA_PATH(filename) "../SphericalHarmonicsLighting/Data/" filename

#define BOX_COUNT 100

#define PI 3.14159265359f

#define DEG_TO_RAD(angle) ((angle) * PI / 180.0f)
#define RAD_TO_DEG(angle) ((angle) / PI * 180.0f)

#define TRAP _asm { int 3 }
