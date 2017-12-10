/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

function _ian_require_log(pack) {
  if (process.env.NODE_ENV !== 'production') {
    console.log('[IAN] Loading '+ pack);
  }
}
@IAN_GEN_JS_REQUIRES@
