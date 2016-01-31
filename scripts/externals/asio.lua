newoption {
  trigger     = "asio-dir",
  value       = "path",
  description = "ASIO base directory (must contain include/)"
}

-- ///////////////////////////////////////////////////// --

local ASIO_DIR = _OPTIONS["asio-dir"]

-- ASIO check
if ASIO_DIR then

  -- Check directory validity
  local f = io.open(ASIO_DIR .."/include/asio/version.hpp", "r")
  if not f then
    premake.warn("Not a valid ASIO directory : ".. ASIO_DIR)
  else
    for l in f:lines() do
      local version = l:match("^#define ASIO_VERSION ([0-9]+)")

      if version then
        version = tonumber(version)
        print("ASIO version : ".. math.floor(version / 100000) ..".".. math.floor(version / 100 % 1000) ..".".. (version % 100))
        break
      end
    end
  end
end

-- ///////////////////////////////////////////////////// --

ian.external "asio"
  export "*"
    if ASIO_DIR then
      includedirs { ASIO_DIR .."/include" }
    end
