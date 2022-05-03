using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace monenumpad_desktop.Marshal
{
    public class FieldDesc
    {
        public string fieldName { get; }

        // last fieldIndex can have bitCount as 0.
        // 0 means the field value can last til the end of the BitVector.
        public int bitCount { get; } 

        public FieldDesc(string fieldName, int bitCount)
        {
            this.fieldName = fieldName;
            this.bitCount = bitCount;
        }
    }

    public class BitFieldDesc: FieldDesc
    {
        public BitFieldDesc(string fieldName, int bitCount)
            : base(fieldName, bitCount)
        {
        }
    }

    public class PackedStructLayout<FieldIndexEnum> where FieldIndexEnum: Enum
    {
        private FieldLayout[] fieldLayouts;

        public readonly int StructSize; //in num of bytes

        public PackedStructLayout(FieldDesc[] fieldDescArray)
        {
            var fieldInfos = new List<FieldLayout>();

            int nextFieldBitIndex = 0;
            int bitFieldCount = 0;
            foreach (FieldDesc fieldDesc in fieldDescArray)
            {
                // group all adjacent bitfields into the same byte.
                if (fieldDesc is BitFieldDesc)
                {
                    var fieldInfo = new FieldLayout(
                        fieldDesc.fieldName, nextFieldBitIndex + (7 - bitFieldCount), fieldDesc.bitCount);
                    fieldInfos.Add(fieldInfo);

                    bitFieldCount += fieldDesc.bitCount;
                    if (bitFieldCount >= 8) {
                        bitFieldCount = bitFieldCount % 8;
                        //adv nextFieldBitIndex to the next byte
                        nextFieldBitIndex += 8;
                    }
                }
                else
                {
                    if (bitFieldCount > 0)
                    {
                        //adv nextFieldBitIndex to the next byte
                        nextFieldBitIndex += 8;
                    }

                    var fieldInfo = new FieldLayout(
                        fieldDesc.fieldName, nextFieldBitIndex, fieldDesc.bitCount);
                    fieldInfos.Add(fieldInfo);
                    nextFieldBitIndex = fieldInfo.bitIndex + fieldInfo.bitCount;
                }
            }

            this.fieldLayouts = fieldInfos.ToArray();
            this.StructSize = ((nextFieldBitIndex - 1) / 8) + 1;
        }

        public FieldLayout GetField(FieldIndexEnum fieldIndex)
        {
            return this.fieldLayouts[Convert.ToInt32(fieldIndex)];
        }

        public T GetFieldValue<T>(BitVector bitVector, FieldIndexEnum fieldIndex, Func<byte[], T> unmarshalFunc)
        {
            var fieldLayout = GetField(fieldIndex);

            var bitCount = fieldLayout.bitCount;
            if (bitCount == 0) {
                // use all the remaining bitVector
                bitCount = bitVector.Count - fieldLayout.bitIndex;
            }
            BitVector subVector = bitVector.getSubVector(fieldLayout.bitIndex, bitCount);
            byte[] valueData = subVector.ToBytes();
            return unmarshalFunc(valueData);
        }

        public void SetFieldValue<T>(BitVector bitVector, FieldIndexEnum fieldIndex, T value, Func<T, byte[]> marshalFunc)
        {
            var fieldLayout = GetField(fieldIndex);
            byte[] valueData = marshalFunc(value);
            var subVector = new BitVector(valueData, fieldLayout.bitCount);
            bitVector.setSubVector(fieldLayout.bitIndex, subVector);
        }

    }


    public struct FieldLayout
    {
        public readonly string fieldName;
        public readonly int bitIndex;
        public readonly int bitCount;

        public FieldLayout(string fieldName, int bitCount)
            : this(fieldName, 0, bitCount)
        {
        }

        public FieldLayout(string fieldName, int bitIndex, int bitCount)
        {
            this.fieldName = fieldName;
            this.bitIndex = bitIndex;
            this.bitCount = bitCount;
        }

    }

}
