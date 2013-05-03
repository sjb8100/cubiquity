#include <stdint.h> //C99 fixed size data types.

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CUBIQUITYC_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CUBIQUITYC_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef CUBIQUITYC_EXPORTS
#define CUBIQUITYC_API __declspec(dllexport)
#else
#define CUBIQUITYC_API __declspec(dllimport)
#endif

extern "C"
{
	// Volume functions
	CUBIQUITYC_API int32_t cuNewColouredCubesVolume(int32_t lowerX, int32_t lowerY, int32_t lowerZ, int32_t upperX, int32_t upperY, int32_t upperZ, uint32_t blockSize, uint32_t baseNodeSize);
	CUBIQUITYC_API int32_t cuNewColouredCubesVolumeFromVolDat(const char* volDatToImport, uint32_t blockSize, uint32_t baseNodeSize);
	CUBIQUITYC_API void cuUpdateVolume(int32_t volumeHandle);
	CUBIQUITYC_API void cuDeleteColouredCubesVolume(int32_t volumeHandle);

	// Octree functions
	CUBIQUITYC_API int32_t cuGetRootOctreeNode(int32_t volumeHandle);

	// Mesh functions
	CUBIQUITYC_API uint32_t cuGetNoOfVertices(int32_t volumeHandle, int32_t octreeNodeHandle);
	CUBIQUITYC_API uint32_t cuGetNoOfIndices(int32_t volumeHandle, int32_t octreeNodeHandle);

	CUBIQUITYC_API float* cuGetVertices(int32_t volumeHandle, int32_t octreeNodeHandle);
	CUBIQUITYC_API uint32_t* cuGetIndices(int32_t volumeHandle, int32_t octreeNodeHandle);
}
