#ifndef GAMEPLAYCOLOUREDCUBESVOLUME_H_
#define GAMEPLAYCOLOUREDCUBESVOLUME_H_

#include "ColouredCubesVolume.h"

#include "gameplay.h"

class GameplayColouredCubesVolume : public ColouredCubesVolume
{
public:
	static GameplayColouredCubesVolume* create(VolumeType type, int lowerX, int lowerY, int lowerZ, int upperX, int upperY, int upperZ, unsigned int regionWidth, unsigned int regionHeight, unsigned int regionDepth)
	{
		GameplayColouredCubesVolume* volume = new GameplayColouredCubesVolume(type, lowerX, lowerY, lowerZ, upperX, upperY, upperZ, regionWidth, regionHeight, regionDepth);
		return volume;
	}

	void performUpdate(void);

public:
	GameplayColouredCubesVolume(VolumeType type, int lowerX, int lowerY, int lowerZ, int upperX, int upperY, int upperZ, unsigned int regionWidth, unsigned int regionHeight, unsigned int regionDepth);
	virtual ~GameplayColouredCubesVolume();

	void syncNode(OctreeNode* octreeNode, gameplay::Node* gameplayNode);

	gameplay::Model* buildModelFromPolyVoxMesh(const PolyVox::SurfaceMesh< PolyVox::PositionMaterial<Colour> >* polyVoxMesh);

	gameplay::Node* mRootGameplayNode;
};

#endif //GAMEPLAYCOLOUREDCUBESVOLUME_H_