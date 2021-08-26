// Modules to control application life and create native browser window
const {app, BrowserWindow} = require('electron')
const path = require('path')

var fs = require('fs');
var streamKey = fs.readFileSync('stream_key.txt', 'utf8');

var addon = require('bindings')('addon');

var obj = new addon.DesktopBroadcaster(
	streamKey, 
	process.cwd() + "/obs_modules/data/libobs/",
	process.cwd() + "/obs_modules/obs-plugins/32bit/",
	process.cwd() + "/obs_modules/data/obs-plugins/",
);

var obsReady = false;

function initObs()
{
	console.log("ObsInitVideo...");
	var initResult_Video = obj.ObsInitVideo(); 
	
	if (initResult_Video != 0)
	{
		console.log("ObsInitVideo failed error code" + initResult_Video);
		return;
	}
	
	console.log("ObsInitAudio...");
	var initResult_Audio = obj.ObsInitAudio(); 
	
	if (initResult_Audio != 0)
	{
		console.log("ObsInitAudio failed error code" + initResult_Video);
		return;
	}
	
	console.log("ObsInitOutput...");
	var initResult_Output = obj.ObsInitOutput();
	
	if (initResult_Output != 0)
	{
		console.log("ObsInitOutput failed error code" + initResult_Output);
		return;
	}
	
	console.log("Obs ready for broadcasting");
	obsReady = true;
}

function createWindow() 
{	
	if (streamKey == "")
	{
		console.log("stream_key.txt is empty!\n\n");
		throw("stream_key.txt is empty");
		return;
	}
	else
	{
		initObs();
		console.log("Starting stream...");
		
		if (obj.Broadcast() == 0)
			console.log("Success");
		else
			console.log("Failure");
		
		console.log("\nUnused function DesktopBroadcaster::IsStreaming()...");
	}
	
  // Create the browser window.
  const mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js')
    }
  })

  // and load the index.html of the app.
  mainWindow.loadFile('index.html')
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.whenReady().then(() => {
  createWindow()
})

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bar to stay active until the user quits
// explicitly with Cmd + Q.
app.on('window-all-closed', function () {
  if (process.platform !== 'darwin') app.quit()
})

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
