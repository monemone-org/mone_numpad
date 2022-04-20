using System;
using System.Runtime.InteropServices;
using System.Threading;
using hidapi;

namespace monenumpad_desktop
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Hello World!");


            //	struct hid_device_info *devs, *cur_dev;

            var version_str = HidAPI.GetVersionStr();
            Console.WriteLine("hidapi test/example tool. Compiled with hidapi version {0}, runtime version {1}.\n",
                Constants.HID_API_VERSION_STR, version_str);

            var hid_version = HidAPI.GetVersion();
            if (hid_version.major == Constants.HID_API_VERSION_MAJOR
                        && hid_version.minor == Constants.HID_API_VERSION_MINOR
                        && hid_version.patch == Constants.HID_API_VERSION_PATCH)
            {
                Console.WriteLine("Compile-time version matches runtime version of hidapi.\n\n");
            }
            else
            {
                Console.WriteLine("Compile-time version is different than runtime version of hidapi.\n\n");
            }


            if (HidAPI.hid_init() != 0)
            {
                Console.WriteLine("hid_init() failed.\n\n");
                return;
            }

            IntPtr devs = HidAPI.hid_enumerate(0x0, 0x0);
            
            IntPtr cur_dev_ptr = devs;
            while (cur_dev_ptr != IntPtr.Zero)
            {
                var cur_dev = new hid_device_info(cur_dev_ptr);
                Console.WriteLine("Device Found\n  vid: 0x{0:X4} pid: 0x{1:X4}\n  path: {2}\n  serial_number: {3}",
                    cur_dev.vendor_id, cur_dev.product_id, cur_dev.path, cur_dev.serial_number);
                Console.WriteLine("  Manufacturer: {0}", cur_dev.manufacturer_string);
                Console.WriteLine("  Product:      {0}", cur_dev.product_string);
                Console.WriteLine("  Release:      0x{0:X4}", cur_dev.release_number);
                Console.WriteLine("  Interface:    {0}", cur_dev.interface_number);
                Console.WriteLine("  Usage (page): 0x{0:X4} (0x{1:X4})", cur_dev.usage, cur_dev.usage_page);
                Console.WriteLine("\n");
                cur_dev_ptr = cur_dev.next;
            }
            HidAPI.hid_free_enumeration(devs);

            var hidDevice = new Device();


            int res = 0;
            byte[] buf = new byte[256];
            //#define MAX_STR 255
            //			wchar_t wstr[MAX_STR];
            //			hid_device* handle;
            //			int i;

            // Set up the command buffer.
            //memset(buf, 0x00, sizeof(buf));
            Array.Clear(buf, 0, buf.Length);
            buf[0] = 0x01;
            buf[1] = 0x81;

            // Open the device using the VID, PID,
            // and optionally the Serial number.
            ////handle = hid_open(0x4d8, 0x3f, L"12345");
            //if (handle == 0)

            //handle = hid_open(0x4d8, 0x3f, NULL);
            //hidapi.Open(0x4d8, 0x3f, null);

            const int monenumpad_vid = 0x05ac;
            const int monenumpad_pid = 0x029c;

            hidDevice.Open(monenumpad_vid, monenumpad_pid, null);
            if (! hidDevice.IsOpen())
            {
                Console.WriteLine("Unable to open device\n");
                return;
            }

            // Read the Manufacturer String
            //wstr[0] = 0x0000;
            //res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
            //if (res < 0)
            //{
            //    Console.WriteLine("Unable to read manufacturer string\n");
            //}
            try
            {
                var wstr = hidDevice.GetManufacturerString();
                Console.WriteLine("Manufacturer String: {0}\n", wstr);
            }
            catch (Exception e)
            {
                Console.WriteLine("Unable to read manufacturer string. e={0}\n", e.Message);
            }
            

            // Read the Product String
            //wstr[0] = 0x0000;
            //res = hid_get_product_string(handle, wstr, MAX_STR);
            //if (res < 0)
            //{
            //    Console.WriteLine("Unable to read product string\n");
            //}
            //Console.WriteLine("Product String: %ls\n", wstr);
            try
            {
                var wstr = hidDevice.GetProductString();
                Console.WriteLine("Product String: {0}\n", wstr);
            }
            catch (Exception e)
            {
                Console.WriteLine("Unable to read product string. e={0]\n", e.Message);
            }


            // Read the Serial Number String
            //wstr[0] = 0x0000;
            //res = hid_get_serial_number_string(handle, wstr, MAX_STR);
            //if (res < 0)
            //{
            //    Console.WriteLine("Unable to read serial number string\n");
            //}
            //Console.WriteLine("Serial Number String: (%d) %ls\n", wstr[0], wstr);
            try
            {
                var wstr = hidDevice.GetSerialNumberString();
                Console.WriteLine("Serial Number String: {0}\n", wstr);
            }
            catch (Exception e)
            {
                Console.WriteLine("Unable to read serial number string. e={0}\n", e.Message);
            }


            // Read Indexed String 1
            //wstr[0] = 0x0000;
            //res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
            //if (res < 0)
            //{
            //    Console.WriteLine("Unable to read indexed string 1\n");
            //}
            //Console.WriteLine("Indexed String 1: %ls\n", wstr);
            try
            {
                var wstr = hidDevice.GetIndexedString(1);
                Console.WriteLine("Indexed String 1:: {0}\n", wstr);
            }
            catch (Exception e)
            {
                Console.WriteLine("Unable to read indexed string. e={0}\n", e.Message);
            }

            // Set the hid_read() function to be non-blocking.
            hidDevice.SetNonBlocking(true);

            // Try to read from the device. There should be no
            // data here, but execution should not block.
            //res = hid_read(handle, buf, 17);
            res = hidDevice.Read(buf, 17);

            // Send a Feature Report to the device
            buf[0] = 0x2;
            buf[1] = 0xa0;
            buf[2] = 0x0a;
            buf[3] = 0x00;
            buf[4] = 0x00;
            //res = hid_send_feature_report(handle, buf, 17);
            //if (res < 0)
            //{
            //    Console.WriteLine("Unable to send a feature report.\n");
            //}
            res = hidDevice.SendFeatureReport(buf, 17);
            if (res < 0)
            {
                Console.WriteLine("Unable to send a feature report. e={0}\n", hidDevice.Error());
            }

            //memset(buf, 0, sizeof(buf));
            Array.Clear(buf, 0, buf.Length);

            // Read a Feature Report from the device
            buf[0] = 0x2;
            //res = hid_get_feature_report(handle, buf, sizeof(buf));
            //if (res < 0)
            //{
            //    Console.WriteLine("Unable to get a feature report.\n");
            //    Console.WriteLine("%ls", hid_error(handle));
            //}
            //else
            //{
            //    // Print out the returned buffer.
            //    Console.WriteLine("Feature Report\n   ");
            //    for (i = 0; i < res; i++)
            //    {
            //        Console.WriteLine("%02hhx ", buf[i]);
            //    }
            //    Console.WriteLine("\n");
            //}
            res = hidDevice.GetFeatureReport(buf, buf.Length);
            if (res < 0)
            {
                Console.WriteLine("Unable to get a feature report\n");
                Console.WriteLine("{0}", hidDevice.Error());
            }
            else
            {
                // Print out the returned buffer.
                Console.WriteLine("Feature Report\n   ");
                for (int i = 0; i < res; i++)
                {
                    Console.Write("{0:X2} ", buf[i]);
                }
                Console.WriteLine("\n");
            }


            //memset(buf, 0, sizeof(buf));
            Array.Clear(buf, 0, buf.Length);

            // Toggle LED (cmd 0x80). The first byte is the report number (0x1).
            buf[0] = 0x1;
            buf[1] = 0x80;
            //res = hid_write(handle, buf, 17);
            //if (res < 0)
            //{
            //    Console.WriteLine("Unable to write()\n");
            //    Console.WriteLine("Error: %ls\n", hid_error(handle));
            //}
            res = hidDevice.Write(buf, 17);
            if (res < 0)
            {
                Console.WriteLine("Unable to write()\n");
                Console.WriteLine("{0}", hidDevice.Error());
            }


            // Request state (cmd 0x81). The first byte is the report number (0x1).
            buf[0] = 0x1;
            buf[1] = 0x81;

            res = hidDevice.Write(buf, 17);
            if (res < 0)
            {
                Console.WriteLine("Unable to write() (2)\n");
                Console.WriteLine("{0}", hidDevice.Error());
            }

            // Read requested state. hid_read() has been set to be
            // non-blocking by the call to hid_set_nonblocking() above.
            // This loop demonstrates the non-blocking nature of hid_read().
            res = 0;
            while (res == 0)
            {
                res = hidDevice.Read(buf, buf.Length);
                if (res == 0)
                {
                    Console.WriteLine("waiting...\n");
                }
                else if (res < 0)
                {
                    Console.WriteLine("Unable to read()\n");
                }
                Thread.Sleep(500);
            }

            Console.WriteLine("Data read:\n   ");
            // Print out the returned buffer.
            for (int i = 0; i < res; i++)
            {
                Console.WriteLine("{0:X2} ", buf[i]);
            }
            Console.WriteLine("\n");

            hidDevice.Close();
            //HidApiInteropCommLayer.hid_close(handle);

            /* Free static HIDAPI objects. */
            hidDevice.ExitHidAPI();
            //HidApiInteropCommLayer.hid_exit();

            Console.WriteLine("Press a key to exit\n");
            Console.ReadKey();

            return;
        }
    }
}

