#pragma once

//#include "AllowWindowsPlatformTypes.h"
//#include "NuiImageBuffer.h"
//#include "HideWindowsPlatformTypes.h"

#include "KinectSensor.generated.h"

#define MAX_PLAYER_INDEX    6

enum DEPTH_TREATMENT
{
	CLAMP_UNRELIABLE_DEPTHS,
	TINT_UNRELIABLE_DEPTHS,
	DISPLAY_ALL_DEPTHS,
};

struct FKinectTextureParams
{

	// Pointer to our Texture's resource
	FTexture2DResource* Texture2DResource;

	// Regions we need to update (for now, the whole image)
	FUpdateTextureRegion2D* UpdateRegions;

};

USTRUCT(BlueprintType)
struct FSkelStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "kinect Skeleton Struct")
		TArray<FVector> JointPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "kinect Skeleton Struct")
		TArray<FRotator> rotationAbsoluteQuaternion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "kinect Skeleton Struct")
		TArray<FRotator> rotationHierarchicalQuaternion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "kinect Skeleton Struct")
		uint8 PlayerID;

		void SetSkelJointPosition(TArray<FVector> NewJointPosition)
		{
			JointPosition = NewJointPosition;
		}
		void SetSkelRotationAbsoluteQuaternion(TArray<FRotator> NewRotationAbsoluteQuaternion)
		{
			rotationAbsoluteQuaternion = NewRotationAbsoluteQuaternion;
		}

		void SetSkelRotationHierarchicalQuaternion(TArray<FRotator> NewRotationHierarchicalQuaternion)
		{
			rotationHierarchicalQuaternion = NewRotationHierarchicalQuaternion;
		}

		void SetPlayerID(int NewPlayerID)
		{
			PlayerID = (uint8)NewPlayerID;
		}

		FSkelStruct(){

		}

};

UCLASS(ClassGroup = Kinect, Blueprintable)
class UKinectSensor : public UObject
{
	GENERATED_BODY()

public:
	static HANDLE                  m_pColorStreamHandle;
	static HANDLE                  m_hNextColorFrameEvent;
	static HANDLE                  m_pDepthStreamHandle;
	static HANDLE                  m_hNextDepthFrameEvent;
	static HANDLE                  m_pSkeletonStreamHandle;
	static HANDLE                  m_hNextSkeletonEvent;

	BYTE*                   m_depthRGBX;
	static const int        cDepthWidth = 640;
	static const int        cDepthHeight = 480;
	static const int        cBytesPerPixel = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kinect")
	TArray<FSkelStruct> FSkeletons;
	
	
	UKinectSensor();

	//virtual void BeginDestroy() override;
	
	BYTE GetIntensity(int depth);
	inline void SetColor(unsigned int* pColor, BYTE red, BYTE green, BYTE blue, BYTE alpha = 255);
	BYTE* ResetBuffer(unsigned int size);
	void InitDepthColorTable();
	void CopyDepth(const BYTE* source, unsigned int size, BOOL nearMode, DEPTH_TREATMENT treatment);
	
	UFUNCTION(BlueprintCallable, Category = "Kinect")
		void CloseKinect();

	void Init(INuiSensor* pNuiSensor);

	UFUNCTION(BlueprintCallable, Category = "Kinect")
		void InitializeSensor(UMaterialInterface* Base_Material);
	
	UFUNCTION(BlueprintCallable, Category = "Kinect")
		FVector4 GetAcelerometro();

	UFUNCTION(BlueprintCallable, Category = "Kinect")
		void UpdateColor();

	UFUNCTION(BlueprintCallable, Category = "Kinect")
		void UpdateDepth();

	UFUNCTION(BlueprintCallable, Category = "Kinect")
		void UpdateSkeleton();


	INuiSensor* mNuiSensor;
	


	/** Material that will be instanced to load UI texture into it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kinect")
		UMaterialInterface* BaseMaterial;

	/** Name of parameter to load UI texture into material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinect")
		FName TextureParameterName = "KinectTexture";

	/** Get the texture data from our UI component */
	UFUNCTION(BlueprintCallable, Category = "Kinect")
		UTexture2D* GetTexture(UTexture2D*& Texture) const;

	/** Material instance that contains texture inside it */
	UFUNCTION(BlueprintCallable, Category = "Kinect")
		UMaterialInstanceDynamic* GetMaterialRGB() const;

	/** Material instance that contains texture inside it */
	UFUNCTION(BlueprintCallable, Category = "Kinect")
		UMaterialInstanceDynamic* GetMaterialDepth() const;
	
	void ProcessColor();
	void ProcessDepth();
	void ProcessSkeleton();
	void TextureUpdate(const void* buffer, UTexture2D*& Texture, FKinectTextureParams& m_RenderParams);
	

protected:
	

	void ResetTexture(UTexture2D*& Texture, UMaterialInstanceDynamic*& MaterialInstance, FKinectTextureParams& m_RenderParams);
	void DestroyTexture(UTexture2D*& Texture);
	void ResetMatInstance(UTexture2D*& Texture, UMaterialInstanceDynamic*& MaterialInstance);
	

	// Store UI state in this UTexture2D
	UTexture2D* TextureColor;
	UTexture2D* Texturedepth;
	UMaterialInstanceDynamic* MaterialInstanceColor;
	UMaterialInstanceDynamic* MaterialInstanceDepth;

	int32 Width;
	int32 Height;

private:

	FKinectTextureParams RenderParamsColor;
	FKinectTextureParams RenderParamsDepth;

	HRESULT OpenColorStream();
	HRESULT OpenDepthStream();
	HRESULT OpenSkeletonStream();

	bool            m_nearMode;

	DEPTH_TREATMENT m_depthTreatment;

	static const BYTE    m_intensityShiftR[MAX_PLAYER_INDEX + 1];
	static const BYTE    m_intensityShiftG[MAX_PLAYER_INDEX + 1];
	static const BYTE    m_intensityShiftB[MAX_PLAYER_INDEX + 1];
	unsigned int         m_depthColorTable[MAX_PLAYER_INDEX + 1][USHRT_MAX + 1];
	unsigned long               m_nSizeInBytes;
};

