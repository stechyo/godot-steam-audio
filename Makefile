install-steam-audio:
	curl -s https://api.github.com/repos/ValveSoftware/steam-audio/releases/latest \
		| grep -E 'browser_download.*steamaudio_[0-9\.]+\.zip' \
		| cut -d : -f 2,3 | tr -d \" | wget -O src/lib/steamaudio.zip -i -
	unzip src/lib/steamaudio.zip -d src/lib/
	rm src/lib/steamaudio.zip
	cp src/lib/steamaudio/lib/linux-x64/* project/addons/godot-steam-audio/bin/
	cp src/lib/steamaudio/lib/windows-x64/* project/addons/godot-steam-audio/bin/
	cp src/lib/steamaudio/lib/osx/* project/addons/godot-steam-audio/bin/
	cp src/lib/steamaudio/lib/android-armv8/* project/addons/godot-steam-audio/bin/android/arm64
	cp src/lib/steamaudio/lib/android-x64/* project/addons/godot-steam-audio/bin/android/x86_64

release:
	scons platform=android arch=arm64 target=template_release && scons platform=android arch=x86_64 target=template_release && \
		scons platform=android arch=arm64 target=template_debug && scons platform=android arch=x86_64 target=template_debug && \
		scons platform=linux target=template_debug && scons platform=windows target=template_debug && \
		scons platform=linux target=template_release && scons platform=windows target=template_release
	mkdir godot-steam-audio-demo
	mkdir godot-steam-audio
	cp -r ./project/* ./godot-steam-audio-demo
	rm -rf ./godot-steam-audio-demo/addons/godot-steam-audio/bin/libphonon.so.dbg
	cp -r ./godot-steam-audio-demo/addons ./godot-steam-audio

macos-release:
	scons platform=macos target=template_debug && scons platform=macos target=template_release
	mkdir godot-steam-audio-demo
	mkdir godot-steam-audio
	cp -r ./project/* ./godot-steam-audio-demo
	rm -rf ./godot-steam-audio-demo/addons/godot-steam-audio/bin/libphonon.so.dbg
	cp -r ./godot-steam-audio-demo/addons ./godot-steam-audio
