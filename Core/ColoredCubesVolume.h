#ifndef COLOUREDCUBESVOLUME_H_
#define COLOUREDCUBESVOLUME_H_

#include "Color.h"
#include "Cubiquity.h"
#include "CubiquityForwardDeclarations.h"
#include "Volume.h"

namespace Cubiquity
{
	class ColoredCubesVolume : public Volume<Color>
	{
	public:
		typedef Color VoxelType;

		ColoredCubesVolume(const Region& region, const std::string& pathToVoxelDatabase, unsigned int baseNodeSize)
			:Volume<Color>(region, pathToVoxelDatabase, baseNodeSize)
		{
			mOctree = new Octree<VoxelType>(this, OctreeConstructionModes::BoundVoxels, baseNodeSize);
		}

		virtual ~ColoredCubesVolume()
		{
			delete mOctree;
		}
	};
}

#endif //COLOUREDCUBESVOLUME_H_
