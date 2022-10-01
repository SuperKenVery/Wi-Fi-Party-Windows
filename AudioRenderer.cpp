#include "pch.h"
#include "AudioRenderer.h"

AudioRenderer::AudioRenderer() {
	HRESULT hr;

	hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);
	if (FAILED(hr)) {
		printf("Failed to start mf\n");
		exit(1);
	}

	m_PlayCallbackEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
	if (m_PlayCallbackEvent == nullptr) {
		printf("Failed to create sample ready event\n");
		exit(1);
	}

	hr = MFCreateAsyncResult(nullptr, &m_xPlayCallback, nullptr, &m_PlayCallbackAsyncResult);
	if (FAILED(hr)) {
		printf("Failed to create async result\n");
		exit(1);
	}

	Platform::String^ m_DeviceIdString = MediaDevice::GetDefaultAudioRenderId(Windows::Media::Devices::AudioDeviceRole::Default); //Renderer specific
	IActivateAudioInterfaceAsyncOperation* asyncOp; 
	IID_IMFAsyncCallback;
	hr = ActivateAudioInterfaceAsync(
		m_DeviceIdString->Data(),
		__uuidof(IAudioClient3),
		nullptr,
		this,
		&asyncOp
	);


}

HRESULT AudioRenderer::ActivateCompleted(IActivateAudioInterfaceAsyncOperation* operation) {
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

	hr=m_AudioClient->GetBufferSize(&m_BufferFrames);
	if (FAILED(hr)) {
		printf("Failed to get buffer size");
		exit(1);
	}

	hr = m_AudioClient->GetService(__uuidof(IAudioRenderClient), (void**)&m_AudioRenderClient); //Renderer specific
	if (FAILED(hr)) {
		printf("Failed to get audio render service\n");
		exit(1);
	}

	hr = m_AudioClient->SetEventHandle(m_PlayCallbackEvent);
	if (FAILED(hr)) {
		printf("Failed to set event handle\n");
		exit(1);
	}

	hr = MFPutWaitingWorkItem(m_PlayCallbackEvent, 0, m_PlayCallbackAsyncResult, &playCallbackKey);
	if (FAILED(hr)) {
		printf("Failed to add OnPlayCallback to queue for the first time\n");
		exit(1);
	}

	hr=m_AudioClient->Start();
	if (FAILED(hr)) {
		printf("Failed to start audio renderer client\n");
		exit(1);
	}
	

	return S_OK;

}

HRESULT AudioRenderer::OnPlayCallback(IMFAsyncResult* pResult) {
	HRESULT hr;

	UINT32 paddingFrames;
	hr = m_AudioClient->GetCurrentPadding(&paddingFrames);
	if (FAILED(hr)) {
		printf("Failed to get current padding\n");
		exit(1);
	}
	UINT32 numFrames = m_BufferFrames - paddingFrames;

	BYTE* buffer;
	hr=m_AudioRenderClient->GetBuffer(numFrames, &buffer);
	if (FAILED(hr)) {
		printf("Failed to get buffer\n");
		exit(1);
	}
	UINT32 length = format.nBlockAlign * numFrames;
	//Fill the buffer with sound!!
	memset(buffer, nosound, length);

	hr=m_AudioRenderClient->ReleaseBuffer(1, 0);
	if (FAILED(hr)) {
		printf("Failed to release buffer\n");
		exit(1);
	}

	hr = MFPutWaitingWorkItem(m_PlayCallbackEvent, 0, m_PlayCallbackAsyncResult, &playCallbackKey);
	if (FAILED(hr)) {
		printf("Failed to add OnPlayCallback to queue again\n");
	}

	return S_OK;
}

AudioRenderer::~AudioRenderer() {

}
