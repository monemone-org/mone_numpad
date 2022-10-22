#pragma once

#include <map>

struct HIDDeviceID
{
	unsigned short vendor_id;
	unsigned short product_id;

	HIDDeviceID():
		vendor_id(0),
		product_id(0)
	{
	}

	HIDDeviceID(
		unsigned short vendor_id_,
		unsigned short product_id_
	):
		vendor_id(vendor_id_),
		product_id(product_id_)
	{
	}


	friend bool operator<(const HIDDeviceID& l, const HIDDeviceID& r)
	{
		return std::tie(l.vendor_id, l.product_id)
			< std::tie(r.vendor_id, r.product_id);
	}

	friend bool operator==(const HIDDeviceID& l, const HIDDeviceID& r)
	{
		return std::tie(l.vendor_id, l.product_id)
			== std::tie(r.vendor_id, r.product_id);
	}
};

