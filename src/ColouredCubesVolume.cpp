#include "ColouredCubesVolume.h"

using namespace PolyVox;

void rescaleCubicVolume(RawVolume<Colour>* pVolSrc, const Region& regSrc, RawVolume<Colour>* pVolDst, const Region& regDst)
{
	POLYVOX_ASSERT(regSrc.getWidthInVoxels() == regDst.getWidthInVoxels() * 2, "Wrong size!");
	POLYVOX_ASSERT(regSrc.getHeightInVoxels() == regDst.getHeightInVoxels() * 2, "Wrong size!");
	POLYVOX_ASSERT(regSrc.getDepthInVoxels() == regDst.getDepthInVoxels() * 2, "Wrong size!");

	for(uint32_t z = 0; z < regDst.getDepthInVoxels(); z++)
	{
		for(uint32_t y = 0; y < regDst.getHeightInVoxels(); y++)
		{
			for(uint32_t x = 0; x < regDst.getWidthInVoxels(); x++)
			{
				Vector3DInt32 srcPos = regSrc.getLowerCorner() + (Vector3DInt32(x, y, z) * 2);
				Vector3DInt32 dstPos = regDst.getLowerCorner() + Vector3DInt32(x, y, z);
				RawVolume<Colour>::Sampler srcSampler(pVolSrc);
				//srcSampler.setPosition(srcPos);

				uint32_t noOfSolidVoxels = 0;
				uint32_t totalRed = 0;
				uint32_t totalGreen = 0;
				uint32_t totalBlue = 0;
				uint32_t totalExposedFaces = 0;

				for(int32_t childZ = -1; childZ < 3; childZ++)
				{
					for(int32_t childY = -1; childY < 3; childY++)
					{
						for(int32_t childX = -1; childX < 3; childX++)
						{
							srcSampler.setPosition(srcPos + Vector3DInt32(childX, childY, childZ));

							Colour child = srcSampler.getVoxel();

							if(child.getAlpha () > 0)
							{
								uint32_t exposedFaces = 0;
								if(srcSampler.peekVoxel0px0py1nz().getAlpha() == 0) exposedFaces++;
								if(srcSampler.peekVoxel0px0py1pz().getAlpha() == 0) exposedFaces++;
								if(srcSampler.peekVoxel0px1ny0pz().getAlpha() == 0) exposedFaces++;
								if(srcSampler.peekVoxel0px1py0pz().getAlpha() == 0) exposedFaces++;
								if(srcSampler.peekVoxel1nx0py0pz().getAlpha() == 0) exposedFaces++;
								if(srcSampler.peekVoxel1px0py0pz().getAlpha() == 0) exposedFaces++;

								totalRed += child.getRed() * exposedFaces;
								totalGreen += child.getGreen() * exposedFaces;
								totalBlue += child.getBlue() * exposedFaces;

								totalExposedFaces += exposedFaces;

								noOfSolidVoxels++;
							}							
						}
					}
				}

				if(totalExposedFaces == 0) totalExposedFaces++; //Avoid div by zero

				if(noOfSolidVoxels > 32)
				{
					Colour colour;
					colour.setColour(totalRed / totalExposedFaces, totalGreen / totalExposedFaces, totalBlue / totalExposedFaces, 15);
					pVolDst->setVoxelAt(dstPos, colour);
				}
				else
				{
					Colour colour;
					colour.setColour(0,0,0,0);
					pVolDst->setVoxelAt(dstPos, colour);
				}
			}
		}
	}
}

ColouredCubesVolume::ColouredCubesVolume(VolumeType type, int lowerX, int lowerY, int lowerZ, int upperX, int upperY, int upperZ, unsigned int regionWidth, unsigned int regionHeight, unsigned int regionDepth)
	:Volume<Colour>(type, lowerX, lowerY, lowerZ, upperX, upperY, upperZ, regionWidth, regionHeight, regionDepth)
{
	PolyVox::Region regionToCover(mVolData->getEnclosingRegion());
	if(getType() == VolumeTypes::SmoothTerrain)
	{
		regionToCover.shiftLowerCorner(-1, -1, -1);
		regionToCover.shiftUpperCorner(1, 1, 1);
	}

	GP_ASSERT(PolyVox::isPowerOf2(mBaseNodeSize));

	uint32_t largestVolumeDimensionInVoxels = std::max(regionToCover.getWidthInVoxels(), std::max(regionToCover.getHeightInVoxels(), regionToCover.getDepthInVoxels()));

	uint32_t octreeTargetSizeInVoxels = PolyVox::upperPowerOfTwo(largestVolumeDimensionInVoxels);

	uint8_t noOfLodLevels = logBase2(octreeTargetSizeInVoxels / mBaseNodeSize) + 1;

	uint32_t widthIncrease = octreeTargetSizeInVoxels - regionToCover.getWidthInVoxels();
	uint32_t heightIncrease = octreeTargetSizeInVoxels - regionToCover.getHeightInVoxels();
	uint32_t depthIncrease = octreeTargetSizeInVoxels - regionToCover.getDepthInVoxels();
	
	if(widthIncrease % 2 == 1)
	{
		upperX++;
		widthIncrease--;
	}

	if(heightIncrease % 2 == 1)
	{
		upperY++;
		heightIncrease--;
	}
	if(depthIncrease % 2 == 1)
	{
		upperZ++;
		depthIncrease--;
	}

	PolyVox::Region octreeRegion(lowerX, lowerY, lowerZ, upperX, upperY, upperZ);
	octreeRegion.grow(widthIncrease / 2, heightIncrease / 2, depthIncrease / 2);

	mRootOctreeNode = new OctreeNode(octreeRegion, 0);
	mRootOctreeNode->mLodLevel = noOfLodLevels - 1;

	buildOctreeNodeTree(mRootOctreeNode, regionToCover, false);
}

/*void ColouredCubesVolume::buildOctreeNodeTree(OctreeNode* parent, const PolyVox::Region& regionToCover)
{
	POLYVOX_ASSERT(parent->mRegion.getWidthInVoxels() == parent->mRegion.getHeightInVoxels(), "Region must be cubic");
	POLYVOX_ASSERT(parent->mRegion.getWidthInVoxels() == parent->mRegion.getDepthInVoxels(), "Region must be cubic");

	if(parent->mRegion.getWidthInVoxels() > mBaseNodeSize)
	{
		PolyVox::Vector3DInt32 baseLowerCorner = parent->mRegion.getLowerCorner();
		int32_t width = parent->mRegion.getWidthInVoxels() / 2;
		int32_t height = parent->mRegion.getHeightInVoxels() / 2;
		int32_t depth = parent->mRegion.getDepthInVoxels() / 2;
		PolyVox::Vector3DInt32 baseUpperCorner = baseLowerCorner + PolyVox::Vector3DInt32(width-1, height-1, depth-1);

		for(int z = 0; z < 2; z++)
		{
			for(int y = 0; y < 2; y++)
			{
				for(int x = 0; x < 2; x++)
				{
					PolyVox::Vector3DInt32 offset (x*width, y*height, z*depth);
					PolyVox::Region childRegion(baseLowerCorner + offset, baseUpperCorner + offset);
					if(intersects(childRegion, regionToCover))
					{
						OctreeNode* volReg = new OctreeNode(childRegion, parent);
						parent->children[x][y][z] = volReg;
						buildOctreeNodeTree(volReg, regionToCover);
					}
				}
			}
		}
	}
}*/

void ColouredCubesVolume::updateMeshImpl(OctreeNode* volReg)
{
	PolyVox::Region lod0Region = volReg->mRegion;

	//Extract the surface
	//ColouredCubesIsQuadNeeded<VoxelType> isQuadNeeded;
	PolyVox::SurfaceMesh<PolyVox::PositionMaterial<VoxelType> >* colouredCubicMesh = new PolyVox::SurfaceMesh<PolyVox::PositionMaterial<VoxelType> >;
	//CubicSurfaceExtractor< RawVolume<VoxelType>, ColouredCubesIsQuadNeeded<VoxelType> > surfaceExtractor(mVolData, lod0Region, &colouredCubicMesh, WrapModes::Border, VoxelType(0), true, isQuadNeeded);
	//surfaceExtractor.execute();

	uint32_t downScaleFactor = 0x0001 << volReg->mLodLevel;

	generateCubicMesh(lod0Region, downScaleFactor, colouredCubicMesh);

	if(colouredCubicMesh->getNoOfIndices() > 0)
	{
		volReg->buildGraphicsMesh(colouredCubicMesh/*, 0*/);
	}
}

void ColouredCubesVolume::generateCubicMesh(const PolyVox::Region& region, uint32_t downSampleFactor, PolyVox::SurfaceMesh<PolyVox::PositionMaterial<VoxelType> >* resultMesh)
{
	ColouredCubesIsQuadNeeded<VoxelType> isQuadNeeded;

	if(downSampleFactor != 2) //HACK
	{
		PolyVox::SurfaceMesh<PolyVox::PositionMaterial<VoxelType> > colouredCubicMesh;
		PolyVox::CubicSurfaceExtractor< PolyVox::RawVolume<VoxelType>, ColouredCubesIsQuadNeeded<VoxelType> > surfaceExtractor(mVolData, region, resultMesh, PolyVox::WrapModes::Border, VoxelType(0), true, isQuadNeeded);
		surfaceExtractor.execute();
	}
	else
	{
		PolyVox::Region lod2Region = region;
		PolyVox::Vector3DInt32 lowerCorner = lod2Region.getLowerCorner();
		PolyVox::Vector3DInt32 upperCorner = lod2Region.getUpperCorner();

		upperCorner = upperCorner - lowerCorner;
		upperCorner = upperCorner / static_cast<int32_t>(downSampleFactor);
		upperCorner = upperCorner + lowerCorner;
		lod2Region.setUpperCorner(upperCorner);

		PolyVox::RawVolume<VoxelType> resampledVolume(lod2Region);
		//VolumeResampler< RawVolume<VoxelType>, RawVolume<VoxelType> > volumeResampler(mVolData, region, &resampledVolume, lod2Region);
		rescaleCubicVolume(mVolData, region, &resampledVolume, lod2Region);
		//volumeResampler.execute();

		PolyVox::SurfaceMesh<PolyVox::PositionMaterial<VoxelType> > colouredCubicMesh;
		PolyVox::CubicSurfaceExtractor< PolyVox::RawVolume<VoxelType>, ColouredCubesIsQuadNeeded<VoxelType> > surfaceExtractor(&resampledVolume, lod2Region, resultMesh, PolyVox::WrapModes::Border, VoxelType(0), true, isQuadNeeded);
		surfaceExtractor.execute();

		resultMesh->scaleVertices(downSampleFactor);
	}
}