#include "UI/AfterlightHUDWidget.h"
#include "UI/AfterlightPresentationFormat.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
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
#include "Types/SlateEnums.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Framework/Application/SlateApplication.h"

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
		UTextBlock* Dialogue = MakeText(TEXT("Dialogue"), FLinearColor(0.9f, 0.86f, 0.78f), 18);
		Dialogue->SetAutoWrapText(true);
		if (UCanvasPanelSlot* DialogueSlot = Cast<UCanvasPanel>(WidgetTree->RootWidget)->AddChildToCanvas(Dialogue))
		{
			DialogueSlot->SetAnchors(FAnchors(0.5f, 0.66f));
			DialogueSlot->SetAlignment(FVector2D(0.5f, 1.f));
			DialogueSlot->SetSize(FVector2D(760.f, 96.f));
		}
		DialogueBlock = Dialogue;
	}
	if (!ChoiceBlock.IsValid())
	{
		UTextBlock* Choices = MakeText(TEXT("Choices"), FLinearColor(0.96f, 0.93f, 0.86f), 22);
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
	if (!TitleScrim.IsValid())
	{
		UImage* Scrim = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("TitleScrim"));
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Brush.TintColor = FSlateColor(FLinearColor::Black);
		Scrim->SetBrush(Brush);
		Scrim->SetColorAndOpacity(FLinearColor::Black);
		if (UCanvasPanelSlot* ScrimSlot = Cast<UCanvasPanel>(WidgetTree->RootWidget)->AddChildToCanvas(Scrim))
		{
			ScrimSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
			ScrimSlot->SetOffsets(FMargin(0.f));
			ScrimSlot->SetZOrder(20);
		}
		Scrim->SetVisibility(ESlateVisibility::Hidden);
		TitleScrim = Scrim;
	}
	if (!TitleBlock.IsValid())
	{
		UTextBlock* Title = MakeText(TEXT("Title"), FLinearColor(0.95f, 0.93f, 0.88f), 42);
		if (UCanvasPanelSlot* TitleSlot = Cast<UCanvasPanel>(WidgetTree->RootWidget)->AddChildToCanvas(Title))
		{
			TitleSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			TitleSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			TitleSlot->SetAutoSize(true);
			TitleSlot->SetZOrder(21);
		}
		Title->SetVisibility(ESlateVisibility::Hidden);
		TitleBlock = Title;
	}
	if (!GuidanceBlock.IsValid())
	{
		UTextBlock* Guide = MakeText(TEXT("Guidance"), FLinearColor(0.86f, 0.84f, 0.78f), 18);
		Guide->SetJustification(ETextJustify::Center);
		if (UCanvasPanelSlot* GuideSlot = Cast<UCanvasPanel>(WidgetTree->RootWidget)->AddChildToCanvas(Guide))
		{
			GuideSlot->SetAnchors(FAnchors(0.5f, 0.88f));
			GuideSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			GuideSlot->SetAutoSize(true);
			GuideSlot->SetZOrder(22);
		}
		Guide->SetVisibility(ESlateVisibility::Hidden);
		GuidanceBlock = Guide;
	}
	if (!EndFooterBlock.IsValid())
	{
		UTextBlock* Footer = MakeText(TEXT("EndFooter"), FLinearColor(0.78f, 0.76f, 0.7f), 18);
		Footer->SetJustification(ETextJustify::Center);
		if (UCanvasPanelSlot* FooterSlot = Cast<UCanvasPanel>(WidgetTree->RootWidget)->AddChildToCanvas(Footer))
		{
			FooterSlot->SetAnchors(FAnchors(0.5f, 0.64f));
			FooterSlot->SetAlignment(FVector2D(0.5f, 0.f));
			FooterSlot->SetAutoSize(true);
			FooterSlot->SetZOrder(22);
		}
		Footer->SetVisibility(ESlateVisibility::Hidden);
		EndFooterBlock = Footer;
	}

	return Super::RebuildWidget();
}

void UAfterlightHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(false);
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UAfterlightHUDWidget::SetCineMode(bool bEnabled)
{
	bCineMode = bEnabled;
	if (DebugBlock.IsValid())
	{
		DebugBlock->SetVisibility((bEnabled || !bDebugVisible) ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	}
}

void UAfterlightHUDWidget::SetPrompt(const FText& Text)
{
	if (!PromptBlock.IsValid())
	{
		return;
	}
	const bool bShow = !Text.IsEmpty() && !bDialogueVisible && !bHoldCard;
	PromptBlock->SetText(Text);
	PromptBlock->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UAfterlightHUDWidget::SetDialogue(FName SpeakerId, const FText& Line, const TArray<FText>& Choices)
{
	bDialogueVisible = true;
	if (DialogueBlock.IsValid())
	{
		const FString Body = SpeakerId.IsNone() || SpeakerId == TEXT("Recording") || Choices.Num() > 0
			? Line.ToString()
			: FString::Printf(TEXT("%s\n%s"), *SpeakerId.ToString(), *Line.ToString());
		DialogueBlock->SetText(FText::FromString(Body));
		DialogueBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	if (ChoiceBlock.IsValid())
	{
		ChoiceBlock->SetText(FText::FromString(FAfterlightPresentationFormat::FormatChoiceList(Choices)));
		ChoiceBlock->SetVisibility(Choices.Num() > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
	if (PromptBlock.IsValid())
	{
		PromptBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	if (GuidanceBlock.IsValid() && !bHoldCard)
	{
		GuidanceBlock->SetVisibility(ESlateVisibility::Hidden);
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

void UAfterlightHUDWidget::SetTitle(const FText& Text)
{
	const bool bShow = !Text.IsEmpty();
	if (TitleScrim.IsValid())
	{
		TitleScrim->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
	if (!TitleBlock.IsValid())
	{
		return;
	}
	TitleBlock->SetText(Text);
	TitleBlock->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	if (!bShow && EndFooterBlock.IsValid())
	{
		EndFooterBlock->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UAfterlightHUDWidget::HideTitle()
{
	SetTitle(FText::GetEmpty());
}

void UAfterlightHUDWidget::SetTitleScrimVisible(bool bVisible)
{
	if (TitleScrim.IsValid())
	{
		TitleScrim->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
	if (TitleBlock.IsValid() && bVisible)
	{
		TitleBlock->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UAfterlightHUDWidget::SetGuidance(const FText& Text)
{
	if (!GuidanceBlock.IsValid())
	{
		return;
	}
	const bool bShow = !Text.IsEmpty();
	GuidanceBlock->SetText(Text);
	GuidanceBlock->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UAfterlightHUDWidget::SetEndFooter(const FText& Text)
{
	if (!EndFooterBlock.IsValid())
	{
		return;
	}
	const bool bShow = !Text.IsEmpty();
	EndFooterBlock->SetText(Text);
	EndFooterBlock->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UAfterlightHUDWidget::SetHoldCard(bool bHold)
{
	bHoldCard = bHold;
	SetIsFocusable(bHold);
	SetVisibility(bHold ? ESlateVisibility::Visible : ESlateVisibility::SelfHitTestInvisible);
	if (bHold && PromptBlock.IsValid())
	{
		PromptBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	if (!bHold && EndFooterBlock.IsValid())
	{
		EndFooterBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	if (bHold)
	{
		SetKeyboardFocus();
	}
	else if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
	}
}

bool UAfterlightHUDWidget::TryAcceptHoldCardInput(const FKey& Key)
{
	if (!bHoldCard)
	{
		return false;
	}
	if (Key != EKeys::LeftMouseButton && Key != EKeys::SpaceBar && Key != EKeys::Enter)
	{
		return false;
	}
	if (AAfterlightPlayerController* PC = GetOwningPlayer<AAfterlightPlayerController>())
	{
		return PC->TryOwnerContinueInput();
	}
	return false;
}

FReply UAfterlightHUDWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (TryAcceptHoldCardInput(InMouseEvent.GetEffectingButton()))
	{
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UAfterlightHUDWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (TryAcceptHoldCardInput(InKeyEvent.GetKey()))
	{
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UAfterlightHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bHoldCard && !HasKeyboardFocus())
	{
		SetKeyboardFocus();
	}
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
	FName RecipeId = NAME_None;
	FName ShotId = NAME_None;
	FName SequenceName = NAME_None;
	FString ViewTargetName = TEXT("(none)");
	FString Authority = TEXT("Gameplay");

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
		RecipeId = Cam->GetActiveRecipeId();
		ShotId = Cam->GetActiveShotId();
		Authority = AfterlightRegisterToName(Cam->GetCurrentRegister()).ToString();
		switch (Cam->GetAuthority())
		{
		case EAfterlightCameraAuthority::Sequencer: Authority = TEXT("Sequencer"); break;
		case EAfterlightCameraAuthority::Register: Authority = TEXT("Register"); break;
		default: Authority = TEXT("Gameplay"); break;
		}
		if (AActor* ViewTarget = Cam->GetCurrentViewTarget())
		{
			ViewTargetName = ViewTarget->GetName();
		}
	}
	if (UAfterlightCinematicCoordinator* Cine = World->GetSubsystem<UAfterlightCinematicCoordinator>())
	{
		bCine = Cine->IsCinematicActive();
		SequenceName = Cine->GetActiveSequenceName();
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

	const TCHAR* InputName = TEXT("Full");
	if (Input == EAfterlightInputState::Locked) InputName = TEXT("Locked");
	else if (Input == EAfterlightInputState::Constrained) InputName = TEXT("Constrained");
	else if (Input == EAfterlightInputState::Scripted) InputName = TEXT("Scripted");

	const FString Body = FString::Printf(
		TEXT("AFTERLIGHT debug\nFlags: %s\nBeat: %s\nTrust: %.2f  Suspicion: %.2f\nCamera: %s\nRecipe: %s\nShot: %s\nViewTarget: %s\nAuthority: %s\nSequence: %s\nInput: %s\nCinematic: %s\nCineMode: %s\nFollowDistance: %.0f\nH cine  F8 debug  F7 explore  F9 dialogue  F10 reveal  F5/F6 save/load  1/2 choices"),
		*Flags,
		*Beat.ToString(),
		Rel.Trust,
		Rel.Suspicion,
		*AfterlightRegisterToName(Camera).ToString(),
		*RecipeId.ToString(),
		*ShotId.ToString(),
		*ViewTargetName,
		*Authority,
		*SequenceName.ToString(),
		InputName,
		bCine ? TEXT("Active") : TEXT("Off"),
		bCineMode ? TEXT("On") : TEXT("Off"),
		Follow);
	SetDebugText(FText::FromString(Body));
}
