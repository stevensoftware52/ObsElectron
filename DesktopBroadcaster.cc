#include "DesktopBroadcaster.h"

Napi::Object DesktopBroadcaster::Init(Napi::Env env, Napi::Object exports)
{
	Napi::Function func = DefineClass(env, "DesktopBroadcaster",
		{ 
			InstanceMethod("Stop", &DesktopBroadcaster::Stop),
			InstanceMethod("Broadcast", &DesktopBroadcaster::Broadcast),
			InstanceMethod("ObsInitVideo", &DesktopBroadcaster::ObsInitVideo),
			InstanceMethod("ObsInitAudio", &DesktopBroadcaster::ObsInitAudio),
			InstanceMethod("IsStreaming", &DesktopBroadcaster::IsStreaming),
			InstanceMethod("ObsInitOutput", &DesktopBroadcaster::ObsInitOutput)
		});  

  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  env.SetInstanceData(constructor);

  exports.Set("DesktopBroadcaster", func);
  return exports;
}

DesktopBroadcaster::DesktopBroadcaster(const Napi::CallbackInfo& info) : 
	Napi::ObjectWrap<DesktopBroadcaster>(info) 
{
	Napi::Env env = info.Env();

	int length = info.Length();
	
	if (length < 4)
	{
		Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
		return;
	}

	std::string arg0 = info[0].As<Napi::String>();
	std::string arg1 = info[1].As<Napi::String>();
	std::string arg2 = info[2].As<Napi::String>();
	std::string arg3 = info[2].As<Napi::String>();
  
	InternalInit(arg0, arg1, arg3, arg3, 1920, 1080, 1920, 1080);
}

DesktopBroadcaster::~DesktopBroadcaster()
{		
	// ??
	StopInternal();
}

Napi::Value DesktopBroadcaster::Stop(const Napi::CallbackInfo& info)
{
	StopInternal();
	return Napi::Number::New(info.Env(), 0);
}

Napi::Value DesktopBroadcaster::IsStreaming(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), m_output != nullptr && obs_output_active(m_output));
}

Napi::Value DesktopBroadcaster::Broadcast(const Napi::CallbackInfo& info)
{
	if (m_output == nullptr || !obs_output_start(m_output))
		return Napi::Number::New(info.Env(),-1);
	
	return Napi::Number::New(info.Env(), ERROR_SUCCESS);
}

Napi::Value DesktopBroadcaster::ObsInitVideo(const Napi::CallbackInfo& info)
{		
	int result = obs_reset_video(&m_ovi);

	if (result != ERROR_SUCCESS)
		return Napi::Number::New(info.Env(), result);

	m_monitor_capture = obs_source_create(std::string(obs_get_latest_input_type_id("monitor_capture")).c_str(), "Display Capture", NULL, nullptr);

	if (m_monitor_capture == nullptr)
		return Napi::Number::New(info.Env(), -1);

	// set monitoring if source monitors by default ?
	uint32_t flags = obs_source_get_output_flags(m_monitor_capture);

	if ((flags & OBS_SOURCE_MONITOR_BY_DEFAULT) != 0) 
		obs_source_set_monitoring_type(m_monitor_capture, OBS_MONITORING_TYPE_MONITOR_ONLY);

	return Napi::Number::New(info.Env(), ERROR_SUCCESS);
}

Napi::Value DesktopBroadcaster::ObsInitAudio(const Napi::CallbackInfo& info)
{
	if (!obs_reset_audio(&m_ai))
		return Napi::Number::New(info.Env(), -1);

	OBSData settings = obs_data_create();
	obs_data_set_string(settings, "device_id", "default");
	m_wasapi_output_capture = obs_source_create("wasapi_output_capture", "", nullptr, nullptr);
	obs_data_release(settings);

	if (m_wasapi_output_capture == nullptr)
		return Napi::Number::New(info.Env(), -1);

	return Napi::Number::New(info.Env(), ERROR_SUCCESS);
}

Napi::Value DesktopBroadcaster::ObsInitOutput(const Napi::CallbackInfo& info)
{
	// Output 
	m_output = obs_output_create("rtmp_output", "test_stream", nullptr, nullptr);

	if (m_output == nullptr)
		return Napi::Number::New(info.Env(), -1);
	
	// Audio Source
	obs_set_output_source(0, m_wasapi_output_capture);

	// Video Source
	obs_set_output_source(1, m_monitor_capture);
	
	auto source = obs_get_output_source(1);
	auto dbg = obs_source_get_volume(source);

	// Service 
	OBSService service = obs_service_create("rtmp_common", "test_service", nullptr, nullptr);
	OBSData service_settings = obs_data_create();	
	obs_data_set_string(service_settings, "service", "Twitch");
	obs_data_set_string(service_settings, "key", m_streamKey.c_str());
	obs_data_set_string(service_settings, "rate_control", "CBR");
	obs_data_set_string(service_settings, "preset", "veryfast");
	obs_data_set_string(service_settings, "bind_ip", "default");
	obs_data_set_string(service_settings, "service", "Twitch");
	obs_data_set_string(service_settings, "stream_key_link", "https://dashboard.twitch.tv/settings/stream");
	obs_data_set_string(service_settings, "rate_control", "CBR");
	obs_data_set_string(service_settings, "x264opts", "scenecut=0");
	obs_data_set_string(service_settings, "server", "auto");
	obs_service_update(service, service_settings);

	// Video Encoder Settings
	OBSEncoder vencoder = obs_video_encoder_create("obs_x264", "test_x264", nullptr, nullptr);
	OBSData vencoder_settings = obs_data_create();
	obs_data_set_int(vencoder_settings, "bitrate", 1500);
	obs_data_set_int(vencoder_settings, "keyint_sec", 2);
	obs_encoder_update(vencoder, vencoder_settings);

	// Audio Encoder Settings
	OBSEncoder aencoder = obs_audio_encoder_create("ffmpeg_aac", "test_aac", nullptr, 0, nullptr);
	OBSData aencoder_settings = obs_data_create();
	obs_data_set_int(aencoder_settings, "bitrate", 160);
	obs_encoder_update(aencoder, aencoder_settings);

	// Connect everything together
	obs_encoder_set_video(vencoder, obs_get_video());
	obs_encoder_set_audio(aencoder, obs_get_audio());
	obs_output_set_video_encoder(m_output, vencoder);
	obs_output_set_audio_encoder(m_output, aencoder, 0);
	obs_output_set_service(m_output, service);
	
	return Napi::Number::New(info.Env(), ERROR_SUCCESS);
}

