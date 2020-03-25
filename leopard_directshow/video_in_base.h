#pragma once
#include <functional>
class video_in_base
{

public:

	typedef void(*callback_function)(uint8_t*, uint32_t); // type callback function
	
	virtual ~video_in_base()
	{
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	virtual bool set_exposure(int exposureVal)
	{
		return dshow_.set_exposure(exposureVal);
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	virtual bool set_trigger(bool isEnabled)
	{
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	virtual uint32_t get_exposure()
	{
		return 0;
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	virtual bool set_gain(uint32_t gain)
	{
		return dshow_.set_gain(gain);
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	virtual long get_gain()
	{
		long gainVal = 0;
		return dshow_.get_gain(&gainVal);

	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	std::vector<directshow::device> get_devices_list()
	{
		return dshow_.get_device_list();
	}


	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	void setup(std::function<void(uint8_t*, uint32_t)> callback_func)
	{
		dshow_.setup_graph(callback_func);
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	int setup(directshow::device selected)
	{
		status_ = dshow_.setup(selected);
		if(status_ != -1 && capIndex_ != -1)
		{
			dshow_.set_camera_format(capIndex_);
		}
		
		return status_;
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	void run()
	{
		if (status_ == 0)
		{
			dshow_.render();
			dshow_.run_graph();
		}

	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	void stop()
	{
		dshow_.shutdown();
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	void print_camera_cap() 
	{
		dshow_.print_camera_cap();
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	virtual directshow::imgFormat get_img_format() const
	{
		return dshow_.get_image_format();
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	void set_callback_c_style(callback_function callback_func)
	{
		dshow_.set_callback(callback_func);
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	void set_callback(std::function<void(uint8_t*, uint32_t)> callback_func)
	{
		dshow_.set_callback(callback_func);
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	void set_format(int capIndex)
	{
		capIndex_ = capIndex;
	}


	directshow dshow_;
	int status_ = -1;
	int capIndex_ = -1;

private:

	
	

};