#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/AfterlightCameraRecipe.h"
#include "AfterlightCameraAnchorComponent.generated.h"

UCLASS(ClassGroup = (Afterlight), meta = (BlueprintSpawnableComponent))
class AFTERLIGHT_API UAfterlightCameraAnchorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAfterlightCameraAnchorComponent();
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Afterlight")
	EAfterlightCameraRegister Register = EAfterlightCameraRegister::Dialogue;
};
