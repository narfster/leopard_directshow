#pragma once
#include "video_in_base.h"
#include <cam_econ/eCAMFwSw.h>

#define CAM_CU51    L"See3CAM_CU51" 
#define CAM_12CUNIR L"See3CAM_12CUNIR"


class econ_cam : public video_in_base
{
public:

	directshow::device device_;


	explicit econ_cam(directshow::device dev)
	{
		device_ = dev;
		init_ext_unit();
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	bool init_ext_unit() const
	{
		auto str = device_.path.c_str();
		bool isSuccess = InitExtensionUnit(const_cast<TCHAR *>(str));
		if(isSuccess == false)
		{
			std::cout << "InitExtensionUnit FAILED" << std::endl;
			return false;
		}
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	void print_firmare_ver()
	{
		UINT8 pMajorVersion = 0;
		UINT8 pMinorVersion1 = 0;
		UINT16 pMinorVersion2 = 0;
		UINT16 pMinorVersion3 = 0;
		bool isSuccess = ReadFirmwareVersion(&pMajorVersion, &pMinorVersion1,&pMinorVersion2, & pMinorVersion3);

		if(isSuccess)
		{
			std::cout << convert_uint_to_string(pMajorVersion) << "." << convert_uint_to_string(pMinorVersion1) << "." << convert_uint_to_string(pMinorVersion2) << "." << convert_uint_to_string(pMinorVersion3) << std::endl;;
		}
		else
		{
			std::cout << "print_firmare_ver : Error" << std::endl;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	bool set_trigger(bool isEnabled) override
	{
		bool isSuccess;
		if(isEnabled)
		{
			isSuccess = EnableTriggerMode();

		}
		else
		{
			isSuccess = EnableMasterMode();
		}

		if(!isSuccess)
		{
			std::cout << "set_trigger : Error" << std::endl;
		}

		return isSuccess;
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	directshow::imgFormat get_img_format() const override
	{
		auto fmt = dshow_.get_image_format();
		if(device_.name == CAM_12CUNIR || 
			device_.name == CAM_CU51)
		{
			fmt.bitsPerPixel = 12;
		}
		return fmt;
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	

private:
	std::string convert_uint_to_string(UINT8 n)
	{
		int tmp = n;
		std::stringstream ss;
		ss << tmp;
		return ss.str();
	}


};
