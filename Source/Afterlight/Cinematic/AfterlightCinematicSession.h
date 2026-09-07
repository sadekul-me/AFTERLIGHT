#pragma once

#include "CoreMinimal.h"
#include "Camera/AfterlightCameraRecipe.h"
#include "AfterlightCinematicSession.generated.h"

USTRUCT(BlueprintType)
struct AFTERLIGHT_API FAfterlightCinematicSession
{
	GENERATED_BODY()

	UPROPERTY()
	bool bActive = false;

	UPROPERTY()
	EAfterlightCameraRegister ReturnRegister = EAfterlightCameraRegister::Explore;

	UPROPERTY()
	FName SequenceName = NAME_None;

	UPROPERTY()
	EAfterlightCameraAuthority Authority = EAfterlightCameraAuthority::Gameplay;

	bool Begin(FName InSequenceName, EAfterlightCameraRegister InReturnRegister)
	{
		bActive = true;
		SequenceName = InSequenceName;
		ReturnRegister = InReturnRegister;
		Authority = EAfterlightCameraAuthority::Sequencer;
		return true;
	}

	bool Complete()
	{
		if (!bActive)
		{
			return false;
		}
		bActive = false;
		SequenceName = NAME_None;
		Authority = EAfterlightCameraAuthority::Gameplay;
		return true;
	}

	bool Cancel()
	{
		return Complete();
	}
};
