#include "UI/AfterlightPresentationSubsystem.h"
#include "UI/AfterlightHUDWidget.h"
#include "UI/AfterlightPresentationFormat.h"
#include "Blueprint/UserWidget.h"
#include "Character/AfterlightPlayerController.h"
#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Interaction/AfterlightInteractionComponent.h"
#include "Character/AfterlightCharacter.h"
#include "Cinematic/AfterlightCinematicCoordinator.h"

void UAfterlightPresentationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	EnsureWidget();
}

void UAfterlightPresentationSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	EnsureWidget();

	if (GuidanceSecondsRemaining > 0.f)
	{
		GuidanceSecondsRemaining = FMath::Max(0.f, GuidanceSecondsRemaining - DeltaTime);
		if (GuidanceSecondsRemaining <= 0.f && Widget && !Widget->IsHoldCard())
		{
			Widget->SetGuidance(FText::GetEmpty());
		}
	}

	bool bSequenceActive = false;
	if (UWorld* World = GetWorld())
	{
		if (UAfterlightCinematicCoordinator* Cinematic = World->GetSubsystem<UAfterlightCinematicCoordinator>())
		{
			bSequenceActive = Cinematic->IsCinematicActive();
		}
	}

	const bool bSuppressDev = bCineMode || bSequenceActive;
	if (Widget)
	{
		Widget->SetDebugVisible(bDebugVisible && !bSuppressDev);
		if (bSequenceActive)
		{
			Widget->SetPrompt(FText::GetEmpty());
		}
	}
	if (bSequenceActive)
	{
		return;
	}
	if (Widget && Widget->IsHoldCard())
	{
		return;
	}
	if (UAfterlightPlayerContextSubsystem* Context = GetWorld()->GetSubsystem<UAfterlightPlayerContextSubsystem>())
	{
		if (AAfterlightCharacter* Protagonist = Context->GetProtagonist())
		{
			if (UAfterlightInteractionComponent* Interaction = Protagonist->GetInteractionComponent())
			{
				SetPrompt(Interaction->GetCurrentPrompt());
			}
		}
	}
}

void UAfterlightPresentationSubsystem::EnsureWidget()
{
	if (Widget)
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	AAfterlightPlayerController* PC = nullptr;
	if (UAfterlightPlayerContextSubsystem* Context = World->GetSubsystem<UAfterlightPlayerContextSubsystem>())
	{
		PC = Context->GetProtagonistController();
	}
	if (!PC)
	{
		return;
	}
	Widget = CreateWidget<UAfterlightHUDWidget>(PC);
	if (Widget)
	{
		Widget->AddToViewport(10);
		Widget->SetCineMode(bCineMode);
		Widget->SetDebugVisible(bDebugVisible);
	}
}

void UAfterlightPresentationSubsystem::ToggleCineMode()
{
	SetCineMode(!bCineMode);
}

void UAfterlightPresentationSubsystem::SetCineMode(bool bEnabled)
{
	bCineMode = bEnabled;
	EnsureWidget();
	if (Widget)
	{
		Widget->SetCineMode(bCineMode);
		Widget->SetDebugVisible(false);
	}
}

void UAfterlightPresentationSubsystem::ToggleDebugOverlay()
{
	SetDeveloperOverlay(!bDebugVisible);
}

void UAfterlightPresentationSubsystem::SetDeveloperOverlay(bool bVisible)
{
	bDebugVisible = bVisible;
	EnsureWidget();
	if (Widget)
	{
		Widget->SetDebugVisible(bDebugVisible && !bCineMode);
	}
}

void UAfterlightPresentationSubsystem::SetPrompt(const FText& Text)
{
	EnsureWidget();
	if (Widget)
	{
		Widget->SetPrompt(FText::FromString(FAfterlightPresentationFormat::FormatInteractPrompt(Text)));
	}
}

void UAfterlightPresentationSubsystem::ShowDialogue(FName SpeakerId, const FText& Line, const TArray<FText>& Choices)
{
	EnsureWidget();
	if (Widget)
	{
		Widget->SetDialogue(SpeakerId, Line, Choices);
	}
}

void UAfterlightPresentationSubsystem::HideDialogue()
{
	if (Widget)
	{
		Widget->HideDialogue();
	}
}

void UAfterlightPresentationSubsystem::ShowTitle(const FText& Text)
{
	EnsureWidget();
	if (Widget)
	{
		Widget->SetHoldCard(!Text.IsEmpty());
		Widget->SetTitle(Text);
		if (Text.IsEmpty())
		{
			Widget->SetEndFooter(FText::GetEmpty());
		}
	}
}

void UAfterlightPresentationSubsystem::HideTitle()
{
	if (Widget)
	{
		Widget->SetHoldCard(false);
		Widget->HideTitle();
		Widget->SetEndFooter(FText::GetEmpty());
		Widget->SetGuidance(FText::GetEmpty());
	}
	GuidanceSecondsRemaining = 0.f;
}

void UAfterlightPresentationSubsystem::ShowBlackScrim()
{
	EnsureWidget();
	if (Widget)
	{
		Widget->SetTitleScrimVisible(true);
	}
}

void UAfterlightPresentationSubsystem::ShowGuidance(const FText& Text, float DurationSeconds)
{
	EnsureWidget();
	if (Widget && Widget->IsHoldCard())
	{
		return;
	}
	if (Widget)
	{
		Widget->SetGuidance(Text);
	}
	GuidanceSecondsRemaining = DurationSeconds;
}

void UAfterlightPresentationSubsystem::ClearGuidance()
{
	GuidanceSecondsRemaining = 0.f;
	if (Widget && !Widget->IsHoldCard())
	{
		Widget->SetGuidance(FText::GetEmpty());
	}
}

void UAfterlightPresentationSubsystem::ShowEntryCard()
{
	EnsureWidget();
	if (!Widget)
	{
		return;
	}
	Widget->SetHoldCard(true);
	Widget->SetTitleScrimVisible(true);
	Widget->SetTitle(NSLOCTEXT("Afterlight", "Title", "AFTERLIGHT"));
	Widget->SetEndFooter(FText::GetEmpty());
	Widget->SetGuidance(NSLOCTEXT("Afterlight", "BeginPrompt", "Click / Press any key to begin"));
	Widget->SetPrompt(FText::GetEmpty());
	GuidanceSecondsRemaining = 0.f;
}

void UAfterlightPresentationSubsystem::ShowEndCard()
{
	EnsureWidget();
	if (!Widget)
	{
		return;
	}
	Widget->SetHoldCard(true);
	Widget->SetTitleScrimVisible(true);
	Widget->SetTitle(NSLOCTEXT("Afterlight", "Title", "AFTERLIGHT"));
	Widget->SetEndFooter(NSLOCTEXT("Afterlight", "EndCard", "END OF VERTICAL SLICE\n\nEsc — Exit\nR — Replay"));
	Widget->SetGuidance(FText::GetEmpty());
	Widget->SetPrompt(FText::GetEmpty());
	Widget->HideDialogue();
	GuidanceSecondsRemaining = 0.f;
}

bool UAfterlightPresentationSubsystem::IsHoldCard() const
{
	return Widget && Widget->IsHoldCard();
}
