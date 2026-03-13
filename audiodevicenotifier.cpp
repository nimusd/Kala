#ifdef _WIN32

#include "audiodevicenotifier.h"
#include "audioengine.h"
#include <QMetaObject>
#include <iostream>
#include <combaseapi.h>

AudioDeviceNotifier::AudioDeviceNotifier(AudioEngine *engine)
    : m_engine(engine)
    , m_enumerator(nullptr)
    , m_refCount(1)
    , m_registered(false)
{
}

AudioDeviceNotifier::~AudioDeviceNotifier()
{
    stop();
}

bool AudioDeviceNotifier::start()
{
    if (m_registered) return true;

    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr,
        CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void**>(&m_enumerator));

    if (FAILED(hr)) {
        std::cerr << "AudioDeviceNotifier: Failed to create IMMDeviceEnumerator (hr=0x"
                  << std::hex << hr << std::dec << ")" << std::endl;
        return false;
    }

    hr = m_enumerator->RegisterEndpointNotificationCallback(this);
    if (FAILED(hr)) {
        std::cerr << "AudioDeviceNotifier: Failed to register notification callback (hr=0x"
                  << std::hex << hr << std::dec << ")" << std::endl;
        m_enumerator->Release();
        m_enumerator = nullptr;
        return false;
    }

    m_registered = true;
    std::cout << "AudioDeviceNotifier: Listening for audio device changes" << std::endl;
    return true;
}

void AudioDeviceNotifier::stop()
{
    if (m_enumerator) {
        if (m_registered) {
            m_enumerator->UnregisterEndpointNotificationCallback(this);
            m_registered = false;
        }
        m_enumerator->Release();
        m_enumerator = nullptr;
    }
    std::cout << "AudioDeviceNotifier: Stopped" << std::endl;
}

// IUnknown

ULONG STDMETHODCALLTYPE AudioDeviceNotifier::AddRef()
{
    return m_refCount.fetch_add(1) + 1;
}

ULONG STDMETHODCALLTYPE AudioDeviceNotifier::Release()
{
    ULONG count = m_refCount.fetch_sub(1) - 1;
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT STDMETHODCALLTYPE AudioDeviceNotifier::QueryInterface(REFIID riid, void **ppvObject)
{
    if (riid == __uuidof(IUnknown) || riid == __uuidof(IMMNotificationClient)) {
        *ppvObject = static_cast<IMMNotificationClient*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

// IMMNotificationClient

HRESULT STDMETHODCALLTYPE AudioDeviceNotifier::OnDefaultDeviceChanged(
    EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
    // Only care about render (output) devices in the default/console role
    if (flow == eRender && role == eConsole) {
        std::cout << "AudioDeviceNotifier: Default output device changed" << std::endl;
        QMetaObject::invokeMethod(m_engine, "onDefaultDeviceChanged", Qt::QueuedConnection);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE AudioDeviceNotifier::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE AudioDeviceNotifier::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE AudioDeviceNotifier::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE AudioDeviceNotifier::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    return S_OK;
}

#endif // _WIN32
