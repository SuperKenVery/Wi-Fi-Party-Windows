#include "pch.h"
#include "AudioCapturer.h"

AudioCapturer::AudioCapturer() {
	HRESULT hr;

	m_RecordCallbackEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
	if (m_RecordCallbackEvent == nullptr) {
		printf("Failed to create sample ready event\n");
		exit(1);
	}

	hr = MFCreateAsyncResult(nullptr, &m_xRecordCallback, nullptr, &m_RecordCallbackAsyncResult);
	if (FAILED(hr)) {
		printf("Failed to create async result\n");
		exit(1);
	}

	IActivateAudioInterfaceAsyncOperation* asyncOp;
	IID_IMFAsyncCallback;
	Platform::String^ m_DeviceIdString = MediaDevice::GetDefaultAudioCaptureId(Windows::Media::Devices::AudioDeviceRole::Default); //Capturer specific
	hr = ActivateAudioInterfaceAsync(
		m_DeviceIdString->Data(),
		__uuidof(IAudioClient3),
		nullptr,
		this,
		&asyncOp
	);


}

HRESULT AudioCapturer::ActivateCompleted(IActivateAudioInterfaceAsyncOperation* operation) {
	HRESULT hr = S_OK;
	HRESULT hrActivateResult = S_OK;
	IUnknown* punkAudioInterface = nullptr;


	hr = operation->GetActivateResult(&hrActivateResult, &punkAudioInterface);
	if (!(SUCCEEDED(hr) && SUCCEEDED(hrActivateResult)))
	{
		printf("Failed to activate audio interface\n");
		exit(1);
	}
	hr=punkAudioInterface->QueryInterface(IID_PPV_ARGS(&m_AudioClient));
	if (FAILED(hr)) {
		printf("Failed to query interface\n");
		exit(1);
	}
	if (m_AudioClient == nullptr) {
		printf("Failed to get audio client interface\n");
		exit(1);
	}

	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.nSamplesPerSec = 44100;
	format.wBitsPerSample = 8;
	format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	format.cbSize = 0; //For pcm format, this is ignored

	/*if (m_AudioClient->IsFormatSupported(&format) != S_OK) {
		printf("Audio format not supported\n");
		exit(1);
	}*/

	UINT32 defaultPeriod, fundamentalPeriod, minPeriod, maxPeriod;
	m_AudioClient->GetSharedModeEnginePeriod(&format, &defaultPeriod, &fundamentalPeriod, &minPeriod, &maxPeriod);

	//1 reference time is 100 ns = 100*10**(-9)s = 10**(-7)s
	//a frame at 44100Hz is 2.26757*10**(-5) = 226.757 reference time
	REFERENCE_TIME one_frame = 227;
	hr = m_AudioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK|AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM,
		one_frame*minPeriod,
		0,
		&format,
		nullptr
	);
	if (FAILED(hr)) {
		printf("Failed to initialize audio client\n");
		exit(1);
	}

	hr = m_AudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&m_AudioCaptureClient); //Renderer specific
	if (FAILED(hr)) {
		printf("Failed to get audio render service\n");
		exit(1);
	}

	hr = m_AudioClient->SetEventHandle(m_RecordCallbackEvent);

	m_AudioClient->Start();
	hr = MFPutWaitingWorkItem(m_RecordCallbackEvent, 0, m_RecordCallbackAsyncResult, &recordCallbackKey);
	if (FAILED(hr)) {
		printf("Failed to add OnPlayCallback to queue for the first time\n");
	}

	return S_OK;

}

HRESULT AudioCapturer::OnRecordCallback(IMFAsyncResult* pResult) {
	BYTE* buffer;
	UINT32 framesAvailable;
	DWORD flags;
	HRESULT hr;
	for
		(
			hr = m_AudioCaptureClient->GetNextPacketSize(&framesAvailable);
			SUCCEEDED(hr) && framesAvailable > 0;
			hr = m_AudioCaptureClient->GetNextPacketSize(&framesAvailable)
		) {
		UINT32 dataLength = framesAvailable * format.nBlockAlign;

		hr = m_AudioCaptureClient->GetBuffer(&buffer, &framesAvailable, &flags, NULL, NULL);
		if (FAILED(hr)) {
			printf("Failed to get buffer\n");
			exit(1);
		}

		if ((flags & AUDCLNT_BUFFERFLAGS_SILENT)){
			continue;
		}
		//Send the sound in the buffer!


		hr = m_AudioCaptureClient->ReleaseBuffer(framesAvailable);
		if (FAILED(hr)) {
			printf("Failed to release buffer\n");
			exit(1);
		}

	}
	

	hr = MFPutWaitingWorkItem(m_RecordCallbackEvent, 0, m_RecordCallbackAsyncResult, &recordCallbackKey);
	if (FAILED(hr)) {
		printf("Failed to add OnPlayCallback to queue again\n");
	}
	return S_OK;
}

AudioCapturer::~AudioCapturer() {

}
