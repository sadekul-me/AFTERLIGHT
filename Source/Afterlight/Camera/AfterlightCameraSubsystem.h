#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Camera/AfterlightCameraRecipe.h"
#include "AfterlightCameraSubsystem.generated.h"

class APlayerController;
class ACineCameraActor;
class UAfterlightCameraRecipe;

UCLASS()
class AFTERLIGHT_API UAfterlightCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Camera")
	void RegisterAnchor(EAfterlightCameraRegister Register, AActor* CameraActor);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Camera")
	void RegisterShot(FName ShotId, AActor* CameraActor);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Camera")
	void RequestRegister(EAfterlightCameraRegister Register, float BlendOverride = -1.f);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Camera")
	void RequestShot(FName ShotId, float BlendOverride = -1.f);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Camera")
	void ReleaseToExplore(float BlendOverride = -1.f);

	UFUNCTION(BlueprintPure, Category = "Afterlight|Camera")
	EAfterlightCameraRegister GetCurrentRegister() const { return CurrentRegister; }

	UFUNCTION(BlueprintPure, Category = "Afterlight|Camera")
	EAfterlightCameraAuthority GetAuthority() const { return Authority; }

	UFUNCTION(BlueprintPure, Category = "Afterlight|Camera")
	FName GetActiveShotId() const { return ActiveShotId; }

	UFUNCTION(BlueprintPure, Category = "Afterlight|Camera")
	FName GetActiveRecipeId() const;

	UFUNCTION(BlueprintPure, Category = "Afterlight|Camera")
	AActor* GetCurrentViewTarget() const { return CurrentViewTarget.Get(); }

	void SetRecipe(EAfterlightCameraRegister Register, UAfterlightCameraRecipe* Recipe);
	void SetAuthority(EAfterlightCameraAuthority NewAuthority);
	UAfterlightCameraRecipe* GetRecipe(EAfterlightCameraRegister Register) const;
	void CycleDebugShot();
	void ForceView(AActor* Target, float BlendTime = 0.f);

private:
	void EnsureDefaultRecipes();
	void ApplyViewTarget(AActor* Target, float BlendTime, EViewTargetBlendFunction BlendFunction);
	void ApplyRecipeToActor(AActor* Target, const UAfterlightCameraRecipe* Recipe, AActor* FocusActor);
	void BeginPush(AActor* Target, const UAfterlightCameraRecipe* Recipe);

	UFUNCTION()
	void TickPush();
	bool IsShotBlocked(AActor* CameraActor, const FVector& FocusLocation) const;
	bool IsShotUsable(AActor* CameraActor, const FVector& FocusLocation) const;
	AActor* ResolveValidatedShot(FName ShotId, AActor* Focus) const;
	AActor* ResolveFallbackTarget(EAfterlightCameraRegister Register) const;
	APlayerController* ResolveController() const;
	AActor* ResolveExploreViewTarget() const;
	AActor* ResolveFocusActor() const;

	EAfterlightCameraRegister CurrentRegister = EAfterlightCameraRegister::Explore;
	EAfterlightCameraAuthority Authority = EAfterlightCameraAuthority::Gameplay;
	FName ActiveShotId = NAME_None;

	UPROPERTY()
	TMap<EAfterlightCameraRegister, TWeakObjectPtr<AActor>> Anchors;

	UPROPERTY()
	TMap<FName, TWeakObjectPtr<AActor>> Shots;

	UPROPERTY()
	TMap<EAfterlightCameraRegister, TObjectPtr<UAfterlightCameraRecipe>> Recipes;

	TWeakObjectPtr<AActor> CurrentViewTarget;
	TWeakObjectPtr<AActor> PushActor;
	FVector PushStart = FVector::ZeroVector;
	FVector PushEnd = FVector::ZeroVector;
	float PushElapsed = 0.f;
	float PushDuration = 0.f;
	FTimerHandle PushTimer;
	TArray<FName> DebugShotOrder;
	int32 DebugShotIndex = 0;
};
