#include "UI/AfterlightPresentationFormat.h"

FString FAfterlightPresentationFormat::FormatChoiceList(const TArray<FText>& Choices)
{
	FString Out;
	for (int32 i = 0; i < Choices.Num(); ++i)
	{
		if (i > 0)
		{
			Out += TEXT("\n\n");
		}
		Out += FString::Printf(TEXT("%d    %s"), i + 1, *Choices[i].ToString());
	}
	return Out;
}

FString FAfterlightPresentationFormat::FormatInteractPrompt(const FText& Prompt)
{
	const FString Body = Prompt.ToString().TrimStartAndEnd();
	if (Body.IsEmpty())
	{
		return FString();
	}
	if (Body.StartsWith(TEXT("E ")) || Body.StartsWith(TEXT("E\t")))
	{
		return Body;
	}
	return FString::Printf(TEXT("E    %s"), *Body);
}

float FAfterlightPresentationFormat::SpokenHoldSeconds(const FText& Line, float OverrideSeconds)
{
	if (OverrideSeconds > 0.05f)
	{
		return OverrideSeconds;
	}
	const int32 Len = Line.ToString().Len();
	return FMath::Clamp(Len * 0.055f + 0.8f, 2.0f, 6.4f);
}

float FAfterlightPresentationFormat::HideRecoveryAlpha(float Elapsed, float DurationSeconds)
{
	const float T = FMath::Clamp(Elapsed / FMath::Max(0.05f, DurationSeconds), 0.f, 1.f);
	return T * T * (3.f - 2.f * T);
}
