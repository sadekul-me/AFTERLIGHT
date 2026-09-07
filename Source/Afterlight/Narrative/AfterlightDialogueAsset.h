#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Narrative/AfterlightNarrativeTypes.h"
#include "AfterlightDialogueAsset.generated.h"

UCLASS(BlueprintType)
class AFTERLIGHT_API UAfterlightDialogueAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FName EntryNodeId = TEXT("Start");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	TArray<FAfterlightDialogueNode> Nodes;

	const FAfterlightDialogueNode* FindNode(FName NodeId) const;
};
