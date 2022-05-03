using System;

namespace hidapi
{
    public struct DeviceID : IEquatable<DeviceID>
    {
        /** Device Vendor ID */
        public ushort vendor_id;

        /** Device Product ID */
        public ushort product_id;

        public DeviceID(ushort vid, ushort pid)
        {
            this.vendor_id = vid;
            this.product_id = pid;
        }

        public bool Equals(DeviceID other)
        {
            return (this.vendor_id == other.vendor_id
                && this.product_id == other.product_id);
        }

        public override bool Equals(Object obj)
        {
            if (obj == null)
                return false;

            if (!(obj is DeviceID))
            {
                return false;
            }

            DeviceID other = (DeviceID)obj;
            return Equals(other);
        }

        public override int GetHashCode()
        {
            var str = String.Format("{0:X4} {1:X4}",
                            this.vendor_id,
                            this.product_id);
            return str.GetHashCode();
        }

        public static bool operator ==(DeviceID id1, DeviceID id2)
        {
            return id1.Equals(id2);
        }

        public static bool operator !=(DeviceID id1, DeviceID id2)
        {
            return !(id1.Equals(id2));
        }
    }


}
