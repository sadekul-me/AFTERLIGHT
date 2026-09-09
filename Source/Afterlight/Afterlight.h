#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

void AfterlightCaptureQaShot(const TCHAR* Name);
void AfterlightLogMem(const TCHAR* Label);
bool AfterlightQaAutoEnabled();
bool AfterlightQaDriveEnabled();
bool AfterlightQaStoryEnabled();
int32 AfterlightQaChoiceIndex();
const TCHAR* AfterlightQaSprintFolder();

class FAfterlightModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
