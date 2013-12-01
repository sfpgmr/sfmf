#pragma once
// Media Foundation headers
//#include <d3d9.h>

namespace sf {


#define VIDEO_WIDTH  1280
#define VIDEO_HEIGHT  720

	extern HRESULT InitializeSinkWriter(IMFSinkWriter **ppWriter, DWORD *pStreamIndex);
	extern HRESULT WriteFrame(
		IMFSinkWriter *pWriter,
		DWORD streamIndex,
		const LONGLONG& rtStart,        // Time stamp.
		const LONGLONG& rtDuration      // Frame duration.
		);
	extern Windows::Foundation::IAsyncActionWithProgress<double>^ WriteAsync(Windows::Storage::Streams::IRandomAccessStream^ stream);

	// Class to start and shutdown Media Foundation
	class AutoMF
	{
	public:
		AutoMF()
			: _bInitialized(false)
		{
			CHK(MFStartup(MF_VERSION));
		}

		~AutoMF()
		{
			if (_bInitialized)
			{
				(void) MFShutdown();
			}
		}

	private:
		bool _bInitialized;
	};

	ref class  VideoWriter sealed
	{
	public:
		VideoWriter(
			Windows::Storage::Streams::IRandomAccessStream^ stream
			);

		virtual ~VideoWriter()
		{
			Finalize();
		};
	internal:
		virtual void Finalize()
		{
			if (sinkWriter_)
			{
				sinkWriter_->Finalize();
			}
		}
		void WriteAudioSample(IMFSample* sample);
		void WriteVideoSample();
		void SetTextureToSample(ID3D11DeviceContext1* context, ID3D11Texture2D* texture);

		property UINT SamplesPerSecond;
		property UINT AverageBytesPerSecond;
		property UINT ChannelCount;
		property UINT BitsPerSample;
    property LONGLONG VideoSampleTime { LONGLONG get() { return videoSampleTime_; }}
	private:
		// 出力先ストリームへのポインタ
		Windows::Storage::Streams::IRandomAccessStream^ stream_;
		// 
		//    IMFSinkWriterPtr sinkWriter_;
		IMFSinkWriterExPtr sinkWriter_;
		IMFByteStreamPtr byteStream_;
		IMFAttributesPtr attr_;
		IMFMediaTypePtr mediaTypeOut_;
		IMFMediaTypePtr mediaTypeIn_;
		IMFMediaTypePtr mediaTypeInAudio_;
		IMFMediaTypePtr mediaTypeOutAudio_;
		DWORD streamIndex_;
		DWORD streamIndexAudio_;

		IMFSamplePtr sample_;
		IMFMediaBufferPtr buffer_;
		LONGLONG videoSampleTime_;
    LONGLONG audioSampleTime_;
  };


	class AudioReaderCallBack : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IMFSourceReaderCallback >
	{
	public:
		typedef boost::signals2::signal<void(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample)> OnReadSampleSignal;

		AudioReaderCallBack()
		{
		}

		~AudioReaderCallBack()
		{
		}

		// IMFSourceReaderCallback methods
		STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex,
			DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample)
		{
			onReadSampleSignal_(hrStatus, dwStreamIndex,
				dwStreamFlags, llTimestamp, pSample);
			return S_OK;

		}

		STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *)
		{
			return S_OK;
		}

		STDMETHODIMP OnFlush(DWORD)
		{
			return S_OK;
		}

		OnReadSampleSignal& SignalOnReadSample(){ return onReadSampleSignal_; }
	private:
		OnReadSampleSignal onReadSampleSignal_;
	};

	Microsoft::WRL::ComPtr<AudioReaderCallBack> AudioReaderCallBackPtr;

	ref class AudioReader sealed  {
	public:
		AudioReader(Windows::Storage::Streams::IRandomAccessStream^ stream);
	internal:
		property UINT SamplesPerSecond;
		property UINT AverageBytesPerSecond;
		property UINT ChannelCount;
		property UINT BitsPerSample;
    property LONGLONG FileSize { LONGLONG get(){ return fileSize_; } }
		DWORD ReadSample(IMFSamplePtr& sample);
		property LONGLONG SampleTime {
			LONGLONG get(){ return videoSampleTime_; }
		}
		IMFMediaTypePtr& NativeMediaType(){ return nativeMediaType_; }
		IMFMediaTypePtr& CurrentMediaType(){ return currentMediaType_; }

	private:
		IMFSourceReaderExPtr reader_;
		IMFByteStreamPtr byteStream_;
		IMFMediaTypePtr nativeMediaType_;
		IMFMediaTypePtr currentMediaType_;
		LONGLONG videoSampleTime_;
    LONGLONG fileSize_;
    
	};

}
