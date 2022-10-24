#pragma once
#include "framework.h"

class IMMVolumeControl
{
public:
    // vol is between 0 .. 1
    virtual float GetVolume() const throw (HRESULT) = 0;
    virtual void SetVolume(float vol) throw (HRESULT) = 0;

    virtual bool IsMute() const throw (HRESULT) = 0;
    virtual void SetMute(bool mute) throw (HRESULT) = 0;
    virtual void toggleMute() throw (HRESULT) = 0;

    virtual LPCWSTR GetDisplayName() const = 0;
    virtual LPCWSTR GetVolControlID() const = 0;
};

