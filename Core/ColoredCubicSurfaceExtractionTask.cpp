#include "ColoredCubicSurfaceExtractionTask.h"

#include "Color.h"
#include "OctreeNode.h"

#include "PolyVoxCore/CubicSurfaceExtractor.h"
#include "PolyVoxCore/RawVolume.h"
#include "PolyVoxCore/LargeVolume.h"

#include <limits>

using namespace PolyVox;

namespace Cubiquity
{
	// Eliminate this
	void scaleVertices(ColoredCubesMesh* mesh, uint32_t amount)
	{
		for (uint32_t ct = 0; ct < mesh->getNoOfVertices(); ct++)
		{
			ColoredCubesVertex& vertex = const_cast<ColoredCubesVertex&>(mesh->getVertex(ct));
			vertex.encodedPosition *= amount;
		}
	}

	// Eliminate this
	/*void translateVertices(ColoredCubesMesh* mesh, const Vector3DUint8& amount)
	{
		for (uint32_t ct = 0; ct < mesh->m_vecVertices.size(); ct++)
		{
			//TODO: Should rethink accessors here to provide faster access
			Vector3DUint8 position = mesh->m_vecVertices[ct].position;
			position += amount;
			mesh->m_vecVertices[ct].position = position;
		}
	}*/

	ColoredCubicSurfaceExtractionTask::ColoredCubicSurfaceExtractionTask(OctreeNode< Color >* octreeNode, ::PolyVox::LargeVolume<Color>* polyVoxVolume)
		:Task()
		,mOctreeNode(octreeNode)
		,mPolyVoxVolume(polyVoxVolume)
		,mPolyVoxMesh(0)
		,mProcessingStartedTimestamp((std::numeric_limits<Timestamp>::max)())
		,mOwnMesh(false)
	{
	}

	ColoredCubicSurfaceExtractionTask::~ColoredCubicSurfaceExtractionTask()
	{
		if(mOwnMesh)
		{
			delete mPolyVoxMesh;
			mPolyVoxMesh = 0;
			mOwnMesh = false;
		}
	}

	void ColoredCubicSurfaceExtractionTask::process(void)
	{
		mProcessingStartedTimestamp = Clock::getTimestamp();

		Region lod0Region = mOctreeNode->mRegion;

		//Extract the surface
		mPolyVoxMesh = new ColoredCubesMesh;
		mOwnMesh = true;

		uint32_t downScaleFactor = 0x0001 << mOctreeNode->mHeight;

		ColoredCubesIsQuadNeeded isQuadNeeded;

		if(downScaleFactor == 1) 
		{
			extractCubicMeshCustom(mPolyVoxVolume, mOctreeNode->mRegion, mPolyVoxMesh, isQuadNeeded, ::PolyVox::WrapModes::Border, Color(), true);
		}
		else if(downScaleFactor == 2)
		{
		
			Region srcRegion = mOctreeNode->mRegion;

			srcRegion.grow(2);

			Vector3I lowerCorner = srcRegion.getLowerCorner();
			Vector3I upperCorner = srcRegion.getUpperCorner();

			upperCorner = upperCorner - lowerCorner;
			upperCorner = upperCorner / static_cast<int32_t>(downScaleFactor);
			upperCorner = upperCorner + lowerCorner;

			Region dstRegion(lowerCorner, upperCorner);

			::PolyVox::RawVolume<Color> resampledVolume(dstRegion);
			rescaleCubicVolume(mPolyVoxVolume, srcRegion, &resampledVolume, dstRegion);

			dstRegion.shrink(1);
		
			//dstRegion.shiftLowerCorner(-1, -1, -1);

			extractCubicMeshCustom(&resampledVolume, dstRegion, mPolyVoxMesh, isQuadNeeded, ::PolyVox::WrapModes::Border, Color(), true);

			scaleVertices(mPolyVoxMesh, downScaleFactor);
			//translateVertices(mPolyVoxMesh, Vector3DFloat(0.5f, 0.5f, 0.5f)); // Removed when going from float positions to uin8_t. Do we need this?
		}
		else if(downScaleFactor == 4)
		{
			Region srcRegion = mOctreeNode->mRegion;

			srcRegion.grow(4);

			Vector3I lowerCorner = srcRegion.getLowerCorner();
			Vector3I upperCorner = srcRegion.getUpperCorner();

			upperCorner = upperCorner - lowerCorner;
			upperCorner = upperCorner / static_cast<int32_t>(2);
			upperCorner = upperCorner + lowerCorner;

			Region dstRegion(lowerCorner, upperCorner);

			::PolyVox::RawVolume<Color> resampledVolume(dstRegion);
			rescaleCubicVolume(mPolyVoxVolume, srcRegion, &resampledVolume, dstRegion);



			lowerCorner = dstRegion.getLowerCorner();
			upperCorner = dstRegion.getUpperCorner();

			upperCorner = upperCorner - lowerCorner;
			upperCorner = upperCorner / static_cast<int32_t>(2);
			upperCorner = upperCorner + lowerCorner;

			Region dstRegion2(lowerCorner, upperCorner);

			::PolyVox::RawVolume<Color> resampledVolume2(dstRegion2);
			rescaleCubicVolume(&resampledVolume, dstRegion, &resampledVolume2, dstRegion2);

			dstRegion2.shrink(1);

			//dstRegion.shiftLowerCorner(-1, -1, -1);

			extractCubicMeshCustom(&resampledVolume2, dstRegion2, mPolyVoxMesh, isQuadNeeded, ::PolyVox::WrapModes::Border, Color(), true);

			scaleVertices(mPolyVoxMesh, downScaleFactor);
			//translateVertices(mPolyVoxMesh, Vector3DFloat(1.5f, 1.5f, 1.5f)); // Removed when going from float positions to uin8_t. Do we need this?
		}

		mOctreeNode->mOctree->mFinishedSurfaceExtractionTasks.push(this);
	}
}
