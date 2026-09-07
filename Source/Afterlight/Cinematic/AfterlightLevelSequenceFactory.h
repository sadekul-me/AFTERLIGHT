#pragma once

#include "CoreMinimal.h"
#include "AfterlightLevelSequenceFactory.generated.h"

class ULevelSequence;
class ACineCameraActor;

UCLASS()
class AFTERLIGHT_API UAfterlightLevelSequenceFactory : public UObject
{
	GENERATED_BODY()

public:
	static ULevelSequence* CreateInspectRevealSequence(UObject* Outer, ACineCameraActor* RevealCamera, float DurationSeconds = 3.f);
	static ULevelSequence* CreateWarningSequence(UObject* Outer, ACineCameraActor* SlateCamera, float DurationSeconds = 18.f);
};
