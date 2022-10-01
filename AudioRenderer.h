#pragma once

#include <Windows.h>
#include <wrl\implements.h>
#include <mfapi.h>
#include <AudioClient.h>
#include <mmdeviceapi.h>

using namespace Microsoft::WRL;
using namespace Windows::Media::Devices;
using namespace Windows::Storage::Streams;


class AudioRenderer : public RuntimeClass<RuntimeClassFlags<ClassicCom>, FtmBase, IActivateAudioInterfaceCompletionHandler> {
public:
    AudioRenderer();
    METHODASYNCCALLBACK(AudioRenderer, PlayCallback, OnPlayCallback);
    //METHODASYNCCALLBACK(AudioRenderer, RecordCallback, OnRecordCallback);
    STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation* operation);

private:
    IMFAsyncResult* m_PlayCallbackAsyncResult;
    ~AudioRenderer();
    HRESULT OnPlayCallback(IMFAsyncResult* pResult);
    //HRESULT OnRecordCallback(IMFAsyncResult* pResult);
    IAudioClient3* m_AudioClient = nullptr;
    IAudioRenderClient* m_AudioRenderClient;
    //IAudioCaptureClient* m_AudioCaptureClient;
    HANDLE m_PlayCallbackEvent;
    WAVEFORMATEX format = { 0 };
    MFWORKITEM_KEY playCallbackKey;
    UINT32 m_BufferFrames;
};

