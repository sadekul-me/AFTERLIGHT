#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Narrative/AfterlightNarrativeTypes.h"
#include "AfterlightDialogueRunner.generated.h"

class UAfterlightDialogueAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAfterlightDialogueLine, FName, SpeakerId, const FText&, Line);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAfterlightDialogueChoices, const TArray<FAfterlightDialogueChoice>&, Choices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAfterlightDialogueFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAfterlightDialogueChoiceMade, FAfterlightDialogueChoice, Choice);

UCLASS()
class AFTERLIGHT_API UAfterlightDialogueRunner : public UObject
{
	GENERATED_BODY()

public:
	void Start(UAfterlightDialogueAsset* Asset);
	void StartGraph(const TArray<FAfterlightDialogueNode>& Nodes, FName EntryNodeId);
	bool SelectChoice(int32 ChoiceIndex);
	void Abort();
	bool IsActive() const { return bActive; }
	const FAfterlightDialogueNode* GetCurrentNode() const { return CurrentNode; }

	UPROPERTY(BlueprintAssignable)
	FAfterlightDialogueLine OnLinePresented;

	UPROPERTY(BlueprintAssignable)
	FAfterlightDialogueChoices OnChoicesPresented;

	UPROPERTY(BlueprintAssignable)
	FAfterlightDialogueFinished OnFinished;

	UPROPERTY(BlueprintAssignable)
	FAfterlightDialogueChoiceMade OnChoiceMade;

private:
	void PresentNode(const FAfterlightDialogueNode& Node);

	UPROPERTY()
	TObjectPtr<UAfterlightDialogueAsset> ActiveAsset;

	TArray<FAfterlightDialogueNode> RuntimeNodes;
	const FAfterlightDialogueNode* CurrentNode = nullptr;
	bool bActive = false;

	const FAfterlightDialogueNode* FindRuntimeNode(FName NodeId) const;
};
