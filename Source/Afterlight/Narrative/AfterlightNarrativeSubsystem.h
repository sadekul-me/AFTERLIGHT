#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Narrative/AfterlightNarrativeTypes.h"
#include "AfterlightNarrativeSubsystem.generated.h"

class UAfterlightBeatAsset;
class UAfterlightDialogueAsset;
class UAfterlightDialogueRunner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAfterlightBeatChanged, FName, BeatId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAfterlightFlagsChanged, const FGameplayTagContainer&, Flags);

UCLASS()
class AFTERLIGHT_API UAfterlightNarrativeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category = "Afterlight|Narrative")
	bool HasFlag(FGameplayTag Flag) const;

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Narrative")
	void GrantFlag(FGameplayTag Flag);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Narrative")
	void GrantFlags(FGameplayTagContainer Flags);

	UFUNCTION(BlueprintPure, Category = "Afterlight|Narrative")
	FGameplayTagContainer GetFlags() const { return State.StoryFlags; }

	UFUNCTION(BlueprintPure, Category = "Afterlight|Narrative")
	FName GetCurrentBeatId() const { return State.CurrentBeatId; }

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Narrative")
	bool TryActivateBeat(UAfterlightBeatAsset* Beat);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Narrative")
	void SetCurrentBeatId(FName BeatId);

	UAfterlightDialogueRunner* GetDialogueRunner() const { return DialogueRunner; }

	FAfterlightNarrativeState GetState() const { return State; }
	void RestoreState(const FAfterlightNarrativeState& InState);

	UPROPERTY(BlueprintAssignable, Category = "Afterlight|Narrative")
	FAfterlightBeatChanged OnBeatChanged;

	UPROPERTY(BlueprintAssignable, Category = "Afterlight|Narrative")
	FAfterlightFlagsChanged OnFlagsChanged;

private:
	UPROPERTY()
	FAfterlightNarrativeState State;

	UPROPERTY()
	TObjectPtr<UAfterlightDialogueRunner> DialogueRunner;
};
