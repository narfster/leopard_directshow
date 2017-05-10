#pragma once
#include <cstdint>

class leopard_cam_ext
{
public:

	bool set_exposure(uint32_t exposureVal);
	uint32_t get_exposure();
	bool set_gain(uint32_t gain);
	long get_gain();
	bool set_trigger(bool isEnabled);
	int set_trigger_delay_time_zero();

};

