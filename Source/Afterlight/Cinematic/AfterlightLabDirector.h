#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Narrative/AfterlightNarrativeTypes.h"
#include "Relationship/AfterlightRelationshipTypes.h"
#include "Camera/AfterlightCameraRecipe.h"
#include "AfterlightLabDirector.generated.h"

class AAfterlightCompanionCharacter;
class AAfterlightInspectableActor;
class ACineCameraActor;
class UAfterlightDialogueAsset;
class ULevelSequence;

UCLASS()
class AFTERLIGHT_API AAfterlightLabDirector : public AActor
{
	GENERATED_BODY()

public:
	AAfterlightLabDirector();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Debug")
	void ResetTechnicalFlow();

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Debug")
	void PlayInspectReveal();

	/** Drives talk → choice → inspect → save/load without requiring a human to click. */
	bool RunTechnicalSmoke(FString& OutReport);

protected:
	void BuildGreybox();
	void BindSystems();
	UAfterlightDialogueAsset* BuildPlaceholderDialogue();
	ACineCameraActor* SpawnShot(FName ShotId, const FVector& Location, const FVector& LookAt, EAfterlightCameraRegister Register);
	void BuildInspectSequence();

	UFUNCTION()
	void HandleCompanionTalk(AActor* Interactor);

	UFUNCTION()
	void HandleInspect(AActor* Interactor);

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

	void RestoreGameplayPresentation();
	void MaybeScheduleCommandLineSmoke();
	bool Check(bool bCondition, const TCHAR* Label, FString& OutReport);

	UPROPERTY()
	TObjectPtr<AAfterlightCompanionCharacter> Companion;

	UPROPERTY()
	TObjectPtr<AAfterlightInspectableActor> Inspectable;

	UPROPERTY()
	TObjectPtr<UAfterlightDialogueAsset> PlaceholderDialogue;

	UPROPERTY()
	TObjectPtr<ACineCameraActor> RevealCamera;

	UPROPERTY()
	TObjectPtr<ULevelSequence> InspectSequence;

	FTimerHandle InspectCinematicHandle;
	FTimerHandle DialogueHoldHandle;

	bool bBuilt = false;
};

