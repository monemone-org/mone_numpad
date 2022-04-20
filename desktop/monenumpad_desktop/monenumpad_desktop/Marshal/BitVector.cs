using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using SysMarshal = System.Runtime.InteropServices.Marshal;
using System.Text;

namespace monenumpad_desktop.Marshal
{
    public struct BitVector
    {
        private readonly byte[] bytes;
        readonly int bitCount;

        private static readonly byte[] BitMasks = {
                0b1000_0000,
                0b0100_0000,
                0b0010_0000,
                0b0001_0000,

                0b0000_1000,
                0b0000_0100,
                0b0000_0010,
                0b0000_0001
            };

        public BitVector(int bitCount)
            : this( new byte[((bitCount + 7) / 8)], bitCount ) 
        {
        }

        public BitVector(byte[] bytes, int bitCount = -1)
        {
            this.bytes = bytes;
            if (bitCount < 0)
            {
                this.bitCount = bytes.Length * 8;
            }
            else
            {
                this.bitCount = bitCount;
            }
        }

        public BitVector(IntPtr ptr, int ptrByteCount, int bitCount = -1)
            : this( BitVector.readBytes(ptr, ptrByteCount), bitCount )
        {
        }

        public BitVector(IntPtr ptr, Func<List<byte>,bool> endOfDataFunc, int bitCount = -1)
            : this(BitVector.readBytes(ptr, endOfDataFunc), bitCount)
        {
        }

        static private byte[] readBytes(IntPtr ptrBytes, int ptrByteCount)
        {
            var bytes = new byte[ptrByteCount];
            for (int i = 0; i < ptrByteCount; ++i)
            {
                bytes[i] = SysMarshal.ReadByte(ptrBytes, i);
            }

            return bytes;
        }

        static private byte[] readBytes(IntPtr ptrBytes, Func<List<byte>, bool> endOfDataFunc)
        {
            var data = new List<byte>();
            int i = 0;
            do
            {
                data.Add(SysMarshal.ReadByte(ptrBytes, i));
                ++i;
            }
            while (!endOfDataFunc(data));

            return  data.ToArray();
        }


        public int Count
        {
            get { return this.bitCount; }
        }

        public bool this[int index]
        {
            get
            {
                int nByte = index / 8;
                int nBit = index % 8;
                byte byteValue = this.bytes[nByte];
                return ((byteValue & BitMasks[nBit]) != 0 ? true : false);
            }

            set
            {
                int nByte = index / 8;
                int nBit = index % 8;
                byte byteValue = this.bytes[nByte];
                if (value)
                {
                    byteValue |= BitMasks[nBit];
                }
                else
                {
                    byteValue &= (byte)(~BitMasks[nBit]);
                }
                this.bytes[nByte] = byteValue;
            }
        }

        public BitVector getSubVector(int offset, int count)
        {
            Debug.Assert(count > 0);

            var subVector = new BitVector(count);
            for (int i=0; i<count; ++i)
            {
                subVector[i] = this[offset + i];
            }
            return subVector;
        }

        //
        // param:
        //      offset: offset in this BitVector to copy the
        //              value of subVector to.
        //      subVector
        public void setSubVector(int offset, BitVector subVector)
        {
            for (int i = 0; i < subVector.Count; ++i)
            {
                this[offset + i] = subVector[i];
            }
        }

        public IEnumerator<bool> GetEnumerator()
        {
            return new ByteArrayBitEnumerator(this);
        }

        public IEnumerator<bool> GetEnumerator(int offset, int count)
        {
            return new ByteArrayBitEnumerator(this, offset, count);
        }

        public byte[] ToBytes() {
            return this.bytes;
        }

        public override string ToString()
        {
            StringBuilder result = new StringBuilder("0b");
            const int bitGroupSize = 4;
            const int byteGroupSize = 4;

            for (int i=0; i<this.Count; ++i)
            {
                bool value = this[i];

                if (i > 0 && (i % (8 * byteGroupSize) == 0))
                {
                    result.Append("\n");
                }

                if (i > 0 && (i % bitGroupSize == 0))
                {
                    result.Append(" ");
                }

                result.Append(value ? "1" : "0");
            }

            return result.ToString();
        }

    }

    public class ByteArrayBitEnumerator : IEnumerator<bool>
    {
        readonly BitVector bitVector;
        readonly int bitOffset;
        readonly int bitCount;

        // Enumerators are positioned before the first element
        // until the first MoveNext() call.
        private int currBitIndex = -1;

        public ByteArrayBitEnumerator(BitVector bitVector)
            : this(bitVector, 0, bitVector.Count)
        {
        }

        public ByteArrayBitEnumerator(BitVector bitVector, int bitOffset, int bitCount)
        {
            Debug.Assert(bitOffset + bitCount < bitVector.Count);
            this.bitVector = bitVector;
            this.bitOffset = bitOffset;
            this.bitCount = bitCount;
        }

        public bool Current
        {
            get
            {
                bool bitValue = this.bitVector[this.bitOffset + this.currBitIndex];
                return bitValue;
            }
        }

        bool IEnumerator<bool>.Current => throw new NotImplementedException();

        object IEnumerator.Current => throw new NotImplementedException();

        #region IEnumerator Implementation

        public void Dispose()
        {
        }

        public bool MoveNext()
        {
            this.currBitIndex += 1;
            return (this.currBitIndex < this.bitCount) && (this.bitOffset + this.currBitIndex < this.bitVector.Count);
        }

        public void Reset()
        {
            this.currBitIndex = -1;
        }


    }
    #endregion
}
