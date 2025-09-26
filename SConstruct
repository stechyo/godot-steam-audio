#!/usr/bin/env python
import os

godot_cpp_path = "src/lib/godot-cpp"
steam_audio_path = "src/lib/steamaudio"
if (not (os.path.isdir(godot_cpp_path) and os.listdir(godot_cpp_path))) or (not (os.path.isdir(steam_audio_path) and os.listdir(steam_audio_path))):
    print("""Git submodule dependencies are missing.
    Run the following command to install them:
    git submodule update --init --recursive""")
    Exit(1)

env = SConscript("src/lib/godot-cpp/SConstruct")

steam_audio_lib_path = env.get("STEAM_AUDIO_LIB_PATH", "src/lib/steamaudio/lib")
if not (os.path.isdir(steam_audio_lib_path) and os.listdir(steam_audio_lib_path)):
    print("""No valid Steam Audio library path was found.
    Run the following command to install the latest release:
    make install-steam-audio""")
    Exit(1)

env.Append(CPPPATH=["src/"])

if env.get("CC", "").lower() == "cl":
    # Building with MSVC
    env.AppendUnique(CCFLAGS=("/I",  "src/lib/steamaudio/unity/include/phonon/"))
else:
    env.AppendUnique(CCFLAGS=("-isystem",  "src/lib/steamaudio/unity/include/phonon/"))

sources = Glob("src/*.cpp")

env.Append(LIBS=["phonon"])

if env["platform"] == "linux":
    env.Append(LIBPATH=[f'{steam_audio_lib_path}/linux-x64'])
    env.Append(LINKFLAGS=["-Wl,--version-script={}".format(env.File("linux_symbols.map").abspath)])
elif env["platform"] == "windows":
    env.Append(LIBPATH=[f'{steam_audio_lib_path}/windows-x64'])
elif env["platform"] == "macos":
    env.Append(LIBPATH=[f'{steam_audio_lib_path}/osx'])
elif env["platform"] == "android":
    if env["arch"] == "arm64":
        env.Append(LIBPATH=[f'{steam_audio_lib_path}/android-armv8'])
    if env["arch"] == "arm32":
        env.Append(LIBPATH=[f'{steam_audio_lib_path}/android-armv7'])
    if env["arch"] == "x86_64":
        env.Append(LIBPATH=[f'{steam_audio_lib_path}/android-x64'])
    if env["arch"] == "x86_32":
        env.Append(LIBPATH=[f'{steam_audio_lib_path}/android-x32'])

if env["target"] in ["editor", "template_debug"]:
    doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
    sources.append(doc_data)

library = env.SharedLibrary(
    "project/addons/godot-steam-audio/bin/libgodot-steam-audio{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
