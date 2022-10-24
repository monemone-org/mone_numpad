#include "framework.h"
#include "HIDDevice.h"
#include "LastError.h"
#include <limits>


void hid_on_disconnected_callback(hid_device* dev, void* user_data)
{
	HIDDevice* hidDev = (HIDDevice*)user_data;
	hidDev->OnDisconnectedCallback();
}

void hid_on_read_callback(hid_device* dev, unsigned char* data, size_t cbData, void* user_data)
{
	HIDDevice* hidDev = (HIDDevice*)user_data;
	hidDev->OnReadCallback(data, cbData);
}

void hid_on_read_failure_callback(hid_device* dev, const wchar_t* pszMessage, void* user_data)
{
	HIDDevice* hidDev = (HIDDevice*)user_data;
	hidDev->OnReadFailureCallback(pszMessage);
}

void HIDDevice::Open(unsigned short vendor_id, unsigned short product_id, const wchar_t* serial_number) throw (HRESULT)
{
	if (m_dev != NULL)
	{
		THROW_E_FAIL( TEXT("A device is already opened; It must be closed first.") );
	}

	m_dev = hid_open(vendor_id, product_id, serial_number);
	//if (m_dev != NULL)
	//    hid_set_nonblocking(m_dev, 1);
	m_id = HIDDeviceID(vendor_id, product_id);

	RegisterCallbacks();
}

void HIDDevice::Open(hid_device_info* dev_info) throw (HRESULT)
{
	if (m_dev != NULL)
	{
		THROW_E_FAIL(TEXT("A device is already opened; It must be closed first."));
	}

	m_dev = hid_open_path(dev_info->path);
	//if (m_dev != NULL)
	//    hid_set_nonblocking(m_dev, 1);
	m_id = HIDDeviceID(dev_info->vendor_id, dev_info->product_id);

	RegisterCallbacks();
}

void HIDDevice::Close()
{
	AssertValidDev();

	hid_close(m_dev);
	m_dev = NULL;

	//StopReadLoop();
}



void HIDDevice::SetNonBlocking(bool nonblock) throw (HRESULT)
{
	int ret = hid_set_nonblocking(m_dev, (nonblock ? (int)1 : (int)0));
	if (ret < 0)
	{
		THROW_E_FAIL(TEXT("Failed to set non blocking."));
	}
}

void HIDDevice::AssertValidDev() const throw (HRESULT)
{
	if (m_dev == NULL)
	{
		THROW_E_FAIL(TEXT("No device opened"));
	}
}

void HIDDevice::RegisterCallbacks()
{
	on_read_callback_entry on_read = {
		.user_data = this,
		.on_read = hid_on_read_callback,
		.on_read_failure = hid_on_read_failure_callback
	};
	on_disconnected_callback_entry on_disconnect = {
		.user_data = this,
		.on_disconnected = hid_on_disconnected_callback
	};

	hid_register_read_callback(m_dev, on_read);
	hid_register_disconnected_callback(m_dev, on_disconnect);
}


void HIDDevice::OnDisconnectedCallback()
{
	if (m_listener)
	{
		m_listener->DeviceDisconnected(this);
	}
}

void HIDDevice::OnReadCallback(unsigned char* data, size_t cbData)
{
	if (m_listener)
	{
		m_listener->DeviceDataReceived(this, data, cbData);
	}
}

void HIDDevice::OnReadFailureCallback(const wchar_t* pszMessage)
{
	if (m_listener)
	{
		m_listener->DeviceDataReadFailure(this, pszMessage);
	}

}

std::wstring HIDDevice::GetProductString() const throw (HRESULT)
{
    AssertValidDev();
	wchar_t buf[250] = { 0 };
	size_t maxLen = sizeof(buf) / sizeof(buf[0]) - 1;
    //buf is wchar_t *, UTF32, 4 bytes
    int ret = hid_get_product_string(m_dev, buf, maxLen);
	if (ret < 0)
	{
		THROW_E_FAIL(TEXT("failed to receive product string"));
	}
	return std::wstring(buf);
}

std::wstring HIDDevice::GetManufacturerString() const throw (HRESULT)
{
    AssertValidDev();
	wchar_t buf[250] = { 0 };
	size_t maxLen = sizeof(buf) / sizeof(buf[0]) - 1;
	int ret = hid_get_manufacturer_string(m_dev, buf, maxLen);
    if (ret < 0)
	{
		THROW_E_FAIL(TEXT("failed to receive manufacturer string"));
	}
	return std::wstring(buf);
}

std::wstring HIDDevice::GetSerialNumberString() const throw (HRESULT)
{
	AssertValidDev();
	wchar_t buf[250] = { 0 };
	size_t maxLen = sizeof(buf) / sizeof(buf[0]) - 1;
	int ret = hid_get_serial_number_string(m_dev, buf, maxLen);
	if (ret < 0)
	{
		THROW_E_FAIL(TEXT("failed to receive serial number string"));
	}
	return std::wstring(buf);
}


int HIDDevice::GetMaxReportLength() const
{
	return hid_get_max_report_length(m_dev);
}

int HIDDevice::Write(unsigned char *data, size_t cbData) throw (HRESULT)
{
    AssertValidDev();

    //Console.WriteLine("HidAPI.hid_write(size={0})", HID_MAX_PACKET_SIZE);
    int ret = hid_write(m_dev, data, cbData);
    //if (ret < 0)
    //    Custom logging
    return ret;
}

std::wstring HIDDevice::Error() const
{
    AssertValidDev();
    std::wstring errMsg = hid_error(m_dev);
	return errMsg;
}

