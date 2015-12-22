
var CLIENT = true;
var SERVER = false;
var ian = {};

// Called when all scripts are loaded
// Can be used to preload resources
ian.onScriptsReady = function () {
  jQuery("noscript").remove();

  // Attach handlers
  // Login button
  jQuery("#ian-login").submit(function(event) {

    // Call connect then clear the password field
    ian_net.connect(SERVER_ADDR, {
      user: jQuery("#ian-login-user").val(),
      pass: jQuery("#ian-login-pass").val() });
    jQuery("#ian-login-pass").val("");

    event.preventDefault();
    event.stopImmediatePropagation();
  });
}

// Called when everything has been loaded
ian.onWindowReady = function() {
  // Hide loading icon and show login form
  jQuery("#ian-loading").hide();
  jQuery("#ian-login").show();
  jQuery("#ian-settings-button").show();
}
