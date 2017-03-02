#pragma once

#include <Dshow.h>
#include "qedit.h"
#include "SampleGrabberCallback.h"

class dshow_graph
{
public:
	dshow_graph();
	dshow_graph(std::function<void(uint8_t*, std::size_t)> callback_func);
	~dshow_graph();
	void setup_graph();
	void run_graph();
	void print_camera_cap();
	void set_camera_format(int capIndex);
	IBaseFilter* getCapFilter() const;
private:
	HRESULT EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum);
	void DisplayDeviceInformation(IEnumMoniker *pEnum);
	
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

	SampleGrabberCallback * sgCallback = NULL;

	GUID CAPTURE_MODE;

	std::function<void(uint8_t*, std::size_t)> callback_func_;
};