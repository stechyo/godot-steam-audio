install-steam-audio:
	curl -s https://api.github.com/repos/ValveSoftware/steam-audio/releases/latest \
		| grep -E 'browser_download.*steamaudio_[0-9\.]+\.zip' \
		| cut -d : -f 2,3 | tr -d \" | wget -O src/lib/steamaudio.zip -i -
	unzip src/lib/steamaudio.zip -d src/lib/
	rm src/lib/steamaudio.zip
	cp src/lib/steamaudio/lib/linux-x64/libphonon.so project/addons/godot-steam-audio/bin

release:
	scons platform=linux target=template_debug && scons platform=windows target=template_debug && \
		scons platform=linux target=template_release && scons platform=windows target=template_release
	mkdir godot-steam-audio-demo
	mkdir godot-steam-audio
	cp -r ./project/* ./godot-steam-audio-demo
	rm -r ./godot-steam-audio-demo/addons/godot-steam-audio/bin/libphonon.so.dbg
	cp -r ./godot-steam-audio-demo/addons ./godot-steam-audio
	zip -r ./godot-steam-audio-vX.Y.Z.zip godot-steam-audio README.md
	zip -r ./godot-steam-audio-demo-vX.Y.Z.zip godot-steam-audio-demo README.md
	rm -r ./godot-steam-audio ./godot-steam-audio-demo
