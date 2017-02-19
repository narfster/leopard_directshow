#include <Dshow.h>
#include "qedit.h"
#include "opencv2/opencv.hpp"
#include "util_image.h"
#include <Vidcap.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include <thread>
#include <stdio.h>
#include <windows.h>
#include <initguid.h>
#include <ks.h>

// {78E321E1-C8AC-40A5-8AC9-75A2A02C74FB}
DEFINE_GUID(GUID_EXTENSION_UNIT_DESCRIPTOR,
	0x78e321e1, 0xc8ac, 0x40a5, 0x8a, 0xc9, 0x75, 0xa2, 0xa0, 0x2c, 0x74, 0xfb);



//capture filter -> grabber filter -> null filter
IBaseFilter *pCapFilter = NULL;			//filter to capture video input from webcam.
IBaseFilter *pGrabberFilter = NULL;		//filter to sample grabber video stream.
IBaseFilter *pNullFilter = NULL;		//filter to output grabber filter.

//Pointers to COM Interfaces.
IGraphBuilder *pGraphInterface = NULL;				    //a graph builder object
ICaptureGraphBuilder2 *pCaptureGraphInterface = NULL;	//Capture graph builder object
IMediaControl *pControlInterface = NULL;				//control interface
IMediaEvent   *pEventInterface = NULL;					//event interface
IAMStreamConfig *pStreamConfigInterface = NULL;			//stream config interface
ISampleGrabber *pGrabberInterface = NULL;				//grabber interface.

GUID CAPTURE_MODE;

//Image buffers:
uint8_t buffRGB[921600] = { 0 };
uint8_t rawBuff[921600] = { 0 };

void display_debug_blocking()
{

	////test display from this thread.
	util_image::gammaCorrection(rawBuff, rawBuff, 640, 480, 10, 1.6);
	util_image::raw_to_rgb(rawBuff, 0, buffRGB, 0);

	cv::Mat image(480, 640, CV_8UC3, buffRGB);


	cv::imshow("Debug blocking display", image);
	cv::waitKey(1);
}

//////////////////////////////  CALLBACK  ////////////////////////////////

//Callback class
class SampleGrabberCallback : public ISampleGrabberCB {
public:

	//------------------------------------------------
	SampleGrabberCallback() {
		InitializeCriticalSection(&critSection);
		freezeCheck = 0;


		bufferSetup = false;
		newFrame = false;
		latestBufferLength = 0;

		hEvent = CreateEvent(NULL, true, false, NULL);
	}


	//------------------------------------------------
	~SampleGrabberCallback() {
		ptrBuffer = NULL;
		DeleteCriticalSection(&critSection);
		CloseHandle(hEvent);
		if (bufferSetup) {
			delete[] pixels;
		}
	}


	//------------------------------------------------
	bool setupBuffer(int numBytesIn) {
		if (bufferSetup) {
			return false;
		}
		else {
			numBytes = numBytesIn;
			pixels = new unsigned char[numBytes];
			bufferSetup = true;
			newFrame = false;
			latestBufferLength = 0;
		}
		return true;
	}


	//------------------------------------------------
	STDMETHODIMP_(ULONG) AddRef() { return 1; }
	STDMETHODIMP_(ULONG) Release() { return 2; }


	//------------------------------------------------
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) {
		*ppvObject = static_cast<ISampleGrabberCB*>(this);
		return S_OK;
	}


	//This method is meant to have less overhead
	//------------------------------------------------
	STDMETHODIMP SampleCB(double Time, IMediaSample *pSample) {
		if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0) return S_OK;

		HRESULT hr = pSample->GetPointer(&ptrBuffer);

		if (hr == S_OK) {
			latestBufferLength = pSample->GetActualDataLength();
			// Leon if(latestBufferLength == numBytes){
			if (1) { // don't check the buffer size, RAW doesn't match
				EnterCriticalSection(&critSection);
				memcpy(pixels, ptrBuffer, latestBufferLength);
				newFrame = true;
				freezeCheck = 1;
				LeaveCriticalSection(&critSection);
				SetEvent(hEvent);
			}
			else {
				printf("ERROR: SampleCB() - buffer sizes do not match\n");
			}
		}

		return S_OK;
	}


	//This method is meant to have more overhead
	STDMETHODIMP BufferCB(double Time, BYTE *pBuffer, long BufferLen) {

		memcpy(rawBuff, pBuffer, 640 * 480 * 2);
		display_debug_blocking();

		return E_NOTIMPL;
	}

	int freezeCheck;

	int latestBufferLength;
	int numBytes;
	bool newFrame;
	bool bufferSetup;
	unsigned char * pixels;
	unsigned char * ptrBuffer;
	CRITICAL_SECTION critSection;
	HANDLE hEvent;
};

SampleGrabberCallback * sgCallback;


HRESULT EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum)
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

void DisplayDeviceInformation(IEnumMoniker *pEnum)
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



int write_to_uvc_extension(IBaseFilter* camera, int property_id, BYTE* bytes, int length, ULONG* ulBytesReturned)
{
	HRESULT hr;
	IKsTopologyInfo *pKsTopologyInfo;
	hr = camera->QueryInterface(__uuidof(IKsTopologyInfo), (void **)&pKsTopologyInfo);

	DWORD numberOfNodes;	hr = pKsTopologyInfo->get_NumNodes(&numberOfNodes);

	DWORD i;	GUID nodeGuid;
	for (i = 0; i < numberOfNodes; i++)
	{
		if (SUCCEEDED(pKsTopologyInfo->get_NodeType(i, &nodeGuid)))
		{
			if (nodeGuid == KSNODETYPE_DEV_SPECIFIC)
			{ // Found the extension node
				DWORD pNodeId = i;
				IKsNodeControl *pUnk;
				IKsControl *pKsControl;

				// create node instance
				hr = pKsTopologyInfo->CreateNodeInstance(i, __uuidof(IUnknown), (VOID**)&pUnk);
				hr = pUnk->QueryInterface(__uuidof(IKsControl), (VOID**)&pKsControl);

				KSP_NODE  s;	//ULONG  ulBytesReturned;

								// this is guid of our device extension unit
				s.Property.Set = GUID_EXTENSION_UNIT_DESCRIPTOR;
				s.Property.Id = property_id;
				s.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
				s.NodeId = i;
				hr = pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), bytes, length, ulBytesReturned);

				if (hr == S_OK)	return 0;
				else return -1;
			}
		}
	}

	return -1;
}


int read_from_uvc_extension(IBaseFilter* camera, int property_id, BYTE* bytes, int length, ULONG* ulBytesReturned)
{
	HRESULT hr;
	IKsTopologyInfo *pKsTopologyInfo;
	hr = camera->QueryInterface(__uuidof(IKsTopologyInfo), (void **)&pKsTopologyInfo);

	DWORD numberOfNodes;
	hr = pKsTopologyInfo->get_NumNodes(&numberOfNodes);

	DWORD i;	GUID nodeGuid;
	for (i = 0; i < numberOfNodes; i++)
	{
		if (SUCCEEDED(pKsTopologyInfo->get_NodeType(i, &nodeGuid)))
		{
			if (nodeGuid == KSNODETYPE_DEV_SPECIFIC)
			{ // Found the extension node
				DWORD pNodeId = i;
				IKsNodeControl *pUnk;
				IKsControl *pKsControl;

				// create node instance
				hr = pKsTopologyInfo->CreateNodeInstance(i, __uuidof(IUnknown), (VOID**)&pUnk);
				hr = pUnk->QueryInterface(__uuidof(IKsControl), (VOID**)&pKsControl);

				KSP_NODE  s;

				// this is guid of our device extension unit
				s.Property.Set = GUID_EXTENSION_UNIT_DESCRIPTOR;
				s.Property.Id = property_id;
				s.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
				s.NodeId = i;
				hr = pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), bytes, length, ulBytesReturned);

				return 0;
			}
		}
	}

	return -1;
}


#include <conio.h>

int exposure = 100;

void thread1_func()
{

	while (true)
	{
		int key_code;

		if (_kbhit())
		{
			key_code = _getch();
			if (key_code == '+')
			{
				exposure += 100;
				BYTE p_data[2];
				p_data[0] = (BYTE)(exposure);
				p_data[1] = (BYTE)(exposure >> 8);

				ULONG p_result[10] = { 0 };
				write_to_uvc_extension(pCapFilter, 0x06, p_data, 2, p_result);
				std::cout << exposure << std::endl;
			}
			if (key_code == '-')
			{
				exposure -= 100;
				BYTE p_data[2];
				p_data[0] = (BYTE)(exposure);
				p_data[1] = (BYTE)(exposure >> 8);

				ULONG p_result[10] = { 0 };
				write_to_uvc_extension(pCapFilter, 0x06, p_data, 2, p_result);
				std::cout << exposure << std::endl;
			}
			if (key_code == 't')
			{
				BYTE p_data[2];
				p_data[0] = (BYTE)(0x03);
				p_data[1] = (BYTE)(0x00);

				ULONG p_result[10] = { 0 };
				write_to_uvc_extension(pCapFilter, 0x0b, p_data, 2, p_result);
				std::cout << "trigger enabled" << std::endl;
			}
			if (key_code == 'r')
			{
				BYTE p_data[2];
				p_data[0] = (BYTE)(0x00);
				p_data[1] = (BYTE)(0x00);

				ULONG p_result[10] = { 0 };
				write_to_uvc_extension(pCapFilter, 0x0b, p_data, 2, p_result);
				std::cout << "trigger disabled" << std::endl;
			}

		}
		else
		{

		}



	}

}

//   
//	|---------|		  |---------|      |---------|
//	| capture |		  |filter	|      | NULL    |
//	| filter  | ----> | Sample  | ---> | filter  |
//	|		  |		  | Grabber |      |         |
//	|---------|		  |---------|      |---------|

void main()
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
	sgCallback->setupBuffer(640 * 480 * 2);
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

	BYTE p_data[2];
	p_data[0] = (BYTE)(exposure);
	p_data[1] = (BYTE)(exposure >> 8);

	ULONG p_result[10] = { 0 };
	write_to_uvc_extension(pCapFilter, 0x06, p_data, 2, p_result);

	//startup thread
	std::thread t1(thread1_func);


	hr = pControlInterface->Run();
	if (FAILED(hr)) {
		printf("ERROR: run");
	}
	else
	{
		long evCode;
		pEventInterface->WaitForCompletion(INFINITE, &evCode);
	}



	CoUninitialize();


	t1.join();

}
