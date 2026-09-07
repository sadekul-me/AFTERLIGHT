#pragma once

#include "CoreMinimal.h"

class UObject;
class USoundWaveProcedural;
class UAudioComponent;
class UWorld;

enum class EAfterlightTempBed : uint8
{
	Silence,
	Rain,
	Electric,
	Drone,
	Pump,
	Warning,
	Sting
};

struct AFTERLIGHT_API FAfterlightTempAudio
{
	static USoundWaveProcedural* CreateBed(UObject* Outer, FName Name, EAfterlightTempBed Bed);
	static UAudioComponent* SpawnLoop(UWorld* World, UObject* Outer, FName Name, EAfterlightTempBed Bed, float Volume);
	static void PlayOneShot(UWorld* World, UObject* Outer, EAfterlightTempBed Bed, float Volume);
};
