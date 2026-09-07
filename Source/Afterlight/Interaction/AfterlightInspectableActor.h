#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AfterlightInteractable.h"
#include "AfterlightInspectableActor.generated.h"

class UStaticMeshComponent;
class UAfterlightInteractableComponent;

UCLASS()
class AFTERLIGHT_API AAfterlightInspectableActor : public AActor, public IAfterlightInteractable
{
	GENERATED_BODY()

public:
	AAfterlightInspectableActor();

	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FText GetPromptText() const override;
	virtual FGameplayTag GetInteractionVerb() const override;
	virtual void ExecuteInteraction(AActor* Interactor) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Afterlight")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Afterlight")
	TObjectPtr<UAfterlightInteractableComponent> Interactable;
};
