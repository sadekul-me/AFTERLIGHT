#include "Camera/AfterlightSliceProgress.h"

bool FAfterlightSliceProgress::ShouldGrantEnteredCut(float PlayerX, float PlayerY, float MayaX, bool bHasChoice, float SecondsSinceCutStart)
{
	if (!bHasChoice)
	{
		return false;
	}
	if (FMath::Abs(PlayerY) > 560.f)
	{
		return false;
	}
	if (PlayerX > 1450.f && PlayerX < 3350.f)
	{
		return true;
	}
	if (MayaX > 1550.f && PlayerX > 700.f && FMath::Abs(PlayerX - MayaX) < 1100.f)
	{
		return true;
	}
	return SecondsSinceCutStart > 22.f && PlayerX > 620.f && PlayerX < 3400.f;
}

bool FAfterlightSliceProgress::IsInsidePlayableXY(float X, float Y)
{
	return X > 40.f && X < 4180.f && Y > -400.f && Y < 560.f;
}

FVector FAfterlightSliceProgress::ClampToPlayable(const FVector& Location)
{
	FVector Out = Location;
	Out.X = FMath::Clamp(Out.X, 80.f, 4120.f);
	Out.Y = FMath::Clamp(Out.Y, -360.f, 520.f);
	Out.Z = 92.f;
	return Out;
}

bool FAfterlightSliceProgress::IsCameraBelowFloor(float CameraZ, float FloorZ)
{
	return CameraZ < FloorZ;
}

bool FAfterlightSliceProgress::IsShotTooClose(float DistanceToFocus, float MinDistance)
{
	return DistanceToFocus < MinDistance;
}

bool FAfterlightSliceProgress::IsShotValidPlacement(const FVector& CameraLocation, const FVector& FocusLocation, float FloorZ)
{
	if (IsCameraBelowFloor(CameraLocation.Z, FloorZ))
	{
		return false;
	}
	return !IsShotTooClose(FVector::Dist(CameraLocation, FocusLocation));
}
