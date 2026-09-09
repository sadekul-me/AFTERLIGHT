#pragma once

#include "CoreMinimal.h"

class UWorld;
class UObject;

struct AFTERLIGHT_API FAfterlightVoice
{
	static FString SpeakerFolder(FName SpeakerId);
	static FString ResolveFilePath(FName SpeakerId, FName VoiceId);
	static float FindDuration(FName SpeakerId, FName VoiceId);
	static float Play(UWorld* World, UObject* Outer, FName SpeakerId, FName VoiceId);
};
