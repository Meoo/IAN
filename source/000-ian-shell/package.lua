
local id = ian.deploy

local is = {}
ian.shell = is

local DEV_JS = path.getabsolute("dev.js")

local CR = id.dev and "\n" or ""


-- ####################################
-- ###      HTML
-- ####################################

is.html = id.transformedTextFile("client/index.html", "shell.html")

is.html.transform.TITLE = id.getOption("ian.shell.title", "IAN Client")

is.html.transform.META = function()
  local s = ""
  for k, v in pairs(id.getOption("ian.shell.meta", {})) do
    s = s .."<meta name=\"".. k .."\" content=\"".. v .."\">".. CR
  end
  return s
end

is.html.cssFiles = {}

is.html.transform.CSS = function()
  local s = ""
  for _, v in ipairs(is.html.cssFiles) do
    s = s .."<link rel=\"stylesheet\" type=\"text/css\" href=\"".. v .."\">".. CR
  end
  return s
end

is.html.jsFiles = {}

is.html.transform.JS = function()
  local s = ""

  if id.release then
    -- Release
    for _, v in ipairs(is.html.jsFiles) do
      s = s .."<script type=\"text/javascript\" src=\"".. v .."\"></script>"
    end

  else
    -- Development
    s = "<script type=\"text/javascript\" src=\"file://".. DEV_JS .."\"></script>\n"
    s = s .."<script type=\"text/javascript\">\n"
    s = s .."ian_dev.load([".. table.implode(is.html.jsFiles, "\"", "\"", ",\n") .."]);\n"
    s = s .."</script>\n"
  end

  return s
end

-- ####################################
-- ###      JS
-- ####################################

-- Generate options first, compiled into the main file
id.generatedText("temp/shell-vars.js", function(f)
    f:write("// Generated file, do not modify manually\n")

    local options = id.getOption("ian.shell.jscfg", {})

    local optlist = {}
    for k, v in pairs(options) do
      if type(v) == "number" then
        table.insert(optlist, k..": ".. v)
      else
        table.insert(optlist, k..": \"".. tostring(v) .."\"")
      end
    end

    f:write("var ian_cfg = {".. table.implode(optlist, "", "", ",") .."};")
  end)

-- Main file
is.js = id.compiledJs("client/ian.js")
is.js:addFiles("shell.js")
is.js:addFiles(id.directory .."/temp/shell-vars.js")

local JQUERY_VERSION = id.getOption("ian.shell.jQuery", "2.1.4")

table.insert(is.html.jsFiles, "https://code.jquery.com/jquery-".. JQUERY_VERSION ..".min.js")
table.insert(is.html.jsFiles, "ian.js") -- relative to client/

-- ####################################
-- ###      CSS
-- ####################################

-- Main file
is.css = id.compiledCss("client/ian.css")
is.css:addFiles("shell.css")

local FONTAWESOME_VERSION = id.getOption("ian.shell.fontawesome", "4.5.0")

table.insert(is.html.cssFiles, "https://maxcdn.bootstrapcdn.com/font-awesome/".. FONTAWESOME_VERSION .."/css/font-awesome.min.css")
table.insert(is.html.cssFiles, "ian.css") -- relative to client/
