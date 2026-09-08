#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

void AfterlightCaptureQaShot(const TCHAR* Name);
bool AfterlightQaAutoEnabled();
bool AfterlightQaDriveEnabled();

class FAfterlightModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
