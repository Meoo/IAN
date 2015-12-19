newoption {
  trigger     = "websocketpp-dir",
  value       = "path",
  description = "Websocket++ base directory"
}

-- ///////////////////////////////////////////////////// --

local WEBSOCKETPP_DIR = _OPTIONS["websocketpp-dir"]

-- ASIO check
if WEBSOCKETPP_DIR then

  -- Check directory validity
  local f = io.open(WEBSOCKETPP_DIR .."/websocketpp/version.hpp", "r")
  if not f then
    premake.warn("Not a valid Websocket++ directory : ".. WEBSOCKETPP_DIR)
  else
    local major
    local minor
    local patch

    for l in f:lines() do
      if not major then
        major = l:match("^static int const major_version = ([0-9]+)")
      end
      if not minor then
        minor = l:match("^static int const minor_version = ([0-9]+)")
      end
      if not patch then
        patch = l:match("^static int const patch_version = ([0-9]+)")
      end

      if major and minor and patch then
        print("Websocket++ version : ".. major ..".".. minor ..".".. patch)
        break
      end
    end

    f:close()
  end
end

-- ///////////////////////////////////////////////////// --

mw.external "websocketpp"
  export "*"
    if WEBSOCKETPP_DIR then
      includedirs { WEBSOCKETPP_DIR }
    end

    import {
      ["asio"] = { "default" },
    }
