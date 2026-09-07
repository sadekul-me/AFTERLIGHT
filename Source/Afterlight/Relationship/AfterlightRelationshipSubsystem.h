#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Relationship/AfterlightRelationshipTypes.h"
#include "AfterlightRelationshipSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAfterlightRelationshipChanged, FAfterlightRelationshipState, State);

UCLASS()
class AFTERLIGHT_API UAfterlightRelationshipSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UAfterlightRelationshipSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category = "Afterlight|Relationship")
	FAfterlightRelationshipState GetState() const { return State; }

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Relationship")
	void ApplyDelta(float TrustDelta, float SuspicionDelta);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Relationship")
	void SetState(FAfterlightRelationshipState NewState);

	UFUNCTION(BlueprintPure, Category = "Afterlight|Relationship")
	FGameplayTagContainer GetPresentationTags() const;

	void RestoreState(const FAfterlightRelationshipState& InState);

	UPROPERTY(BlueprintAssignable, Category = "Afterlight|Relationship")
	FAfterlightRelationshipChanged OnRelationshipChanged;

	UPROPERTY(EditAnywhere, Category = "Afterlight|Relationship")
	FAfterlightRelationshipThresholds Thresholds;

private:
	void Broadcast();

	UPROPERTY()
	FAfterlightRelationshipState State;
};
