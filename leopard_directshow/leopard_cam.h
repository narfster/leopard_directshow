#pragma once

#include "IVideoIn.h"
#include "dshow_graph.h"
#include "util_uvc_ext.h"

#include <iostream>
#include "leopard_cam_ext.h"
#define LOG_MSG(lvl,msg ) std::cout << msg


#define XU_MODE_SWITCH		  (0x01)
#define XU_WINDOW_REPOSITION  (0x02)
#define XU_LED_MODES		  (0x03)
#define XU_GAIN_CONTROL_RGB   (0x04)
#define XU_GAIN_CONTROL_A	  (0x05)
#define XU_EXPOSURE_TIME	  (0x06)
#define XU_UUID_HWFW_REV	  (0x07)
#define XU_DEFECT_PIXEL_TABLE (0x08)
#define XU_SOFT_TRIGGER		  (0x09)
#define XU_TRIGGER_MODE		  (0x0b)
#define XU_TRIGGER_DELAY_TIME (0x0a)
// for sensor register configuration into flash
#define XU_SENSOR_REGISTER_CONFIGURATION  (0x0c)
#define XU_EXTENSION_INFO	  (0x0d)
#define XU_GENERIC_REG_RW     (0x0e)

class leopard_cam : public IVideoIn
{
public:


	leopard_cam()
	{
		
	}

	~leopard_cam()
	{
		//dshow_graph destrctor will be called automaticly.
	}


	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	bool set_exposure(int exposureVal) override
	{
		if (status_ == 0)
		{
			int errCode = 0;
			auto pCapFilter = dshow_.getCapFilter();

			if (pCapFilter == nullptr)
			{
				return false;
			}

			uint8_t p_data[2];
			p_data[0] = (uint8_t)(exposureVal);
			p_data[1] = (uint8_t)(exposureVal >> 8);

			ULONG p_result[10] = { 0 };

			errCode = util_uvc_ext::write_to_uvc_extension(pCapFilter, XU_EXPOSURE_TIME, p_data, 2, p_result);

			LOG_MSG(trace, "exposrue set to: " << exposureVal << std::endl);
			return true;
		}

		return false;
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	bool set_trigger(bool isEnabled) 
	{

		if (status_ == 0)
		{

			auto pCapFilter = dshow_.getCapFilter();

			uint8_t p_data[2];
			if (isEnabled == true)
			{
				p_data[0] = (uint8_t)(0x03);
			}
			else
			{
				p_data[0] = (uint8_t)(0x00);
			}

			p_data[1] = (uint8_t)(0x00);

			ULONG p_result[10] = { 0 };
			status_ = util_uvc_ext::write_to_uvc_extension(pCapFilter, XU_TRIGGER_MODE, p_data, 2, p_result);
			if (status_ == 0)
			{
				LOG_MSG(trace, "camera trigger = " << isEnabled << std::endl);
			}
			else
			{
				LOG_MSG(trace, "ERROR: set camera trigger" << std::endl);
			}
		}

		return status_;
	}


	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	uint32_t get_exposure() 
	{
		int exposure = -1;

		if (status_ == 0)
		{
			uint8_t p_data[2];
			uint8_t val1;
			uint8_t val2;
			unsigned long length_returned;
			auto pCapFilter = dshow_.getCapFilter();

			status_ = util_uvc_ext::read_from_uvc_extension(pCapFilter, XU_EXPOSURE_TIME, p_data, 2, &length_returned);
			if (status_ == 0)
			{

			}
			else
			{

			}
			val1 = p_data[0];
			val2 = p_data[1];
			exposure = ((int)p_data[1] << 8 | (int)p_data[0]) & 0x0000ffff;
		}

		return exposure;
	}



	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	int set_trigger_delay_time_zero() 
	{
		if (status_ == 0)
		{
			auto pCapFilter = dshow_.getCapFilter();

			uint8_t p_data[4];
			p_data[0] = 0;
			p_data[1] = 0;
			p_data[2] = 0;
			p_data[3] = 0;

			ULONG p_result[10] = { 0 };

			status_ = util_uvc_ext::write_to_uvc_extension(pCapFilter, XU_TRIGGER_DELAY_TIME, p_data, 4, p_result);

			LOG_MSG(trace, "set_trigger_delay_time_zero " << std::endl);
		}

		return status_;
	}



	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	int get_device_status() const
	{
		return status_;
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

private:



};
