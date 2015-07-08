// Some copyright should be here...

#include "KinectPrivatePCH.h"

DEFINE_LOG_CATEGORY(KinectLog);

#define LOCTEXT_NAMESPACE "FKinectModule"

void FKinectModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(KinectLog, Warning, TEXT("Modulo Kinect Cargado"));
}

void FKinectModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(KinectLog, Warning, TEXT("Modulo Kinect Descargado"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FKinectModule, Kinect)