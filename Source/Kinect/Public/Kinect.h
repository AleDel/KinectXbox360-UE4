// Some copyright should be here...

#pragma once

#include "ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(KinectLog, Log, All);

class FKinectModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};