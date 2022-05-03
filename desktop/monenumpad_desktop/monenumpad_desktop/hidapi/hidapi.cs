
using System;
using System.Globalization;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using monenumpad_desktop;
using monenumpad_desktop.Marshal;


namespace hidapi
{
    public static class Constants
    {
        /** @brief Static/compile-time major version of the library.

            @ingroup API
        */
        public const int HID_API_VERSION_MAJOR = 0;

        /** @brief Static/compile-time minor version of the library.

            @ingroup API
        */
        public const int HID_API_VERSION_MINOR = 11;
        /** @brief Static/compile-time patch version of the library.

            @ingroup API
        */
        public const int HID_API_VERSION_PATCH = 2;

        public const string HID_API_VERSION_STR = "0.11.2";

    }

    public struct hid_api_version
    {
        enum FieldIndex
        {
            major,
            minor,
            patch
        };

        public int major;
        public int minor;
        public int patch;

        private readonly static PackedStructLayout<FieldIndex> Layout;

        static hid_api_version()
        {
            Layout = new PackedStructLayout<FieldIndex>(new FieldDesc[]{
                new FieldDesc("major", 32), //int
                new FieldDesc("minor", 32), //int
                new FieldDesc("patch", 32), //int
            });
        }

        public hid_api_version(BitVector bitVector)
        {
            //int
            this.major = Layout.GetFieldValue(bitVector, FieldIndex.major, FieldMarshalFunc.ToInt32);

            //int
            this.minor = Layout.GetFieldValue(bitVector, FieldIndex.minor, FieldMarshalFunc.ToInt32);

            //int
            this.patch = Layout.GetFieldValue(bitVector, FieldIndex.patch, FieldMarshalFunc.ToInt32);
        }


        public hid_api_version(IntPtr ptr)
            : this(new BitVector(ptr, Layout.StructSize, Layout.StructSize * 8))
        {
        }
    };


    /** hidapi info structure */
    public struct hid_device_info
    {
        enum FieldIndex
        {
            path,
            vendor_id,
            product_id,
            serial_number,
            release_number,
            manufacturer_string,
            product_string,
            usage_page,
            usage,
            interface_number,
            next
        };
        private readonly static PackedStructLayout<FieldIndex> Layout;

        static hid_device_info()
        {
            Layout = new PackedStructLayout<FieldIndex>(new FieldDesc[]{
                new FieldDesc("path", Platform.POINTER_SIZE), //char *
                new FieldDesc("vendor_id", 16), //unsigned short 
                new FieldDesc("product_id", 16), //unsigned short
                new FieldDesc("serial_number", Platform.POINTER_SIZE), //wchar_t *
                new FieldDesc("release_number", 16), //unsigned short
                new FieldDesc("manufacturer_string", Platform.POINTER_SIZE), //wchar_t *
                new FieldDesc("product_string", Platform.POINTER_SIZE), //wchar_t *
                new FieldDesc("usage_page", 16), // unsigned short
                new FieldDesc("usage", 16), // unsigned short
                new FieldDesc("interface_number", 32), //int
                new FieldDesc("next", Platform.POINTER_SIZE) //struct hid_device_info *
            });
        }

        public hid_device_info(BitVector bitVector)
        {
            //char *
            IntPtr pathPtr = Layout.GetFieldValue(bitVector, FieldIndex.path, FieldMarshalFunc.ToIntPtr);
            this.path = Marshal.PtrToStringAnsi(pathPtr) ?? "";

            //unsigned short
            this.vendor_id = Layout.GetFieldValue(bitVector, FieldIndex.vendor_id, FieldMarshalFunc.ToUInt16);

            //unsigned short
            this.product_id = Layout.GetFieldValue(bitVector, FieldIndex.product_id, FieldMarshalFunc.ToUInt16);

            //wchar_t *
            IntPtr serial_numberPtr = Layout.GetFieldValue(bitVector, FieldIndex.serial_number, FieldMarshalFunc.ToIntPtr);
            this.serial_number = FieldMarshalFunc.PtrToStringWChar_t(serial_numberPtr) ?? "";

            //unsigned short
            this.release_number = Layout.GetFieldValue(bitVector, FieldIndex.release_number, FieldMarshalFunc.ToUInt16);

            //wchar_t *
            IntPtr manufacturer_stringPtr = Layout.GetFieldValue(bitVector, FieldIndex.manufacturer_string, FieldMarshalFunc.ToIntPtr);
            this.manufacturer_string = FieldMarshalFunc.PtrToStringWChar_t(manufacturer_stringPtr) ?? "";

            //wchar_t *
            IntPtr product_stringPtr = Layout.GetFieldValue(bitVector, FieldIndex.product_string, FieldMarshalFunc.ToIntPtr);
            this.product_string = FieldMarshalFunc.PtrToStringWChar_t(product_stringPtr) ?? "";

            // unsigned short
            this.usage_page = Layout.GetFieldValue(bitVector, FieldIndex.usage_page, FieldMarshalFunc.ToUInt16);

            // unsigned short
            this.usage = Layout.GetFieldValue(bitVector, FieldIndex.usage, FieldMarshalFunc.ToUInt16);

            // int
            this.interface_number = Layout.GetFieldValue(bitVector, FieldIndex.interface_number, FieldMarshalFunc.ToInt32);

            // struct hid_device_info *
            this.next = Layout.GetFieldValue(bitVector, FieldIndex.next, FieldMarshalFunc.ToIntPtr);

            this.device_id = new DeviceID(this.vendor_id, this.product_id);
        }


        public hid_device_info(IntPtr ptr)
            :this(new BitVector(ptr, Layout.StructSize, Layout.StructSize * 8))
        {
        }

        // to help identify the Device instance
        public DeviceID device_id;

        /** Platform-specific device path */
        //char *
        public string path;

        /** Device Vendor ID */
        public ushort vendor_id;

        /** Device Product ID */
        public ushort product_id;

        /** Serial Number */
        //wchar_t*
        public string serial_number;

        /** Device Release Number in binary-coded decimal,
            also known as Device Version Number */
        public ushort release_number;

        /** Manufacturer String */
        //wchar_t*
        public string manufacturer_string;

        /** Product string */
        //wchar_t*
        public string product_string;

        /** Usage Page for this Device/Interface
            (Windows/Mac/hidraw only) */
        public ushort usage_page;

        /** Usage for this Device/Interface
            (Windows/Mac/hidraw only) */
        public ushort usage;

        /** The USB interface which this logical device
            represents.

            * Valid on both Linux implementations in all cases.
            * Valid on the Windows implementation only if the device
              contains more than one interface.
            * Valid on the Mac implementation if and only if the device
              is a USB HID device. */
        public Int32 interface_number;

        /** Pointer to the next device */
        //struct hid_device_info *;
        public IntPtr next;
    };

    internal class Utf32Marshaler : ICustomMarshaler
    {
        private static Utf32Marshaler instance = new Utf32Marshaler();

        public static ICustomMarshaler GetInstance(string s)
        {
            return instance;
        }

        public void CleanUpManagedData(object o)
        {
        }

        public void CleanUpNativeData(IntPtr pNativeData)
        {
            Marshal.FreeHGlobal(pNativeData);
            //UnixMarshal.FreeHeap(pNativeData);
        }

        public int GetNativeDataSize()
        {
            return IntPtr.Size;
        }

        public IntPtr MarshalManagedToNative(object obj)
        {
            string s = obj as string;
            if (s == null)
                return IntPtr.Zero;
            return Marshal.StringToHGlobalAuto(s);
        }

        public object MarshalNativeToManaged(IntPtr pNativeData)
        {
            return Marshal.PtrToStringAuto(pNativeData);
        }
    }

    public class DataEventArgs : EventArgs
    {
        public DataEventArgs(byte[] data)
        {
            Data = data;
        }

        public byte[] Data { get; private set; }
    }


    public class HidAPI
    {
#if WINDOWS
        private const string HIDAPI_DLL = "hidapi.dll";
#else
        private const string HIDAPI_DLL = "hidapi.dylib";
#endif

        public static string GetVersionStr()
        {
            var version_str_ptr = hid_version_str();
            var version_str = Marshal.PtrToStringAuto(version_str_ptr);
            return version_str;
        }

        public static hid_api_version GetVersion()
        {
            var version_ptr = hid_version();
            var version = new hid_api_version(version_ptr);
            return version;
        }

        #region DllImports

        // data is byte array
        public delegate void hid_on_read_callback(IntPtr device, IntPtr data, uint length);
        public delegate void hid_on_disconnected_callback(IntPtr device);

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_register_read_callback(
            IntPtr device,
            [MarshalAs(UnmanagedType.FunctionPtr)] hid_on_read_callback on_read_callback);

        [DllImport(HIDAPI_DLL)]
        public static extern void hid_unregister_read_callback(IntPtr device);

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_register_disconnected_callback(
            IntPtr device,
            [MarshalAs(UnmanagedType.FunctionPtr)] hid_on_disconnected_callback on_disconnected_callback);

        [DllImport(HIDAPI_DLL)]
        public static extern void hid_unregister_disconnected_callback(IntPtr device);


        [DllImport(HIDAPI_DLL)]
        public static extern int hid_read(IntPtr device, [Out, MarshalAs(UnmanagedType.LPArray)] byte[] data, uint length);

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_read_timeout(IntPtr device, [Out, MarshalAs(UnmanagedType.LPArray)] byte[] data, uint length, int timeout);

        [DllImport(HIDAPI_DLL)]
        public static extern IntPtr hid_open(ushort vid, ushort pid, [MarshalAs(UnmanagedType.LPWStr)] string serial);

        [DllImport(HIDAPI_DLL)]
        public static extern void hid_close(IntPtr device);

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_init();

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_exit();

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_get_product_string(IntPtr device, [Out] byte[] _string, uint length);

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_get_manufacturer_string(IntPtr device, [Out] byte[] _string, uint length);

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_get_feature_report(IntPtr device, [Out, MarshalAs(UnmanagedType.LPArray)] byte[] data, uint length);

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_get_serial_number_string(IntPtr device, [Out] byte[] serial, uint maxlen);

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_get_indexed_string(IntPtr device, int string_index, [Out] byte[] _string, uint maxlen);

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_get_max_report_length(IntPtr device);        

        [DllImport(HIDAPI_DLL)]
        public static extern IntPtr hid_error(IntPtr device);

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_send_feature_report(IntPtr device, [In, MarshalAs(UnmanagedType.LPArray)] byte[] data, uint length);

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_set_nonblocking(IntPtr device, int nonblocking);

        [DllImport(HIDAPI_DLL)]
        public static extern int hid_write(IntPtr device, [In, MarshalAs(UnmanagedType.LPArray)] byte[] data, uint length);

        [DllImport(HIDAPI_DLL)]
        public static extern IntPtr hid_open_path([In, MarshalAs(UnmanagedType.LPStr)] string path);

        // Mone added:
        [DllImport(HIDAPI_DLL, CharSet = CharSet.Ansi)]
        private static extern IntPtr hid_version_str();

        // returns hid_api_version*
        [DllImport(HIDAPI_DLL)]
        private static extern IntPtr hid_version();

        [DllImport(HIDAPI_DLL)]
        public static extern IntPtr hid_enumerate(ushort vendor_id, ushort product_id);

        /** @brief Free an enumeration Linked List

		    This function frees a linked list created by hid_enumerate().

			@ingroup API
		    @param devs Pointer to a list of struct_device returned from
		    	      hid_enumerate().
		*/
        [DllImport(HIDAPI_DLL)]
        public static extern void hid_free_enumeration(/*struct hid_device_info * */IntPtr devs);

        public delegate void hid_on_add_device_callback(/*struct hid_device_info * */IntPtr dev);

        [DllImport(HIDAPI_DLL)]
        public static extern IntPtr hid_enumerate_ex(ushort vendor_id,
                                                    ushort product_id,
                                                    ushort usage_page,
                                                    ushort usage,
                                                    [MarshalAs(UnmanagedType.FunctionPtr)] hid_on_add_device_callback on_added_device);

        #endregion DllImports
    }
}