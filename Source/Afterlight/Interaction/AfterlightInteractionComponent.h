#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AfterlightInteractionComponent.generated.h"

class IAfterlightInteractable;

UCLASS(ClassGroup = (Afterlight), meta = (BlueprintSpawnableComponent))
class AFTERLIGHT_API UAfterlightInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAfterlightInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool TryInteract();
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }
	FText GetCurrentPrompt() const { return CurrentPrompt; }

	UPROPERTY(EditAnywhere, Category = "Afterlight")
	float TraceDistance = 280.f;

	UPROPERTY(EditAnywhere, Category = "Afterlight")
	float Radius = 80.f;

private:
	void RefreshTarget();
	IAfterlightInteractable* ResolveInterface(AActor* Actor) const;
	void ConsiderActor(AActor* Actor, AActor* Owner, const FVector& From, const FVector& Forward, AActor*& Best, float& BestScore) const;

	TWeakObjectPtr<AActor> CurrentTarget;
	FText CurrentPrompt;
};
