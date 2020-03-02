# rotatescreen
A simple Win32 app for hotkey screen rotation

## Build
- Open `rotatescreen.sln` in Visual Studio 2017.
- Project targets Windows SDK Version `10.0.17134.0`
- Platform Toolset is `Visual Studio 2017 (v141)`

## Usage
- Run `rotatescreen.exe`
- The app runs in the taskbar notification area (aka the system tray).
- Double-clicking the notification icon will allow you to specify the hotkey combination and display that should be rotated.
- Once these settings are applied, pressing the hotkey will rotate the specified display by 90 degrees.
- The app is most useful when it runs automatically, so you may want to configure it to do so. For example, place a shortcut to the rotatescreen.exe in `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup`
- Application settings are stored in the registry under `HKEY_CURRENT_USER\Software\mattjustice\rotatescreen`.

