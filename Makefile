install-steam-audio:
	curl -s https://api.github.com/repos/ValveSoftware/steam-audio/releases/latest \
		| grep -E 'browser_download.*steamaudio_[0-9\.]+\.zip' \
		| cut -d : -f 2,3 | tr -d \" | wget -O src/lib/steamaudio.zip -i -
	unzip src/lib/steamaudio.zip -d src/lib/
	rm src/lib/steamaudio.zip
	cp src/lib/steamaudio/lib/linux-x64/libphonon.so project/bin

release:
	scons platform=linux target=template_debug && scons platform=windows target=template_debug
	cp -r ./project ./godot-steam-audio
	rm -rI ./godot-steam-audio/.godot ./godot-steam-audio/.gitignore ./godot-steam-audio/.gitattributes ./godot-steam-audio/bin/libphonon.so
	zip -r ./godot-steam-audio-vX.Y.Z.zip godot-steam-audio README.md
	rm -rI ./godot-steam-audio
