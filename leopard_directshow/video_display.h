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

	mutable double videoFramesPerSec_ = 0;

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

		//mesaure this function interval.
		static auto nbFrames = 0;
		static auto t = static_cast<double>(cv::getTickCount());

		nbFrames++;
		double elapsedTimeMs = ((static_cast<double>(cv::getTickCount()) - t) / cv::getTickFrequency()) * 1000;
		if (elapsedTimeMs >= 1000)
		{
			videoFramesPerSec_ = elapsedTimeMs / static_cast<double>(nbFrames);
			nbFrames = 0;
			t = static_cast<double>(cv::getTickCount());
		}



		memcpy(pBuffRaw_, p_rawImage, buffLength);

		////test display from this thread.
		//util_image::gammaCorrection(rawBuff, rawBuff, 1280, 720, 12, 1.6);

		util_image::raw_to_rgb(pBuffRaw_, 0, pBuffRGB_, 0, iWidth_*iHeight_, bitsPerPix_);

		cv::Mat image(iHeight_, iWidth_, CV_8UC3, pBuffRGB_);

		std::string str = "fps: " + std::to_string(1 / videoFramesPerSec_ * 1000);
		cv::putText(image, str, cvPoint(30, 450),
			cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6, cvScalar(255, 0, 00), 1, CV_AA);



		cv::imshow("Debug blocking display", image);
		cv::waitKey(1);
	}



};
