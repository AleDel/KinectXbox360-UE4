#pragma once

#if PLATFORM_WINDOWS
#include "AllowWindowsPlatformTypes.h"
#endif
#include <windows.h>
#include <ole2.h>
#include "NuiApi.h"
#if PLATFORM_WINDOWS
#include "HideWindowsPlatformTypes.h"
#endif

#include "KinectManager.generated.h"

UCLASS(ClassGroup = Kinect, Blueprintable)
class UKinectManager : public UObject
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UKinectManager();

	/** Initialize function, should be called after properties are set */
	UFUNCTION(BlueprintCallable, Category = "Kinect")
		TArray<class UKinectSensor*> GetSensor();

	void EnumerateSensors();

	void UpdateMainWindow(PCWSTR instanceName, HRESULT sensorStatus);

	//TMap<FName, class UKinectSensor*> m_sensorMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kinect")
		TArray<class UKinectSensor*> m_sensorArray;

	

};