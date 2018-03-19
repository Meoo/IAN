#
# This file is part of the IAN project - https://github.com/Meoo/IAN
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Input:
# HASH_FILES : files to hash
# HASH_OUT : generated header
# HASH_DEF : define name

separate_arguments(HASH_FILES)

string(SHA1 HASH "default")

foreach(FILE ${HASH_FILES})
  file(SHA1 ${FILE} FILE_HASH)
  string(SHA1 HASH "${HASH} ${FILE} ${FILE_HASH}")
endforeach()

set(HASH_LEN 0)
set(HASH_BIN "")
set(HASH_C "${HASH}")

while(HASH_C)
  math(EXPR HASH_LEN "${HASH_LEN}+1")
  string(SUBSTRING "${HASH_C}" 0 2 CHAR)
  string(SUBSTRING "${HASH_C}" 2 -1 HASH_C)
  if(HASH_BIN)
    set(HASH_BIN "${HASH_BIN}, ")
  endif()
  set(HASH_BIN "${HASH_BIN}0x${CHAR}")
endwhile()

file(WRITE ${HASH_OUT} "#define ${HASH_DEF}_STR \"${HASH}\"\n#define ${HASH_DEF}_BIN ${HASH_BIN}\n#define ${HASH_DEF}_LEN ${HASH_LEN}\n")
