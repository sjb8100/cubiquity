#include "ExportImageSlices.h"

#include "HeaderOnlyLibs.h"

#include "CubiquityC.h"

#include <iomanip>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <vector>

using namespace std;

bool exportImageSlices(ez::ezOptionParser& options)
{
	LOG(INFO) << "Exporting as image slices...";

	string folder;
	options.get("-imageslices")->getString(folder);
	string pathToVoxelDatabase;
	options.get("-coloredcubes")->getString(pathToVoxelDatabase);

	cout << "Exporting images from '" << pathToVoxelDatabase << "' and into '" << folder << "'";

	uint32_t volumeHandle = 0;
	if (cuNewColoredCubesVolumeFromVDB(pathToVoxelDatabase.c_str(), CU_READWRITE, 32, &volumeHandle) != CU_OK)
	{
		cerr << "Error opening VDB database" << endl;
		return EXIT_FAILURE;
	}

	int lowerX, lowerY, lowerZ, upperX, upperY, upperZ;
	if (cuGetEnclosingRegion(volumeHandle, &lowerX, &lowerY, &lowerZ, &upperX, &upperY, &upperZ) != CU_OK)
	{
		cerr << "Error geting enclosing region" << endl;
		return EXIT_FAILURE;
	}

	// Note that 'y' and 'z' axis are flipped as Gameplay physics engine assumes 'y' is up.
	uint32_t imageWidth = upperX - lowerX + 1;
	uint32_t imageHeight = upperZ - lowerZ + 1;
	uint32_t sliceCount = upperY - lowerY + 1;
	std::string sliceExtension("png");
	uint32_t componentCount = 4;
	std::string componentType("u");
	uint32_t componentSize = 8;

	int outputSliceDataSize = imageWidth * imageHeight * componentCount * (componentSize / CHAR_BIT);
	unsigned char* outputSliceData = new unsigned char[outputSliceDataSize];

	for (uint32_t slice = 0; slice < sliceCount; slice++)
	{
		std::fill(outputSliceData, outputSliceData + imageWidth * imageHeight, 0);

		for (uint32_t x = 0; x < imageWidth; x++)
		{
			for (uint32_t z = 0; z < imageHeight; z++)
			{
				unsigned char* pixelData = outputSliceData + (z * imageWidth + x) * componentCount;

				CuColor color;
				if (cuGetVoxel(volumeHandle, x + lowerX, slice + lowerY, z + lowerZ, &color) != CU_OK)
				{
					cerr << "Error getting voxel value" << endl;
					return EXIT_FAILURE;
				}

				cuGetAllComponents(color, pixelData + 0, pixelData + 1, pixelData + 2, pixelData + 3);
			}
		}

		// Now save the slice data as an image file.
		std::stringstream ss;
		ss << folder << "/" << std::setfill('0') << std::setw(6) << slice << "." << sliceExtension;
		int result = stbi_write_png(ss.str().c_str(), imageWidth, imageHeight, componentCount, outputSliceData, imageWidth * componentCount);
		if (result == 0)
		{
			cerr << "Failed to write " << ss.str() << endl;
		}
	}

	delete[] outputSliceData;

	return false;
}