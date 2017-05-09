#pragma once

#include <Dshow.h>
#include "qedit.h"
#include "SampleGrabberCallback.h"
#include <vector>

class dshow_graph
{
public:
	struct imgFormat
	{
		uint32_t width;
		uint32_t height;
		float fps;
		uint32_t frameSize;
		int bytesPerPixel;
		int bitsPerPixel;
	};

	struct device
	{
		std::wstring name;
		std::wstring path;
	};

	dshow_graph();
	~dshow_graph();
	void run_graph();
	void print_camera_cap();
	void set_camera_format(int capIndex);
	void setup_graph(std::function<void(uint8_t*, uint32_t)> callback_func);
	int setup(dshow_graph::device selected);
	void render();
	imgFormat get_image_format() const;
	IBaseFilter* getCapFilter() const;
	void set_callback(std::function<void(uint8_t*, uint32_t)> function);
	bool set_gain(uint32_t gainVal);
	bool get_gain(long *gainVal);
	std::vector<dshow_graph::device> get_device_list();
	
private:
	
	HRESULT EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum);
	bool DisplayDeviceInformation(IEnumMoniker* pEnum);
	std::vector<dshow_graph::device> buid_device_list(IEnumMoniker* pEnum);


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

	long evCode; //event code;

	std::function<void(uint8_t*, std::size_t)> callback_func_ = nullptr;
	
	device selected_device_;
};
