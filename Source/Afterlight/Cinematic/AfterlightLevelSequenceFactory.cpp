#include "Cinematic/AfterlightLevelSequenceFactory.h"
#include "LevelSequence.h"
#include "MovieScene.h"
#include "MovieSceneSection.h"
#include "Tracks/MovieSceneCameraCutTrack.h"
#include "Sections/MovieSceneCameraCutSection.h"
#include "MovieSceneObjectBindingID.h"
#include "CineCameraActor.h"
#include "Engine/World.h"
#include "Core/AfterlightLog.h"

ULevelSequence* UAfterlightLevelSequenceFactory::CreateInspectRevealSequence(UObject* Outer, ACineCameraActor* RevealCamera, float DurationSeconds)
{
	if (!Outer)
	{
		return nullptr;
	}

	ULevelSequence* Sequence = NewObject<ULevelSequence>(Outer, TEXT("LS_LabInspectReveal"));
	Sequence->Initialize();
	UMovieScene* MovieScene = Sequence->GetMovieScene();
	if (!MovieScene)
	{
		UE_LOG(LogAfterlight, Warning, TEXT("Failed to initialize inspect reveal Level Sequence."));
		return Sequence;
	}

	const FFrameRate Tick = MovieScene->GetTickResolution();
	const FFrameNumber Duration = (FMath::Max(0.5f, DurationSeconds) * Tick).RoundToFrame();
	MovieScene->SetPlaybackRange(0, Duration.Value);

	if (RevealCamera)
	{
		const FGuid CameraGuid = MovieScene->AddPossessable(TEXT("RevealCamera"), RevealCamera->GetClass());
		UObject* BindingContext = Outer->GetWorld() ? static_cast<UObject*>(Outer->GetWorld()) : Outer;
		Sequence->BindPossessableObject(CameraGuid, *RevealCamera, BindingContext);
		if (UMovieSceneCameraCutTrack* CutTrack = Cast<UMovieSceneCameraCutTrack>(MovieScene->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass())))
		{
			const FMovieSceneObjectBindingID BindingID = UE::MovieScene::FRelativeObjectBindingID(CameraGuid);
			if (UMovieSceneCameraCutSection* Section = CutTrack->AddNewCameraCut(BindingID, FFrameNumber(0)))
			{
				Section->SetRange(TRange<FFrameNumber>(FFrameNumber(0), Duration));
			}
		}
	}

	UE_LOG(LogAfterlight, Log, TEXT("Created technical inspect Level Sequence (%.2fs)."), DurationSeconds);
	return Sequence;
}

ULevelSequence* UAfterlightLevelSequenceFactory::CreateWarningSequence(UObject* Outer, ACineCameraActor* SlateCamera, float DurationSeconds)
{
	if (!Outer)
	{
		return nullptr;
	}

	ULevelSequence* Sequence = NewObject<ULevelSequence>(Outer, TEXT("LS_Slice01_Warning"));
	Sequence->Initialize();
	UMovieScene* MovieScene = Sequence->GetMovieScene();
	if (!MovieScene)
	{
		UE_LOG(LogAfterlight, Warning, TEXT("Failed to initialize Slice01 warning Level Sequence."));
		return Sequence;
	}

	const FFrameRate Tick = MovieScene->GetTickResolution();
	const FFrameNumber Duration = (FMath::Max(0.5f, DurationSeconds) * Tick).RoundToFrame();
	MovieScene->SetPlaybackRange(0, Duration.Value);

	if (SlateCamera)
	{
		const FGuid CameraGuid = MovieScene->AddPossessable(TEXT("WarningCamera"), SlateCamera->GetClass());
		UObject* BindingContext = Outer->GetWorld() ? static_cast<UObject*>(Outer->GetWorld()) : Outer;
		Sequence->BindPossessableObject(CameraGuid, *SlateCamera, BindingContext);
		if (UMovieSceneCameraCutTrack* CutTrack = Cast<UMovieSceneCameraCutTrack>(MovieScene->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass())))
		{
			const FMovieSceneObjectBindingID BindingID = UE::MovieScene::FRelativeObjectBindingID(CameraGuid);
			if (UMovieSceneCameraCutSection* Section = CutTrack->AddNewCameraCut(BindingID, FFrameNumber(0)))
			{
				Section->SetRange(TRange<FFrameNumber>(FFrameNumber(0), Duration));
			}
		}
	}

	UE_LOG(LogAfterlight, Log, TEXT("Created Slice01 warning Level Sequence (%.2fs)."), DurationSeconds);
	return Sequence;
}
