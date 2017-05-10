#pragma once
#include "IVideoIn.h"
#include <cam_econ/eCAMFwSw.h>
class econ_cam : public IVideoIn
{
public:

	econ_cam()
	{
	
	}

	explicit econ_cam(std::wstring devicePath)
	{
		init_ext_unit(devicePath);
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	static bool init_ext_unit(std::wstring devicePath)
	{
		auto str = devicePath.c_str();
		bool isSuccess = InitExtensionUnit((TCHAR *)str);
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

private:
	std::string convert_uint_to_string(UINT8 n)
	{
		int tmp = n;
		std::stringstream ss;
		ss << tmp;
		return ss.str();
	}


};
