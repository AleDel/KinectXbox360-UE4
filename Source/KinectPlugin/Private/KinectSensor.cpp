#include "KinectPluginPrivatePCH.h"

#include <algorithm>


HANDLE UKinectSensor::m_pColorStreamHandle;
HANDLE UKinectSensor::m_hNextColorFrameEvent;
HANDLE UKinectSensor::m_pDepthStreamHandle;
HANDLE UKinectSensor::m_hNextDepthFrameEvent;
HANDLE UKinectSensor::m_pSkeletonStreamHandle;
HANDLE UKinectSensor::m_hNextSkeletonEvent;

#define BYTES_PER_PIXEL_RGB         4
#define BYTES_PER_PIXEL_INFRARED    2
#define BYTES_PER_PIXEL_BAYER       1
#define BYTES_PER_PIXEL_DEPTH       sizeof(NUI_DEPTH_IMAGE_PIXEL)

#define COLOR_INDEX_BLUE            0
#define COLOR_INDEX_GREEN           1
#define COLOR_INDEX_RED             2
#define COLOR_INDEX_ALPHA           3

#define MIN_DEPTH                   400
#define MAX_DEPTH                   16383
#define UNKNOWN_DEPTH               0
#define UNKNOWN_DEPTH_COLOR         0x003F3F07
#define TOO_NEAR_COLOR              0x001F7FFF
#define TOO_FAR_COLOR               0x007F0F3F
#define NEAREST_COLOR               0x00FFFFFF

// intensity shift table to generate different render colors for different tracked players
const BYTE UKinectSensor::m_intensityShiftR[] = { 0, 2, 0, 2, 0, 0, 2 };
const BYTE UKinectSensor::m_intensityShiftG[] = { 0, 2, 2, 0, 2, 0, 0 };
const BYTE UKinectSensor::m_intensityShiftB[] = { 0, 0, 2, 2, 0, 2, 0 };

UKinectSensor::UKinectSensor()
{
	Width = 640;
	Height = 480;
	m_nearMode = false;
	
}

/*
void UKinectSensor::BeginDestroy(){
	//Super::BeginDestroy();

}*/

void UKinectSensor::CloseKinect(){
	if (mNuiSensor)
	{
		mNuiSensor->NuiShutdown();
	}

	if (m_hNextColorFrameEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hNextColorFrameEvent);
	}
	if (m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hNextDepthFrameEvent);
	}

	//delete(m_imageBuffer);
	mNuiSensor->Release();

	/*if (mNuiSensor){
		delete(mNuiSensor);
	}*/
	// done with depth pixel data
	//delete[] m_depthRGBX;


}

void UKinectSensor::Init(INuiSensor* pNuiSensor){
	mNuiSensor = pNuiSensor;

	m_depthTreatment = DISPLAY_ALL_DEPTHS;

	m_nSizeInBytes = 0;
	// create heap storage for depth pixel data in RGBX format
	m_depthRGBX = new BYTE[cDepthWidth*cDepthHeight*cBytesPerPixel];
	InitDepthColorTable();
	
}

void UKinectSensor::InitializeSensor(UMaterialInterface* Base_Material){

	BaseMaterial = Base_Material;
	
	HRESULT hr;

	if (NULL != mNuiSensor)
	{
		// Initialize the Kinect and specify that we'll be using color
		hr = mNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON);
		
		if (SUCCEEDED(hr))
		{
			UE_LOG(KinectLog, Warning, TEXT("Sensor inicializado"));
			
			if (SUCCEEDED(OpenColorStream())){
				UE_LOG(KinectLog, Warning, TEXT("Color Stream Abierto"));
			}
			if (SUCCEEDED(OpenDepthStream())){
				UE_LOG(KinectLog, Warning, TEXT("Depth Stream Abierto"));
			}
			if (SUCCEEDED(OpenSkeletonStream())){
				UE_LOG(KinectLog, Warning, TEXT("Skeleton Stream Abierto"));
			}
			
		}
	}

	if (NULL == mNuiSensor || FAILED(hr))
	{
		UE_LOG(KinectLog, Warning, TEXT("No ready Kinect found!"));
	}

	///// Prepara Textura 
	// Set the texture update region (for now the whole image)
	RenderParamsColor.UpdateRegions = new FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);
	RenderParamsDepth.UpdateRegions = new FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);
	
	ResetTexture(TextureColor, MaterialInstanceColor, RenderParamsColor);
	ResetTexture(Texturedepth, MaterialInstanceDepth, RenderParamsDepth);
}


HRESULT UKinectSensor::OpenColorStream(){
	HRESULT hr;

	m_hNextColorFrameEvent = NULL;
	m_pColorStreamHandle = NULL;

	// Create an event that will be signaled when color data is available
	m_hNextColorFrameEvent = CreateEvent(NULL, true, false, NULL);

	// Open a color image stream to receive color frames
	hr = mNuiSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,
		NUI_IMAGE_RESOLUTION_640x480,
		0,
		2,
		m_hNextColorFrameEvent,
		&m_pColorStreamHandle);

	return hr;
}

HRESULT UKinectSensor::OpenDepthStream(){
	HRESULT hr;

	m_hNextDepthFrameEvent = NULL;
	m_pDepthStreamHandle = NULL;

	// Create an event that will be signaled when color data is available
	m_hNextDepthFrameEvent = CreateEvent(NULL, true, false, NULL);

	// Open a color image stream to receive color frames
	hr = mNuiSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
		NUI_IMAGE_RESOLUTION_640x480,
		0,
		2,
		m_hNextDepthFrameEvent,
		&m_pDepthStreamHandle);

	return hr;
}

HRESULT UKinectSensor::OpenSkeletonStream(){
	HRESULT hr;
	m_pSkeletonStreamHandle = NULL;
	m_hNextSkeletonEvent = NULL;

	// Create an event that will be signaled when skeleton data is available
	m_hNextSkeletonEvent = CreateEventW(NULL, true, false, NULL);

	// Open a skeleton stream to receive skeleton data
	hr = mNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0);

	return hr;
}



FVector4 UKinectSensor::GetAcelerometro(){
	FVector4 reading = FVector4(0.0, 0.0, 0.0, 1.0);
	if (NULL != mNuiSensor)
	{
		HRESULT hr = mNuiSensor->NuiAccelerometerGetCurrentReading((Vector4*)&reading);
		//if (S_OK == hr)
		//{

		//return FVector4(reading.x, reading.y, reading.z, reading.w);
		//}
	}
	return reading;

}



//---------------------
// TEXTURE MATERIAL
//---------------------
void UKinectSensor::ResetTexture(UTexture2D*& Texture, UMaterialInstanceDynamic*& MaterialInstance, FKinectTextureParams& m_RenderParams)
{

	// Here we init the texture to its initial state
	DestroyTexture(Texture);

	// init the new Texture2D
	Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	Texture->AddToRoot();
	Texture->UpdateResource();

	m_RenderParams.Texture2DResource = (FTexture2DResource*)Texture->Resource;

	////////////////////////////////////////////////////////////////////////////////
	ResetMatInstance(Texture, MaterialInstance);


}

void UKinectSensor::DestroyTexture(UTexture2D*& Texture)
{
	// Here we destory the texture and its resource
	if (Texture)
	{
		Texture->RemoveFromRoot();

		if (Texture->Resource)
		{
			BeginReleaseResource(Texture->Resource);
			FlushRenderingCommands();
		}

		Texture->MarkPendingKill();
		Texture = nullptr;
	}
	else{
		UE_LOG(KinectLog, Warning, TEXT("UUUffffffffffffffffffffUUUUUUUUUUUUUUUUU"));
	}
}

void UKinectSensor::ResetMatInstance(UTexture2D*& Texture, UMaterialInstanceDynamic*& MaterialInstance)
{
	

	if (!Texture || !BaseMaterial || TextureParameterName.IsNone())
	{
		UE_LOG(KinectLog, Warning, TEXT("UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU"));
		return;
	}

	// Create material instance
	if (!MaterialInstance)
	{
		MaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, NULL);
		if (!MaterialInstance)
		{
			UE_LOG(KinectLog, Warning, TEXT("UI Material instance can't be created"));
			return;
		}
	}

	// Check again, we must have material instance
	if (!MaterialInstance)
	{
		UE_LOG(KinectLog, Error, TEXT("UI Material instance wasn't created"));
		return;
	}

	// Check we have desired parameter
	UTexture* Tex = nullptr;
	if (!MaterialInstance->GetTextureParameterValue(TextureParameterName, Tex))
	{
		UE_LOG(KinectLog, Warning, TEXT("UI Material instance Texture parameter not found"));
		return;
	}

	MaterialInstance->SetTextureParameterValue(TextureParameterName, GetTexture(Texture));
}

void UKinectSensor::TextureUpdate(const void *buffer, UTexture2D*& Texture, FKinectTextureParams& m_RenderParams)
{
	/*if (!browser || !bEnabled)
	{
	UE_LOG(LogBlu, Warning, TEXT("NO BROWSER ACCESS OR NOT ENABLED"))
	return;
	}*/
	
	if (Texture && Texture->Resource)
	{

		// Is our texture ready?
		auto ref = static_cast<FTexture2DResource*>(Texture->Resource)->GetTexture2DRHI();
		if (!ref)
		{
			UE_LOG(KinectLog, Warning, TEXT("NO REF"))
				return;
		}

		if (buffer == nullptr)
		{
			UE_LOG(KinectLog, Warning, TEXT("NO TEXTDATA"))
				return;
		}

		ENQUEUE_UNIQUE_RENDER_COMMAND_FOURPARAMETER(
			void,
			const void*, ImageData, buffer,
			UTexture2D*, TargetTexture, Texture,
			int32, Stride, Width * 4,
			FKinectTextureParams, Params, m_RenderParams,
			{

				RHIUpdateTexture2D(Params.Texture2DResource->GetTexture2DRHI(), 0, *Params.UpdateRegions, Stride, (uint8*)ImageData);

			});

	}
	else {
		UE_LOG(KinectLog, Warning, TEXT("no Texture or Texture->resource"))
	}

}

UTexture2D* UKinectSensor::GetTexture(UTexture2D*& Texture) const
{
	if (!Texture)
	{
		return UTexture2D::CreateTransient(Width, Height);
	}

	return Texture;
}

UMaterialInstanceDynamic* UKinectSensor::GetMaterialRGB() const
{
	return MaterialInstanceColor;
}

UMaterialInstanceDynamic* UKinectSensor::GetMaterialDepth() const
{
	return MaterialInstanceDepth;
}


void UKinectSensor::UpdateColor()
{
	
	if (NULL == mNuiSensor)
	{
		return;
	}

	if (0 == (long)WaitForSingleObject(m_hNextColorFrameEvent, 0))
	{
		ProcessColor();
	}
}

void UKinectSensor::UpdateDepth()
{

	if (NULL == mNuiSensor)
	{
		return;
	}

	if (0 == (long)WaitForSingleObject(m_hNextDepthFrameEvent, 0))
	{
		ProcessDepth();
	}
}

void UKinectSensor::UpdateSkeleton(){
	if (NULL == mNuiSensor)
	{
		return;
	}

	// Wait for 0ms, just quickly test if it is time to process a skeleton
	if (0 == (long)WaitForSingleObject(m_hNextSkeletonEvent, 0))
	{
		ProcessSkeleton();
	}
}


void UKinectSensor::ProcessColor()
{
	HRESULT hr;
	NUI_IMAGE_FRAME imageFrame;

	// Attempt to get the color frame
	hr = mNuiSensor->NuiImageStreamGetNextFrame(m_pColorStreamHandle, 0, &imageFrame);
	if (FAILED(hr))
	{
		UE_LOG(KinectLog, Warning, TEXT("Fallo al coger color frame"));
		return;
	}

	INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	// Make sure we've received valid data
	if (LockedRect.Pitch != 0)
	{
		TextureUpdate(static_cast<BYTE *>(LockedRect.pBits), TextureColor, RenderParamsColor);
	}

	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);

	// Release the frame
	mNuiSensor->NuiImageStreamReleaseFrame(m_pColorStreamHandle, &imageFrame);
	
}



BYTE UKinectSensor::GetIntensity(int depth)
{
	// Validate arguments
	if (depth < MIN_DEPTH || depth > MAX_DEPTH)
	{
		return UCHAR_MAX;
	}

	// Use a logarithmic scale that shows more detail for nearer depths.
	// The constants in this formula were chosen such that values between
	// MIN_DEPTH and MAX_DEPTH will map to the full range of possible
	// byte values.
	const float depthRangeScale = 500.0f;
	const int intensityRangeScale = 74;
	return (BYTE)(~(BYTE)std::fmin(
		UCHAR_MAX,
		log((double)(depth - MIN_DEPTH) / depthRangeScale + 1) * intensityRangeScale));
}

void UKinectSensor::SetColor(unsigned int* pColor, BYTE red, BYTE green, BYTE blue, BYTE alpha)
{
	if (!pColor)
		return;

	BYTE* c = (BYTE*)pColor;
	c[COLOR_INDEX_RED] = red;
	c[COLOR_INDEX_GREEN] = green;
	c[COLOR_INDEX_BLUE] = blue;
	c[COLOR_INDEX_ALPHA] = alpha;
}

BYTE* UKinectSensor::ResetBuffer(unsigned int size)
{
	if (!m_depthRGBX || m_nSizeInBytes != size)
	{
		delete(m_depthRGBX);
		//SafeDeleteArray(m_pBuffer);

		if (0 != size)
		{
			m_depthRGBX = new BYTE[size];
		}
		m_nSizeInBytes = size;
	}

	return m_depthRGBX;
}

void UKinectSensor::InitDepthColorTable()
{
	// Get the min and max reliable depth
	USHORT minReliableDepth = (m_nearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
	USHORT maxReliableDepth = (m_nearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

	ZeroMemory(m_depthColorTable, sizeof(m_depthColorTable));

	// Set color for unknown depth
	m_depthColorTable[0][UNKNOWN_DEPTH] = UNKNOWN_DEPTH_COLOR;

	switch (m_depthTreatment)
	{
	case CLAMP_UNRELIABLE_DEPTHS:
		// Fill in the "near" portion of the table with solid color
		for (int depth = UNKNOWN_DEPTH + 1; depth < minReliableDepth; depth++)
		{
			m_depthColorTable[0][depth] = TOO_NEAR_COLOR;
		}

		// Fill in the "far" portion of the table with solid color
		for (int depth = maxReliableDepth + 1; depth <= USHRT_MAX; depth++)
		{
			m_depthColorTable[0][depth] = TOO_FAR_COLOR;
		}
		break;

	case TINT_UNRELIABLE_DEPTHS:
	{
		// Fill in the "near" portion of the table with a tinted gradient
		for (int depth = UNKNOWN_DEPTH + 1; depth < minReliableDepth; depth++)
		{
			BYTE intensity = GetIntensity(depth);
			BYTE r = intensity >> 3;
			BYTE g = intensity >> 1;
			BYTE b = intensity;
			SetColor(&m_depthColorTable[0][depth], r, g, b);
		}

		// Fill in the "far" portion of the table with a tinted gradient
		for (int depth = maxReliableDepth + 1; depth <= USHRT_MAX; depth++)
		{
			BYTE intensity = GetIntensity(depth);
			BYTE r = intensity;
			BYTE g = intensity >> 3;
			BYTE b = intensity >> 1;
			SetColor(&m_depthColorTable[0][depth], r, g, b);
		}
	}
	break;

	case DISPLAY_ALL_DEPTHS:
		minReliableDepth = MIN_DEPTH;
		maxReliableDepth = MAX_DEPTH;

		for (int depth = UNKNOWN_DEPTH + 1; depth < minReliableDepth; depth++)
		{
			m_depthColorTable[0][depth] = NEAREST_COLOR;
		}
		break;

	default:
		break;
	}

	for (USHORT depth = minReliableDepth; depth <= maxReliableDepth; depth++)
	{
		BYTE intensity = GetIntensity(depth);

		for (int index = 0; index <= MAX_PLAYER_INDEX; index++)
		{
			BYTE r = intensity >> m_intensityShiftR[index];
			BYTE g = intensity >> m_intensityShiftG[index];
			BYTE b = intensity >> m_intensityShiftB[index];
			SetColor(&m_depthColorTable[index][depth], r, g, b);
		}
	}
}

void UKinectSensor::CopyDepth(const BYTE* pImage, unsigned int size, BOOL nearMode, DEPTH_TREATMENT treatment)
{
	// Check source buffer size
	if (size != cDepthWidth * cDepthHeight * BYTES_PER_PIXEL_DEPTH)
	{
		return;
	}

	// Check if range mode and depth treatment have been changed. Re-initlialize depth-color table with changed parameters
	if (m_depthTreatment != treatment)
	{
		//m_nearMode       = (FALSE != nearMode);
		m_depthTreatment = treatment;

		InitDepthColorTable();
	}

	// Converted image size is equal to source image size
	//m_width = m_srcWidth;
	//m_height = m_srcHeight;

	// Allocate buffer for color image. If required buffer size hasn't changed, the previously allocated buffer is returned
	unsigned int* rgbrun = (unsigned int*)ResetBuffer(cDepthWidth * cDepthHeight * BYTES_PER_PIXEL_RGB);

	// Initialize pixel pointers to start and end of image buffer
	NUI_DEPTH_IMAGE_PIXEL* pPixelRun = (NUI_DEPTH_IMAGE_PIXEL*)pImage;
	NUI_DEPTH_IMAGE_PIXEL* pPixelEnd = pPixelRun + cDepthWidth * cDepthHeight;

	// Run through pixels
	while (pPixelRun < pPixelEnd)
	{
		// Get pixel depth and player index
		USHORT depth = pPixelRun->depth;
		USHORT index = pPixelRun->playerIndex;

		// Get mapped color from depth-color table
		*rgbrun = m_depthColorTable[index][depth];

		// Move the pointers to next pixel
		++rgbrun;
		++pPixelRun;
	}
}


void UKinectSensor::ProcessDepth()
{
	HRESULT hr;
	NUI_IMAGE_FRAME imageFrame;

	// Attempt to get the color frame
	hr = mNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame);
	if (FAILED(hr))
	{
		UE_LOG(KinectLog, Warning, TEXT("Fallo al coger Depth frame"));
		return;
	}

	INuiFrameTexture * pTexture;
	BOOL nearMode = false;

	// Get the depth image pixel texture
	hr = mNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(m_pDepthStreamHandle, &imageFrame, &nearMode, &pTexture);
	if (FAILED(hr))
	{
		goto ReleaseFrame;
	}

	NUI_LOCKED_RECT LockedRect;

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	// Make sure we've received valid data
	if (LockedRect.Pitch != 0)
	{

		// Conver depth data to color image and copy to image buffer
		//m_imageBuffer.CopyDepth(LockedRect.pBits, LockedRect.size, nearMode, m_depthTreatment);
		
		
		
		CopyDepth(LockedRect.pBits, LockedRect.size, nearMode, m_depthTreatment);
		
		//const NuiImageBuffer* m_pImage = nullptr;
		//m_pImage =&m_imageBuffer;
		if (m_nSizeInBytes){

			TextureUpdate(m_depthRGBX, Texturedepth, RenderParamsDepth);
			//TextureUpdate(&m_imageBuffer, Texturedepth, RenderParamsDepth);
		}
		
	}

	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);
	pTexture->Release();

ReleaseFrame:
	// Release the frame
	mNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);

}

void UKinectSensor::ProcessSkeleton(){


	NUI_TRANSFORM_SMOOTH_PARAMETERS defaultParams = { 0.5f, 0.5f, 0.5f, 0.05f, 0.04f };

	NUI_SKELETON_FRAME skeletonFrame = { 0 };
	
	HRESULT hr = mNuiSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);
	if (FAILED(hr))
	{
		return;
	}

	// smooth out the skeleton data
	mNuiSensor->NuiTransformSmooth(&skeletonFrame, &defaultParams);

	FSkeletons.Empty();
	
	TArray<FVector> TempJointPosition;
	TArray<FRotator> TempJointAbsoluteRotation;
	TArray<FRotator> TempJointHierarchicalRotation;
	TempJointPosition.Empty();
	TempJointAbsoluteRotation.Empty();
	TempJointHierarchicalRotation.Empty();

	for (int i = 0; i < NUI_SKELETON_COUNT; i++)
	{
		const NUI_SKELETON_DATA & skeleton = skeletonFrame.SkeletonData[i];
		
		NUI_SKELETON_TRACKING_STATE trackingState = skeleton.eTrackingState;

		FSkelStruct newFSkelStruc;
		newFSkelStruc.SetPlayerID(i);

		
		NUI_SKELETON_BONE_ORIENTATION boneOrientations[NUI_SKELETON_POSITION_COUNT];
		NuiSkeletonCalculateBoneOrientations(&skeleton, boneOrientations);

		if (NUI_SKELETON_TRACKED == trackingState)
		{
			
			for (int p = 0; p < NUI_SKELETON_POSITION_COUNT; ++p)
			{
				Vector4 hh = skeleton.SkeletonPositions[p];
				
				TempJointPosition.Add(FVector(hh.z, hh.x*-1.0, hh.y));

				//bone orientation
				NUI_SKELETON_BONE_ORIENTATION & orientation = boneOrientations[p];

				TempJointAbsoluteRotation.Add(
					FQuat(orientation.absoluteRotation.rotationQuaternion.x,
						orientation.absoluteRotation.rotationQuaternion.y,
						orientation.absoluteRotation.rotationQuaternion.z,
						orientation.absoluteRotation.rotationQuaternion.w).Rotator()
				);

				
				TempJointHierarchicalRotation.Add(
					FQuat(orientation.hierarchicalRotation.rotationQuaternion.x,
						orientation.hierarchicalRotation.rotationQuaternion.y,
						orientation.hierarchicalRotation.rotationQuaternion.z,
						orientation.hierarchicalRotation.rotationQuaternion.w).Rotator());
				
			}
			
			newFSkelStruc.SetSkelJointPosition(TempJointPosition);
			newFSkelStruc.SetSkelRotationAbsoluteQuaternion(TempJointAbsoluteRotation);
			newFSkelStruc.SetSkelRotationHierarchicalQuaternion(TempJointHierarchicalRotation);
			FSkeletons.Add(newFSkelStruc);
		}
		else if (NUI_SKELETON_POSITION_ONLY == trackingState)
		{
			// we've only received the center point of the skeleton, draw that
			
		}
	}
}



/*
void UKinectSensor::ProcessDepth()
{
	HRESULT hr;
	NUI_IMAGE_FRAME imageFrame;

	// Attempt to get the color frame
	hr = mNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame);
	if (FAILED(hr))
	{
		UE_LOG(KinectLog, Warning, TEXT("Fallo al coger Depth frame"));
		return;
	}

	INuiFrameTexture * pTexture;// = imageFrame.pFrameTexture;
	BOOL nearMode = false;

	// Get the depth image pixel texture
	hr = mNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(m_pDepthStreamHandle, &imageFrame, &nearMode, &pTexture);
	if (FAILED(hr))
	{
		goto ReleaseFrame;
	}
	
	NUI_LOCKED_RECT LockedRect;

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	// Make sure we've received valid data
	if (LockedRect.Pitch != 0)
	{
		
		// Get the min and max reliable depth for the current frame
		int minDepth = (nearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
		int maxDepth = (nearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

		BYTE * rgbrun = m_depthRGBX;
		const NUI_DEPTH_IMAGE_PIXEL * pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits);

		// end pixel is start + width*height - 1
		const NUI_DEPTH_IMAGE_PIXEL * pBufferEnd = pBufferRun + (cDepthWidth * cDepthHeight);

		while (pBufferRun < pBufferEnd)
		{
			// discard the portion of the depth that contains only the player index
			USHORT depth = pBufferRun->depth;

			// To convert to a byte, we're discarding the most-significant
			// rather than least-significant bits.
			// We're preserving detail, although the intensity will "wrap."
			// Values outside the reliable depth range are mapped to 0 (black).

			// Note: Using conditionals in this loop could degrade performance.
			// Consider using a lookup table instead when writing production code.
			BYTE intensity = static_cast<BYTE>(depth >= minDepth && depth <= maxDepth ? depth % 256 : 0);

			// Write out blue byte
			*(rgbrun++) = intensity;

			// Write out green byte
			*(rgbrun++) = intensity;

			// Write out red byte
			*(rgbrun++) = intensity;

			// We're outputting BGR, the last byte in the 32 bits is unused so skip it
			// If we were outputting BGRA, we would write alpha here.
			++rgbrun;

			// Increment our index into the Kinect's depth buffer
			++pBufferRun;
		}
		
		TextureUpdate(m_depthRGBX, Texturedepth, RenderParamsDepth);
		
	}
	
	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);
	pTexture->Release();

ReleaseFrame:
	// Release the frame
	mNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);
	
}*/

