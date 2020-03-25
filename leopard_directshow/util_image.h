#pragma once
//#include <opencv/opencv2/core/cvdef.h>
#include <cstdint>


namespace util_image
{

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	

	////////////// GAMMA_CORRECTION ///////////////////////
	static unsigned short linear_to_gamma[65536];
	static double gammaValue = -1;
	static int gBPP = 0;

    void raw_to_rgb(void* inBuff, int inBuffSize, void* outBuff, int outBuffSize, uint32_t numOfPixels, int bitPerPixel);

	// create gamma table
    void initGammaTable(double gamma, int bpp);

    void gammaCorrection(uint8_t* in_bytes, uint8_t* out_bytes, int width, int height, int bpp, double gamma);

}
