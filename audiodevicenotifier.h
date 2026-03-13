#ifndef AUDIODEVICENOTIFIER_H
#define AUDIODEVICENOTIFIER_H

#ifdef _WIN32

#include <mmdeviceapi.h>
#include <atomic>

class AudioEngine;

class AudioDeviceNotifier : public IMMNotificationClient
{
public:
    AudioDeviceNotifier(AudioEngine *engine);
    ~AudioDeviceNotifier();

    bool start();
    void stop();

    // IUnknown
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;

    // IMMNotificationClient
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override;
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override;
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override;

private:
    AudioEngine *m_engine;
    IMMDeviceEnumerator *m_enumerator;
    std::atomic<ULONG> m_refCount;
    bool m_registered;
};

#endif // _WIN32
#endif // AUDIODEVICENOTIFIER_H
