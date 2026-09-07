#pragma once

#include "CoreMinimal.h"

struct AFTERLIGHT_API FAfterlightPresentationFormat
{
	static FString FormatChoiceList(const TArray<FText>& Choices);
	static FString FormatInteractPrompt(const FText& Prompt);
	static float SpokenHoldSeconds(const FText& Line, float OverrideSeconds = 0.f);
	static float HideRecoveryAlpha(float Elapsed, float DurationSeconds);
};
