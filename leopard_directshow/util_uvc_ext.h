#pragma once
#include <amstream.h>





namespace util_uvc_ext
{
	int write_to_uvc_extension(IBaseFilter* camera, int property_id, BYTE* bytes, int length, ULONG* ulBytesReturned);
	int read_from_uvc_extension(IBaseFilter* camera, int property_id, BYTE* bytes, int length, ULONG* ulBytesReturned);
}
