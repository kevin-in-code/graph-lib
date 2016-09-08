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
   flags { "Symbols" }

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

programDirs = os.matchdirs("programs/*")
for index,dir in ipairs(programDirs) do
   local path = dir.."/premake5.inc"
   if os.isfile(path) then
      include(path)
   end
end
