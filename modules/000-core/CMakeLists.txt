
# jQuery
set(IAN_JQUERY_VERSION "3.2.1.slim.min")

# Fontawesome
set(IAN_FONTAWESOME_VERSION "4.7.0")


ian_client_add(
  BODY
    core.html
  JS
    "https://code.jquery.com/jquery-${IAN_JQUERY_VERSION}.js"
    core.js
    network.js
    window-manager.js
  CSS
    "https://maxcdn.bootstrapcdn.com/font-awesome/${IAN_FONTAWESOME_VERSION}/css/font-awesome.min.css"
    core.css
    notifications.css
  )
