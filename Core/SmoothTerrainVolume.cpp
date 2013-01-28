#include "SmoothTerrainVolume.h"

#include <algorithm>

using namespace std;
using namespace PolyVox;

SmoothTerrainVolume::SmoothTerrainVolume(int lowerX, int lowerY, int lowerZ, int upperX, int upperY, int upperZ, unsigned int blockSize, unsigned int baseNodeSize)
	:Volume<MultiMaterial4>(lowerX, lowerY, lowerZ, upperX, upperY, upperZ, blockSize, OctreeConstructionModes::BoundCells, baseNodeSize)
{
}


void SmoothTerrainVolume::updateMeshImpl(OctreeNode* volReg)
{
	PolyVox::Region lod0Region = volReg->mRegion;

	//Extract the surface
	PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal< typename MultiMaterialMarchingCubesController<VoxelType>::MaterialType > >* mesh = new PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal< typename MultiMaterialMarchingCubesController<VoxelType>::MaterialType > >;

	generateSmoothMesh(lod0Region, volReg->mLodLevel, mesh);

	if(volReg->mLodLevel > 0)
	{
		recalculateMaterials(mesh, static_cast<PolyVox::Vector3DFloat>(lod0Region.getLowerCorner()), mVolData);
	}

	if(mesh->getNoOfIndices() > 0)
	{
		volReg->buildGraphicsMesh(mesh/*, 0*/);
	}
}

void SmoothTerrainVolume::generateSmoothMesh(const PolyVox::Region& region, uint32_t lodLevel, PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal< typename MultiMaterialMarchingCubesController<VoxelType>::MaterialType > >* resultMesh)
{
	MultiMaterialMarchingCubesController<VoxelType> controller;

	if(lodLevel == 0)
	{
		//SurfaceMesh<PositionMaterialNormal< typename MultiMaterialMarchingCubesController<VoxelType>::MaterialType > > mesh;
		PolyVox::MarchingCubesSurfaceExtractor< PolyVox::RawVolume<VoxelType>, MultiMaterialMarchingCubesController<VoxelType> > surfaceExtractor(mVolData, region, resultMesh, PolyVox::WrapModes::Border, VoxelType(0), controller);
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

		PolyVox::RawVolume<VoxelType> resampledVolume(lowRegion);
		//lod1Volume.m_bClampInsteadOfBorder = true; //We're extracting right to the edge of our small volume, so this keeps the normals correct(ish)
		//PolyVox::VolumeResampler< PolyVox::RawVolume<VoxelType>, PolyVox::RawVolume<VoxelType> > volumeResampler(mVolData, highRegion, &resampledVolume, lowRegion);
		//volumeResampler.execute();

		resampleVolume(downSampleFactor, mVolData, highRegion, &resampledVolume, lowRegion);

		lowRegion.shrink(1, 1, 1);

		PolyVox::MarchingCubesSurfaceExtractor< PolyVox::RawVolume<VoxelType>, MultiMaterialMarchingCubesController<VoxelType> > surfaceExtractor(&resampledVolume, lowRegion, resultMesh, PolyVox::WrapModes::Border, VoxelType(0), controller);
		surfaceExtractor.execute();

		resultMesh->scaleVertices(downSampleFactor);
	}
}

void SmoothTerrainVolume::recalculateMaterials(PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal< typename MultiMaterialMarchingCubesController<MultiMaterial4>::MaterialType > >* mesh, const PolyVox::Vector3DFloat& meshOffset,  PolyVox::RawVolume<MultiMaterial4>* volume)
{
	std::vector< PositionMaterialNormal< typename MultiMaterialMarchingCubesController<VoxelType>::MaterialType > >& vertices = mesh->getRawVertexData();
	for(int ct = 0; ct < vertices.size(); ct++)
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
			value = VoxelType(0);
			value.setMaterial(0, 255);
		}

		vertices[ct].setMaterial(value);
	}
}

MultiMaterial4 SmoothTerrainVolume::getInterpolatedValue(PolyVox::RawVolume<MultiMaterial4>* volume, const PolyVox::Vector3DFloat& position)
{
	PolyVox::RawVolume<MultiMaterial4>::Sampler sampler(volume);

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

	VoxelType v000 = sampler.peekVoxel0px0py0pz();
	VoxelType v100 = sampler.peekVoxel1px0py0pz();
	VoxelType v010 = sampler.peekVoxel0px1py0pz();
	VoxelType v110 = sampler.peekVoxel1px1py0pz();
	VoxelType v001 = sampler.peekVoxel0px0py1pz();
	VoxelType v101 = sampler.peekVoxel1px0py1pz();
	VoxelType v011 = sampler.peekVoxel0px1py1pz();
	VoxelType v111 = sampler.peekVoxel1px1py1pz();

	VoxelType result = trilerp(v000, v100, v010, v110, v001, v101, v011, v111, fOffsetX, fOffsetY, fOffsetZ);

	return result;
}

void SmoothTerrainVolume::resampleVolume(uint32_t factor, RawVolume<MultiMaterial4>* srcVolume, const Region& srcRegion, RawVolume<MultiMaterial4>* dstVolume, const Region& dstRegion)
{
	POLYVOX_ASSERT(srcRegion.getWidthInCells() == dstRegion.getWidthInCells() * factor, "Destination volume must be half the size of source volume");
	POLYVOX_ASSERT(srcRegion.getHeightInCells() == dstRegion.getHeightInCells() * factor, "Destination volume must be half the size of source volume");
	POLYVOX_ASSERT(srcRegion.getDepthInCells() == dstRegion.getDepthInCells() * factor, "Destination volume must be half the size of source volume");

	for(int32_t dz = dstRegion.getLowerCorner().getZ(); dz <= dstRegion.getUpperCorner().getZ(); dz++)
	{
		for(int32_t dy = dstRegion.getLowerCorner().getY(); dy <= dstRegion.getUpperCorner().getY(); dy++)
		{
			for(int32_t dx = dstRegion.getLowerCorner().getX(); dx <= dstRegion.getUpperCorner().getX(); dx++)
			{
				int32_t sx = (dx - dstRegion.getLowerCorner().getX()) * factor + srcRegion.getLowerCorner().getX();
				int32_t sy = (dy - dstRegion.getLowerCorner().getY()) * factor + srcRegion.getLowerCorner().getY();
				int32_t sz = (dz - dstRegion.getLowerCorner().getZ()) * factor + srcRegion.getLowerCorner().getZ();

				const MultiMaterial4& srcVoxel = srcVolume->getVoxelWithWrapping(sx,sy,sz);
				dstVolume->setVoxelAt(dx,dy,dz,srcVoxel);
			}
		}
	}
}