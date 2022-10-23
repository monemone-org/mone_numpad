#pragma once

#include "framework.h"
#include <string>

typedef struct MMDeviceID
{
    std::wstring ID;

    MMDeviceID()
    {}

    MMDeviceID(LPCWSTR pszID) :
        ID(pszID)
    {
    }

    MMDeviceID(const MMDeviceID& copy)
    {
        this->ID = copy.ID;
    }

    MMDeviceID& operator=(const MMDeviceID& copy)
    {
        this->ID = copy.ID;
        return *this;
    }

    bool operator==(const MMDeviceID& copy) const
    {
        return this->ID == copy.ID;
    }

    bool operator!=(const MMDeviceID& copy) const
    {
        return !(*this == copy);
    }

    void clear()
    {
        this->ID.clear();
    }

    LPCWSTR ToString() const
    {
        return this->ID.c_str();
    }

} MMDeviceID;


