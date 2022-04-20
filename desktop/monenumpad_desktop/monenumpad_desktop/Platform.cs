using System;

namespace monenumpad_desktop
{
    public static class Platform
    {
        public static readonly int POINTER_SIZE;

        public static readonly bool BigEndian;

        public static readonly bool Is64Bit;

        static Platform()
        {
            POINTER_SIZE = System.Environment.Is64BitOperatingSystem ? 64 : 32;
            Is64Bit = System.Environment.Is64BitOperatingSystem;
            BigEndian = !BitConverter.IsLittleEndian;
        }
    }

}

