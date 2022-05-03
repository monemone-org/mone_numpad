using System;

#nullable enable

namespace monenumpad_desktop.Maxmix
{
    public static class ArrayExtensions
    {
        //public static T? First<T>(this T[] array) where T: struct
        //{
        //    if (array.Length == 0)
        //        return null;

        //    return array[0];
        //}

        //public static T? Last<T>(this T[] array) where T : struct
        //{
        //    if (array.Length == 0)
        //        return null;

        //    int lastIndex = array.Length - 1;
        //    return array[lastIndex];
        //}

        public static T? First<T>(this T[] array)
        {
            if (array.Length == 0)
                return default(T?);

            return array[0];
        }

        public static T? Last<T>(this T[] array) 
        {
            if (array.Length == 0)
                return default(T?);

            int lastIndex = array.Length - 1;
            return array[lastIndex];
        }

    }
}
