#include "Audio/AfterlightVoice.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundWaveProcedural.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Engine/World.h"

namespace
{
	FName CanonicalSpeaker(FName SpeakerId)
	{
		const FString Name = SpeakerId.ToString();
		if (Name.Equals(TEXT("Maya"), ESearchCase::IgnoreCase))
		{
			return TEXT("Maya");
		}
		return TEXT("Eli");
	}

	bool ParseWav(const TArray<uint8>& File, int32& OutSampleRate, int32& OutChannels, TArray<uint8>& OutPcm, float& OutDuration)
	{
		if (File.Num() < 44)
		{
			return false;
		}
		if (FMemory::Memcmp(File.GetData(), "RIFF", 4) != 0 || FMemory::Memcmp(File.GetData() + 8, "WAVE", 4) != 0)
		{
			return false;
		}

		int32 SampleRate = 0;
		int32 Channels = 0;
		int32 Bits = 16;
		int32 DataOffset = -1;
		int32 DataBytes = 0;
		int32 Offset = 12;
		while (Offset + 8 <= File.Num())
		{
			const char* ChunkId = reinterpret_cast<const char*>(File.GetData() + Offset);
			const int32 ChunkSize = *reinterpret_cast<const int32*>(File.GetData() + Offset + 4);
			const int32 Next = Offset + 8 + ChunkSize + (ChunkSize & 1);
			if (FMemory::Memcmp(ChunkId, "fmt ", 4) == 0 && Offset + 24 <= File.Num())
			{
				Channels = *reinterpret_cast<const int16*>(File.GetData() + Offset + 10);
				SampleRate = *reinterpret_cast<const int32*>(File.GetData() + Offset + 12);
				Bits = *reinterpret_cast<const int16*>(File.GetData() + Offset + 22);
			}
			else if (FMemory::Memcmp(ChunkId, "data", 4) == 0)
			{
				DataOffset = Offset + 8;
				DataBytes = FMath::Clamp(ChunkSize, 0, File.Num() - DataOffset);
				break;
			}
			Offset = Next;
		}

		if (SampleRate <= 0 || Channels <= 0 || DataOffset < 0 || DataBytes <= 0 || Bits != 16)
		{
			return false;
		}

		OutSampleRate = SampleRate;
		OutChannels = Channels;
		OutPcm.SetNumUninitialized(DataBytes);
		FMemory::Memcpy(OutPcm.GetData(), File.GetData() + DataOffset, DataBytes);
		const int32 SampleCount = DataBytes / (Channels * 2);
		OutDuration = SampleCount / static_cast<float>(SampleRate);
		return OutDuration > 0.04f;
	}
}

FString FAfterlightVoice::SpeakerFolder(FName SpeakerId)
{
	return CanonicalSpeaker(SpeakerId).ToString();
}

FString FAfterlightVoice::ResolveFilePath(FName SpeakerId, FName VoiceId)
{
	if (VoiceId.IsNone())
	{
		return FString();
	}
	const FString Folder = FPaths::ProjectContentDir() / TEXT("Audio/VO/Temp") / SpeakerFolder(SpeakerId);
	const FString Stem = VoiceId.ToString();
	const FString Wav = Folder / (Stem + TEXT(".wav"));
	if (FPaths::FileExists(Wav))
	{
		return Wav;
	}
	const FString WavUpper = Folder / (Stem + TEXT(".WAV"));
	if (FPaths::FileExists(WavUpper))
	{
		return WavUpper;
	}
	return FString();
}

float FAfterlightVoice::FindDuration(FName SpeakerId, FName VoiceId)
{
	const FString Path = ResolveFilePath(SpeakerId, VoiceId);
	if (Path.IsEmpty())
	{
		return 0.f;
	}
	TArray<uint8> File;
	if (!FFileHelper::LoadFileToArray(File, *Path))
	{
		return 0.f;
	}
	int32 SampleRate = 0;
	int32 Channels = 0;
	TArray<uint8> Pcm;
	float Duration = 0.f;
	return ParseWav(File, SampleRate, Channels, Pcm, Duration) ? Duration : 0.f;
}

float FAfterlightVoice::Play(UWorld* World, UObject* Outer, FName SpeakerId, FName VoiceId)
{
	if (!World || VoiceId.IsNone())
	{
		return 0.f;
	}

	const FString Speaker = SpeakerFolder(SpeakerId);
	const FString AssetPath = FString::Printf(TEXT("/Game/Audio/VO/Temp/%s/%s.%s"), *Speaker, *VoiceId.ToString(), *VoiceId.ToString());
	if (USoundWave* Asset = LoadObject<USoundWave>(nullptr, *AssetPath))
	{
		UGameplayStatics::PlaySound2D(World, Asset, 0.92f);
		return FMath::Max(Asset->Duration, 0.f);
	}

	const FString Path = ResolveFilePath(SpeakerId, VoiceId);
	if (Path.IsEmpty())
	{
		return 0.f;
	}
	TArray<uint8> File;
	if (!FFileHelper::LoadFileToArray(File, *Path))
	{
		return 0.f;
	}
	int32 SampleRate = 0;
	int32 Channels = 0;
	TArray<uint8> Pcm;
	float Duration = 0.f;
	if (!ParseWav(File, SampleRate, Channels, Pcm, Duration))
	{
		return 0.f;
	}

	USoundWaveProcedural* Wave = Outer
		? NewObject<USoundWaveProcedural>(Outer)
		: NewObject<USoundWaveProcedural>();
	Wave->SetSampleRate(SampleRate);
	Wave->NumChannels = Channels;
	Wave->bLooping = false;
	Wave->bProcedural = true;
	Wave->SoundGroup = SOUNDGROUP_Voice;
	Wave->Duration = Duration;
	Wave->QueueAudio(Pcm.GetData(), Pcm.Num());
	UGameplayStatics::PlaySound2D(World, Wave, 0.92f);
	return Duration;
}
