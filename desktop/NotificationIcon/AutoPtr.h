#pragma once

#include "framework.h"

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


