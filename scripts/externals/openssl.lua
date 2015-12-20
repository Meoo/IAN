
newoption {
  trigger     = "openssl-dir",
  value       = "path",
  description = "OpenSSL installation directory"
}

newoption {
  trigger     = "openssl-suffix",
  value       = "string",
  description = "OpenSSL library suffix"
}

-- ///////////////////////////////////////////////////// --

local OPENSSL_DIR = _OPTIONS["openssl-dir"]
local OPENSSL_LIB_SUFFIX = _OPTIONS["openssl-suffix"]

local OPENSSL_64_DIRS = false

local OPENSSL_INCLUDE_DIR
local OPENSSL_LIBS_DIR
local OPENSSL_LINKS = { "libeay32", "ssleay32" }

if OPENSSL_LIB_SUFFIX then
  OPENSSL_LINKS =
    table.translate(OPENSSL_LINKS, function(v) return v..OPENSSL_LIB_SUFFIX end)
end

-- Try to find in install directory
if OPENSSL_DIR then

  -- Try to find 64 bit version first
  local config = os.matchfiles(OPENSSL_DIR .."/include64/openssl/opensslv.h")

  if #config == 0 then
    -- Otherwise get normal version
    config = os.matchfiles(OPENSSL_DIR .."/include/openssl/opensslv.h")
  else
    OPENSSL_64_DIRS = true
  end

  if #config == 0 then
    premake.warn("Not a valid OpenSSL installation : ".. OPENSSL_DIR)
  else
    OPENSSL_INCLUDE_DIR = OPENSSL_DIR .."/include"
    OPENSSL_LIBS_DIR = OPENSSL_DIR .."/lib"

    local f = io.open(OPENSSL_DIR .."/include/openssl/opensslv.h", "r")
    if f then
      local version

      for l in f:lines() do
        version = l:match("OPENSSL_VERSION_TEXT.*OpenSSL (%d+%.%d+%.%d+%l?)")

        if version then
          print("OpenSSL version : ".. version)
          break
        end
      end

      f:close()
    end

  end
end

-- ///////////////////////////////////////////////////// --

mw.external "openssl"
  export "*"

    if OPENSSL_64_DIRS then

      filter { "architecture:x86" }
        libdirs ( OPENSSL_LIBS_DIR )
        includedirs ( OPENSSL_INCLUDE_DIR )

      filter { "architecture:x86_64" }
        if OPENSSL_LIBS_DIR then
          libdirs ( OPENSSL_LIBS_DIR .."64" )
        end
        if OPENSSL_INCLUDE_DIR then
          includedirs ( OPENSSL_INCLUDE_DIR .."64" )
        end

      filter {}
    else

      libdirs ( OPENSSL_LIBS_DIR )
      includedirs ( OPENSSL_INCLUDE_DIR )
    end

    links       ( OPENSSL_LINKS )
