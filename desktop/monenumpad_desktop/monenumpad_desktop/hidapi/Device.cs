using System;
using System.IO;
using System.Runtime.InteropServices;
using monenumpad_desktop.Marshal;

namespace hidapi
{
    public class Device
    {
        private int HID_MAX_PACKET_SIZE => GetMaxReportLength();

        #region Interop
        protected IntPtr _device;

        private Object _locker = new object();

        public DeviceID? id { get; private set; }

        public bool IsOpen()
        {
            return _device != IntPtr.Zero;
        }

        public void Open(ushort vid, ushort pid, string serial)
        {
            if (_device != IntPtr.Zero)
                throw new Exception("a device is already opened; close it first.");

            IntPtr ret = HidAPI.hid_open(vid, pid, serial);
            _device = ret;
            //if (_device != IntPtr.Zero)
            //    hid_set_nonblocking(_device, 1);

            this.id = new DeviceID(vid, pid);

            HidAPI.hid_register_read_callback(_device, hid_on_read_callback);
        }

        public void Open(hid_device_info dev)
        {
            if (_device != IntPtr.Zero)
                throw new Exception("a device is already opened; close it first.");

            IntPtr ret = HidAPI.hid_open_path(dev.path);
            _device = ret;
            //if (_device != IntPtr.Zero)
            //    hid_set_nonblocking(_device, 1);

            this.id = dev.device_id;

            HidAPI.hid_register_read_callback(_device, hid_on_read_callback);
            HidAPI.hid_register_disconnected_callback(_device, hid_on_disconnected_callback);
        }

        void hid_on_read_callback(IntPtr device, IntPtr data, uint length)
        {
            var bytes = BitVector.readPtrBytes(data, (int)length);
            this.DataReceived(this, new DataEventArgs(bytes));
        }

        void hid_on_disconnected_callback(IntPtr device)
        {
            this.Disconnected(this, new EventArgs());
        }

        public void SetNonBlocking(bool nonblock)
        {
            int ret = HidAPI.hid_set_nonblocking(_device, (nonblock ? (int)1 : (int)0));
            if (ret < 0)
                throw new Exception("Failed to set non blocking.");
        }

        // non-blocking read
        // return bytes_read
        //public int Read(byte[] buffer, int length)
        //{
        //    //Mone: this method doesn't modify state, we shouldn't need to lock
        //    lock (_locker)
        //    {
        //        AssertValidDev();
        //        int ret = HidAPI.hid_read_timeout(_device, buffer, (uint)length, 0/*1*/);
        //        //if (ret < 0)
        //        //    throw new Exception("Failed to Read.");

        //        return ret;
        //    }
        //}

        //public int ReadBlocking(byte[] buffer, int length)
        //{
        //    //Mone: this method doesn't modify state, we shouldn't need to lock
        //    //lock (_locker)
        //    //{
        //    AssertValidDev();
        //    int ret = HidAPI.hid_read_timeout(_device, buffer, (uint)length, -1);
        //    //if (ret < 0)
        //    //    throw new Exception("Failed to ReadBlocking.");

        //    return ret;
        //    //}
        //}

        public void Close()
        {
            AssertValidDev();
            HidAPI.hid_close(_device);
            _device = IntPtr.Zero;

            foreach (Delegate d in this.DataReceived.GetInvocationList())
            {
                this.DataReceived -= (EventHandler<DataEventArgs>)d;
            }

            foreach (Delegate d in this.Disconnected.GetInvocationList())
            {
                this.Disconnected -= (EventHandler)d;
            }

            //StopReadLoop();
        }

        public int ExitHidAPI()
        {
            return HidAPI.hid_exit();
        }

        public String GetProductString()
        {
            AssertValidDev();
            byte[] buf = new byte[1000];
            //buf is wchar_t *, UTF32, 4 bytes
            int ret = HidAPI.hid_get_product_string(_device, buf, (uint)(buf.Length / 4) - 1);
            if (ret < 0)
                throw new Exception("failed to receive product string");
            return EncodeBuffer(buf);
        }

        public String GetManufacturerString()
        {
            AssertValidDev();
            byte[] buf = new byte[1000];
            // buf is wchar_t *, UTF32, 4 bytes
            int ret = HidAPI.hid_get_manufacturer_string(_device, buf, (uint)(buf.Length / 4) - 1);
            if (ret < 0)
                throw new Exception("failed to receive manufacturer string");
            return EncodeBuffer(buf);
        }

        public int GetFeatureReport(byte[] buffer, int length)
        {
            AssertValidDev();
            int ret = HidAPI.hid_get_feature_report(_device, buffer, (uint)length);
            //if (ret < 0)
            //    throw new Exception("failed to get feature report");
            return ret;
        }

        public int SendFeatureReport(byte[] buffer, int length)
        {
            int ret = HidAPI.hid_send_feature_report(_device, buffer, (uint)length);
            //if (ret < 0)
            //    throw new Exception("failed to send feature report");
            return ret;
        }

        

        public int Write(byte[] buffer, int length)
        {
            lock (_locker)
            {
                AssertValidDev();
                //uint size = (uint)Math.Min(length, HID_MAX_PACKET_SIZE + 1);

                byte[] writeBuf = new byte[HID_MAX_PACKET_SIZE];
                buffer.CopyTo(writeBuf, 0);

                //Console.WriteLine("HidAPI.hid_write(size={0})", HID_MAX_PACKET_SIZE);
                int ret = HidAPI.hid_write(_device, writeBuf, (uint)HID_MAX_PACKET_SIZE);
                //if (ret < 0)
                //    Custom logging
                return ret;
            }
        }

        public String Error()
        {
            AssertValidDev();
            IntPtr ret = HidAPI.hid_error(_device);
            return Marshal.PtrToStringAuto(ret);
        }

        public int GetMaxReportLength()
        {
            return HidAPI.hid_get_max_report_length(_device);
        }

        public string GetIndexedString(int index)
        {
            AssertValidDev();
            byte[] buf = new byte[1000];
            // buf is wchar_t *, UTF32, 4 bytes
            int ret = HidAPI.hid_get_indexed_string(_device, index, buf, (uint)(buf.Length / 4) - 1);
            if (ret < 0)
                throw new Exception("failed to receive indexed string");
            return EncodeBuffer(buf);
        }

        public string GetSerialNumberString()
        {
            AssertValidDev();
            byte[] buf = new byte[1000];
            // buf is wchar_t *, UTF32, 4 bytes
            int ret = HidAPI.hid_get_serial_number_string(_device, buf, (uint)(buf.Length / 4) - 1);
            if (ret < 0)
                throw new Exception("failed to receive serial number string");
            return EncodeBuffer(buf);
        }

        private string EncodeBuffer(byte[] buffer)
        {
            //return Encoding.UTF32.GetString(buffer).Trim('\0');
            return FieldMarshalFunc.BytesToStringWChar_t(buffer);
        }

        private void AssertValidDev()
        {
            if (_device == IntPtr.Zero) throw new Exception("No device opened");
        }



        #endregion Interop

        //#region Constructors
        //public static Device GetDevice(ushort vid, ushort pid)
        //{
        //    try
        //    {
        //        Device layer = new Device();
        //        layer.Open(vid, pid, null);
        //        return layer._device == IntPtr.Zero ? null : layer;
        //    }
        //    catch (System.BadImageFormatException)
        //    {
        //        //Custom logging
        //        return null;
        //    }
        //    catch (Exception)
        //    {
        //        //Custom logging
        //        return null;
        //    }
        //}

        //#endregion Constructors

        #region ICommunicationLayer

        //public void StartReadLoop()
        //{
        //    try
        //    {
        //        if (IsOpen())
        //        {
        //            ContinueReadProcessing = true;
        //            ReadThread = new Thread(new ThreadStart(ReadLoop));
        //            ReadThread.Name = "HidApiReadThread";
        //            ReadThread.Start();
        //        }
        //        else
        //        {
        //            StopReadLoop();
        //        }
        //    }
        //    catch (Exception)
        //    {
        //        //Custom logging
        //        throw;
        //    }
        //}

        //public bool SendData(byte[] data)
        //{
        //    try
        //    {
        //        MemoryStream stream = new MemoryStream(HID_MAX_PACKET_SIZE + 1);
        //        stream.WriteByte(0);
        //        stream.Write(data, 0, HID_MAX_PACKET_SIZE);

        //        var dataToWrite = stream.ToArray();
        //        int ret = Write(dataToWrite, dataToWrite.Length);
        //        if (ret >= 0)
        //            return true;
        //        else
        //        {
        //            return false;
        //        }
        //    }
        //    catch (Exception)
        //    {
        //        //Custom logging
        //        return false;
        //    }
        //}

        public event EventHandler<DataEventArgs> DataReceived;

        public event EventHandler Disconnected;

        //public void Start()
        //{
        //    ContinueReadProcessing = true;
        //}

        //public void Stop()
        //{
        //    StopReadLoop();
        //}

        #endregion ICommunicationLayer

        //private Thread ReadThread = null;
        //protected volatile bool ContinueReadProcessing = true;

        //private void ReadLoop()
        //{
        //    var culture = CultureInfo.InvariantCulture;
        //    Thread.CurrentThread.CurrentCulture = culture;
        //    Thread.CurrentThread.CurrentUICulture = culture;
        //    Thread.CurrentThread.Priority = ThreadPriority.AboveNormal;

        //    while (ContinueReadProcessing)
        //    {
        //        try
        //        {
        //            byte[] report = new byte[HID_MAX_PACKET_SIZE];

        //            //Mone: changed to use Readblocking so we don't waste CPU cycle.
        //            var result = ReadBlocking(report, HID_MAX_PACKET_SIZE);

        //            if (result > 0)
        //            {
        //                DataReceived(this, new DataEventArgs(report));
        //            }
        //            else if (result < 0)
        //            {
        //                StopReadLoop();
        //            }
        //        }
        //        catch (Exception)
        //        {
        //            StopReadLoop();
        //        }

        //        Thread.Sleep(100);
        //    }
        //}

        //private void StopReadLoop()
        //{
        //    ContinueReadProcessing = false;
        //    Disconnected(this, EventArgs.Empty);
        //}

        #region IDisposable Members

        public void Dispose()
        {
//            ContinueReadProcessing = false;
//            ReadThread.Join(500);
//            if (ReadThread.IsAlive)
//            {
//#pragma warning disable SYSLIB0006 // Type or member is obsolete
//                ReadThread.Abort();
//#pragma warning restore SYSLIB0006 // Type or member is obsolete
//            }

            if (IsOpen())
                Close();
            int res = ExitHidAPI();
        }

        #endregion IDisposable Members
    }

}
