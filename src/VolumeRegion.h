#ifndef VOLUME_REGION_H_
#define VOLUME_REGION_H_

#include "Node.h"
#include "Ref.h"

#include "Colour.h"

#include "GameplayMarchingCubesController.h"
#include "MultiMaterial.h"

#include "PolyVoxCore/Material.h"
#include "PolyVoxCore/MaterialDensityPair.h"
#include "PolyVoxCore/Region.h"
#include "PolyVoxCore/SurfaceMesh.h"

template <typename VoxelType>
class Volume;

class VolumeRegion
{
public:	
	VolumeRegion(PolyVox::Region region, VolumeRegion* parentRegion, uint32_t lodLevel);
	~VolumeRegion();

	// Builds the graphics mesh for cubic terrain (Colour material type and PositionMaterial vertex type)
	void buildGraphicsMesh(const PolyVox::SurfaceMesh<PolyVox::PositionMaterial<Colour> >& polyVoxMesh);
	// Builds the graphics mesh for smooth terrain (MultiMaterial material type and PositionMaterialNormal vertex type)
	void buildGraphicsMesh(const PolyVox::SurfaceMesh< PolyVox::PositionMaterialNormal< GameplayMarchingCubesController< MultiMaterial4 >::MaterialType > >& polyVoxMesh);

	// Although we never use Colour with PositionMaterialNormal or MultiMaterial with PositionMaterial the
	// compiler still needs these to exist because it compiles all combinations. So we just provide dummy functions.
	void buildGraphicsMesh(const PolyVox::SurfaceMesh<PolyVox::PositionMaterial<MultiMaterial4> >& polyVoxMesh);
	void buildGraphicsMesh(const PolyVox::SurfaceMesh< PolyVox::PositionMaterialNormal< Colour > >& polyVoxMesh);

	void setMaterial(const char* material); //Should be const material - fix gameplay

	PolyVox::Region mRegion;
	gameplay::Node* mNode;
	bool mIsMeshUpToDate;
	uint32_t mLodLevel;
};

#endif //VOLUME_REGION_H_
