#include "Audio/AfterlightTempAudio.h"
#include "Sound/SoundWaveProcedural.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

namespace
{
	int32 NextSeed = 7919;

	int16 SampleNoise(float Amplitude)
	{
		NextSeed = NextSeed * 1103515245 + 12345;
		const float Unit = ((NextSeed >> 16) & 0x7fff) / 32767.f * 2.f - 1.f;
		return static_cast<int16>(FMath::Clamp(Unit * Amplitude * 32767.f, -32767.f, 32767.f));
	}

	void QueueBed(USoundWaveProcedural* Wave, EAfterlightTempBed Bed, int32 Samples)
	{
		if (!Wave || Samples <= 0)
		{
			return;
		}
		TArray<uint8> PCM;
		PCM.Reserve(Samples * 2);
		static float PhaseA = 0.f;
		static float PhaseB = 0.f;
		const float Rate = 22050.f;
		for (int32 i = 0; i < Samples; ++i)
		{
			int16 S = 0;
			switch (Bed)
			{
			case EAfterlightTempBed::Rain:
				S = SampleNoise((i & 3) == 0 ? 0.18f : 0.04f);
				break;
			case EAfterlightTempBed::Electric:
				PhaseA += 2.f * PI * 92.f / Rate;
				S = static_cast<int16>(FMath::Sin(PhaseA) * 1800.f + SampleNoise(0.04f));
				break;
			case EAfterlightTempBed::Drone:
				PhaseA += 2.f * PI * 148.f / Rate;
				PhaseB += 2.f * PI * 6.f / Rate;
				S = static_cast<int16>(FMath::Sin(PhaseA) * (2200.f + FMath::Sin(PhaseB) * 400.f) + SampleNoise(0.08f));
				break;
			case EAfterlightTempBed::Pump:
				PhaseA += 2.f * PI * 54.f / Rate;
				S = static_cast<int16>(FMath::Sin(PhaseA) * 2400.f);
				break;
			case EAfterlightTempBed::Warning:
				S = SampleNoise(0.55f);
				break;
			case EAfterlightTempBed::Sting:
				PhaseA += 2.f * PI * 740.f / Rate;
				S = static_cast<int16>(FMath::Sin(PhaseA) * 9000.f * FMath::Clamp(1.f - (i / static_cast<float>(Samples)), 0.f, 1.f));
				break;
			case EAfterlightTempBed::Footstep:
			{
				const float Env = FMath::Clamp(1.f - (i / static_cast<float>(Samples)), 0.f, 1.f);
				S = static_cast<int16>(SampleNoise(0.35f * Env) + FMath::Sin(i * 0.18f) * 2200.f * Env);
				break;
			}
			default:
				S = 0;
				break;
			}
			PCM.Add(static_cast<uint8>(S & 0xff));
			PCM.Add(static_cast<uint8>((S >> 8) & 0xff));
		}
		Wave->QueueAudio(PCM.GetData(), PCM.Num());
	}
}

USoundWaveProcedural* FAfterlightTempAudio::CreateBed(UObject* Outer, FName Name, EAfterlightTempBed Bed)
{
	USoundWaveProcedural* Wave = NewObject<USoundWaveProcedural>(Outer, Name);
	Wave->SetSampleRate(22050);
	Wave->NumChannels = 1;
	Wave->bLooping = Bed != EAfterlightTempBed::Sting && Bed != EAfterlightTempBed::Warning && Bed != EAfterlightTempBed::Footstep;
	Wave->SoundGroup = SOUNDGROUP_Effects;
	Wave->bProcedural = true;
	Wave->Duration = Wave->bLooping ? INDEFINITELY_LOOPING_DURATION : (Bed == EAfterlightTempBed::Footstep ? 0.12f : 0.45f);
	Wave->OnSoundWaveProceduralUnderflow.BindLambda([Bed](USoundWaveProcedural* Procedural, int32 SamplesNeeded)
	{
		QueueBed(Procedural, Bed, FMath::Clamp(SamplesNeeded, 256, 4096));
	});
	QueueBed(Wave, Bed, 2048);
	return Wave;
}

UAudioComponent* FAfterlightTempAudio::SpawnLoop(UWorld* World, UObject* Outer, FName Name, EAfterlightTempBed Bed, float Volume)
{
	if (!World)
	{
		return nullptr;
	}
	USoundWaveProcedural* Wave = CreateBed(Outer, Name, Bed);
	UAudioComponent* Comp = UGameplayStatics::SpawnSound2D(World, Wave, Volume, 1.f, 0.f, nullptr, true, false);
	return Comp;
}

void FAfterlightTempAudio::PlayOneShot(UWorld* World, UObject* Outer, EAfterlightTempBed Bed, float Volume)
{
	if (!World)
	{
		return;
	}
	USoundWaveProcedural* Wave = CreateBed(Outer, NAME_None, Bed);
	UGameplayStatics::PlaySound2D(World, Wave, Volume);
}
