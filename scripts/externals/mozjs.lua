newoption {
  trigger     = "mozjs-version",
  value       = "string",
  description = "SpiderMonkey version"
}

newoption {
  trigger     = "mozjs-dir",
  value       = "path",
  description = "SpiderMonkey installation directory"
}

newoption {
  trigger     = "mozjs-build-dir",
  value       = "path",
  description = "SpiderMonkey build directory"
}

-- ///////////////////////////////////////////////////// --

local MOZJS_VERSION     = _OPTIONS["mozjs-version"]

local MOZJS_INSTALL_DIR = _OPTIONS["mozjs-dir"]
local MOZJS_BUILD_DIR   = _OPTIONS["mozjs-build-dir"]

local MOZJS_INCLUDE_DIR
local MOZJS_NSPR_INCLUDE_DIR
local MOZJS_LIBS_DIR
local MOZJS_LINKS

-- Search for default in /usr/local
if not MOZJS_INSTALL_DIR and os.matchfiles("/usr/local/bin/js-config") == 1 then
  MOZJS_INSTALL_DIR = "/usr/local"
end

-- Try to find in install directory
if MOZJS_INSTALL_DIR then
  -- Auto detect version
  if not MOZJS_VERSION then
    local incdirs = os.matchdirs(MOZJS_INSTALL_DIR .."/include/mozjs-*")

    if #incdirs == 1 then
      MOZJS_VERSION = incdirs[1]:match("%-([%.0-9a-z]*)$")
      if MOZJS_VERSION then
        print("SpiderMonkey version : ".. MOZJS_VERSION)
      end
    elseif #incdirs == 0 then
      premake.warn("Not a valid SpiderMonkey installation : ".. MOZJS_INSTALL_DIR)
    else
      premake.warn("Multiple SpiderMonkey versions detected : ".. MOZJS_INSTALL_DIR)
    end
  end

  if MOZJS_VERSION then
    MOZJS_INCLUDE_DIR = MOZJS_INSTALL_DIR .."/include/mozjs-".. MOZJS_VERSION
    MOZJS_LIBS_DIR = MOZJS_INSTALL_DIR .."/lib"

    -- Check libraries
    if #os.matchfiles(MOZJS_LIBS_DIR .."/mozjs-".. MOZJS_VERSION ..".*") == 0
    and #os.matchfiles(MOZJS_LIBS_DIR .."/libmozjs-".. MOZJS_VERSION ..".*") == 0 then
      premake.warn("No SpiderMonkey libraries found : ".. MOZJS_LIBS_DIR)
    end

    -- Open js-config and locate libs and nspr include dir
    local f = io.open(MOZJS_INSTALL_DIR .."/bin/js-config", "r")
    if f then
      local nsprinc
      local libs

      for l in f:lines() do
        if not nsprinc then
          nsprinc = l:match("^NSPR_CFLAGS='%-I(.*)'$")
        end
        if not libs then
          libs = l:match("^JS_CONFIG_LIBS='(.*)'$")
        end

        if nsprinc and libs then break end
      end

      f:close()

      MOZJS_NSPR_INCLUDE_DIR = nsprinc
      MOZJS_LINKS = string.explode(libs, " +") -- TODO Remove -l and empty
      table.insert(MOZJS_LINKS, 1, "mozjs-".. MOZJS_VERSION)
    else
      premake.warn("Could not find js-config in ".. MOZJS_INSTALL_DIR .."/bin")
    end
  end
end

-- Try to find in build directory
if MOZJS_BUILD_DIR then
  -- Auto detect version
  if not MOZJS_VERSION then
    local mozlibs = os.matchfiles(MOZJS_BUILD_DIR .."/dist/lib/mozjs-*")

    if #mozlibs == 1 then
      MOZJS_VERSION = mozlibs[1]:match("%-([%.0-9a-z]*)%.[liba]+$")
      if MOZJS_VERSION then
        print("SpiderMonkey version : ".. MOZJS_VERSION)
      end
    elseif #mozlibs == 0 then
      premake.warn("Not a valid SpiderMonkey build directory : ".. MOZJS_BUILD_DIR)
    else
      premake.warn("Multiple SpiderMonkey versions detected : ".. MOZJS_BUILD_DIR)
    end
  end

  if MOZJS_VERSION then
    MOZJS_INCLUDE_DIR = MOZJS_BUILD_DIR .."/dist/include"
    MOZJS_LIBS_DIR = MOZJS_BUILD_DIR .."/dist/lib"

    -- Open js-config and locate libs and nspr include dir
    local f = io.open(MOZJS_BUILD_DIR .."/js/src/js-config", "r")
    if f then
      local nsprinc
      local libs

      local l = f:read("*l")
      while l do
        if not nsprinc then
          nsprinc = l:match("^NSPR_CFLAGS='%-I(.*)'$")
        end
        if not libs then
          libs = l:match("^JS_CONFIG_LIBS='(.*)'$")
        end

        if nsprinc and libs then break end

        l = f:read("*l")
      end

      f:close()

      MOZJS_NSPR_INCLUDE_DIR = nsprinc
      MOZJS_LINKS = string.explode(libs, " +") -- TODO Remove -l and empty
      table.insert(MOZJS_LINKS, 1, "mozjs-".. MOZJS_VERSION)
    else
      premake.warn("Could not find js-config in ".. MOZJS_BUILD_DIR .."/js/src")
    end
  end
end

-- If we only have version
if MOZJS_VERSION and not MOZJS_LINKS then
  MOZJS_LINKS = "mozjs-".. MOZJS_VERSION
end

-- ///////////////////////////////////////////////////// --

ian.external "mozjs"
  export "*"
    includedirs ( MOZJS_INCLUDE_DIR )
    includedirs ( MOZJS_NSPR_INCLUDE_DIR )
    libdirs     ( MOZJS_LIBS_DIR )
    links       ( MOZJS_LINKS )
