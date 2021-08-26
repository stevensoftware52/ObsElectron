#ifndef MYOBJECT_H
#define MYOBJECT_H

#include <napi.h>

#include <thread>
#include <vector>
#include <mutex>
#include <Windows.h>
#include <string>

#include <obs.h>
#include <obs.hpp>
#include <obs-module.h>

class DesktopBroadcaster : public Napi::ObjectWrap<DesktopBroadcaster>
{
public:
	static Napi::Object Init(Napi::Env env, Napi::Object exports);
	
public:
	DesktopBroadcaster(const Napi::CallbackInfo& info);
	~DesktopBroadcaster();

public:
	Napi::Value Stop(const Napi::CallbackInfo& info);
	Napi::Value Broadcast(const Napi::CallbackInfo& info);
	Napi::Value ObsInitVideo(const Napi::CallbackInfo& info);
	Napi::Value ObsInitAudio(const Napi::CallbackInfo& info);
	Napi::Value ObsInitOutput(const Napi::CallbackInfo& info);
	Napi::Value IsStreaming(const Napi::CallbackInfo& info);

private:	
	void InternalInit(const std::string& streamKey, const std::string& dataPath, const std::string& modulePath_Bin, const std::string& modulePath, 
		const int base_width, const int base_height, const int output_width, const int output_height)
	{
		m_streamKey = streamKey;
		
		obs_startup("en-US", nullptr, nullptr);	
		obs_add_data_path(dataPath.c_str());	
		obs_add_module_path(modulePath_Bin.c_str(), modulePath.c_str());
		
		obs_load_all_modules();
		obs_post_load_modules();

		// Video
		m_ovi.graphics_module = "libobs-d3d11.dll";
		m_ovi.base_width = base_width;
		m_ovi.base_height = base_height;
		m_ovi.output_width = output_width;
		m_ovi.output_height = output_height;
		m_ovi.output_format = VIDEO_FORMAT_I420;
		m_ovi.colorspace = VIDEO_CS_709;
		m_ovi.range = VIDEO_RANGE_PARTIAL;
		m_ovi.gpu_conversion = true;
		m_ovi.scale_type = OBS_SCALE_BILINEAR;
		m_ovi.adapter = 0; // main monitor.......?
		m_ovi.fps_num = 24;
		m_ovi.fps_den = 1;

		// Audio
		m_ai.speakers = SPEAKERS_STEREO;
		m_ai.samples_per_sec = 48000;
	}

	void StopInternal()
	{
		if (m_output != nullptr)
			obs_output_stop(m_output);
	
		obs_source_release(m_monitor_capture);
		obs_source_release(m_wasapi_output_capture);
		obs_output_stop(m_output);
		obs_output_release(m_output);
		obs_shutdown();		
	}
	
	obs_video_info m_ovi;
	obs_audio_info m_ai;
	obs_source_t* m_monitor_capture{nullptr};
	obs_source_t* m_wasapi_output_capture{nullptr};
	obs_output_t* m_output{nullptr};
	std::string m_streamKey;
};

#endif