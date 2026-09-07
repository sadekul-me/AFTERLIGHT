#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AfterlightCinematicCoordinator.generated.h"

class ULevelSequence;
class ALevelSequenceActor;
class ULevelSequencePlayer;
class AAfterlightPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAfterlightCinematicFinished);

UCLASS()
class AFTERLIGHT_API UAfterlightCinematicCoordinator : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Afterlight|Cinematic")
	void RequestCinematic(ULevelSequence* Sequence);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Cinematic")
	void RequestCinematicControl();

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Cinematic")
	void ReleaseCinematic();

	UFUNCTION(BlueprintPure, Category = "Afterlight|Cinematic")
	bool IsCinematicActive() const { return bActive; }

	UPROPERTY(BlueprintAssignable)
	FAfterlightCinematicFinished OnCinematicFinished;

private:
	UFUNCTION()
	void HandleSequenceFinished();
	AAfterlightPlayerController* ResolveController() const;

	bool bActive = false;

	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> Player;

	UPROPERTY()
	TObjectPtr<ALevelSequenceActor> SequenceActor;
};
