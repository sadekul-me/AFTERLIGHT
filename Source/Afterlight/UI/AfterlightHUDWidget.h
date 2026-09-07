#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AfterlightHUDWidget.generated.h"

class UTextBlock;
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
	bool IsDialogueVisible() const { return bDialogueVisible; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void RefreshDebug();

	TWeakObjectPtr<UTextBlock> PromptBlock;
	TWeakObjectPtr<UTextBlock> DialogueBlock;
	TWeakObjectPtr<UTextBlock> ChoiceBlock;
	TWeakObjectPtr<UTextBlock> DebugBlock;

	bool bCineMode = false;
	bool bDialogueVisible = false;
	bool bDebugVisible = true;
};
