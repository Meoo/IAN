
# jQuery
set(IAN_JQUERY_VERSION "3.2.1.slim.min")

# Fontawesome
set(IAN_FONTAWESOME_VERSION "4.7.0")


ian_client_add_body(core.html)

ian_client_add_js("https://code.jquery.com/jquery-${IAN_JQUERY_VERSION}.js")
ian_client_add_js(core.js)
ian_client_add_js(network.js)
ian_client_add_js(window-manager.js)

ian_client_add_css("https://maxcdn.bootstrapcdn.com/font-awesome/${IAN_FONTAWESOME_VERSION}/css/font-awesome.min.css")
ian_client_add_css(core.css)
ian_client_add_css(notifications.css)
