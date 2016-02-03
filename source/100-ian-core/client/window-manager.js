/*
 * Copyright (c) 2016 Bastien Brunnenstein
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

var ian_wm = (function() {
  "use strict";

  // Create a div to contain and display notifications
  var notifContainer = jQ("<div>", {id:"ian-notif-container"}).appendTo("body");

  // ian_wm public functions
  return {

    // Send a notification to the user
    notify: function(message, cssclass, timeout) {
      // Create notification
      var notif = jQ("<p>")
        .addClass("ian-notif")
        .text(message)
        .appendTo(notifContainer);

      if (typeof(cssclass) !== "undefined") notif.addClass("ian-notif-"+cssclass);
      if (typeof(timeout) == "undefined") timeout = 15;

      // TODO Icons ?
      /*if (cssclass === "error") {
        jQ("<i>").addClass("fa").addClass("fa-fw")
          .addClass("fa-exclamation-triangle").prependTo(notif);
      }*/

      // Dismiss on click
      notif.click(function() {
        notif.fadeOut(200, function() {
          notif.remove();
        });
      });

      // Dismiss on timeout
      if (timeout > 0) {
        setTimeout(function() {
          notif.fadeOut(600, function() {
            notif.remove();
          });
        }, timeout * 1000);
      }
    }

  };
}());
