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

protected:
	void UpdateFacing(float DeltaSeconds);
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
};
