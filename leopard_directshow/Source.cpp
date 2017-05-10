
#include "opencv2/opencv.hpp"
#include "util_image.h"
#include "util_uvc_ext.h"
#include <thread>
#include "dshow_graph.h"
#include <conio.h>
#include "leopard_cam.h"
#include "video_display.h"
#include "econ_cam.h"


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void thread1_func(IVideoIn *video_in)
{
	int exposure = 2;


	while (true)
	{
		int key_code;

		if (_kbhit())
		{
			key_code = _getch();
			if (key_code == '+')
			{
				exposure += 1;
				if (video_in->set_exposure(exposure) == false)
				{
					exposure--;
				}
				std::cout << "exposure " << exposure << std::endl;
			}
			if (key_code == '-')
			{
				exposure -= 1;
				if (video_in->set_exposure(exposure) == false)
				{
					exposure++;
				}
				std::cout << "exposure " << exposure << std::endl;
			}
			if (key_code == 't')
			{
				video_in->set_trigger(true);
			}
			if (key_code == 'r')
			{
				video_in->set_trigger(false);
			}

			if (key_code == 'p')
			{
				std::cout << "//=============//" << std::endl;
				video_in->print_camera_cap();
				std::cout << "//=============//" << std::endl;
			}
			if (key_code == 'd')
			{
				std::cout << "//=============//" << std::endl;
				if ((typeid(*video_in) == typeid(leopard_cam)))
				{
					static_cast<leopard_cam*>(video_in)->set_trigger_delay_time_zero();
				}
				else
				{
					std::cout << "cannot call a leo cam function" << std::endl;
				}

				std::cout << "//=============//" << std::endl;
			}

		}
		else
		{

		}

	}

}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
int main()
{
	//std::wstring dev_name{ L"MT9V034" };
	//std::wstring dev_name{ L"See3CAM_CU51" };
	//std::wstring dev_name{ L"See3CAM_12CUNIR" };
	//std::wstring dev_name{ L"MT9M021M" };


	auto video_in = std::make_unique<IVideoIn>();
	video_display vdisplay;

	bool isDeviceFound = false;
	auto deviceList = video_in->get_devices_list();
	int i = 0;
	for (i = 0; i < deviceList.size() && isDeviceFound == false; i++)
	{
		if (deviceList[i].name == L"MT9M021M")
		{
			video_in = std::make_unique<leopard_cam>();
			video_in->set_format(6);
			isDeviceFound = true;
		}
		else if (deviceList[i].name == L"MT9V034")
		{
			video_in = std::make_unique<leopard_cam>();
			video_in->set_format(0);
			isDeviceFound = true;
		}
		else if (deviceList[i].name == L"See3CAM_12CUNIR")
		{
			video_in = std::make_unique<econ_cam>();
			video_in->set_format(4);
			isDeviceFound = true;
		}
		else
		{

		}

	}

	if (isDeviceFound == true)
	{
		video_in->setup(deviceList[i-1]);
		auto imgFmt = video_in->get_img_format();
		vdisplay.set_img_format(imgFmt.width, imgFmt.height, imgFmt.bytesPerPixel, 12);
		auto func = std::bind(&video_display::display_blocking, &vdisplay, std::placeholders::_1, std::placeholders::_2);
		video_in->set_callback(func);

		//startup thread for controling capture
		std::thread t1(thread1_func, video_in.get());
		std::thread t2(&IVideoIn::run, video_in.get());

		t1.join();
		t2.join();
	}
	else
	{
		std::cout << " no device found. " << std::endl;
	}




}
