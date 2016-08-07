-- premake5.project.lua
project "Main_TCS_2015"
   kind "ConsoleApp"
   language "C++"
   targetdir ("../../bin")
   debugdir "../../bin"
   libdirs {
      "../../lib/%{cfg.platform}/%{cfg.buildcfg}"
   }
   links {
      "GraphLib"
   }
   flags { "EnableSSE", "EnableSSE2" }

   files {
      "**.hpp",
      "**.h",
      "**.cpp",
      "**.c"
   }
   includedirs {
      "../../include"
   }
