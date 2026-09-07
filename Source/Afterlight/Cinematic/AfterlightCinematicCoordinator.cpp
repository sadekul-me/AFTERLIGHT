#include "Cinematic/AfterlightCinematicCoordinator.h"
#include "Character/AfterlightPlayerController.h"
#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Camera/AfterlightCameraSubsystem.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "Core/AfterlightLog.h"

AAfterlightPlayerController* UAfterlightCinematicCoordinator::ResolveController() const
{
	if (const UAfterlightPlayerContextSubsystem* Context = GetWorld()->GetSubsystem<UAfterlightPlayerContextSubsystem>())
	{
		return Context->GetProtagonistController();
	}
	return nullptr;
}

void UAfterlightCinematicCoordinator::RequestCinematicControl()
{
	bActive = true;
	if (AAfterlightPlayerController* PC = ResolveController())
	{
		PC->SetInputState(EAfterlightInputState::Locked);
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestRegister(EAfterlightCameraRegister::Cinematic);
	}
}

void UAfterlightCinematicCoordinator::RequestCinematic(ULevelSequence* Sequence)
{
	RequestCinematicControl();
	if (!Sequence)
	{
		UE_LOG(LogAfterlight, Log, TEXT("Cinematic requested without a sequence; holding control briefly."));
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			HandleSequenceFinished();
		});
		return;
	}

	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bPauseAtEnd = true;
	ALevelSequenceActor* OutActor = nullptr;
	Player = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), Sequence, Settings, OutActor);
	SequenceActor = OutActor;
	if (!Player)
	{
		HandleSequenceFinished();
		return;
	}
	Player->OnFinished.AddDynamic(this, &UAfterlightCinematicCoordinator::HandleSequenceFinished);
	Player->Play();
}

void UAfterlightCinematicCoordinator::HandleSequenceFinished()
{
	ReleaseCinematic();
	OnCinematicFinished.Broadcast();
}

void UAfterlightCinematicCoordinator::ReleaseCinematic()
{
	if (Player)
	{
		Player->OnFinished.RemoveAll(this);
		Player = nullptr;
	}
	SequenceActor = nullptr;
	bActive = false;
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->ReleaseToExplore(0.7f);
	}
	if (AAfterlightPlayerController* PC = ResolveController())
	{
		PC->SetInputState(EAfterlightInputState::Full);
	}
}
