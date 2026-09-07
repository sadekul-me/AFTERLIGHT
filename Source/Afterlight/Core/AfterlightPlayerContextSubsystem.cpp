#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Character/AfterlightCharacter.h"
#include "Character/AfterlightPlayerController.h"

void UAfterlightPlayerContextSubsystem::RegisterProtagonist(AAfterlightCharacter* Character)
{
	Protagonist = Character;
}

void UAfterlightPlayerContextSubsystem::RegisterController(AAfterlightPlayerController* Controller)
{
	ProtagonistController = Controller;
}

AAfterlightCharacter* UAfterlightPlayerContextSubsystem::GetProtagonist() const
{
	return Protagonist.Get();
}

AAfterlightPlayerController* UAfterlightPlayerContextSubsystem::GetProtagonistController() const
{
	return ProtagonistController.Get();
}
