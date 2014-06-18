#ifndef CUBIQUITYTOOLS_IMPORTMAGICAVOXEL_H_
#define CUBIQUITYTOOLS_IMPORTMAGICAVOXEL_H_

#include <string>

// Check is this is a Magica Voxel file.
bool isMagicaVoxel(const std::string& filename);
// Read MagicaVoxel map: https://voxel.codeplex.com/
bool importMagicaVoxel(const std::string& filename, const std::string& pathToVoxelDatabase);

#endif //CUBIQUITYTOOLS_IMPORTMAGICAVOXEL_H_