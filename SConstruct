#!/usr/bin/env python

env = SConscript("src/lib/godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])

if env.get("CC", "").lower() == "cl":
    # Building with MSVC
    env.AppendUnique(CCFLAGS=("/I",  "src/lib/steamaudio/unity/include/phonon/"))
else:
    env.AppendUnique(CCFLAGS=("-isystem",  "src/lib/steamaudio/unity/include/phonon/"))

sources = Glob("src/*.cpp")

steam_audio_lib_path = env.get("STEAM_AUDIO_LIB_PATH", "src/lib/steamaudio/lib")

if env["platform"] == "linux":
    env.Append(LIBPATH=[f'{steam_audio_lib_path}/linux-x64'])
    env.Append(LIBS=["libphonon.so"])
elif env["platform"] == "windows":
    env.Append(LIBPATH=[f'{steam_audio_lib_path}/windows-x64'])
    env.Append(LIBS=["phonon"])
elif env["platform"] == "macos":
    env.Append(LIBPATH=[f'{steam_audio_lib_path}/osx'])
    env.Append(LIBS=["libphonon.dylib"])

if env["target"] in ["editor", "template_debug"]:
    doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
    sources.append(doc_data)

library = env.SharedLibrary(
    "project/addons/godot-steam-audio/bin/godot-steam-audio{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
