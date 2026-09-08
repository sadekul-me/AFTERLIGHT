#pragma once

#include "CoreMinimal.h"

class ACharacter;
class USkeletalMeshComponent;
class UAnimationAsset;

struct AFTERLIGHT_API FAfterlightPlaceholderVisuals
{
	static void Attach(ACharacter* Character, bool bCompanion);
	static void Tick(ACharacter* Character, float DeltaSeconds);
	static void BeginRecover(ACharacter* Character);

	static bool UsesMannequin(const ACharacter* Character);
};
