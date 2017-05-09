#pragma once
#include <functional>
class IVideoIn
{
public:
	virtual ~IVideoIn()
	{
	}

	virtual void setup(std::function<void(uint8_t*, uint32_t)> callback_func) = 0;
	virtual void run() = 0;
	virtual bool set_exposure(uint32_t exposure) = 0;
	virtual uint32_t get_exposure() = 0;
	virtual bool set_gain(uint32_t gain) = 0;
	virtual long get_gain() = 0;
	virtual bool set_trigger(bool isEnabled) = 0;
	virtual void print_camera_cap() = 0;
	virtual int set_trigger_delay_time_zero() = 0;
private:
};