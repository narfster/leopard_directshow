
#include "opencv2/opencv.hpp"
#include "util_image.h"
#include "util_uvc_ext.h"
#include <thread>
#include "dshow_graph.h"
#include <conio.h>

#define RAW_IMAGE_SIZE (640 * 480 * 2)


//Image buffers:
uint8_t buffRGB[921600] = { 0 };
uint8_t rawBuff[921600] = { 0 };

void display_debug_blocking(uint8_t *p_rawImage, uint32_t buffLength)
{
	memcpy(rawBuff, p_rawImage, buffLength);

	////test display from this thread.
	util_image::gammaCorrection(rawBuff, rawBuff, 640, 480, 10, 1.6);
	util_image::raw_to_rgb(rawBuff, 0, buffRGB, 0);

	cv::Mat image(480, 640, CV_8UC3, buffRGB);


	cv::imshow("Debug blocking display", image);
	cv::waitKey(1);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void thread1_func(dshow_graph dshowGraphFilter)
{
	int exposure = 100;
	auto pCapFilter = dshowGraphFilter.getCapFilter();

	while (true)
	{
		int key_code;

		if (_kbhit())
		{
			key_code = _getch();
			if (key_code == '+')
			{
				exposure += 100;
				BYTE p_data[2];
				p_data[0] = (BYTE)(exposure);
				p_data[1] = (BYTE)(exposure >> 8);

				ULONG p_result[10] = { 0 };
				util_uvc_ext::write_to_uvc_extension(pCapFilter, 0x06, p_data, 2, p_result);
				std::cout << exposure << std::endl;
			}
			if (key_code == '-')
			{
				exposure -= 100;
				BYTE p_data[2];
				p_data[0] = (BYTE)(exposure);
				p_data[1] = (BYTE)(exposure >> 8);

				ULONG p_result[10] = { 0 };
				util_uvc_ext::write_to_uvc_extension(pCapFilter, 0x06, p_data, 2, p_result);
				std::cout << exposure << std::endl;
			}
			if (key_code == 't')
			{
				BYTE p_data[2];
				p_data[0] = (BYTE)(0x03);
				p_data[1] = (BYTE)(0x00);

				ULONG p_result[10] = { 0 };
				util_uvc_ext::write_to_uvc_extension(pCapFilter, 0x0b, p_data, 2, p_result);
				std::cout << "trigger enabled" << std::endl;
			}
			if (key_code == 'r')
			{
				BYTE p_data[2];
				p_data[0] = (BYTE)(0x00);
				p_data[1] = (BYTE)(0x00);

				ULONG p_result[10] = { 0 };
				util_uvc_ext::write_to_uvc_extension(pCapFilter, 0x0b, p_data, 2, p_result);
				std::cout << "trigger disabled" << std::endl;
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
	auto dshowGraphFilter = dshow_graph(display_debug_blocking);

	dshowGraphFilter.setup_graph();

	//startup thread for controling capture
	std::thread t1(thread1_func, dshowGraphFilter);

	dshowGraphFilter.run_graph();

	t1.join();

}
