#pragma once

#include "CoreMinimal.h"

struct AFTERLIGHT_API FAfterlightSliceProgress
{
	static bool ShouldGrantEnteredCut(float PlayerX, float PlayerY, float MayaX, bool bHasChoice, float SecondsSinceCutStart);
	static bool IsInsidePlayableXY(float X, float Y);
	static FVector ClampToPlayable(const FVector& Location);
	static bool IsCameraBelowFloor(float CameraZ, float FloorZ = 20.f);
	static bool IsShotTooClose(float DistanceToFocus, float MinDistance = 110.f);
	static bool IsShotValidPlacement(const FVector& CameraLocation, const FVector& FocusLocation, float FloorZ = 20.f);
};
