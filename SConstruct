#!/usr/bin/env python

env = SConscript("src/lib/godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

if env["platform"] == "linux":
    env.AppendUnique(CCFLAGS=("-isystem",  "src/lib/steamaudio/include/"))
    env.Append(LIBPATH=["src/lib/steamaudio/lib/linux-x64"])
    env.Append(LIBS=["libphonon.so"])
elif env["platform"] == "windows":
    env.AppendUnique(CCFLAGS=("/I",  "src/lib/steamaudio/include/"))
    env.Append(LIBPATH=["src/lib/steamaudio/lib/windows-x64"])
    env.Append(LIBS=["phonon"])


library = env.SharedLibrary(
    "project/addons/godot-steam-audio/bin/godot-steam-audio{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
