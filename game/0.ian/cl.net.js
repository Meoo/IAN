
const SERVER_ADDR = "wss://sandbox.kaazing.net/echo";

const ian_net = (function() {

  // Websocket instance
  var socket;

  // Websocket callbacks
  var onOpen = function(event) {
    jQuery("#ian-loading-text").text("Connected");

    alert("onOpen " + event.data);
  }

  var onClose = function(event) {
    jQuery("#ian-login").show();
    jQuery("#ian-loading").hide();

    alert("onClose " + event.code + " " + event.reason);
  }

  var onError = function(event) {
    alert("onError");
  }

  var onMessage = function(event) {

    alert("onMessage " + event.data);
  }

  // ian_net public functions
  return {

    // Called when the user press the "Connect" button on the login form
    connect: function(credentials) {
      // Hide the login form and show a wait message
      jQuery("#ian-login").hide();
      jQuery("#ian-loading-text").text("Connecting");
      jQuery("#ian-loading").show();

      socket = new WebSocket(SERVER_ADDR);
      socket.binaryType = "arraybuffer";
      socket.onopen = onOpen;
      socket.onclose = onClose;
      socket.onerror = onError;
      socket.onmessage = onMessage;
    },

  };
}());
