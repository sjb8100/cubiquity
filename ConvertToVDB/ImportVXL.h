#ifndef CUBIQUITYTOOLS_IMPORTVXL_H_
#define CUBIQUITYTOOLS_IMPORTVXL_H_

#include <string>

// Read AoS/B&S map - based on: http://silverspaceship.com/aosmap/aos_file_format.html
bool importVxl(const std::string& vxlFilename, const std::string& pathToVoxelDatabase, bool dryRun = false);

#endif //CUBIQUITYTOOLS_IMPORTVXL_H_