#include "KinectPluginPrivatePCH.h"

// Sets default values
UKinectManager::UKinectManager()
{

}

TArray<class UKinectSensor*> UKinectManager::GetSensor()
{
	EnumerateSensors();
	if (m_sensorArray.Num() > 0){
		UE_LOG(KinectLog, Warning, TEXT("el array tiene elementos"));
	}
	else{
		UE_LOG(KinectLog, Warning, TEXT("el array esta vacio"));
	}
	return m_sensorArray;

}

/// <summary>
/// Enumerate and construct all the sensors when the app starts up
/// </summary>
void UKinectManager::EnumerateSensors()
{
	UE_LOG(KinectLog, Warning, TEXT("searching sensors..."));
    int iCount = 0;
    HRESULT hr = NuiGetSensorCount(&iCount);
    if (FAILED(hr))
    {
		UE_LOG(KinectLog, Warning, TEXT("Error %s"), hr);
		UE_LOG(KinectLog, Warning, TEXT("check drivers installed o conected kinects!!"));
        return;
    }

    for (int i = 0; i < iCount; ++i)
    {
        INuiSensor* pNuiSensor = nullptr;

        if (SUCCEEDED(NuiCreateSensorByIndex(i, &pNuiSensor)))
        {
			UE_LOG(KinectLog, Warning, TEXT("Kinect Sensor encontrado: %d --- %s"), i, pNuiSensor->NuiDeviceConnectionId());
			
			UpdateMainWindow(pNuiSensor->NuiDeviceConnectionId(), pNuiSensor->NuiStatus());
        }

        //SafeRelease(pNuiSensor);
		pNuiSensor->Release();
    }
}

/// <summary>
/// Update the main window status
/// </summary>
void UKinectManager::UpdateMainWindow(PCWSTR instanceName, HRESULT sensorStatus)
{
	
    // The new status is "not connected"
    if (E_NUI_NOTCONNECTED == sensorStatus)
    {
		//UE_LOG(KinectLog, Warning, TEXT("Kinect Sensor no conectado: %s"), sensorStatus);
		UE_LOG(KinectLog, Warning, TEXT("Kinect Sensor no conectado"));
    }
    else
    {
		//UE_LOG(KinectLog, Warning, TEXT("Kinect Sensor conectado: %s"), sensorStatus);
		UE_LOG(KinectLog, Warning, TEXT("Kinect Sensor conectado"));
		
		INuiSensor* pNuiSensor = nullptr;

		if (SUCCEEDED(NuiCreateSensorById(instanceName, &pNuiSensor)))
		{
			/*
			UKinectSensor* tempSensor = nullptr;
			NewObject<UKinectSensor>(tempSensor);
			//UKinectSensor* tempSensor = new UKinectSensor();
			tempSensor->Init(pNuiSensor);
			*/
			UKinectSensor* tempSensor = Cast<UKinectSensor>(StaticConstructObject_Internal(UKinectSensor::StaticClass()));
			tempSensor->Init(pNuiSensor);

			//m_sensorMap.Add(instanceName, tempSensor);
			m_sensorArray.Add(tempSensor);
		}
		else
		{
			return;
		}

		
        
		
    }
/**/
    // Insert the new log item to status log list
    //m_pStatusLogListControl->AddLog(instanceName, sensorStatus);
}