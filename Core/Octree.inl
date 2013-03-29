#include "OctreeNode.h"
#include "Volume.h"

namespace Cubiquity
{
	template <typename VoxelType>
	Octree<VoxelType>::Octree(Volume<VoxelType>* volume, OctreeConstructionMode octreeConstructionMode, unsigned int baseNodeSize)
		:mVolume(volume)
		,mRootOctreeNode(0)
		,mBaseNodeSize(baseNodeSize)
	{
		Region regionToCover(mVolume->getEnclosingRegion());
		if(octreeConstructionMode == OctreeConstructionModes::BoundVoxels)
		{
			regionToCover.shiftUpperCorner(1, 1, 1);
		}
		else if(octreeConstructionMode == OctreeConstructionModes::BoundCells)
		{
			regionToCover.shiftLowerCorner(-1, -1, -1);
			regionToCover.shiftUpperCorner(1, 1, 1);
		}

		POLYVOX_ASSERT(::PolyVox::isPowerOf2(mBaseNodeSize), "Node size must be a power of two");

		uint32_t largestVolumeDimension = std::max(regionToCover.getWidthInVoxels(), std::max(regionToCover.getHeightInVoxels(), regionToCover.getDepthInVoxels()));
		if(octreeConstructionMode == OctreeConstructionModes::BoundCells)
		{
			largestVolumeDimension--;
		}

		uint32_t octreeTargetSize = ::PolyVox::upperPowerOfTwo(largestVolumeDimension);

		uint8_t noOfLodLevels = logBase2((octreeTargetSize) / mBaseNodeSize) + 1;

		uint32_t regionToCoverWidth = (octreeConstructionMode == OctreeConstructionModes::BoundCells) ? regionToCover.getWidthInCells() : regionToCover.getWidthInVoxels();
		uint32_t regionToCoverHeight = (octreeConstructionMode == OctreeConstructionModes::BoundCells) ? regionToCover.getHeightInCells() : regionToCover.getHeightInVoxels();
		uint32_t regionToCoverDepth = (octreeConstructionMode == OctreeConstructionModes::BoundCells) ? regionToCover.getDepthInCells() : regionToCover.getDepthInVoxels();

		uint32_t widthIncrease = octreeTargetSize - regionToCoverWidth;
		uint32_t heightIncrease = octreeTargetSize - regionToCoverHeight;
		uint32_t depthIncrease = octreeTargetSize - regionToCoverDepth;

		Region octreeRegion(regionToCover);
	
		if(widthIncrease % 2 == 1)
		{
			octreeRegion.setUpperX(octreeRegion.getUpperX() + 1);
			widthIncrease--;
		}

		if(heightIncrease % 2 == 1)
		{
			octreeRegion.setUpperY(octreeRegion.getUpperY() + 1);
			heightIncrease--;
		}
		if(depthIncrease % 2 == 1)
		{
			octreeRegion.setUpperZ(octreeRegion.getUpperZ() + 1);
			depthIncrease--;
		}

		octreeRegion.grow(widthIncrease / 2, heightIncrease / 2, depthIncrease / 2);

		mNodes.push_back(0); //DUMMY NODE AT ZERO - HACK!!
		mRootOctreeNode = createNode(octreeRegion, 0);
		mNodes[mRootOctreeNode]->mLodLevel = noOfLodLevels - 1;

		buildOctreeNodeTree(mRootOctreeNode, regionToCover, octreeConstructionMode);
	}

	template <typename VoxelType>
	uint16_t Octree<VoxelType>::createNode(Region region, uint16_t parent)
	{
		OctreeNode< VoxelType >* node = new OctreeNode< VoxelType >(region, parent, this);
		mNodes.push_back(node);
		POLYVOX_ASSERT(mNodes.size() <= std::numeric_limits<uint16_t>::max(), "Too many octree nodes!");
		uint16_t index = mNodes.size() - 1;
		return index;
	}

	template <typename VoxelType>
	void Octree<VoxelType>::update(const Vector3F& viewPosition, float lodThreshold)
	{
		mNodes[mRootOctreeNode]->clearWantedForRendering();
		mNodes[mRootOctreeNode]->determineWantedForRendering(viewPosition, lodThreshold);

		mNodes[mRootOctreeNode]->sceduleUpdateIfNeeded(viewPosition);


		// Make sure any surface extraction tasks which were scheduled on the main thread get processed before we determine what to render.
		gMainThreadTaskProcessor.processAllTasks(); //Doesn't really belong here

		// This will include tasks from both the background and main threads.
		while(!mFinishedSurfaceExtractionTasks.empty())
		{
			VoxelTraits<VoxelType>::SurfaceExtractionTaskType* task;
			mFinishedSurfaceExtractionTasks.wait_and_pop(task);

			task->mOctreeNode->updateFromCompletedTask(task);

			if(task->mOctreeNode->mLastSurfaceExtractionTask == task)
			{
				task->mOctreeNode->mLastSurfaceExtractionTask = 0;
			}

			delete task;
		}

		mNodes[mRootOctreeNode]->determineWhetherToRender();
	}

	template <typename VoxelType>
	void Octree<VoxelType>::markDataAsModified(int32_t x, int32_t y, int32_t z, Timestamp newTimeStamp, UpdatePriority updatePriority)
	{
		mRootOctreeNode->markDataAsModified(x, y, z, newTimeStamp, updatePriority);
	}

	template <typename VoxelType>
	void Octree<VoxelType>::markDataAsModified(const Region& region, Timestamp newTimeStamp, UpdatePriority updatePriority)
	{
		mRootOctreeNode->markDataAsModified(region, newTimeStamp, updatePriority);
	}

	template <typename VoxelType>
	void Octree<VoxelType>::buildOctreeNodeTree(uint16_t parent, const Region& regionToCover, OctreeConstructionMode octreeConstructionMode)
	{
		POLYVOX_ASSERT(mNodes[parent]->mRegion.getWidthInVoxels() == mNodes[parent]->mRegion.getHeightInVoxels(), "Region must be cubic");
		POLYVOX_ASSERT(mNodes[parent]->mRegion.getWidthInVoxels() == mNodes[parent]->mRegion.getDepthInVoxels(), "Region must be cubic");

		//We know that width/height/depth are all the same.
		uint32_t parentSize = static_cast<uint32_t>((octreeConstructionMode == OctreeConstructionModes::BoundCells) ? mNodes[parent]->mRegion.getWidthInCells() : mNodes[parent]->mRegion.getWidthInVoxels());

		if(parentSize > mBaseNodeSize)
		{
			Vector3I baseLowerCorner = mNodes[parent]->mRegion.getLowerCorner();
			int32_t childSize = (octreeConstructionMode == OctreeConstructionModes::BoundCells) ? mNodes[parent]->mRegion.getWidthInCells() / 2 : mNodes[parent]->mRegion.getWidthInVoxels() / 2;

			Vector3I baseUpperCorner;
			if(octreeConstructionMode == OctreeConstructionModes::BoundCells)
			{
				baseUpperCorner = baseLowerCorner + Vector3I(childSize, childSize, childSize);
			}
			else
			{
				baseUpperCorner = baseLowerCorner + Vector3I(childSize-1, childSize-1, childSize-1);
			}

			for(int z = 0; z < 2; z++)
			{
				for(int y = 0; y < 2; y++)
				{
					for(int x = 0; x < 2; x++)
					{
						Vector3I offset (x*childSize, y*childSize, z*childSize);
						Region childRegion(baseLowerCorner + offset, baseUpperCorner + offset);
						if(intersects(childRegion, regionToCover))
						{
							uint16_t octreeNode = createNode(childRegion, parent);
							mNodes[parent]->children[x][y][z] = octreeNode;
							buildOctreeNodeTree(octreeNode, regionToCover, octreeConstructionMode);
						}
					}
				}
			}
		}
	}
}