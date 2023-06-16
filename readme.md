# mone_numpad

## Introduction
Mone_numpad is a handwired numpad with a volume dial that runs a custom VIAL/QMK firware. It can control the machine's input and output volume.

* the electronic design and schematic 
* The STL for the case
* The custom VIAL/QMK firmware
* A Windows desktop notification icon software that can help the numpad to control the followings:
    * default audio output volume control
    * default audio input volume control
    * volumne control for active applications with audio channels

[toc]

## Projects/

### keyboard-layout_editor/

It contains the json file for [keyboard-layout_editor](http://www.keyboard-layout-editor.com)

### schematics/

Mone_numpad's Wiring schematics

### stl/

Modified docukboard STL for mone_numpad

### vial layouts/

vial layout files

### vial-qmk/

A modified forted version of vial-qmk. It added supports for

1. Mac Fn key
2. Custom command prefix for mone_numpad
3. intercepting vial_encoder_update()


### vial-qmk-keyboards-monenumpad/

The firware code for mone_numpad.

See: `compile_monenumpad_vial.sh` for build steps.


### desktop/

Code for the desktop companion app for mone_numpad

#####  desktop/NotificationIcon

* NotificationIcon is the Win32 desktop companion app for mone_numpad.
* It provides the following functionalities:
	* displays as an notification icon on the taskbar.
	* provides volumn channel info to monenumpad:
 		1. default output
   		2. default input
   		3. active applications tha has volume channels

##### desktop/USBDeviceSwift/RaceflightControllerHIDExample
* A modified Swift sample app to help test monenumpad's customized vial/qmk firmware on Mac.

##### desktop/hidapi

A cutomized fork of hidapi (an open source project) with the following changes.

* 1 byte packing C struct. 
	* At one point, I was writing C# and Swift clients.  Changing the byte packing to 1 byte made it easier to marshel the C structure data to/from C# and Swift.

* added support to send notifications when a HID device is connected/disconnected.

	```C++
		
	typedef struct on_added_device_callback_entry {
		void (*on_added_device)(struct hid_device_info*, void* user_data);
		void* user_data;
	} on_added_device_callback_entry;

	struct hid_device_info HID_API_EXPORT * HID_API_CALL hid_enumerate_ex(
																		unsigned short vendor_id,
																		unsigned short product_id,
																		unsigned short usage_page,
																		unsigned short usage,
																		on_added_device_callback_entry on_added_device_callback);
	void HID_API_EXPORT HID_API_CALL hid_stop_enumerate_on_added_device_callback();

	typedef struct on_disconnected_callback_entry {
		void* user_data;
		void (*on_disconnected)(hid_device*, void* user_data);
	} on_disconnected_callback_entry;

	int HID_API_EXPORT hid_register_disconnected_callback(hid_device *dev, on_disconnected_callback_entry on_disconnected);
	void HID_API_EXPORT hid_unregister_disconnected_callback(hid_device *dev);


	int HID_API_EXPORT_CALL hid_get_max_report_length(hid_device *dev);

	```

* added support to send notifications when an input record is read from a HID device.
	```C++

		typedef struct on_read_callback_entry {
			void* user_data;
			void (*on_read)(hid_device*, unsigned char*, size_t, void* user_data);
			void (*on_read_failure)(hid_device*, const wchar_t* pszMessage, void* user_data);
		} on_read_callback_entry;

        int HID_API_EXPORT HID_API_CALL hid_register_read_callback(hid_device *dev, on_read_callback_entry on_read);
        void HID_API_EXPORT HID_API_CALL hid_unregister_read_callback(hid_device *dev);

	```
##### desktop/xcodeProj
* the xcodeproj for `desktop/hidapi` to helpe edit and build on Mac

##### desktop/monenumpad_desktop

The original C# POC test client app before I switched to develop `NotificationIcon`.  

My original plan was to create a C# desktop app to talk to monenum_pad via hidapi. I got the marshalling code working. However C# WinForm (or any C# Windows client framework) does not have easy support to add an icon to the taskbar's notification area.  The easiest way is still Win32. So I created `NotificationIcon` instead and abandoned this project.



	
