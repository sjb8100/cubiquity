#ifndef CUBIQUITY_VOLUMESERIALISATION_H_
#define CUBIQUITY_VOLUMESERIALISATION_H_

#include "ColouredCubesVolume.h"
#include "CubiquityForwardDeclarations.h"
#include "SmoothTerrainVolume.h"
#include "UpdatePriorities.h"

#include "PolyVoxCore\Impl\ErrorHandling.h"

#define STBI_HEADER_FILE_ONLY
#include "stb_image.c"
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "boost/filesystem.hpp"

#include <climits>
#include <map>

namespace Cubiquity
{
	std::map<std::string, std::string> parseIndexFile(const std::string& filename);
	void convertStringToInt(const std::string& str, int& i);

	ColouredCubesVolume* importColouredSlices(std::string folder);
	void exportSlices(ColouredCubesVolume* volume, std::string folder);

	SmoothTerrainVolume* importSmoothSlices(std::string folder);
	void exportSlices(SmoothTerrainVolume* volume, std::string folder);

	template <typename VoxelType>
	void pixelToVoxel(uint8_t* pixelData, VoxelType& voxelData, uint32_t componentCount)
	{
		POLYVOX_ASSERT(false, "NOT IMPLEMENTED");
	}

	void pixelToVoxel(uint8_t* pixelData, Colour& voxelData, uint32_t componentCount);
	void pixelToVoxel(uint8_t* pixelData, MultiMaterial& voxelData, uint32_t componentCount);

	template <typename VoxelType>
	void voxelToPixel(VoxelType& voxelData, uint8_t* pixelData, uint32_t componentCount)
	{
		POLYVOX_ASSERT(false, "NOT IMPLEMENTED");
	}

	void voxelToPixel(Colour& voxelData, uint8_t* pixelData, uint32_t componentCount);
	void voxelToPixel(MultiMaterial& voxelData, uint8_t* pixelData, uint32_t componentCount);

	ColouredCubesVolume* importVxl(const std::string& filename, const std::string& pageFolder);

	// --------------------------------------------------
	// Imports data in the VolDat format.
	// --------------------------------------------------
	template <typename CubiquityVolumeType>
	CubiquityVolumeType* importVolDat(std::string folder, const std::string& pageFolder, uint32_t baseNodeSize)
	{
		if((folder.back() != '/') && (folder.back() != '\\'))
		{
			logWarning() << "Folder name " << folder << " is missing a trailing '/' or '\\'. Please to provide this to avoid confusion!";
			folder.append("/");
		}

		std::string indexFileName(folder);
		indexFileName = indexFileName + "Volume.idx";
		std::map<std::string, std::string> index = parseIndexFile(indexFileName);

		//Create the volume
		int volumeWidth;    convertStringToInt(index["Width"], volumeWidth);
		int volumeHeight;   convertStringToInt(index["Height"], volumeHeight);
		int sliceCount;     convertStringToInt(index["SliceCount"], sliceCount);
		int componentCount; convertStringToInt(index["ComponentCount"], componentCount);

		// When importing we treat 'y' as up because the Gameplay physics engine makes some
		// assumptions about this. This means we need to swap the 'y' and 'slice' indices.
		CubiquityVolumeType* volume = new CubiquityVolumeType(Region(0, 0, 0, volumeWidth - 1, sliceCount - 1, volumeHeight - 1), pageFolder, baseNodeSize);

		// Now iterate over each slice and import the data.
		for(int slice = 0; slice < sliceCount; slice++)
		{
			std::stringstream ss;
			ss << folder << std::setfill('0') << std::setw(6) << slice << "." << index["SliceExtension"];
			std::string imageFileName = ss.str();

			int imageWidth = 0, imageHeight = 0, imageChannels;
			unsigned char *sliceData = stbi_load(imageFileName.c_str(), &imageWidth, &imageHeight, &imageChannels, 0);
			assert(imageWidth == volumeWidth);
			assert(imageHeight == volumeHeight);
			assert(imageChannels == componentCount);

			// Now iterate over each pixel.
			for(int x = 0; x < imageWidth; x++)
			{
				for(int y = 0; y < imageHeight; y++)
				{
					unsigned char *pixel = sliceData + (y * imageWidth + x) * imageChannels;

					CubiquityVolumeType::VoxelType voxel;
					pixelToVoxel(pixel, voxel, componentCount);

					// When importing we treat 'y' as up because the Gameplay physics engine makes some
					// assumptions about this. This means we need to swap the 'y' and 'slice' indices.
					volume->setVoxelAt(x, slice, y, voxel, UpdatePriorities::DontUpdate);
				}
			}
		}

		volume->markAsModified(volume->getEnclosingRegion(), UpdatePriorities::Background);

		return volume;
	}

	// --------------------------------------------------
	// Exports data in the VolDat format.
	// --------------------------------------------------
	template <typename CubiquityVolumeType>
	void exportVolDat(CubiquityVolumeType* volume, std::string folder)
	{
		boost::filesystem::path path(folder);
		if((!is_directory(path)) && (!boost::filesystem::create_directory(path)))
		{
			POLYVOX_THROW(std::runtime_error, "Failed to create directory \'" + folder + "\'for export");
		}

		// Note that 'y' and 'z' axis are flipped as Gameplay physics engine assumes 'y' is up.
		uint32_t imageWidth = volume->getWidth();
		uint32_t imageHeight = volume->getDepth();
		uint32_t sliceCount = volume->getHeight();
		std::string sliceExtension("png");
		uint32_t componentCount = 4;
		std::string componentType("u");
		uint32_t componentSize = 8;

		int outputSliceDataSize = imageWidth * imageHeight * componentCount * (componentSize / CHAR_BIT);
		unsigned char* outputSliceData = new unsigned char[outputSliceDataSize];

		for(uint32_t slice = 0; slice < sliceCount; slice++)
		{
			std::fill(outputSliceData, outputSliceData + imageWidth * imageHeight, 0);

			for(uint32_t x = 0; x < imageWidth; x++)
			{
				for(uint32_t y = 0; y < imageHeight; y++)
				{
					unsigned char* pixel = outputSliceData + (y * imageWidth + x) * componentCount;

					// Note that 'y' and 'z' axis are flipped as Gameplay physics engine assumes 'y' is up.
					CubiquityVolumeType::VoxelType voxel = volume->getVoxelAt(x, slice, y);

					voxelToPixel(voxel, pixel, componentCount);
				}
			}

			// Now save the slice data as an image file.
			std::stringstream ss;
			ss << folder << std::setfill('0') << std::setw(6) << slice << "." << sliceExtension;
			int result = stbi_write_png(ss.str().c_str(), imageWidth, imageHeight, componentCount, outputSliceData, imageWidth * componentCount);
			assert(result); //If crashing here then make sure the output folder exists.
		}

		delete[] outputSliceData;

		FILE *fp;
		fp=fopen((folder + "Volume.idx").c_str(), "w");
		fprintf(fp, "Width = %d\n", imageWidth);
		fprintf(fp, "Height = %d\n", imageHeight);
		fprintf(fp, "SliceCount = %d\n", sliceCount);
		fprintf(fp, "SliceExtension = %s\n", sliceExtension.c_str());
		fprintf(fp, "ComponentCount = %d\n", componentCount);
		fprintf(fp, "ComponentType = %s\n", componentType.c_str());
		fprintf(fp, "ComponentSize = %d\n", componentSize);
		
		fclose(fp);
	}
}

#endif //CUBIQUITY_VOLUMESERIALISATION_H_
