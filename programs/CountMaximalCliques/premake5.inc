-- premake5.project.lua
project "CountMaximalCliques"
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
      "programs/%{prj.name}/**.hpp",
      "programs/%{prj.name}/**.h",
      "programs/%{prj.name}/**.cpp",
      "programs/%{prj.name}/**.c"
   }
   includedirs {
      "include"
   }
