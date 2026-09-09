#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interaction/AfterlightInteractable.h"
#include "Character/AfterlightCompanion.h"
#include "AfterlightCompanionCharacter.generated.h"

class UAfterlightInteractableComponent;
class UAfterlightFramingTargetsComponent;

UCLASS()
class AFTERLIGHT_API AAfterlightCompanionCharacter : public ACharacter, public IAfterlightInteractable, public IAfterlightCompanion
{
	GENERATED_BODY()

public:
	AAfterlightCompanionCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FText GetPromptText() const override;
	virtual FGameplayTag GetInteractionVerb() const override;
	virtual void ExecuteInteraction(AActor* Interactor) override;

	virtual void NotifyDialogueStarted() override;
	virtual void NotifyDialogueEnded() override;
	virtual void ApplyPresentationTags(const FGameplayTagContainer& PresentationTags) override;

	float GetFollowDistance() const { return CurrentFollowDistance; }
	UAfterlightFramingTargetsComponent* GetFramingTargets() const { return Framing; }

	void SetLeadPath(const TArray<FVector>& Points, bool bWaitForPlayer);
	void ClearLeadPath();
	void SetMoveEnabled(bool bEnabled);
	void SetWorldLookTargets(const TArray<FVector>& Points);
	void SetPreferPlayerLook(bool bPreferPlayer);
	void GlanceAt(const FVector& WorldLocation);
	void HoldStill(float Seconds);
	void FaceToward(const FVector& WorldLocation, float HoldSeconds);
	bool HasReachedPathEnd() const;
	bool IsWaitingForPlayer() const { return bWaitingForPlayer; }

protected:
	void UpdateFacing(float DeltaSeconds);
	void UpdatePath(float DeltaSeconds);
	void UpdatePresence(float DeltaSeconds);
	APawn* ResolveProtagonist() const;

	UPROPERTY(VisibleAnywhere, Category = "Afterlight")
	TObjectPtr<UAfterlightInteractableComponent> Interactable;

	UPROPERTY(VisibleAnywhere, Category = "Afterlight")
	TObjectPtr<UAfterlightFramingTargetsComponent> Framing;

	UPROPERTY(EditAnywhere, Category = "Afterlight")
	float CloseFollowDistance = 140.f;

	UPROPERTY(EditAnywhere, Category = "Afterlight")
	float FarFollowDistance = 280.f;

	UPROPERTY(EditAnywhere, Category = "Afterlight")
	float DefaultFollowDistance = 200.f;

	float CurrentFollowDistance = 200.f;
	bool bInDialogue = false;
	bool bMoveEnabled = true;
	bool bWaitForPlayer = true;
	bool bWaitingForPlayer = false;
	bool bPreferPlayerLook = true;
	TArray<FVector> PathPoints;
	TArray<FVector> WorldLookTargets;
	int32 PathIndex = 0;
	FVector GlanceLocation = FVector::ZeroVector;
	float GlanceHold = 0.f;
	float PresenceTimer = 0.f;
	float PathPause = 0.f;
	int32 PresenceLookIndex = 0;
};
