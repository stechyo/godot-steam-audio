#!/usr/bin/env python

env = SConscript("src/lib/godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])
env.AppendUnique(CCFLAGS=("-isystem",  "src/lib/steamaudio/include/"))
sources = Glob("src/*.cpp")

if env["platform"] == "linux":
    env.Append(LIBPATH=["src/lib/steamaudio/lib/linux-x64"])
    env.Append(LIBS=["libphonon.so"])
elif env["platform"] == "windows":
    env.Append(LIBPATH=["src/lib/steamaudio/lib/windows-x64"])
    env.Append(LIBS=["phonon.dll"])


library = env.SharedLibrary(
    "project/bin/godot-steam-audio{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
