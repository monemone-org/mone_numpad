using System;
using System.Collections.Generic;
using hidapi;
using System.Windows;

namespace monenumpad_desktop.Maxmix
{
    public interface HIDDeviceMonitorDelegate
    {
        void deviceFound(hidapi.hid_device_info device);
        //void deviceGone(hidapi.hid_device_info device);
    }

    public struct HIDMonitorData
    {
        public int vendorId;
        public int productId;
        public int usagePage;
        public int usage;
    };

    public interface IHIDDeviceMonitor
    {
        public void addDelegate(HIDDeviceMonitorDelegate dlgte);
        public void removeDelegate(HIDDeviceMonitorDelegate dlgte);

        public bool isRunning { get; }
        public void start();
        public void stop();
    }

    public class HIDDeviceMonitor: IHIDDeviceMonitor
    {
        private HIDMonitorData monitorData;
        private List<HIDDeviceMonitorDelegate> delgates_ = new List< HIDDeviceMonitorDelegate>();

        public HIDDeviceMonitor(HIDMonitorData monitorData)
        {
            this.monitorData = monitorData;
        }

        public void addDelegate(HIDDeviceMonitorDelegate dlgte)
        {
            this.delgates_.Add(dlgte);
        }

        public void removeDelegate(HIDDeviceMonitorDelegate dlgte)
        {
            this.delgates_.Remove(dlgte);
        }

        public bool isRunning => true;

        void hid_on_add_device(/*struct hid_device_info * */IntPtr dev_ptr)
        {
            var dev = new hid_device_info(dev_ptr);

            //TODO: marshal to notify on main-thread
            foreach (HIDDeviceMonitorDelegate dlgte in this.delgates_)
            {
                dlgte.deviceFound(dev);                  
            }
        }

        public void start()
        {
            HidAPI.hid_init();

            var devs = HidAPI.hid_enumerate_ex(
                (ushort)this.monitorData.vendorId,
                (ushort)this.monitorData.productId,
                (ushort)this.monitorData.usagePage,
                (ushort)this.monitorData.usage,
                this.hid_on_add_device);

            IntPtr cur_dev_ptr = devs;
            while (cur_dev_ptr != IntPtr.Zero)
            {
                var cur_dev = new hid_device_info(cur_dev_ptr);

                foreach (HIDDeviceMonitorDelegate dlgte in this.delgates_)
                {
                    dlgte.deviceFound(cur_dev);
                }

                cur_dev_ptr = cur_dev.next;
            }
            HidAPI.hid_free_enumeration(devs);

        }

        public void stop()
        {
            HidAPI.hid_exit();
        }

    }
}
