#include "Save/AfterlightSaveSubsystem.h"
#include "Save/AfterlightSaveGame.h"
#include "Narrative/AfterlightNarrativeSubsystem.h"
#include "Relationship/AfterlightRelationshipSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Core/AfterlightLog.h"

const FString UAfterlightSaveSubsystem::TestSlotName = TEXT("AfterlightDevSlot");

bool UAfterlightSaveSubsystem::SaveTestSlot()
{
	UAfterlightSaveGame* Save = Cast<UAfterlightSaveGame>(UGameplayStatics::CreateSaveGameObject(UAfterlightSaveGame::StaticClass()));
	if (!Save)
	{
		return false;
	}
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Save->Narrative = Narrative->GetState();
	}
	if (UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>())
	{
		Save->Relationship = Relationship->GetState();
	}
	const bool bOk = UGameplayStatics::SaveGameToSlot(Save, TestSlotName, 0);
	UE_LOG(LogAfterlight, Log, TEXT("SaveTestSlot %s"), bOk ? TEXT("ok") : TEXT("failed"));
	return bOk;
}

bool UAfterlightSaveSubsystem::LoadTestSlot()
{
	UAfterlightSaveGame* Save = Cast<UAfterlightSaveGame>(UGameplayStatics::LoadGameFromSlot(TestSlotName, 0));
	if (!Save)
	{
		UE_LOG(LogAfterlight, Warning, TEXT("LoadTestSlot: no save"));
		return false;
	}
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->RestoreState(Save->Narrative);
	}
	if (UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>())
	{
		Relationship->RestoreState(Save->Relationship);
	}
	UE_LOG(LogAfterlight, Log, TEXT("LoadTestSlot ok"));
	return true;
}
