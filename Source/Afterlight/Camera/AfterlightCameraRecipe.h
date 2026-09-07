#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AfterlightCameraRecipe.generated.h"

UENUM(BlueprintType)
enum class EAfterlightCameraRegister : uint8
{
	Explore,
	Dialogue,
	Cinematic,
	Threat
};

UCLASS(BlueprintType)
class AFTERLIGHT_API UAfterlightCameraRecipe : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	EAfterlightCameraRegister Register = EAfterlightCameraRegister::Explore;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	float FocalLength = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	float Aperture = 2.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	float BlendTime = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FramingHeightBias = 0.55f;
};

inline FName AfterlightRegisterToName(EAfterlightCameraRegister Register)
{
	switch (Register)
	{
	case EAfterlightCameraRegister::Dialogue: return TEXT("Dialogue");
	case EAfterlightCameraRegister::Cinematic: return TEXT("Cinematic");
	case EAfterlightCameraRegister::Threat: return TEXT("Threat");
	default: return TEXT("Explore");
	}
}
