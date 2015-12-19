newoption {
  trigger     = "sfml2-dir",
  value       = "path",
  description = "SFML2 base directory (must contain include/ and extlibs/)"
}

newoption {
  trigger     = "sfml2-libs-dir",
  value       = "path",
  description = "SFML2 libraries directory"
}

newoption {
  trigger     = "sfml2-static",
  value       = "bool",
  description = "Link SFML2 as a static library",
  allowed = {
    { "true",   "Static linking" },
    { "false",  "Dynamic linking (default)" },
  }
}

newoption {
  trigger     = "sfml2-debug",
  value       = "string",
  description = "Link SFML2 debug version",
  allowed = {
    { "true",   "Use debug version" },
    { "false",  "Use release version (default)" },
    { "auto",   "Only use debug for debug builds" },
  }
}

-- ///////////////////////////////////////////////////// --

local SFML_DIR          = _OPTIONS["sfml2-dir"] or ""
local SFML_LIBS_DIR
local SFML_LIBS_DIR_VS  = false

local SFML_STATIC       = mw.booloption("sfml2-static", false)

local SFML_DEBUG_AUTO   = _OPTIONS["sfml2-debug"] == "auto"
local SFML_DEBUG        = SFML_DEBUG_AUTO or mw.booloption("sfml2-debug", false)


-- SFML2 check
if SFML_DIR ~= "" then
  -- Try to find libs directory if not set
  if _OPTIONS["sfml2-libs-dir"] then
    SFML_LIBS_DIR = _OPTIONS["sfml2-libs-dir"]

  elseif #os.matchdirs(SFML_DIR.."/bin/lib") > 0 then
    SFML_LIBS_DIR = SFML_DIR.."/bin/lib"

  else
    SFML_LIBS_DIR = SFML_DIR.."/lib"
  end

  -- Check directory validity
  local f = io.open(SFML_DIR .."/include/SFML/Config.hpp", "r")
  if not f then
    premake.warn("Not a valid SFML2 directory : ".. SFML_DIR)
  else
    local major
    local minor
    local patch

    for l in f:lines() do
      if not major then
        major = l:match("^#define SFML_VERSION_MAJOR ([0-9]+)")
      end
      if not minor then
        minor = l:match("^#define SFML_VERSION_MINOR ([0-9]+)")
      end
      if not patch then
        patch = l:match("^#define SFML_VERSION_PATCH ([0-9]+)")
      end

      if major and minor and patch then
        print("SFML version : ".. major ..".".. minor ..".".. patch)
        break
      end
    end

    f:close()
  end

  if #os.matchfiles(SFML_LIBS_DIR .."/libsfml-*") == 0 then

    if (#os.matchdirs(SFML_LIBS_DIR .."/Debug")
       + #os.matchdirs(SFML_LIBS_DIR .."/Release")) ~= 0 then
      SFML_LIBS_DIR_VS = true

      if #os.matchfiles(SFML_LIBS_DIR .."/*/sfml-*") == 0 then
        premake.warn("No SFML2 libraries in directory : ".. SFML_LIBS_DIR)
      end
    else
      premake.warn("No SFML2 libraries in directory : ".. SFML_LIBS_DIR)
    end
  end
end

-- ///////////////////////////////////////////////////// --

local function sfmllib(name)
  export(name)
    local s = ""
    if SFML_STATIC then
      s = "-s"
    end

    if SFML_DEBUG_AUTO then
      -- Auto
      filter "configurations:Debug"
        if name ~= "main" then
            links("sfml-" .. name .. s .. "-d")
        else
            links("sfml-" .. name .. "-d")
        end

      filter "configurations:Release"
        if name ~= "main" then
            links("sfml-" .. name .. s)
        else
            links("sfml-" .. name)
        end

      filter {}

    elseif SFML_DEBUG then
      -- Debug
      if name ~= "main" then
          links("sfml-" .. name .. s .. "-d")
      else
          links("sfml-" .. name .. "-d")
      end

    else
      -- Release
      if name ~= "main" then
          links("sfml-" .. name .. s)
      else
          links("sfml-" .. name)
      end
    end
end

mw.external "sfml2"
  export "*"
    -- SFML paths
    includedirs { SFML_DIR .."/include" }
    
    if SFML_LIBS_DIR_VS then
      filter "configurations:Debug"
        libdirs   { SFML_LIBS_DIR .."/Debug" }
      filter "configurations:Release"
        libdirs   { SFML_LIBS_DIR .."/Release" }
    else
      libdirs     { SFML_LIBS_DIR }
    end

    -- Flags
    if SFML_STATIC then
      defines { "SFML_STATIC" }
    else
      defines { "SFML_DYNAMIC" }
    end

    -- External libs paths
    -- Will cause warnings when linking if you do not specify a platform
    filter { "action:VS*", "architecture:x86" }
      libdirs { SFML_DIR .."/extlibs/libs-msvc/x86/" }

    filter { "action:VS*", "architecture:x86_64" }
      libdirs { SFML_DIR .."/extlibs/libs-msvc/x64/" }

    filter { "system:Windows", "action:GMake", "architecture:x86" }
      libdirs { SFML_DIR .."/extlibs/libs-mingw/x86/" }

    filter { "system:Windows", "action:GMake", "architecture:x86_64" }
      libdirs { SFML_DIR .."/extlibs/libs-mingw/x64/" }

  -- Exported libs
  sfmllib "network"
    import { ["sfml2"] = "system" }

    filter "system:Windows"
      links { "ws2_32" }


  sfmllib "audio"
    import { ["sfml2"] = "system" }

    filter "system:Windows"
      links { "openal32", "FLAC", "vorbisfile", "vorbisenc", "vorbis", "ogg" }

    filter "system:Linux"
      links { "OpenAL", "FLAC", "vorbisfile", "vorbisenc", "vorbis", "ogg" }


  sfmllib "graphics"
    import { ["sfml2"] = "window" }

    filter "system:Windows"
      links { "opengl32", "freetype", "jpeg" }

    filter "system:Linux"
      links { "GL", "GLEW", "freetype", "jpeg" }


  sfmllib "window"
    import { ["sfml2"] = "system" }

    filter "system:Windows"
      links { "winmm", "gdi32" }

    filter "system:Linux"
      links { "X11", "Xrandr" }


  sfmllib "system"
    filter "system:Linux"
      links { "pthread" }


  sfmllib "main"
