{
  "targets": [
    {
      "target_name": "addon",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "addon.cc", "DesktopBroadcaster.cc" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "libobs"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
	  "libraries": [		
		"../libobs/bin/obs.lib"
	  ]
    }
  ]
}
