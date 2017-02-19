#pragma once
#include "qedit.h"
#include <functional>

class SampleGrabberCallback : public ISampleGrabberCB {
public:

	//------------------------------------------------
	SampleGrabberCallback(): numBytes(0), pixels(nullptr), ptrBuffer(nullptr)
	{
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
	bool setupBuffer(std::function<void(uint8_t*, std::size_t)> callback_func) {
		if (bufferSetup) {
			return false;
		}
		else {
			callback_func_ = callback_func;
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
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) override {
		*ppvObject = static_cast<ISampleGrabberCB*>(this);
		return S_OK;
	}


	//This method is meant to have less overhead
	//------------------------------------------------
	STDMETHODIMP SampleCB(double Time, IMediaSample *pSample) override{
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
				//printf("ERROR: SampleCB() - buffer sizes do not match\n");
			}
		}

		return S_OK;
	}


	//This method is meant to have more overhead
	STDMETHODIMP BufferCB(double Time, BYTE *pBuffer, long BufferLen) override{

		callback_func_(pBuffer, BufferLen);
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

	std::function<void(uint8_t*, std::size_t)> callback_func_;
};
