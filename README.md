# godot-steam-audio
This is a GDExtension that integrates the [steam-audio](https://valvesoftware.github.io/steam-audio/) library into Godot 4.2. 

### [Demo Video](https://youtu.be/9ltXpaphysc)  
![A picture of the editor screen with some godot-steam-audio nodes.](doc/imgs/editor.png)

This extension is in a pre-alpha phase, will have bugs and missing polish, and might crash. Linux and Windows
are currently working. Mac probably works, but I don't have the time nor the money to support that, sorry.

### Features 
 - Spatial ambisonics audio 
 - Occlusion and transmission through geometry 
 - Distance attenuation
 - Reflections (reverb)

 To come: 
 - More editor configuration
 - Baked scenes for higher-performance reflections
 - More raycasting support

### Installation
 - Download the [latest release](https://github.com/stechyo/godot/releases/tag/steam-audio) or clone and build [my fork of Godot](https://github.com/stechyo/godot/tree/4.2-gdext/audio-stream-funcs).
 - Download the [latest release](https://github.com/stechyo/godot-steam-audio/releases/latest) of the extension or build it yourself.
 - Download the [latest supported release](https://github.com/ValveSoftware/steam-audio/releases/tag/v4.5.0) of steam-audio, and copy the binaries 
   that you find in the `lib` folder for your operating system to the `bin/` folder of your project, which 
   should contain a `libgodot-steam-audio` library (.dll or .so) and a .gdextension file.

Your project should have a `bin/` folder with:
 - `libgodot-steam-audio.gdextension`
 - `libgodot-steam-audio.linux.template_debug.x86_64` for Linux, or a similar .dll for Windows
 - `libphonon.so` for Linux, or `phonon.dll`, `phonon.lib` and some other files for Windows - these are the
   steam-audio library binaries.

### Project setup
In the `project` folder you have an example setup. Basically, you need:
 - A `SteamAudioConfig` object in your scene
 - A `SteamAudioListener`, which you probably want to keep inside your `Camera3D`
 - `SteamAudioPlayer`s for each sound source that you want to use steam-audio on, each with a set sub_stream
   and with a `SteamAudioStream`
 - `SteamAudioGeometry` for each `MeshInstance3D` that you want to use for sound occlusion

### Why do I need your fork of Godot? That's annoying.
I agree, but I can't do much about it, because some internal functions aren't exposed
(https://github.com/godotengine/godot-proposals/issues/8609). Once either my PR gets merged or someone else
exposes these functions, the extension will work with an official version of godot.

### Contributing 
This extension expects the steam-audio lib to be placed in `src/lib/steamaudio` in order to compile.
Check the Makefile for an example install.

### Acknowledgements
godot-steam-audio uses the Steam® Audio SDK. Steam® is a trademark or registered trademark of Valve
Corporation in the United States of America and elsewhere.  

Vespergamedev's [GDNative module](https://github.com/vespergamedev/godot_steamaudio) was helpful in figuring
out how to use the steam-audio lib.

