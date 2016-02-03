/*
 * Copyright (c) 2016 Bastien Brunnenstein
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

 /*
  * This file is only included when deploying a development build.
  * You should never call any of theses functions explicitly, except for
  * debugging purpose.
  */

var ian_dev = (function() {
  "use strict";

  var loading = false;
  var activeLoadQueue;

  var doLoadNext = function() {
    if (activeLoadQueue.length === 0) return;

    var filename = activeLoadQueue.splice(0, 1)[0];

    var script = document.createElement("script");
    script.setAttribute("type","text/javascript");
    script.setAttribute("src", filename);
    document.getElementsByTagName("head")[0].appendChild(script);

    loading = true;

    script.addEventListener("load", function() {
      loading = false;
      doLoadNext();
    });
  }

  return {

    // Load a list of javascript files
    load: function(files) {
      if (files.length === 0) return;

      if (!loading) {
        activeLoadQueue = files;
        doLoadNext();
      }
      else {
        activeLoadQueue = files.concat(activeLoadQueue);
      }
    }

  };
}());
