#include "util_image.h"
#include <cmath>
#include <cstdint>


	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	//Tested with 12bit, and 10 bit pixel depth.
	//fucntion will not work for pixel depth < 8 
    void util_image::raw_to_rgb (void* inBuff, int inBuffSize, void* outBuff, int outBuffSize, uint32_t numOfPixels, int bitPerPixel)
	{
		ushort temp;
		auto dst = (uchar *)outBuff;

		int shift = bitPerPixel - 8;  //i.e. 10bit - 8bit(1 byte) = 2, 12bit - 8bit = 4

		ushort* tmp = (ushort *)inBuff;
		uchar c = 0;
		for (int i = 0; i < numOfPixels; i++)
		{
			temp = (*tmp++) >> shift; //12 bit shift 4, 10bit shift 2
			*dst++ = (uchar)temp;
			*dst++ = (uchar)temp;
			*dst++ = (uchar)temp;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	////////////// GAMMA_CORRECTION ///////////////////////
	//unsigned short util_image::linear_to_gamma[65536];
	//double util_image::gammaValue = -1;
	//int util_image::gBPP = 0;

	// create gamma table
    void util_image::initGammaTable(double gamma, int bpp)
	{
		int result;
		double dMax;
		int iMax;

		if (bpp > 12)
			return;

		dMax = pow(2, (double)bpp);
		iMax = (int)dMax;

		for (int i = 0; i < iMax; i++) {
			result = (int)(pow((double)i / dMax, 1.0 / gamma)*dMax);

			linear_to_gamma[i] = result;
		}

		gammaValue = gamma;
		gBPP = bpp;
	}



	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

    void util_image::gammaCorrection(uint8_t* in_bytes, uint8_t* out_bytes, int width, int height, int bpp, double gamma)
	{
		int i;
		uint16_t *srcShort;
		uint16_t *dstShort;

		if (gamma != gammaValue || gBPP != bpp)
			initGammaTable(gamma, bpp);

		if (bpp > 8)
		{
			srcShort = reinterpret_cast<uint16_t *>(in_bytes);
			dstShort = reinterpret_cast<uint16_t *>(out_bytes);
			for (i = 0; i < width*height; i++)
				*dstShort++ = linear_to_gamma[*srcShort++];
		}
		else
		{
			for (i = 0; i < width*height; i++)
				*out_bytes++ = static_cast<uint8_t>(linear_to_gamma[*in_bytes++]);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////


