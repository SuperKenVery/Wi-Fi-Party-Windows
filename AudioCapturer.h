#pragma once

#include <Windows.h>
#include <wrl\implements.h>
#include <mfapi.h>
#include <AudioClient.h>
#include <mmdeviceapi.h>

using namespace Microsoft::WRL;
using namespace Windows::Media::Devices;
using namespace Windows::Storage::Streams;

class AudioCapturer : public RuntimeClass<RuntimeClassFlags<ClassicCom>, FtmBase, IActivateAudioInterfaceCompletionHandler> {
public:
    AudioCapturer();
    METHODASYNCCALLBACK(AudioCapturer, RecordCallback, OnRecordCallback);
    //METHODASYNCCALLBACK(AudioRenderer, RecordCallback, OnRecordCallback);
    STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation* operation);

private:
    IMFAsyncResult* m_RecordCallbackAsyncResult;
    ~AudioCapturer();
    //HRESULT OnPlayCallback(IMFAsyncResult* pResult);
    HRESULT OnRecordCallback(IMFAsyncResult* pResult);
    IAudioClient3* m_AudioClient = nullptr;
    //IAudioRenderClient* m_AudioRenderClient;
    IAudioCaptureClient* m_AudioCaptureClient;
    HANDLE m_RecordCallbackEvent;
    WAVEFORMATEX format = { 0 };
    MFWORKITEM_KEY recordCallbackKey;
};

