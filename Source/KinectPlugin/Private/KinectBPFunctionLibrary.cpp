#include "KinectPluginPrivatePCH.h"

UKinectManager* UKinectBPFunctionLibrary::KinectManager(UObject* WorldContextObject)
{

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
	UKinectManager* tempObject = Cast<UKinectManager>(StaticConstructObject_Internal(UKinectManager::StaticClass()));

	return tempObject;

}