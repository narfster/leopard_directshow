
#include "dshow_graph.h"


dshow_graph::dshow_graph()
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

dshow_graph::dshow_graph(std::function<void(uint8_t*, std::size_t)> callback_func)
{
	callback_func_ = callback_func;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

dshow_graph::~dshow_graph()
{
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


void dshow_graph::DisplayDeviceInformation(IEnumMoniker *pEnum)
{
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
			char tmp[10];
			memcpy(tmp, var.bstrVal, 7);
			if (memcmp(var.bstrVal, "M", 1) == 0)
			{

				hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCapFilter);
				if (SUCCEEDED(hr))
				{
					//hr = m_pGraph->AddFilter(pCapFilter, L"Capture Filter");
				}
			}
			printf("%S\n", var.bstrVal);
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
void dshow_graph::setup_graph()
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
	hr = pGraphInterface->QueryInterface(IID_IMediaEvent, (void **)&pEventInterface);		 //the interface contains methods for retrieving event 
																							 //notifications and for overriding the Filter Graph Manager's default handling of events. 
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
		printf("ERROR: AddSourceFilter");
	}


	CAPTURE_MODE = PIN_CATEGORY_CAPTURE;
	//we do this because webcams don't have a preview mode
	hr = pCaptureGraphInterface->FindInterface(&CAPTURE_MODE, &MEDIATYPE_Video, pCapFilter, IID_IAMStreamConfig, (void **)&pStreamConfigInterface);
	if (FAILED(hr)) {
		printf("ERROR: AddSourceFilter");
	}

	// CREATE SAMPLE GRABBER FILTER
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pGrabberFilter));
	if (FAILED(hr))
	{
		printf("ERROR: AddSourceFilter");
	}

	hr = pGraphInterface->AddFilter(pGrabberFilter, L"Sample Grabber");
	if (FAILED(hr))
	{
		printf("ERROR: AddSourceFilter");
	}

	hr = pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabberInterface);
	if (FAILED(hr)) {
		printf("ERROR: QueryInterface");
	}


	//Set Params - One Shot should be false unless you want to capture just one buffer
	hr = pGrabberInterface->SetOneShot(FALSE);
	if (FAILED(hr)) {
		printf("ERROR: SetOneShot");
	}

	pGrabberInterface->SetBufferSamples(FALSE);
	if (FAILED(hr)) {
		printf("ERROR: SetBufferSamples");
	}


	//Tell the grabber to use our callback function 
	//We use SampleCB
	sgCallback = new SampleGrabberCallback();
	sgCallback->newFrame = false;
	sgCallback->setupBuffer(callback_func_);
	pGrabberInterface->SetCallback(sgCallback, 1);//- 0 is for SampleCB and 1 for BufferCB
	if (FAILED(hr)) {
		printf("ERROR: SetCallback");
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
		printf("ERROR: QueryInterface");
	}


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
		long evCode;
		pEventInterface->WaitForCompletion(INFINITE, &evCode);
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


IBaseFilter* dshow_graph::getCapFilter() const
{
	return pCapFilter;
}
