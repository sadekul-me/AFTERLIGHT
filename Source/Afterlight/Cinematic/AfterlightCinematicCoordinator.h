#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Cinematic/AfterlightCinematicSession.h"
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
	void RequestCinematic(ULevelSequence* Sequence, EAfterlightCameraRegister ReturnRegister = EAfterlightCameraRegister::Explore, float BlendOutTime = 0.8f);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Cinematic")
	void RequestCinematicControl();

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Cinematic")
	void ReleaseCinematic();

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Cinematic")
	void CancelCinematic();

	UFUNCTION(BlueprintPure, Category = "Afterlight|Cinematic")
	bool IsCinematicActive() const { return Session.bActive; }

	UFUNCTION(BlueprintPure, Category = "Afterlight|Cinematic")
	FName GetActiveSequenceName() const { return Session.SequenceName; }

	UFUNCTION(BlueprintPure, Category = "Afterlight|Cinematic")
	EAfterlightCameraRegister GetReturnRegister() const { return Session.ReturnRegister; }

	const FAfterlightCinematicSession& GetSession() const { return Session; }

	UPROPERTY(BlueprintAssignable)
	FAfterlightCinematicFinished OnCinematicFinished;

private:
	UFUNCTION()
	void HandleSequenceFinished();
	AAfterlightPlayerController* ResolveController() const;
	void StopPlayer();

	FAfterlightCinematicSession Session;
	float BlendOut = 0.8f;
	FTimerHandle SafetyHandle;

	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> Player;

	UPROPERTY()
	TObjectPtr<ALevelSequenceActor> SequenceActor;
};
