#include "SmoothSurfaceExtractionTask.h"

#include "ColouredCubesIsQuadNeeded.h"

#include "SmoothTerrainVolume.h" //Temporary

#include "PolyVoxCore/RawVolume.h"

using namespace PolyVox;

SmoothSurfaceExtractionTask::SmoothSurfaceExtractionTask(OctreeNode< typename VoxelTraits<MultiMaterial4>::VertexType >* octreeNode, PolyVox::SimpleVolume<typename MultiMaterialMarchingCubesController<MultiMaterial4>::MaterialType>* polyVoxVolume)
	:mOctreeNode(octreeNode)
	,mPolyVoxVolume(polyVoxVolume)
	,mSmoothMesh(0)
{
}

SmoothSurfaceExtractionTask::~SmoothSurfaceExtractionTask()
{
	//Should delete mesh here?
}

void SmoothSurfaceExtractionTask::process(void)
{
	//Extract the surface
	mSmoothMesh = new PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal< typename MultiMaterialMarchingCubesController<MultiMaterial4>::MaterialType > >;

	generateSmoothMesh(mOctreeNode->mRegion, mOctreeNode->mLodLevel, mSmoothMesh);
}

void SmoothSurfaceExtractionTask::generateSmoothMesh(const PolyVox::Region& region, uint32_t lodLevel, PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal< typename MultiMaterialMarchingCubesController<MultiMaterial4>::MaterialType > >* resultMesh)
{
	MultiMaterialMarchingCubesController<MultiMaterial4> controller;

	if(lodLevel == 0)
	{
		//SurfaceMesh<PositionMaterialNormal< typename MultiMaterialMarchingCubesController<VoxelType>::MaterialType > > mesh;
		PolyVox::MarchingCubesSurfaceExtractor< PolyVox::SimpleVolume<MultiMaterial4>, MultiMaterialMarchingCubesController<MultiMaterial4> > surfaceExtractor(mPolyVoxVolume, region, resultMesh, PolyVox::WrapModes::Border, MultiMaterial4(0), controller);
		surfaceExtractor.execute();
	}
	else
	{
		uint32_t downSampleFactor = 0x0001 << lodLevel;

		int crackHidingFactor = 5; //This should probably be configurable?
		controller.setThreshold(controller.getThreshold() + (downSampleFactor * crackHidingFactor));

		PolyVox::Region highRegion = region;
		highRegion.grow(downSampleFactor, downSampleFactor, downSampleFactor);

		PolyVox::Region lowRegion = highRegion;
		PolyVox::Vector3DInt32 lowerCorner = lowRegion.getLowerCorner();
		PolyVox::Vector3DInt32 upperCorner = lowRegion.getUpperCorner();

		upperCorner = upperCorner - lowerCorner;
		upperCorner = upperCorner / static_cast<int32_t>(downSampleFactor);
		upperCorner = upperCorner + lowerCorner;
		lowRegion.setUpperCorner(upperCorner);

		PolyVox::RawVolume<MultiMaterial4> resampledVolume(lowRegion);

		resampleVolume(downSampleFactor, mPolyVoxVolume, highRegion, &resampledVolume, lowRegion);

		lowRegion.shrink(1, 1, 1);

		PolyVox::MarchingCubesSurfaceExtractor< PolyVox::RawVolume<MultiMaterial4>, MultiMaterialMarchingCubesController<MultiMaterial4> > surfaceExtractor(&resampledVolume, lowRegion, resultMesh, PolyVox::WrapModes::Border, MultiMaterial4(0), controller);
		surfaceExtractor.execute();

		resultMesh->scaleVertices(static_cast<float>(downSampleFactor));

		recalculateMaterials(resultMesh, static_cast<PolyVox::Vector3DFloat>(mOctreeNode->mRegion.getLowerCorner()), mPolyVoxVolume);
	}
}

void recalculateMaterials(PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal< typename MultiMaterialMarchingCubesController<MultiMaterial4>::MaterialType > >* mesh, const PolyVox::Vector3DFloat& meshOffset,  PolyVox::SimpleVolume<MultiMaterial4>* volume)
{
	std::vector< PositionMaterialNormal< typename MultiMaterialMarchingCubesController<MultiMaterial4>::MaterialType > >& vertices = mesh->getRawVertexData();
	for(uint32_t ct = 0; ct < vertices.size(); ct++)
	{
		const Vector3DFloat& vertexPos = vertices[ct].getPosition() + meshOffset;
		MultiMaterial4 value = getInterpolatedValue(volume, vertexPos);

		// It seems that sometimes the vertices can fall in an empty cell. The reason for this
		// isn't clear but it might be inaccuraceies in the lower LOD mesh. It also might only 
		// happen right on the edge of the volume so wrap modes might help. Hopefully we can
		// remove this hack in the future.
		Vector<4, float> matAsVec = value;
		if(matAsVec.length() < 0.001f)
		{
			value = MultiMaterial4(0);
			value.setMaterial(0, 255);
		}

		vertices[ct].setMaterial(value);
	}
}


MultiMaterial4 getInterpolatedValue(PolyVox::SimpleVolume<MultiMaterial4>* volume, const PolyVox::Vector3DFloat& position)
{
	PolyVox::SimpleVolume<MultiMaterial4>::Sampler sampler(volume);

	int32_t iLowerX = PolyVox::roundTowardsNegInf(position.getX());
	int32_t iLowerY = PolyVox::roundTowardsNegInf(position.getY());
	int32_t iLowerZ = PolyVox::roundTowardsNegInf(position.getZ());

	float fOffsetX = position.getX() - iLowerX;
	float fOffsetY = position.getY() - iLowerY;
	float fOffsetZ = position.getZ() - iLowerZ;

	/*int32_t iCeilX = iFloorX + 1;
	int32_t iCeilY = iFloorY + 1;
	int32_t iCeilZ = iFloorZ + 1;*/

	sampler.setPosition(iLowerX, iLowerY, iLowerZ);

	MultiMaterial4 v000 = sampler.peekVoxel0px0py0pz();
	MultiMaterial4 v100 = sampler.peekVoxel1px0py0pz();
	MultiMaterial4 v010 = sampler.peekVoxel0px1py0pz();
	MultiMaterial4 v110 = sampler.peekVoxel1px1py0pz();
	MultiMaterial4 v001 = sampler.peekVoxel0px0py1pz();
	MultiMaterial4 v101 = sampler.peekVoxel1px0py1pz();
	MultiMaterial4 v011 = sampler.peekVoxel0px1py1pz();
	MultiMaterial4 v111 = sampler.peekVoxel1px1py1pz();

	MultiMaterial4 result = trilerp(v000, v100, v010, v110, v001, v101, v011, v111, fOffsetX, fOffsetY, fOffsetZ);

	return result;
}