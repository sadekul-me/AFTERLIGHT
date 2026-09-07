#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Narrative/AfterlightNarrativeTypes.h"
#include "Relationship/AfterlightRelationshipTypes.h"
#include "AfterlightSlice01Director.generated.h"

class AAfterlightInspectableActor;
class AAfterlightLanternDrone;
class AAfterlightCompanionCharacter;
class ACineCameraActor;
class APointLight;
class AStaticMeshActor;
class UAfterlightDialogueAsset;
class ULevelSequence;
class UAudioComponent;

UCLASS()
class AFTERLIGHT_API AAfterlightSlice01Director : public AActor
{
	GENERATED_BODY()

public:
	AAfterlightSlice01Director();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Slice01")
	void JumpCheckpoint(FName Checkpoint);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Slice01")
	void ResetSlice();

	bool TryAcceptContinue();
	bool TryReplay();
	bool IsAwaitingEntry() const { return bAwaitingEntry; }
	bool IsSliceComplete() const { return bSliceEnded; }
	void NotifyPlayerMoved();

	bool RunSliceSmoke(FString& OutReport);

protected:
	void ClearSliceRuntime();
	void BuildWorld();
	void BindSystems();
	void BuildDialogue();
	void SpawnShot(FName ShotId, const FVector& Location, const FVector& LookAt, uint8 Register);
	AStaticMeshActor* SpawnBox(const FVector& Location, const FVector& Scale, const FLinearColor& Color);
	void SpawnSign(const FVector& Location, const FRotator& Rotation, const FString& Text, float Size, const FColor& Color);
	APointLight* SpawnLight(const FVector& Location, const FLinearColor& Color, float Intensity, float Radius);

	void SetBeat(FName BeatId);
	void ApplyInputFull();
	void BeginWake();
	void BeginEntry();
	void UpdateOwnerGuidance(float DeltaSeconds);
	void BeginContact();
	void BeginCut();
	void BeginSweep();
	void FinishSweep();
	void BeginQuiet();
	void BeginTinReaction();
	void BeginWarning();
	void BeginAfterRecording();
	void BeginTitle();
	void AdvanceDialogue();
	void StartHideRecovery();
	void CompleteSweep();
	void UpdateHideRecovery(float DeltaSeconds);
	void UpdateWarningPresentation(float DeltaSeconds);
	void UpdateAudioBeds();
	void PullPlayerToHide();
	bool IsPlayerInHide() const;
	bool IsPlayerNearHide() const;
	bool ChoseFollow() const;
	float DialogueDelay(const FText& Line, float OverrideSeconds) const;
	void MaybeScheduleCommandLineSmoke();
	bool Check(bool bCondition, const TCHAR* Label, FString& OutReport);

	UFUNCTION()
	void HandleMayaTalk(AActor* Interactor);
	UFUNCTION()
	void HandleDoor(AActor* Interactor);
	UFUNCTION()
	void HandleTin(AActor* Interactor);
	UFUNCTION()
	void HandleLine(FName SpeakerId, const FText& Line);
	UFUNCTION()
	void HandleChoices(const TArray<FAfterlightDialogueChoice>& Choices);
	UFUNCTION()
	void HandleChoice(FAfterlightDialogueChoice Choice);
	UFUNCTION()
	void HandleDialogueFinished();
	UFUNCTION()
	void HandleRelationshipChanged(FAfterlightRelationshipState State);
	UFUNCTION()
	void HandleCinematicFinished();
	UFUNCTION()
	void HandleWarningLine();

	UPROPERTY()
	TObjectPtr<AAfterlightCompanionCharacter> Maya;
	UPROPERTY()
	TObjectPtr<AAfterlightInspectableActor> Door;
	UPROPERTY()
	TObjectPtr<AAfterlightInspectableActor> Tin;
	UPROPERTY()
	TObjectPtr<AAfterlightLanternDrone> Drone;
	UPROPERTY()
	TObjectPtr<ACineCameraActor> WarningCamera;
	UPROPERTY()
	TObjectPtr<APointLight> WitnessLed;
	UPROPERTY()
	TObjectPtr<APointLight> SlateLight;
	UPROPERTY()
	TObjectPtr<APointLight> HatchLight;
	UPROPERTY()
	TObjectPtr<APointLight> TinLight;
	UPROPERTY()
	TObjectPtr<AStaticMeshActor> WarningSlate;
	UPROPERTY()
	TObjectPtr<UAudioComponent> RainBed;
	UPROPERTY()
	TObjectPtr<UAudioComponent> HumBed;
	UPROPERTY()
	TObjectPtr<UAudioComponent> PumpBed;
	UPROPERTY()
	TObjectPtr<UAudioComponent> DroneBed;
	UPROPERTY()
	TObjectPtr<UAfterlightDialogueAsset> ContactDialogue;
	UPROPERTY()
	TObjectPtr<UAfterlightDialogueAsset> CutDialogue;
	UPROPERTY()
	TObjectPtr<UAfterlightDialogueAsset> QuietFollowDialogue;
	UPROPERTY()
	TObjectPtr<UAfterlightDialogueAsset> QuietQuestionDialogue;
	UPROPERTY()
	TObjectPtr<UAfterlightDialogueAsset> AfterWarningDialogue;
	UPROPERTY()
	TObjectPtr<ULevelSequence> WarningSequence;

	FTimerHandle WakeHandle;
	FTimerHandle DialogueAdvanceHandle;
	FTimerHandle SweepHandle;
	FTimerHandle QuietHoldHandle;
	FTimerHandle TitleHandle;
	FTimerHandle WarningLineHandle;
	FTimerHandle AutoTalkHandle;
	FTimerHandle TitleBlackHandle;
	FTimerHandle EntryHandle;
	FTimerHandle EndCardHandle;

	TArray<FText> WarningLines;
	int32 WarningLineIndex = 0;
	int32 GraphKind = 0;
	bool bBuilt = false;
	bool bSweepStarted = false;
	bool bContactStarted = false;
	bool bQuietStarted = false;
	bool bWarningStarted = false;
	bool bTitleStarted = false;
	bool bHideRecovering = false;
	bool bSweepResolved = false;
	bool bSmoke = false;
	bool bAwaitingEntry = false;
	bool bSliceEnded = false;
	bool bMoveHintShown = false;
	bool bLostHintShown = false;
	bool bFollowHintShown = false;
	bool bHideHintShown = false;
	float LostHintElapsed = 0.f;
	float FollowWaitElapsed = 0.f;
	float HideHintElapsed = 0.f;
	FVector HideRecoverStart = FVector::ZeroVector;
	FVector WarningCamStart = FVector::ZeroVector;
	float HideRecoverElapsed = 0.f;
	float HideRecoverDuration = 1.2f;
	float WarningPresentElapsed = 0.f;
};
