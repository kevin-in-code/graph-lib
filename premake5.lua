-- premake5.lua
workspace "GraphLib"
   configurations { "Debug", "Release" }
   platforms { "Win64", "Win32" }
   location (_ACTION)

filter "configurations:Debug*"
   defines { "DEBUG" }
   flags { "Symbols" }

filter "configurations:Release*"
   defines { "NDEBUG" }
   optimize "On"

filter { "platforms:Win64" }
   system "Windows"
   architecture "x64"

filter { "platforms:Win32" }
   system "Windows"
   architecture "x86"

project "GraphLib"
   kind "StaticLib"
   language "C++"
   targetdir ("lib/%{cfg.platform}/%{cfg.buildcfg}")

   flags { "EnableSSE", "EnableSSE2" }

   files {
      "include/**.hpp",
      "include/**.h",
      "src/**.hpp",
      "src/**.h",
      "src/**.cpp",
      "src/**.c"
   }
   includedirs {
      "include"
   }

project "Main_TCS_2015"
   kind "ConsoleApp"
   language "C++"
   targetdir ("bin")
   debugdir "bin"
   libdirs {
      "lib/%{cfg.platform}/%{cfg.buildcfg}"
   }
   links {
      "GraphLib"
   }
   flags { "EnableSSE", "EnableSSE2" }

   files {
      "tests/%{prj.name}/**.hpp",
      "tests/%{prj.name}/**.h",
      "tests/%{prj.name}/**.cpp",
      "tests/%{prj.name}/**.c"
   }
   includedirs {
      "include"
   }
