
#include "opencv2/opencv.hpp"
#include "util_image.h"
#include "util_uvc_ext.h"
#include <thread>
#include "dshow_graph.h"
#include <conio.h>
#include "leopard_cam.h"
#include "video_display.h"


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void thread1_func(leopard_cam *video_in)
{
	int exposure = 100;


	while (true)
	{
		int key_code;

		if (_kbhit())
		{
			key_code = _getch();
			if (key_code == '+')
			{
				exposure += 10;
				video_in->set_exposure(exposure);
			}
			if (key_code == '-')
			{
				exposure -= 10;
				if (exposure < 0) exposure = 0;
				video_in->set_exposure(exposure);
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
				video_in->set_trigger_delay_time_zero();
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
	//std::wstring dev_name { L"MT9V034" };
	//std::wstring dev_name{ L"See3CAM_CU51" };
	std::wstring dev_name{ L"See3CAM_12CUNIR" };
	//std::wstring dev_name{ L"MT9M021M" };
	

	leopard_cam leo_cam;
	video_display vdisplay;


	auto deviceList = leo_cam.get_devices_list();
	for (auto i = 0; i < deviceList.size(); i++)
	{
		if (deviceList[i].name == dev_name)
		{
			leo_cam.setup(deviceList[i]);
			leo_cam.set_format(6);
			auto imgFmt = leo_cam.get_img_format();
			vdisplay.set_img_format(imgFmt.width, imgFmt.height, imgFmt.bytesPerPixel, 12);
			auto func = std::bind(&video_display::display_blocking, &vdisplay, std::placeholders::_1, std::placeholders::_2);
			leo_cam.set_callback(func);

			break;
		}
	}

	//startup thread for controling capture
	std::thread t1(thread1_func, &leo_cam);
	std::thread t2(&leopard_cam::run, leo_cam);

	t1.join();
	t2.join();


}
