#include "Cinematic/AfterlightCinematicCoordinator.h"
#include "Character/AfterlightPlayerController.h"
#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Camera/AfterlightCameraSubsystem.h"
#include "UI/AfterlightPresentationSubsystem.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "TimerManager.h"
#include "Core/AfterlightLog.h"

AAfterlightPlayerController* UAfterlightCinematicCoordinator::ResolveController() const
{
	if (const UAfterlightPlayerContextSubsystem* Context = GetWorld()->GetSubsystem<UAfterlightPlayerContextSubsystem>())
	{
		return Context->GetProtagonistController();
	}
	return nullptr;
}

void UAfterlightCinematicCoordinator::StopPlayer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SafetyHandle);
	}
	if (Player)
	{
		Player->OnFinished.RemoveAll(this);
		Player->Stop();
		Player = nullptr;
	}
	SequenceActor = nullptr;
}

void UAfterlightCinematicCoordinator::RequestCinematicControl()
{
	Session.Begin(NAME_None, EAfterlightCameraRegister::Explore);
	if (AAfterlightPlayerController* PC = ResolveController())
	{
		PC->SetInputState(EAfterlightInputState::Locked);
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->SetAuthority(EAfterlightCameraAuthority::Sequencer);
		Camera->RequestRegister(EAfterlightCameraRegister::Cinematic);
	}
}

void UAfterlightCinematicCoordinator::RequestCinematic(ULevelSequence* Sequence, EAfterlightCameraRegister ReturnRegister, float BlendOutTime)
{
	StopPlayer();
	BlendOut = BlendOutTime;
	Session.Begin(Sequence ? Sequence->GetFName() : FName(TEXT("TransientReveal")), ReturnRegister);

	if (AAfterlightPlayerController* PC = ResolveController())
	{
		PC->SetInputState(EAfterlightInputState::Locked);
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->HideDialogue();
		UI->ClearGuidance();
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->SetAuthority(EAfterlightCameraAuthority::Sequencer);
		Camera->RequestRegister(ReturnRegister == EAfterlightCameraRegister::Explore ? EAfterlightCameraRegister::Reveal : EAfterlightCameraRegister::Cinematic);
	}

	if (!Sequence)
	{
		UE_LOG(LogAfterlight, Log, TEXT("Cinematic requested without a sequence; holding Reveal then releasing."));
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			HandleSequenceFinished();
		});
		return;
	}

	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bPauseAtEnd = false;
	Settings.bHideHud = false;
	ALevelSequenceActor* OutActor = nullptr;
	Player = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), Sequence, Settings, OutActor);
	SequenceActor = OutActor;
	if (!Player)
	{
		UE_LOG(LogAfterlight, Warning, TEXT("Failed to create Level Sequence player; releasing cinematic control."));
		HandleSequenceFinished();
		return;
	}
	Player->OnFinished.AddDynamic(this, &UAfterlightCinematicCoordinator::HandleSequenceFinished);
	const float SafetySeconds = FMath::Max(0.75f, Player->GetDuration().AsSeconds() + 0.25f);
	GetWorld()->GetTimerManager().SetTimer(SafetyHandle, this, &UAfterlightCinematicCoordinator::HandleSequenceFinished, SafetySeconds, false);
	Player->Play();
}

void UAfterlightCinematicCoordinator::HandleSequenceFinished()
{
	if (!Session.bActive)
	{
		StopPlayer();
		return;
	}
	const EAfterlightCameraRegister ReturnRegister = Session.ReturnRegister;
	const float OutBlend = BlendOut;
	StopPlayer();
	Session.Complete();
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->SetAuthority(EAfterlightCameraAuthority::Gameplay);
		Camera->RequestRegister(ReturnRegister, OutBlend);
	}
	if (AAfterlightPlayerController* PC = ResolveController())
	{
		PC->SetInputState(EAfterlightInputState::Full);
	}
	OnCinematicFinished.Broadcast();
}

void UAfterlightCinematicCoordinator::ReleaseCinematic()
{
	if (!Session.bActive)
	{
		StopPlayer();
		return;
	}
	HandleSequenceFinished();
}

void UAfterlightCinematicCoordinator::CancelCinematic()
{
	if (!Session.bActive)
	{
		return;
	}
	UE_LOG(LogAfterlight, Log, TEXT("Cinematic cancelled."));
	HandleSequenceFinished();
}
