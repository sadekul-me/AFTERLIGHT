#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Camera/PlayerCameraManager.h"
#include "AfterlightCameraRecipe.generated.h"

class AAfterlightPlayerController;

UENUM(BlueprintType)
enum class EAfterlightCameraRegister : uint8
{
	Explore = 0,
	Dialogue = 1,
	Cinematic = 2,
	Threat = 3,
	Intimate = 4,
	Reveal = 5
};

UENUM(BlueprintType)
enum class EAfterlightCameraAuthority : uint8
{
	Gameplay,
	Register,
	Sequencer
};

UENUM(BlueprintType)
enum class EAfterlightCameraFocusMode : uint8
{
	None,
	Manual,
	Target
};

UCLASS(BlueprintType)
class AFTERLIGHT_API UAfterlightCameraRecipe : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	EAfterlightCameraRegister Register = EAfterlightCameraRegister::Explore;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FName RecipeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Lens")
	float FocalLength = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Lens")
	float Aperture = 2.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Lens")
	float GameplayFOV = 58.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Focus")
	EAfterlightCameraFocusMode FocusMode = EAfterlightCameraFocusMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Focus")
	float ManualFocusDistance = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Focus")
	bool bEnableDOF = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Framing")
	FVector CameraOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Framing", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float ShoulderSide = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Framing")
	float CameraHeight = 52.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Framing")
	float ArmLength = 360.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Blend")
	float BlendTime = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Blend")
	TEnumAsByte<EViewTargetBlendFunction> BlendFunction = VTBlend_Cubic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Feel")
	float MovementLag = 5.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Feel")
	float RotationLag = 9.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Feel")
	float PushInDistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight|Feel")
	float PushInTime = 0.f;

	static UAfterlightCameraRecipe* CreateDefault(UObject* Outer, EAfterlightCameraRegister Register);
};

namespace AfterlightShotIds
{
	const FName DialogueOTSCompanion(TEXT("Dialogue.OTS.Companion"));
	const FName DialogueOTSProtagonist(TEXT("Dialogue.OTS.Protagonist"));
	const FName DialogueTwoShot(TEXT("Dialogue.TwoShot"));
	const FName DialogueCloseUpCompanion(TEXT("Dialogue.CloseUp.Companion"));
	const FName RevealInsert(TEXT("Reveal.Insert"));
	const FName ThreatPressure(TEXT("Threat.Pressure"));
}

AFTERLIGHT_API FName AfterlightRegisterToName(EAfterlightCameraRegister Register);
AFTERLIGHT_API EAfterlightCameraRegister AfterlightRegisterFromName(FName Name);
AFTERLIGHT_API bool AfterlightIsKnownRegisterName(FName Name);
