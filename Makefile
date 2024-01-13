install-steam-audio:
	curl -s https://api.github.com/repos/ValveSoftware/steam-audio/releases/latest \
		| grep -E 'browser_download.*steamaudio_[0-9\.]+\.zip' \
		| cut -d : -f 2,3 | tr -d \" | wget -O src/lib/steamaudio.zip -i -
	unzip src/lib/steamaudio.zip -d src/lib/
	rm src/lib/steamaudio.zip
	cp src/lib/steamaudio/lib/linux-x64/libphonon.so project/addons/godot-steam-audio/bin

release:
	scons platform=linux target=template_debug && scons platform=windows target=template_debug
	mkdir godot-steam-audio
	cp -r ./project ./godot-steam-audio/demo
	rm -rI ./godot-steam-audio/demo/.godot ./godot-steam-audio/demo/.gitignore ./godot-steam-audio/demo/.gitattributes ./godot-steam-audio/demo/addons/godot-steam-audio/bin/libphonon.so
	cp -r ./godot-steam-audio/demo/addons ./godot-steam-audio/addons
	zip -r ./godot-steam-audio-vX.Y.Z.zip godot-steam-audio README.md
	rm -rI ./godot-steam-audio
