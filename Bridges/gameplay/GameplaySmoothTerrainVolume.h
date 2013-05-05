#ifndef GAMEPLAYSMOOTHTERRAINVOLUME_H_
#define GAMEPLAYSMOOTHTERRAINVOLUME_H_

#include "SmoothTerrainVolume.h"
#include "SmoothSurfaceExtractionTask.h"

#include "gameplay.h"

#include "Impl/GameplayVolume.h"

namespace Cubiquity
{
	/**
	 * A volume containing smooth terrain.
	 */
	class GameplaySmoothTerrainVolume : public GameplayVolume<SmoothTerrainVolume>
	{
	public:
		/**
		 * Creates a new GameplaySmoothTerrainVolume.
		 *
		 * @script{create}
		 */
		static GameplaySmoothTerrainVolume* create(int lowerX, int lowerY, int lowerZ, int upperX, int upperY, int upperZ, unsigned int blockSize, unsigned int baseNodeSize)
		{
			GameplaySmoothTerrainVolume* volume = new GameplaySmoothTerrainVolume(lowerX, lowerY, lowerZ, upperX, upperY, upperZ, blockSize, baseNodeSize);
			return volume;
		}

		/**
		 * Creates a new GameplaySmoothTerrainVolume.
		 *
		 * @script{create}
		 */
		static GameplaySmoothTerrainVolume* create(const char* dataToLoad, unsigned int blockSize, unsigned int baseNodeSize)
		{
			GameplaySmoothTerrainVolume* volume = new GameplaySmoothTerrainVolume(dataToLoad, blockSize, baseNodeSize);
			return volume;
		}

		// Ugly hack, as luagen can't see the base class implementation of this function (probably it can't handle templated base classes)
		gameplay::Node* getRootNodeForLua(int dummyParamForLuagen)
		{
			return GameplayVolume<SmoothTerrainVolume>::getRootNode();
		}

		void performUpdate(const gameplay::Vector3& viewPosition, float lodThreshold);

	protected:
		GameplaySmoothTerrainVolume(int lowerX, int lowerY, int lowerZ, int upperX, int upperY, int upperZ, unsigned int blockSize, unsigned int baseNodeSize);
		GameplaySmoothTerrainVolume(const char* dataToLoad, unsigned int blockSize, unsigned int baseNodeSize);
		virtual ~GameplaySmoothTerrainVolume();

	private:

		gameplay::Model* buildModelFromPolyVoxMesh(const PolyVox::SurfaceMesh< typename VoxelTraits<typename CubiquityVolumeType::VoxelType>::VertexType>* polyVoxMesh);
	};
}

#endif //GAMEPLAYSMOOTHTERRAINVOLUME_H_
