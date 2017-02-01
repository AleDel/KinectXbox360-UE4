#pragma once

#include "KinectBPFunctionLibrary.generated.h"

UCLASS(ClassGroup = Kinect, Blueprintable)
class UKinectBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Create Kinect", CompactNodeTitle = "Kinect", Keywords = "new create aaa Kinect"), Category = Kinect)
		static UKinectManager* KinectManager(UObject* WorldContextObject);
};