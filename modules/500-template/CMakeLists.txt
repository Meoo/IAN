
# Remove this line to enable the file
return()


# Global


# Please prefix all you projects and libraries with "module-"
project(module-template)

# Minimal IAN version required
ian_version_required(1.0)

# Declare version for this module
ian_module_version(template 1.0.0.0)

# Check version of another module
#ian_module_version_required(other-module 1.0)



# Server

# Use this instead of add_library
# The library will be linked to the module bundle
# Module SDK is also linked to this project automatically
ian_server_module(module-template
  src/TemplateModule.hpp
  src/TemplateModule.cpp)

# Declare header(s) for your module classe(s)
ian_server_add_module_header(src/TemplateModule.hpp)

# Class has to be in the module namespace and implement IModule: "module::Template"
ian_server_add_module_class(Template)



# Client


# Add HTML chunk to client
#ian_client_add_body(template.html)

# Add JS file to client
#ian_client_add_js(template.js)

# Add CSS file to client
#ian_client_add_css(template.css)
