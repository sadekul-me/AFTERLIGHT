#include "UI/AfterlightHUDWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Character/AfterlightPlayerController.h"
#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Narrative/AfterlightNarrativeSubsystem.h"
#include "Relationship/AfterlightRelationshipSubsystem.h"
#include "Camera/AfterlightCameraSubsystem.h"
#include "Cinematic/AfterlightCinematicCoordinator.h"
#include "Character/AfterlightCompanionCharacter.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"

TSharedRef<SWidget> UAfterlightHUDWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}

	UCanvasPanel* Root = Cast<UCanvasPanel>(GetRootWidget());
	if (!Root)
	{
		Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;
	}

	auto MakeText = [this](const FName Name, const FLinearColor& Color, int32 Size) -> UTextBlock*
	{
		UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Block->SetColorAndOpacity(FSlateColor(Color));
		FSlateFontInfo Font = Block->GetFont();
		Font.Size = Size;
		Block->SetFont(Font);
		return Block;
	};

	if (!PromptBlock.IsValid())
	{
		UTextBlock* Prompt = MakeText(TEXT("Prompt"), FLinearColor(0.92f, 0.92f, 0.9f), 18);
		if (UCanvasPanelSlot* PromptSlot = Cast<UCanvasPanel>(WidgetTree->RootWidget)->AddChildToCanvas(Prompt))
		{
			PromptSlot->SetAnchors(FAnchors(0.5f, 0.82f));
			PromptSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			PromptSlot->SetAutoSize(true);
		}
		PromptBlock = Prompt;
	}
	if (!DialogueBlock.IsValid())
	{
		UTextBlock* Dialogue = MakeText(TEXT("Dialogue"), FLinearColor(0.95f, 0.9f, 0.82f), 20);
		Dialogue->SetAutoWrapText(true);
		if (UCanvasPanelSlot* DialogueSlot = Cast<UCanvasPanel>(WidgetTree->RootWidget)->AddChildToCanvas(Dialogue))
		{
			DialogueSlot->SetAnchors(FAnchors(0.5f, 0.7f));
			DialogueSlot->SetAlignment(FVector2D(0.5f, 1.f));
			DialogueSlot->SetSize(FVector2D(900.f, 120.f));
		}
		DialogueBlock = Dialogue;
	}
	if (!ChoiceBlock.IsValid())
	{
		UTextBlock* Choices = MakeText(TEXT("Choices"), FLinearColor(0.75f, 0.85f, 0.95f), 16);
		if (UCanvasPanelSlot* ChoiceSlot = Cast<UCanvasPanel>(WidgetTree->RootWidget)->AddChildToCanvas(Choices))
		{
			ChoiceSlot->SetAnchors(FAnchors(0.5f, 0.74f));
			ChoiceSlot->SetAlignment(FVector2D(0.5f, 0.f));
			ChoiceSlot->SetAutoSize(true);
		}
		ChoiceBlock = Choices;
	}
	if (!DebugBlock.IsValid())
	{
		UTextBlock* Debug = MakeText(TEXT("Debug"), FLinearColor(0.6f, 0.95f, 0.7f), 13);
		if (UCanvasPanelSlot* DebugSlot = Cast<UCanvasPanel>(WidgetTree->RootWidget)->AddChildToCanvas(Debug))
		{
			DebugSlot->SetAnchors(FAnchors(0.02f, 0.02f));
			DebugSlot->SetAlignment(FVector2D(0.f, 0.f));
			DebugSlot->SetAutoSize(true);
		}
		DebugBlock = Debug;
	}

	return Super::RebuildWidget();
}

void UAfterlightHUDWidget::SetCineMode(bool bEnabled)
{
	bCineMode = bEnabled;
	if (DebugBlock.IsValid())
	{
		DebugBlock->SetVisibility((bEnabled || !bDebugVisible) ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	}
	if (PromptBlock.IsValid() && !bDialogueVisible)
	{
		PromptBlock->SetVisibility(bEnabled ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	}
}

void UAfterlightHUDWidget::SetPrompt(const FText& Text)
{
	if (!PromptBlock.IsValid())
	{
		return;
	}
	const bool bShow = !Text.IsEmpty() && !bCineMode && !bDialogueVisible;
	PromptBlock->SetText(Text);
	PromptBlock->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UAfterlightHUDWidget::SetDialogue(FName SpeakerId, const FText& Line, const TArray<FText>& Choices)
{
	bDialogueVisible = true;
	if (DialogueBlock.IsValid())
	{
		const FString Body = SpeakerId.IsNone() ? Line.ToString() : FString::Printf(TEXT("%s: %s"), *SpeakerId.ToString(), *Line.ToString());
		DialogueBlock->SetText(FText::FromString(Body));
		DialogueBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	if (ChoiceBlock.IsValid())
	{
		FString ChoiceLines;
		for (int32 i = 0; i < Choices.Num(); ++i)
		{
			ChoiceLines += FString::Printf(TEXT("[%d] %s\n"), i + 1, *Choices[i].ToString());
		}
		ChoiceBlock->SetText(FText::FromString(ChoiceLines));
		ChoiceBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	if (PromptBlock.IsValid())
	{
		PromptBlock->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UAfterlightHUDWidget::HideDialogue()
{
	bDialogueVisible = false;
	if (DialogueBlock.IsValid())
	{
		DialogueBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	if (ChoiceBlock.IsValid())
	{
		ChoiceBlock->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UAfterlightHUDWidget::SetDebugText(const FText& Text)
{
	if (DebugBlock.IsValid())
	{
		DebugBlock->SetText(Text);
	}
}

void UAfterlightHUDWidget::SetDebugVisible(bool bVisible)
{
	bDebugVisible = bVisible;
	if (DebugBlock.IsValid())
	{
		DebugBlock->SetVisibility((!bVisible || bCineMode) ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	}
}

void UAfterlightHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshDebug();
}

void UAfterlightHUDWidget::RefreshDebug()
{
	if (bCineMode || !bDebugVisible)
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FString Flags = TEXT("(none)");
	FName Beat = NAME_None;
	FAfterlightRelationshipState Rel;
	EAfterlightCameraRegister Camera = EAfterlightCameraRegister::Explore;
	EAfterlightInputState Input = EAfterlightInputState::Full;
	bool bCine = false;
	float Follow = -1.f;

	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UAfterlightNarrativeSubsystem* Narrative = GI->GetSubsystem<UAfterlightNarrativeSubsystem>())
		{
			Flags = Narrative->GetFlags().ToStringSimple();
			Beat = Narrative->GetCurrentBeatId();
		}
		if (UAfterlightRelationshipSubsystem* Relationship = GI->GetSubsystem<UAfterlightRelationshipSubsystem>())
		{
			Rel = Relationship->GetState();
		}
	}
	if (UAfterlightCameraSubsystem* Cam = World->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera = Cam->GetCurrentRegister();
	}
	if (UAfterlightCinematicCoordinator* Cine = World->GetSubsystem<UAfterlightCinematicCoordinator>())
	{
		bCine = Cine->IsCinematicActive();
	}
	if (UAfterlightPlayerContextSubsystem* Context = World->GetSubsystem<UAfterlightPlayerContextSubsystem>())
	{
		if (AAfterlightPlayerController* PC = Context->GetProtagonistController())
		{
			Input = PC->GetInputState();
		}
	}
	for (TActorIterator<AAfterlightCompanionCharacter> It(World); It; ++It)
	{
		Follow = It->GetFollowDistance();
		break;
	}

	const FString Body = FString::Printf(
		TEXT("AFTERLIGHT debug\nFlags: %s\nBeat: %s\nTrust: %.2f  Suspicion: %.2f\nCamera: %s\nInput: %s\nCinematic: %s\nFollowDistance: %.0f\nH cine  F8 debug  F5/F6 save/load  1/2 choices"),
		*Flags,
		*Beat.ToString(),
		Rel.Trust,
		Rel.Suspicion,
		*AfterlightRegisterToName(Camera).ToString(),
		Input == EAfterlightInputState::Locked ? TEXT("Locked") : Input == EAfterlightInputState::Constrained ? TEXT("Constrained") : TEXT("Full"),
		bCine ? TEXT("Active") : TEXT("Off"),
		Follow);
	SetDebugText(FText::FromString(Body));
}
