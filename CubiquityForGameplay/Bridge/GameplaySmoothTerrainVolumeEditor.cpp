#include "GameplaySmoothTerrainVolumeEditor.h"

#include "Brush.h"
#include "SmoothTerrainVolumeEditor.h"

namespace Cubiquity
{
	GameplaySmoothTerrainVolumeEditor::GameplaySmoothTerrainVolumeEditor(GameplaySmoothTerrainVolume* volume)
	{
		mSmoothTerrainVolume = dynamic_cast<SmoothTerrainVolumeImpl*>(volume->getCubiquityVolume());
	}

	GameplaySmoothTerrainVolumeEditor::~GameplaySmoothTerrainVolumeEditor()
	{
	}

	void GameplaySmoothTerrainVolumeEditor::applyPaint(const gameplay::Vector3& centre, float radius, uint32_t materialToPaintWith, float timeElapsedInSeconds, float amount)
	{
		Vector3F v3dCentre(centre.x, centre.y, centre.z);
		//edit(v3dCentre, radius, materialToPaintWith, EditActions::Paint, timeElapsedInSeconds,amount, 0.0f);
		Brush brush(radius, radius, amount);
		paintSmoothTerrainVolume(mSmoothTerrainVolume, v3dCentre, brush, materialToPaintWith);
	}

	void GameplaySmoothTerrainVolumeEditor::smooth(const gameplay::Vector3& centre, float radius, float timeElapsedInSeconds, float smoothBias, float amount)
	{
		Vector3F v3dCentre(centre.x, centre.y, centre.z);
		// '0' is a dummy as the smooth operations smooths *all* materials
		//edit(v3dCentre, radius, 0, EditActions::Smooth, timeElapsedInSeconds, amount, smoothBias);
		//smoothVolume(v3dCentre, radius);
		Brush brush(radius, radius, amount);
		blurSmoothTerrainVolume(mSmoothTerrainVolume, v3dCentre, brush);
	}

	void GameplaySmoothTerrainVolumeEditor::addMaterial(const gameplay::Vector3& centre, float radius, uint32_t materialToAdd, float timeElapsedInSeconds, float amount)
	{
		Vector3F v3dCentre(centre.x, centre.y, centre.z);
		Brush brush(radius, radius, amount);
		sculptSmoothTerrainVolume(mSmoothTerrainVolume, v3dCentre, brush);
	}

	void GameplaySmoothTerrainVolumeEditor::subtractMaterial(const gameplay::Vector3& centre, float radius, float timeElapsedInSeconds, float amount)
	{
		Vector3F v3dCentre(centre.x, centre.y, centre.z);
		Brush brush(radius, radius, amount * -1.0f);
		sculptSmoothTerrainVolume(mSmoothTerrainVolume, v3dCentre, brush);
	}
}
