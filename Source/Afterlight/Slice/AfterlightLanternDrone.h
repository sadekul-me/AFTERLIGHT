#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AfterlightLanternDrone.generated.h"

class UStaticMeshComponent;
class USpotLightComponent;
class UPointLightComponent;
class UTextRenderComponent;

UCLASS()
class AFTERLIGHT_API AAfterlightLanternDrone : public AActor
{
	GENERATED_BODY()

public:
	AAfterlightLanternDrone();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void BeginSweep(const FVector& Start, const FVector& End, float DurationSeconds);
	bool IsSweeping() const { return bSweeping; }
	bool HasFinished() const { return bFinished; }
	void ResetSweep();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpotLightComponent> Spotlight;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> Beacon;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> Label;

	FVector SweepStart = FVector::ZeroVector;
	FVector SweepEnd = FVector::ZeroVector;
	float SweepDuration = 8.f;
	float SweepElapsed = 0.f;
	float ScanYaw = 0.f;
	bool bSweeping = false;
	bool bFinished = false;
};
