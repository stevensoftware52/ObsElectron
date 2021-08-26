#include <napi.h>
#include "DesktopBroadcaster.h"
	
// https://github.com/nodejs/node-addon-examples/tree/39faeb2d9bcf4f8a411e18944c33070df7368000/6_object_wrap/node-addon-api
Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return DesktopBroadcaster::Init(env, exports);
}
	
NODE_API_MODULE(addon, InitAll)