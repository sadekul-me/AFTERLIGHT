#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AfterlightPresentationSubsystem.generated.h"

class UAfterlightHUDWidget;

UCLASS()
class AFTERLIGHT_API UAfterlightPresentationSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UAfterlightPresentationSubsystem, STATGROUP_Tickables); }

	UFUNCTION(BlueprintCallable, Category = "Afterlight|UI")
	void ToggleCineMode();

	UFUNCTION(BlueprintCallable, Category = "Afterlight|UI")
	void SetCineMode(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Afterlight|UI")
	bool IsCineMode() const { return bCineMode; }

	UFUNCTION(BlueprintCallable, Category = "Afterlight|UI")
	void ToggleDebugOverlay();

	UFUNCTION(BlueprintCallable, Category = "Afterlight|UI")
	void SetPrompt(const FText& Text);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|UI")
	void ShowDialogue(FName SpeakerId, const FText& Line, const TArray<FText>& Choices);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|UI")
	void HideDialogue();

	UFUNCTION(BlueprintCallable, Category = "Afterlight|UI")
	void ShowTitle(const FText& Text);

	UFUNCTION(BlueprintCallable, Category = "Afterlight|UI")
	void HideTitle();

	UAfterlightHUDWidget* GetWidget() const { return Widget; }

private:
	void EnsureWidget();

	bool bCineMode = false;
	bool bDebugVisible = true;

	UPROPERTY()
	TObjectPtr<UAfterlightHUDWidget> Widget;
};
