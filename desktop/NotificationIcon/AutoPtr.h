#pragma once

#include "framework.h"

class CCoTaskAllocator
{
public:
	_Ret_maybenull_ _Post_writable_byte_size_(nBytes) _ATL_DECLSPEC_ALLOCATOR static void* Reallocate(
		_In_ void* p,
		_In_ size_t nBytes) throw()
	{
		return CoTaskMemRealloc(p, nBytes);
	}

	_Ret_maybenull_ _Post_writable_byte_size_(nBytes) _ATL_DECLSPEC_ALLOCATOR static void* Allocate(_In_ size_t nBytes) throw()
	{
		return CoTaskMemAlloc(nBytes);
	}

	static void Free(_In_ void* p) throw()
	{
		CoTaskMemFree(p);
	}
};



/* auto free LocalFree pointer */
class CLocalFreeAutoPtr: public CHeapPtr<LPVOID, CLocalAllocator>
{
public:
	bool IsNull() const {
		return this->m_pData == NULL;
	}

	SIZE_T GetSize() const
	{
	    return LocalSize(m_pData);
	}

};

struct CPROPVARIANT : public PROPVARIANT
{
    CPROPVARIANT()
    {
        PropVariantInit(this);
    }

    ~CPROPVARIANT()
    {
        PropVariantClear(this);
    }
};


