#pragma once
#include <opencv2/core/cvdef.h>
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

    void raw_to_rgb(const void* inBuff, int inBuffSize, const void* outBuff, int outBuffSize);

	// create gamma table
    void initGammaTable(double gamma, int bpp);

    void gammaCorrection(uint8_t* in_bytes, uint8_t* out_bytes, int width, int height, int bpp, double gamma);

}
