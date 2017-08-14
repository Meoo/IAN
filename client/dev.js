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

  var scriptsLoadQueue;
  var htmlIncludes;


  var doLoadNextScript = function() {
    if (scriptsLoadQueue.length === 0) return;

    var filename = scriptsLoadQueue.splice(0, 1)[0];

    var script = document.createElement("script");
    script.setAttribute("type","text/javascript");
    script.setAttribute("src", filename);
    document.getElementsByTagName("head")[0].appendChild(script);

    script.addEventListener("load", function() {
      doLoadNextScript();
    });
  }


  return {

    // Load a list of javascript files
    load: function(files) {
      // Delete itself
      ian_dev.load = null;

      scriptsLoadQueue = files;
      doLoadNextScript();
    }

  };
}());
