/*
 * Copyright (c) 2016 Bastien Brunnenstein
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

var CLIENT = true;
var SERVER = false;
var ian = {};

// Called when everything has been loaded
jQ(window).load(function() {

  jQ("noscript").remove();

  // Attach handlers
  // Login button
  jQ("#ian-login").submit(function(event) {

    // Call connect then clear the password field
    ian_net.connect("wss://" + ian_cfg.server_addr + ":" + ian_cfg.server_port, {
      user: jQ("#ian-login-user").val(),
      pass: jQ("#ian-login-pass").val() });
    jQ("#ian-login-pass").val("");

    event.preventDefault();
    event.stopImmediatePropagation();
  });

  // Hide loading icon and show login form
  jQ("#ian-loading").hide();
  jQ("#ian-login").show();
  jQ("#ian-settings-button").show();

});