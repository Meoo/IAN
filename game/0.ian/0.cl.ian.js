
var CLIENT = true;
var SERVER = false;
var ian = {};

// Called when all scripts are loaded
// Can be used to preload resources
ian.onScriptsReady = function () {
  jQ("noscript").remove();

  // Attach handlers
  // Login button
  jQ("#ian-login").submit(function(event) {

    // Call connect then clear the password field
    ian_net.connect(SERVER_ADDR, {
      user: jQ("#ian-login-user").val(),
      pass: jQ("#ian-login-pass").val() });
    jQ("#ian-login-pass").val("");

    event.preventDefault();
    event.stopImmediatePropagation();
  });
}

// Called when everything has been loaded
ian.onWindowReady = function() {
  // Hide loading icon and show login form
  jQ("#ian-loading").hide();
  jQ("#ian-login").show();
  jQ("#ian-settings-button").show();
}
