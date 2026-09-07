#include "UI/AfterlightPresentationSubsystem.h"
#include "UI/AfterlightHUDWidget.h"
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
		if (bSuppressDev)
		{
			Widget->SetPrompt(FText::GetEmpty());
		}
	}
	if (bSuppressDev)
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
		if (bCineMode)
		{
			Widget->SetPrompt(FText::GetEmpty());
		}
	}
}

void UAfterlightPresentationSubsystem::ToggleDebugOverlay()
{
	bDebugVisible = !bDebugVisible;
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
		Widget->SetPrompt(bCineMode ? FText::GetEmpty() : Text);
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
		Widget->SetTitle(Text);
	}
}

void UAfterlightPresentationSubsystem::HideTitle()
{
	if (Widget)
	{
		Widget->HideTitle();
	}
}
