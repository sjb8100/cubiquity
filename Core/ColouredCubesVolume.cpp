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

				Colour purple;
				purple.setColour(16, 16, 0, 16);
				srcSampler.setWrapMode(WrapModes::Border);
				//srcSampler.setPosition(srcPos);

				uint32_t noOfSolidVoxels = 0;
				for(int32_t childZ = 0; childZ < 2; childZ++)
				{
					for(int32_t childY = 0; childY < 2; childY++)
					{
						for(int32_t childX = 0; childX < 2; childX++)
						{
							srcSampler.setPosition(srcPos + Vector3DInt32(childX, childY, childZ));

							Colour child = srcSampler.getVoxel();

							if(child.getAlpha () > 0)
							{
								noOfSolidVoxels++;
							}
						}
					}
				}

				if(noOfSolidVoxels > 7)
				{
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
								}							
							}
						}
					}

					if(totalExposedFaces == 0) totalExposedFaces++;

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

ColouredCubesVolume::ColouredCubesVolume(int lowerX, int lowerY, int lowerZ, int upperX, int upperY, int upperZ, unsigned int blockSize, unsigned int baseNodeSize)
	:Volume<Colour>(lowerX, lowerY, lowerZ, upperX, upperY, upperZ, blockSize, OctreeConstructionModes::BoundVoxels, baseNodeSize)
{
}

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
		
		PolyVox::Region srcRegion = region;

		srcRegion.grow(2);

		PolyVox::Vector3DInt32 lowerCorner = srcRegion.getLowerCorner();
		PolyVox::Vector3DInt32 upperCorner = srcRegion.getUpperCorner();

		upperCorner = upperCorner - lowerCorner;
		upperCorner = upperCorner / static_cast<int32_t>(downSampleFactor);
		upperCorner = upperCorner + lowerCorner;

		PolyVox::Region dstRegion(lowerCorner, upperCorner);

		PolyVox::RawVolume<VoxelType> resampledVolume(dstRegion);
		//VolumeResampler< RawVolume<VoxelType>, RawVolume<VoxelType> > volumeResampler(mVolData, region, &resampledVolume, lod2Region);
		rescaleCubicVolume(mVolData, srcRegion, &resampledVolume, dstRegion);
		//volumeResampler.execute();

		dstRegion.shrink(1);

		//dstRegion.shiftLowerCorner(-1, -1, -1);

		Colour purple;
		purple.setColour(16, 16, 0, 16);

		PolyVox::SurfaceMesh<PolyVox::PositionMaterial<VoxelType> > colouredCubicMesh;
		PolyVox::CubicSurfaceExtractor< PolyVox::RawVolume<VoxelType>, ColouredCubesIsQuadNeeded<VoxelType> > surfaceExtractor(&resampledVolume, dstRegion, resultMesh, PolyVox::WrapModes::Border, VoxelType(0), true, isQuadNeeded);
		surfaceExtractor.execute();

		resultMesh->scaleVertices(downSampleFactor);
		resultMesh->translateVertices(Vector3DFloat(0.5f, 0.5f, 0.5f));
	}
}