#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AfterlightHUDWidget.generated.h"

class UTextBlock;
class UImage;
class UVerticalBox;

UCLASS()
class AFTERLIGHT_API UAfterlightHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetCineMode(bool bEnabled);
	void SetPrompt(const FText& Text);
	void SetDialogue(FName SpeakerId, const FText& Line, const TArray<FText>& Choices);
	void HideDialogue();
	void SetDebugText(const FText& Text);
	void SetDebugVisible(bool bVisible);
	void SetTitle(const FText& Text);
	void HideTitle();
	void SetTitleScrimVisible(bool bVisible);
	void SetGuidance(const FText& Text);
	void SetEndFooter(const FText& Text);
	void SetHoldCard(bool bHold);
	bool IsDialogueVisible() const { return bDialogueVisible; }
	bool IsHoldCard() const { return bHoldCard; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	bool TryAcceptHoldCardInput(const FKey& Key);

private:
	void RefreshDebug();

	TWeakObjectPtr<UTextBlock> PromptBlock;
	TWeakObjectPtr<UTextBlock> DialogueBlock;
	TWeakObjectPtr<UTextBlock> ChoiceBlock;
	TWeakObjectPtr<UTextBlock> DebugBlock;
	TWeakObjectPtr<UTextBlock> TitleBlock;
	TWeakObjectPtr<UTextBlock> GuidanceBlock;
	TWeakObjectPtr<UTextBlock> EndFooterBlock;
	TWeakObjectPtr<UImage> TitleScrim;

	bool bCineMode = false;
	bool bDialogueVisible = false;
	bool bDebugVisible = false;
	bool bHoldCard = false;
	FName LastSpeakerId = NAME_None;
	float DialogueOpacity = 0.f;
	float ChoiceOpacity = 0.f;
	float DialogueTarget = 0.f;
	float ChoiceTarget = 0.f;
};
