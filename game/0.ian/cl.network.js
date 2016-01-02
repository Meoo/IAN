
var ian_net = (function() {

  // Websocket instance
  var socket;

  // Websocket callbacks
  var onOpen = function(event) {
    jQ("#ian-loading-text").text("Connected");

    // Send handshake
  }

  var onClose = function(event) {
    jQ("#ian-login").show();
    jQ("#ian-loading").hide();

    ian_wm.notify("onClose " + event.code + " " + event.reason);
  }

  var onError = function(event) {
    ian_wm.notify("onError", "error");
  }

  var onHandshakeMessage = function(event) {

    socket.onmessage = onMessage;
  }

  var onMessage = function(event) {

    ian_wm.notify("onMessage " + event.data);
  }

  // ian_net public functions
  return {

    // Called when the user press the "Connect" button on the login form
    connect: function(server, credentials) {
      // Hide the login form and show a wait message
      jQ("#ian-login").hide();
      jQ("#ian-loading-text").text("Connecting");
      jQ("#ian-loading").show();

      socket = new WebSocket(server);
      socket.binaryType = "arraybuffer";
      socket.onopen = onOpen;
      socket.onclose = onClose;
      socket.onerror = onError;
      socket.onmessage = onHandshakeMessage;
    }

  };
}());
