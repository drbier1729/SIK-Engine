#include "stdafx.h"

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_HDR

#ifdef _DEBUG
#define STBI_FAILURE_USERMSG
#else
#define STBI_NO_FAILURE_STRINGS
#endif

//stb_image
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
