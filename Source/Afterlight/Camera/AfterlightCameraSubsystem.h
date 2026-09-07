#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Camera/AfterlightCameraRecipe.h"
#include "AfterlightCameraSubsystem.generated.h"

class ACameraActor;
class APlayerController;

UCLASS()
class AFTERLIGHT_API UAfterlightCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Camera")
	void RegisterAnchor(EAfterlightCameraRegister Register, AActor* CameraActor);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Camera")
	void RequestRegister(EAfterlightCameraRegister Register, float BlendOverride = -1.f);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Camera")
	void ReleaseToExplore(float BlendOverride = -1.f);

	UFUNCTION(BlueprintPure, Category = "Afterlight|Camera")
	EAfterlightCameraRegister GetCurrentRegister() const { return CurrentRegister; }

	void SetRecipe(EAfterlightCameraRegister Register, UAfterlightCameraRecipe* Recipe);

private:
	void ApplyViewTarget(AActor* Target, float BlendTime);
	APlayerController* ResolveController() const;
	AActor* ResolveExploreViewTarget() const;

	EAfterlightCameraRegister CurrentRegister = EAfterlightCameraRegister::Explore;

	UPROPERTY()
	TMap<EAfterlightCameraRegister, TWeakObjectPtr<AActor>> Anchors;

	UPROPERTY()
	TMap<EAfterlightCameraRegister, TObjectPtr<UAfterlightCameraRecipe>> Recipes;
};
