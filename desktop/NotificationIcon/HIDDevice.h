#pragma once

#include "hidapi.h"
#include "LastError.h"
#include "HIDDeviceID.h"

void hid_on_disconnected_callback(hid_device* dev, void* user_data);
void hid_on_read_callback(hid_device* dev, unsigned char* data, size_t cbData, void* user_data);

class HIDDevice;

class HIDDeviceListener
{
public:
	virtual void DeviceDataReceived(HIDDevice* dev, BYTE* data, size_t cbData);
	virtual void DeviceDisconnected(HIDDevice* dev);
};

class HIDDevice
{
protected:
	hid_device* m_dev;
	HIDDeviceID m_id;
	HIDDeviceListener* m_listener;
	
public:
	HIDDevice():
		m_dev(NULL),
		m_id(),
		m_listener(NULL)
	{
	}

	~HIDDevice()
	{
		if (m_dev)
		{
			Close();
		}
	}

	const HIDDeviceID& GetID() const {
		return m_id;
	}

	bool IsSameDevice(hid_device* dev) const {
		return m_dev == dev;
	}

	bool IsOpen() const
	{
		return m_dev != NULL;
	}

	void Open(unsigned short vendor_id, unsigned short product_id, const wchar_t* serial_number) throw (HRESULT);
	void Open(hid_device_info* dev_info) throw (HRESULT);

	void Close();
	
	void SetNonBlocking(bool nonblock) throw (HRESULT);

    std::wstring GetProductString() const throw (HRESULT);
    std::wstring GetManufacturerString() const throw (HRESULT);
	std::wstring GetSerialNumberString() const throw (HRESULT);
	int GetMaxReportLength() const;

    int Write(unsigned char* data, size_t cbData) throw (HRESULT);

	void SetListener(HIDDeviceListener* listener) {
		m_listener = listener;
	}

    std::wstring Error() const;

protected:
	void RegisterCallbacks();

public:
	void AssertValidDev() const throw (HRESULT);

	void OnDisconnectedCallback();
	void OnReadCallback(unsigned char* data, size_t cbData);

};