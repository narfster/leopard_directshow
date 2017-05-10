#include "dshow_graph.h"
#include <iostream>
#include <vector>


dshow_graph::dshow_graph()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		printf("ERROR: CoInitializeEx\r\n");
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

dshow_graph::~dshow_graph()
{
	
	if(pControlInterface!=nullptr)
	{
		pControlInterface->Stop();
	}
	
	if(pEventInterface != nullptr)
	{
		pEventInterface->CancelDefaultHandling(evCode);
	}
	
	

	// Enumerate the filters in the graph.
	IEnumFilters *pEnum = NULL;
	if (pGraphInterface != nullptr)
	{
		HRESULT hr = pGraphInterface->EnumFilters(&pEnum);
		if (SUCCEEDED(hr))
		{
			IBaseFilter *pFilter = NULL;
			while (S_OK == pEnum->Next(1, &pFilter, NULL))
			{
				// Remove the filter.
				pGraphInterface->RemoveFilter(pFilter);
				// Reset the enumerator.
				pEnum->Reset();
				pFilter->Release();
			}
			pEnum->Release();
		}

		pEventInterface->Release();
		pControlInterface->Release();
	}

	CoUninitialize();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

HRESULT dshow_graph::EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum)
{
	// Create the System Device Enumerator.
	ICreateDevEnum *pDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the category.
		hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
		}
		pDevEnum->Release();
	}
	return hr;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * if device was not found return false
 */
bool dshow_graph::DisplayDeviceInformation(IEnumMoniker* pEnum)
{
	auto retVal = false;

	IMoniker *pMoniker = NULL;

	while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
	{
		IPropertyBag *pPropBag;
		HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue;
		}

		VARIANT var;
		VariantInit(&var);

		// Get description or friendly name.
		hr = pPropBag->Read(L"Description", &var, 0);
		if (FAILED(hr))
		{
			hr = pPropBag->Read(L"FriendlyName", &var, 0);
		}
		if (SUCCEEDED(hr))
		{
			

			//Convert from wstring to BSTR
			BSTR bs = SysAllocStringLen(selected_device_.name.data(), selected_device_.name.size());
			if (0 == wcscmp(var.bstrVal, bs))
			{
				hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCapFilter);
				if (SUCCEEDED(hr))
				{
					std::cout << std::endl << "*********************************** " << std::endl;
					std::cout << "capture device set: ";
					printf("%S\n", var.bstrVal);
					std::cout << "*********************************** " << std::endl << std::endl;

					retVal = true;
				}
			}

			VariantClear(&var);
		}

		hr = pPropBag->Write(L"FriendlyName", &var);

		// WaveInID applies only to audio capture devices.
		hr = pPropBag->Read(L"WaveInID", &var, 0);
		if (SUCCEEDED(hr))
		{
			printf("WaveIn ID: %d\n", var.lVal);
			VariantClear(&var);
		}

		hr = pPropBag->Read(L"DevicePath", &var, 0);
		if (SUCCEEDED(hr))
		{
			// The device path is not intended for display.
			printf("Device path: %S\n", var.bstrVal);
			VariantClear(&var);
		}

		pPropBag->Release();
		pMoniker->Release();
	}

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


dshow_graph::imgFormat dshow_graph::get_image_format() const
{
	AM_MEDIA_TYPE *pmt;
	HRESULT hr;
	imgFormat fmt{0};

	if(pStreamConfigInterface == nullptr)
	{
		std::cout << "ERROR: get_image_format() " << std::endl;
		return fmt;
	}

	hr = pStreamConfigInterface->GetFormat(&pmt);
	if (hr == S_OK)
	{
		if ((pmt->majortype == MEDIATYPE_Video) &&
			(pmt->formattype == FORMAT_VideoInfo) &&
			(pmt->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
			(pmt->pbFormat != NULL))
		{
			VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmt->pbFormat;
			fmt.fps = 10000000 / pVih->AvgTimePerFrame;
			fmt.width = pVih->bmiHeader.biWidth;
			fmt.height = pVih->bmiHeader.biHeight;
			fmt.frameSize = pmt->lSampleSize;
			fmt.bytesPerPixel = fmt.frameSize / (fmt.width * fmt.height);
			fmt.bitsPerPixel = 10; //Needs to be reset in superclass
		}
	}

	return fmt;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void dshow_graph::print_camera_cap()
{
	int iCount, iSize;
	BYTE *pSCC = NULL;
	AM_MEDIA_TYPE *pmt;
	HRESULT hr;

	hr = pStreamConfigInterface->GetNumberOfCapabilities(&iCount, &iSize);

	//allocate dynamic buff to hold capabilites.
	pSCC = new BYTE[iSize];
	if (pSCC == NULL)
	{
		// TODO: Out of memory error.
	}

	//run on all formats.
	std::cout << "printing capabilites of camera: " << std::endl;
	for(int i = 0; i< iCount; i++)
	{
		hr = pStreamConfigInterface->GetStreamCaps(i, &pmt, pSCC);
		if (hr == S_OK)
		{
			//reading metrial
			//subtype defines the media fortmat.. see https://msdn.microsoft.com/en-us/library/windows/desktop/dd757532(v=vs.85).aspx
			//and also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd387907(v=vs.85).aspx
			
			if ((pmt->majortype == MEDIATYPE_Video) &&
				(pmt->formattype == FORMAT_VideoInfo) &&
				(pmt->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
				(pmt->pbFormat != NULL))
			{
				//read video header
				VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmt->pbFormat;
				VIDEO_STREAM_CONFIG_CAPS *caps = (VIDEO_STREAM_CONFIG_CAPS *)pSCC;

				float fps = 10000000 / pVih->AvgTimePerFrame;		
				float fps_min = 10000000 / (caps->MaxFrameInterval); //from units of 100-nanoseconds to frames per second
				float fps_max = 10000000 / (caps->MinFrameInterval); //from units of 100-nanoseconds to frames per second
				
				auto lWidth = pVih->bmiHeader.biWidth;
				auto lHeight = pVih->bmiHeader.biHeight;
				auto horizontalStepSize = caps->OutputGranularityX;
				auto verticalStepSize = caps->OutputGranularityY;
				
				std::cout << std::endl;
				std::cout << i << ") ";

				std::cout << "Resolution: " << lWidth << " x " << lHeight << std::endl;
				std::cout << " Min Res: " << caps->MinOutputSize.cx << " x " << caps->MinOutputSize.cy << std::endl;
				std::cout << " Max Res: " << caps->MaxOutputSize.cx << " x " << caps->MaxOutputSize.cy << std::endl;
				std::cout << " Step size: " << "horizontal " <<horizontalStepSize << " vertical " << verticalStepSize << std::endl;

				std::cout << " fps: " << fps <<" , " << fps_min << " (min) " << fps_max << " (max)" << std::endl;
			}
		}
	}
	//release buffer with capabilites.
	delete[] pSCC;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void dshow_graph::set_camera_format(int capIndex)
{
	HRESULT hr;
	int iCount, iSize;
	BYTE *pSCC = NULL;
	AM_MEDIA_TYPE *pmt;

	if(pStreamConfigInterface == nullptr)
	{
		std::cout << "ERROR: set_camera_format()" << std::endl;
		return;
	}

	hr = pStreamConfigInterface->GetNumberOfCapabilities(&iCount, &iSize);

	pSCC = new BYTE[iSize];
	if (pSCC == NULL)
	{
		// TODO: Out of memory error.
	}

	// Get the first format.
	hr = pStreamConfigInterface->GetStreamCaps(capIndex, &pmt, pSCC);
	if (hr == S_OK)
	{
		hr = pStreamConfigInterface->SetFormat(pmt);
		if (FAILED(hr))
		{
			// TODO: Error handling.
		}
	}
	delete[] pSCC;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------|
//				                                 capture graph  |
//	|---------|		  |---------|      |---------|              |
//	| capture |		  |filter	|      | NULL    |              |
//	| filter  | ----> | Sample  | ---> | filter  |              |
//	|		  |		  | Grabber |      |         |              |
//	|---------|		  |---------|      |---------|              |
//---------------------------------------------------------------
void dshow_graph::setup_graph(std::function<void(uint8_t*, uint32_t)> callback_func)
{
	// COM stuff //

	//Initializes the COM library for use by the calling thread, sets the thread's concurrency model, 
	//and creates a new apartment for the thread if one is required.
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		printf("ERROR: CoInitializeEx");
	}

	// CREATE THE CAPTURE GRAPH BUILDER //
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&pCaptureGraphInterface);
	if (FAILED(hr))
	{
		printf("ERROR: CoCreateInstance");
	}

	// CREATE THE REGULAR GRAPH BUILDER //
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraphInterface);
	if (FAILED(hr))
	{
		printf("ERROR: CoCreateInstance");
	}

	//SET THE FILTERGRAPH//
	hr = pCaptureGraphInterface->SetFiltergraph(pGraphInterface); //specifies a filter graph for the capture graph builder to use.
	if (FAILED(hr))
	{
		printf("ERROR: pCaptureGraphInterface->SetFiltergraph");
	}

	//Retrieves pointers to the supported interfaces on an object.
	hr = pGraphInterface->QueryInterface(IID_IMediaControl, (void **)&pControlInterface);  //the interface provides methods for controlling the flow of data through the filter graph.
	if (FAILED(hr))
	{
		printf("ERROR: QueryInterface(IID_IMediaControl");
	}


	hr = pGraphInterface->QueryInterface(IID_IMediaEvent, (void **)&pEventInterface);		 //the interface contains methods for retrieving event 																						 //notifications and for overriding the Filter Graph Manager's default handling of events. 
	if (FAILED(hr))
	{
		printf("ERROR: QueryInterface(IID_IMediaEvent");
	}


	//FIND VIDEO DEVICE AND ADD TO GRAPH//
	IEnumMoniker *pEnum;
	hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
	if (SUCCEEDED(hr))
	{
		DisplayDeviceInformation(pEnum);
		pEnum->Release();
	}

	hr = pGraphInterface->AddFilter(pCapFilter, L"Capture Filter");
	if (FAILED(hr))
	{
		printf("ERROR: AddSourceFilter\r\n");
	}


	CAPTURE_MODE = PIN_CATEGORY_CAPTURE;
	//we do this because webcams don't have a preview mode
	hr = pCaptureGraphInterface->FindInterface(&CAPTURE_MODE, &MEDIATYPE_Video, pCapFilter, IID_IAMStreamConfig, (void **)&pStreamConfigInterface);
	if (FAILED(hr)) {
		printf("ERROR: AddSourceFilter\r\n");
	}

	// CREATE SAMPLE GRABBER FILTER
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pGrabberFilter));
	if (FAILED(hr))
	{
		printf("ERROR: AddSourceFilter\r\n");
	}

	hr = pGraphInterface->AddFilter(pGrabberFilter, L"Sample Grabber");
	if (FAILED(hr))
	{
		printf("ERROR: AddSourceFilter\r\n");
	}

	hr = pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabberInterface);
	if (FAILED(hr)) {
		printf("ERROR: QueryInterface\r\n");
	}


	//Set Params - One Shot should be false unless you want to capture just one buffer
	hr = pGrabberInterface->SetOneShot(FALSE);
	if (FAILED(hr)) {
		printf("ERROR: SetOneShot\r\n");
	}

	pGrabberInterface->SetBufferSamples(FALSE);
	if (FAILED(hr)) {
		printf("ERROR: SetBufferSamples\r\n");
	}


	//Tell the grabber to use our callback function 
	//We use SampleCB
	sgCallback = new SampleGrabberCallback();
	sgCallback->newFrame = false;
	sgCallback->setupBuffer(callback_func);
	pGrabberInterface->SetCallback(sgCallback, 1);//- 0 is for SampleCB and 1 for BufferCB
	if (FAILED(hr)) {
		printf("ERROR: SetCallback\n");
	}
	else {
		printf("SETUP: Capture callback set\n");
	}

	//NULL RENDERER//
	//used to give the video stream somewhere to go to.
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)(&pNullFilter));
	if (FAILED(hr)) {
		printf("ERROR: QueryInterface");
	}

	hr = pGraphInterface->AddFilter(pNullFilter, L"NullRenderer");
	if (FAILED(hr)) {
		printf("ERROR: QueryInterface\r\n");
	}


	set_camera_format(0);

	//RENDER STREAM//
	//This is where the stream gets put together.
	hr = pCaptureGraphInterface->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pCapFilter, pGrabberFilter, pNullFilter);

	if (FAILED(hr)) {
		printf("ERROR: RenderStream\r\n");
	}


}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int dshow_graph::setup(dshow_graph::device selected)
{
	selected_device_ = selected;

	auto retVal = 0;
	// COM stuff //

	//Initializes the COM library for use by the calling thread, sets the thread's concurrency model, 
	//and creates a new apartment for the thread if one is required.
	HRESULT hr;

	// CREATE THE CAPTURE GRAPH BUILDER //
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&pCaptureGraphInterface);
	if (FAILED(hr))
	{
		printf("ERROR: CoCreateInstance\r\n");
	}

	// CREATE THE REGULAR GRAPH BUILDER //
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraphInterface);
	if (FAILED(hr))
	{
		printf("ERROR: CoCreateInstance\r\n");
	}

	//SET THE FILTERGRAPH//
	hr = pCaptureGraphInterface->SetFiltergraph(pGraphInterface); //specifies a filter graph for the capture graph builder to use.
	if (FAILED(hr))
	{
		printf("ERROR: pCaptureGraphInterface->SetFiltergraph\r\n");
	}

	//Retrieves pointers to the supported interfaces on an object.
	hr = pGraphInterface->QueryInterface(IID_IMediaControl, (void **)&pControlInterface);  //the interface provides methods for controlling the flow of data through the filter graph.
	if (FAILED(hr))
	{
		printf("ERROR: QueryInterface(IID_IMediaControl\r\n");
	}
	hr = pGraphInterface->QueryInterface(IID_IMediaEvent, (void **)&pEventInterface);		 //the interface contains methods for retrieving event 
																							 //notifications and for overriding the Filter Graph Manager's default handling of events. 
	if (FAILED(hr))
	{
		printf("ERROR: QueryInterface(IID_IMediaEvent\r\n");
	}


	//FIND VIDEO DEVICE AND ADD TO GRAPH//
	IEnumMoniker *pEnum;
	hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
	if (SUCCEEDED(hr))
	{
		auto foundDevice = DisplayDeviceInformation(pEnum);
		pEnum->Release();

		if(!foundDevice)
		{
			std::cout <<std::endl << "***********************************" << std::endl;
			std::cout << "ERROR: Not compatiable device found" << std::endl;
			std::cout << "***********************************" << std::endl << std::endl;
			retVal = -1;
		}
	}

	hr = pGraphInterface->AddFilter(pCapFilter, L"Capture Filter");
	if (FAILED(hr))
	{
		printf("ERROR: AddSourceFilter\r\n");
	}


	CAPTURE_MODE = PIN_CATEGORY_CAPTURE;
	//we do this because webcams don't have a preview mode
	hr = pCaptureGraphInterface->FindInterface(&CAPTURE_MODE, &MEDIATYPE_Video, pCapFilter, IID_IAMStreamConfig, (void **)&pStreamConfigInterface);
	if (FAILED(hr)) {
		printf("ERROR: AddSourceFilter\r\n");
	}

	// CREATE SAMPLE GRABBER FILTER
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pGrabberFilter));
	if (FAILED(hr))
	{
		printf("ERROR: AddSourceFilter\r\n");
	}

	hr = pGraphInterface->AddFilter(pGrabberFilter, L"Sample Grabber");
	if (FAILED(hr))
	{
		printf("ERROR: AddSourceFilter\r\n");
	}

	hr = pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabberInterface);
	if (FAILED(hr)) {
		printf("ERROR: QueryInterface\r\n");
	}


	//Set Params - One Shot should be false unless you want to capture just one buffer
	hr = pGrabberInterface->SetOneShot(FALSE);
	if (FAILED(hr)) {
		printf("ERROR: SetOneShot\r\n");
	}

	pGrabberInterface->SetBufferSamples(FALSE);
	if (FAILED(hr)) {
		printf("ERROR: SetBufferSamples\r\n");
	}


	//Tell the grabber to use our callback function 
	//We use SampleCB
	sgCallback = new SampleGrabberCallback();
	sgCallback->newFrame = false;
	//sgCallback->setupBuffer(nullptr);//get everything ready but don't define callback function yet
	pGrabberInterface->SetCallback(sgCallback, 1);//- 0 is for SampleCB and 1 for BufferCB
	if (FAILED(hr)) {
		printf("ERROR: SetCallback\r\n");
	}
	else {
		printf("SETUP: Capture callback set\r\n");
	}

	//NULL RENDERER//
	//used to give the video stream somewhere to go to.
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)(&pNullFilter));
	if (FAILED(hr)) {
		printf("ERROR: QueryInterface\r\n");
	}

	hr = pGraphInterface->AddFilter(pNullFilter, L"NullRenderer");
	if (FAILED(hr)) {
		printf("ERROR: QueryInterface\r\n");
	}

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void dshow_graph::render()
{
	HRESULT hr;

	//RENDER STREAM//
	//This is where the stream gets put together.
	hr = pCaptureGraphInterface->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pCapFilter, pGrabberFilter, pNullFilter);

	if (FAILED(hr)) {
		printf("ERROR: RenderStream");
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void dshow_graph::run_graph()
{
	HRESULT hr;
	hr = pControlInterface->Run();
	if (FAILED(hr)) {
		printf("ERROR: run");
	}
	else
	{
		
		pEventInterface->WaitForCompletion(INFINITE, &evCode); //used to be INFINITE
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


IBaseFilter* dshow_graph::getCapFilter() const
{
	return pCapFilter;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void dshow_graph::set_callback(std::function<void(uint8_t*, uint32_t)> function)
{
	sgCallback->setupBuffer(function);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool dshow_graph::set_gain(uint32_t gainVal)
{
	HRESULT hr;

	// Query the capture filter for the IAMVideoProcAmp interface.
	IAMVideoProcAmp *pProcAmp = 0;
	hr = pCapFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);																				 //notifications and for overriding the Filter Graph Manager's default handling of events. 
	if (FAILED(hr))
	{
		printf("ERROR: QueryInterface(IID_IAMVideoProcAmp");
		return false;
	}

	//Specifies the gain adjustment.Zero
	//is normal.Positive values are brighter and negative values are darker.
	//The range of values depends on the device.
	pProcAmp->Set(VideoProcAmp_Gain, gainVal, VideoProcAmp_Flags_Manual);

	long getGainVal = -1;
	long getFlagsVal = -1;
	pProcAmp->Get(VideoProcAmp_Gain, &getGainVal, &getFlagsVal);

	if(getGainVal != gainVal && VideoProcAmp_Flags_Manual != getFlagsVal)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool dshow_graph::get_gain(long *gainVal)
{
	HRESULT hr;

	// Query the capture filter for the IAMVideoProcAmp interface.
	IAMVideoProcAmp *pProcAmp = 0;
	//notifications and for overriding the Filter Graph Manager's default handling of events. 
	hr = pCapFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);	
	if (FAILED(hr))
	{
		printf("ERROR: QueryInterface(IID_IAMVideoProcAmp");
		return false;
	}

	long getGainVal = -1;
	long getFlagsVal = -1;
	pProcAmp->Get(VideoProcAmp_Gain, &getGainVal, &getFlagsVal);

	*gainVal = getGainVal;

	return true;

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::vector<dshow_graph::device> dshow_graph::get_device_list()
{
	HRESULT hr;
	std::vector<dshow_graph::device> list; //just an empty list, somthing to return;

	IEnumMoniker *pEnum;
	hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
	if (SUCCEEDED(hr))
	{
		return buid_device_list(pEnum);
	}

	
	return list;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool dshow_graph::set_exposure(int val)
{
	HWND hTrackbar = nullptr; // Handle to the trackbar control. 
					// Initialize hTrackbar (not shown).
	HRESULT hr;

					// Query the capture filter for the IAMVideoProcAmp interface.
	IAMVideoProcAmp *pCamControl = 0;
	hr = pCapFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCamControl);
	if (FAILED(hr))
	{
		return false;
	}
	else
	{
		long Min, Max, Step, Default, Flags, currentVal;

		// Get the range and default value. 
		hr = pCamControl->GetRange(CameraControl_Exposure, &Min, &Max, &Step,
			&Default, &Flags);
		if (SUCCEEDED(hr))
		{
			// Get the current value.
			hr = pCamControl->Get(CameraControl_Exposure, &currentVal, &Flags);
		}

		if(val >= Min && val <=Max)
		{
			if (SUCCEEDED(hr))
			{
				// Set the current value.
				hr = pCamControl->Set(CameraControl_Exposure, val, CameraControl_Flags_Manual);

				return true;
			}
		}

	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::vector<dshow_graph::device> dshow_graph::buid_device_list(IEnumMoniker* pEnum)
{
	auto retVal = false;

	std::vector<dshow_graph::device> deviceList;

	IMoniker *pMoniker = NULL;

	while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
	{
		device currentDevice;

		IPropertyBag *pPropBag;
		HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue;
		}

		VARIANT var;
		VariantInit(&var);

		// Get description or friendly name.
		hr = pPropBag->Read(L"Description", &var, 0);
		if (FAILED(hr))
		{
			hr = pPropBag->Read(L"FriendlyName", &var, 0);
		}
		if (SUCCEEDED(hr))
		{
			std::wstring ws(var.bstrVal, SysStringLen(var.bstrVal));
			currentDevice.name = ws;
			VariantClear(&var);
		}

		hr = pPropBag->Write(L"FriendlyName", &var);

		// WaveInID applies only to audio capture devices.
		hr = pPropBag->Read(L"WaveInID", &var, 0);
		if (SUCCEEDED(hr))
		{
			printf("WaveIn ID: %d\n", var.lVal);
			VariantClear(&var);
		}

		hr = pPropBag->Read(L"DevicePath", &var, 0);
		if (SUCCEEDED(hr))
		{
			// The device path is not intended for display.
			printf("Device path: %S\n", var.bstrVal);
			std::wstring ws(var.bstrVal, SysStringLen(var.bstrVal));
			currentDevice.path = ws;
			VariantClear(&var);
		}

		pPropBag->Release();
		pMoniker->Release();

		deviceList.push_back(currentDevice);
	}


	return deviceList;
}