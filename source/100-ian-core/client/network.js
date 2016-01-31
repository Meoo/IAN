/*
 * Copyright (c) 2016 Bastien Brunnenstein
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

var ian_net = (function() {

  // Keep a local reference of sensitive data
  var GAME_MAGIC = ian_cfg.magic;

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

    if (event.reason !== "") {
      ian_wm.notify("Connection closed : " + event.reason, "error");
    }
    else {
      if (event.code === 1000) {
        ian_wm.notify("Disconnected from server");
      }
      else if (event.code === 1006) {
        ian_wm.notify("Connection error", "error");
      }
      else {
        ian_wm.notify("net:onClose " + event.code);
      }
    }
  }

  var onError = function(event) {
    ian_wm.notify("net:onError");
  }

  var onMessage = function(event) {

  }

  var onHandshakeMessage = function(event) {
    try {
      var data = new DataView(event.data);

      // Received magic
      var k1 = data.getInt32(0);
      var k2 = data.getInt32(4);
      var k3 = data.getInt32(8);
      var k4 = data.getInt32(12);

      // Our magic
      var m1 = parseInt(GAME_MAGIC.slice(0, 7), 16) << 0;
      var m2 = parseInt(GAME_MAGIC.slice(8, 15), 16) << 0;
      var m3 = parseInt(GAME_MAGIC.slice(16, 23), 16) << 0;
      var m4 = parseInt(GAME_MAGIC.slice(24, 31), 16) << 0;

      // Check our magic against received magic
      if (k1 === m1 && k2 === m2 && k3 === m3 && k4 === m4) {

        // Magic is valid
        ian_wm.notify("net:onHandshakeMessage good magic", "info");

        // TODO Send credentials

        socket.onmessage = onMessage;
      }
      else {
        ian_wm.notify("net:onHandshakeMessage bad magic", "error");
        socket.close(); // TODO Error code
      }
    }
    catch(err) {
      ian_wm.notify("net:onHandshakeMessage exception", "error");
      socket.close(); // TODO Error code
    }
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
