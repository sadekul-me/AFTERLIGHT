#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AfterlightFramingTargetsComponent.generated.h"

UCLASS(ClassGroup = (Afterlight), meta = (BlueprintSpawnableComponent))
class AFTERLIGHT_API UAfterlightFramingTargetsComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAfterlightFramingTargetsComponent();

	UFUNCTION(BlueprintPure, Category = "Afterlight|Camera")
	FVector GetHeadLocation() const;

	UFUNCTION(BlueprintPure, Category = "Afterlight|Camera")
	FVector GetChestLocation() const;

	UFUNCTION(BlueprintPure, Category = "Afterlight|Camera")
	FVector GetDialogueLookLocation() const;

	UFUNCTION(BlueprintPure, Category = "Afterlight|Camera")
	FVector GetCinematicFocusLocation() const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Afterlight")
	TObjectPtr<USceneComponent> HeadTarget;

	UPROPERTY(VisibleAnywhere, Category = "Afterlight")
	TObjectPtr<USceneComponent> ChestTarget;

	UPROPERTY(VisibleAnywhere, Category = "Afterlight")
	TObjectPtr<USceneComponent> DialogueLookTarget;

	UPROPERTY(VisibleAnywhere, Category = "Afterlight")
	TObjectPtr<USceneComponent> CinematicFocusTarget;
};
