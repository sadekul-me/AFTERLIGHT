#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Narrative/AfterlightNarrativeTypes.h"
#include "Relationship/AfterlightRelationshipTypes.h"
#include "AfterlightSaveGame.generated.h"

UCLASS()
class AFTERLIGHT_API UAfterlightSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Afterlight")
	FAfterlightNarrativeState Narrative;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Afterlight")
	FAfterlightRelationshipState Relationship;
};
