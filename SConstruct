#!/usr/bin/env python

env = SConscript("src/lib/godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])

if env.get("CC", "").lower() == "cl":
    # Building with MSVC
    env.AppendUnique(CCFLAGS=("/I",  "src/lib/steamaudio/include/"))
else:
    env.AppendUnique(CCFLAGS=("-isystem",  "src/lib/steamaudio/include/"))

sources = Glob("src/*.cpp")

if env["platform"] == "linux":
    env.Append(LIBPATH=["src/lib/steamaudio/lib/linux-x64"])
    env.Append(LIBS=["libphonon.so"])
elif env["platform"] == "windows":
    env.Append(LIBPATH=["src/lib/steamaudio/lib/windows-x64"])
    env.Append(LIBS=["phonon"])


library = env.SharedLibrary(
    "project/addons/godot-steam-audio/bin/godot-steam-audio{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
