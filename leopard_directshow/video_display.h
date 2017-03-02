#pragma once
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
#include <cstdint>
#include <cstring>
#include "util_image.h"
#include "opencv2/opencv.hpp"


class video_display
{
public:
	uint32_t iWidth_;

	int bytesPerPix_;

	uint32_t iHeight_;

	int bitsPerPix_;

	uint8_t* pBuffRGB_ = nullptr;
	
	uint8_t* pBuffRaw_ = nullptr;

	uint32_t buffRawSize_;

	video_display(): iWidth_(0), bytesPerPix_(0), iHeight_(0), bitsPerPix_(0), buffRawSize_(0)
	{
	}

	void set_img_format(uint32_t width, uint32_t height, int bytesPerPix, int bitsPerPix)
	{
		iWidth_ = width;
		iHeight_ = height;
		bytesPerPix_ = bytesPerPix;
		bitsPerPix_ = bitsPerPix;
		pBuffRGB_ = new uint8_t[width * height * 3];
		pBuffRaw_ = new uint8_t[width * height * bytesPerPix];
		buffRawSize_ = width * height * bytesPerPix;
	}

	void display_blocking(uint8_t *p_rawImage, uint32_t buffLength) const
	{
		memcpy(pBuffRaw_, p_rawImage, buffLength);

		////test display from this thread.
		//util_image::gammaCorrection(rawBuff, rawBuff, 1280, 720, 12, 1.6);

		util_image::raw_to_rgb(pBuffRaw_, 0, pBuffRGB_, 0, iWidth_*iHeight_, bitsPerPix_);

		cv::Mat image(iHeight_, iWidth_, CV_8UC3, pBuffRGB_);


		cv::imshow("Debug blocking display", image);
		cv::waitKey(1);
	}



};
