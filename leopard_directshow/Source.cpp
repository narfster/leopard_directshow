
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

void thread1_func(IVideoIn *video_in)
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
				exposure += 100;
				video_in->set_exposure(exposure);
			}
			if (key_code == '-')
			{	
				exposure -= 100;
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

		}
		else
		{

		}

	}

}
//MT9M021M
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
int main()
{
	leopard_cam leo_cam;
	video_display vdisplay;


	leo_cam.set_format(0);
	auto imgFmt = leo_cam.get_img_format();
	vdisplay.set_img_format(imgFmt.width, imgFmt.height, imgFmt.bytesPerPixel, imgFmt.bitsPerPixel);
	auto func = std::bind(&video_display::display_blocking, &vdisplay, std::placeholders::_1, std::placeholders::_2);
	leo_cam.set_callback(func);

	//startup thread for controling capture
	std::thread t1(thread1_func, &leo_cam);

	leo_cam.run();

	t1.join();

}
