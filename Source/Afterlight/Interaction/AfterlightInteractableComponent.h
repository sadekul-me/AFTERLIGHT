#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Interaction/AfterlightInteractable.h"
#include "AfterlightInteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAfterlightInteracted, AActor*, Instigator);

UCLASS(ClassGroup = (Afterlight), meta = (BlueprintSpawnableComponent))
class AFTERLIGHT_API UAfterlightInteractableComponent : public UActorComponent, public IAfterlightInteractable
{
	GENERATED_BODY()

public:
	UAfterlightInteractableComponent();

	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FText GetPromptText() const override;
	virtual FGameplayTag GetInteractionVerb() const override;
	virtual void ExecuteInteraction(AActor* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Afterlight")
	bool bAvailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Afterlight")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Afterlight")
	FGameplayTag Verb;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Afterlight")
	FGameplayTag GrantFlag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Afterlight")
	FGameplayTag RequiredFlag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Afterlight")
	FGameplayTag BlockedFlag;

	UPROPERTY(BlueprintAssignable, Category = "Afterlight")
	FAfterlightInteracted OnInteracted;
};
