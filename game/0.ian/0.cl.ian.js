var ian = {};

// Called when all scripts are loaded
// Can be used to preload resources
ian.onScriptsReady = function () {
  jQuery("noscript").remove();
}

// Called when everything has been loaded
ian.onWindowReady = function() {
  // Hide loading icon and show login form
  jQuery("#ian-loading").hide();
  jQuery("#ian-login").show();
  jQuery("#ian-settings-button").show();
}
