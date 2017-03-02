
#include "opencv2/opencv.hpp"
#include "util_image.h"
#include "util_uvc_ext.h"
#include <thread>
#include "dshow_graph.h"
#include <conio.h>
#include "leopard_cam.h"

//VGA
#define RAW_IMAGE_SIZE_VGA_PIX (640 * 480)
#define RAW_IMAGE_SIZE_VGA_RAW (640 * 480 * 2)
#define RAW_IMAGE_SIZE_VGA_RGB (640 * 480 * 3)
//720p
#define RAW_IMAGE_SIZE_720p_PIX (1280 * 720)
#define RAW_IMAGE_SIZE_720p_RAW (RAW_IMAGE_SIZE_720p_PIX* 2)
#define RAW_IMAGE_SIZE_720p_RGB (RAW_IMAGE_SIZE_720p_PIX * 3)
//5MP
#define RAW_IMAGE_SIZE_5MP_PIX (2592 * 1944)
#define RAW_IMAGE_SIZE_5MP_RAW (RAW_IMAGE_SIZE_720p_PIX * 2)
#define RAW_IMAGE_SIZE_5MP_RGB (RAW_IMAGE_SIZE_720p_RGB * 3)


//Image buffers:
uint8_t buffRGB[RAW_IMAGE_SIZE_720p_RGB] = { 0 };
uint8_t rawBuff[RAW_IMAGE_SIZE_720p_RGB] = { 0 };

void display_debug_blocking(uint8_t *p_rawImage, uint32_t buffLength)
{
	memcpy(rawBuff, p_rawImage, buffLength);

	////test display from this thread.
	//util_image::gammaCorrection(rawBuff, rawBuff, 1280, 720, 12, 1.6);
	
	util_image::raw_to_rgb(rawBuff, 0, buffRGB, 0, RAW_IMAGE_SIZE_720p_PIX, 12);

	cv::Mat image(720, 1280, CV_8UC3, buffRGB);


	cv::imshow("Debug blocking display", image);
	cv::waitKey(1);
}

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


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void main()
{
	leopard_cam leo_cam;

	leo_cam.setup(display_debug_blocking);

	//startup thread for controling capture
	std::thread t1(thread1_func, &leo_cam);

	leo_cam.run();

	t1.join();

}
