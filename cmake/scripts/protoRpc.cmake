
# Input:
# RPC_DESCRIPTOR : descriptor .cmake file
# RPC_HEADER_OUT : output header
# RPC_FBS_OUT : output flatbuffers file


# IDL

set(RPC_IF_LIST)

macro(rpc_namespace NS)
  set(RPC_NAMESPACE ${NS})
endmacro()

macro(rpc_interface IF)
  set(RPC_CURRENT_IF ${IF})
  list(APPEND RPC_IF_LIST ${IF})
  set(RPC_D_${IF}_FUNCTIONS)
endmacro()

macro(rpc_function)
  set(opts)
  set(oneArgs NAME)
  set(multiArgs IN OUT)
  cmake_parse_arguments(ARG "${opts}" "${oneArgs}" "${multiArgs}" ${ARGN})

  if(NOT RPC_CURRENT_IF)
    message(FATAL_ERROR "rpc_function called with no active interface")
  endif()

  if(NOT ARG_NAME)
    message(FATAL_ERROR "rpc_function called with no name in ${RPC_CURRENT_IF}")
  endif()

  list(APPEND RPC_D_${RPC_CURRENT_IF}_FUNCTIONS ${ARG_NAME})

  set(RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_IN_TYPES)
  set(RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_IN_VARS)
  set(RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_IN_PAIRS)
  set(RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_OUT_TYPES)
  set(RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_OUT_VARS)
  set(RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_OUT_PAIRS)

  set(NEXT_VAR OFF)
  foreach(IN_V ${ARG_IN})
    if(NOT NEXT_VAR)
      set(IN_T ${IN_V})
      list(APPEND RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_IN_TYPES ${IN_V})
      set(NEXT_VAR ON)
    else()
      list(APPEND RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_IN_VARS ${IN_V})
      list(APPEND RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_IN_PAIRS "${IN_T} ${IN_V}")
      set(NEXT_VAR OFF)
    endif()
  endforeach()
  if(NEXT_VAR)
    message(FATAL_ERROR "Malformed IN for ${RPC_CURRENT_IF}.${ARG_NAME}")
  endif()

  foreach(OUT_V ${ARG_OUT})
    if(NOT NEXT_VAR)
      set(OUT_T ${OUT_V})
      list(APPEND RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_OUT_TYPES ${OUT_V})
      set(NEXT_VAR ON)
    else()
      list(APPEND RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_OUT_VARS ${OUT_V})
      list(APPEND RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_OUT_PAIRS "${OUT_T} ${OUT_V}")
      set(NEXT_VAR OFF)
    endif()
  endforeach()
  if(NEXT_VAR)
    message(FATAL_ERROR "Malformed OUT for ${RPC_CURRENT_IF}.${ARG_NAME}")
  endif()
endmacro()


#

include(${RPC_DESCRIPTOR})

#


if(NOT RPC_NAMESPACE)
  message(FATAL_ERROR "A namespace must be defined")
endif()


# Generate .fbs
file(WRITE "${RPC_FBS_OUT}" "// Generated file\n")
file(APPEND "${RPC_FBS_OUT}" "namespace ${RPC_NAMESPACE};\n\n")

set(MSG_TYPES)
foreach(IF ${RPC_IF_LIST})
  list(APPEND MSG_TYPES "${IF}Request" "${IF}Reply")
endforeach()
string(REPLACE ";" ", " UNION_LIST "${MSG_TYPES}")
file(APPEND "${RPC_FBS_OUT}" "union RpcMessage {\n  ${UNION_LIST}\n}\n")
file(APPEND "${RPC_FBS_OUT}" "table RpcMessageT { message:RpcMessage; }\n\n")

# Loop on interfaces
foreach(IF ${RPC_IF_LIST})
  file(APPEND "${RPC_FBS_OUT}" "table ${IF}Request {}\n")
  file(APPEND "${RPC_FBS_OUT}" "table ${IF}Reply {}\n\n")
endforeach()


# Generate .hpp
macro(write_fun_receiver FILE IF FUN)
  set(FUN_ARGS ${RPC_D_${IF}_${FUN}_IN_PAIRS})
  set(REP_TYPES ${RPC_D_${IF}_${FUN}_OUT_TYPES})
  if(NOT REP_TYPES)
    set(REP_TYPES "void")
  endif()
  string(REPLACE ";" ", " REP_TYPES_LIST "${REP_TYPES}")
  list(INSERT FUN_ARGS 0 "ReturnCallback<${REP_TYPES_LIST}> & callback")
  string(REPLACE ";" ", " ARG_LIST "${FUN_ARGS}")
  file(APPEND "${FILE}" "  virtual void ${FUN}(${ARG_LIST}) = 0;\n")
endmacro()

macro(write_fun_sender FILE IF FUN)
  file(APPEND "${FILE}" "  void ${FUN}();\n")
endmacro()

file(WRITE "${RPC_HEADER_OUT}" "// Generated file\n")
file(APPEND "${RPC_HEADER_OUT}" "namespace ${RPC_NAMESPACE}\n{\n\n")

# Loop on interfaces
foreach(IF ${RPC_IF_LIST})
  file(APPEND "${RPC_HEADER_OUT}" "class ${IF}Receiver\n{\n")
  file(APPEND "${RPC_HEADER_OUT}" "public:\n")
  file(APPEND "${RPC_HEADER_OUT}" "  using Request = ${IF}Request;\n")

  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    write_fun_receiver("${RPC_HEADER_OUT}" ${IF} ${FUN})
  endforeach(FUN)

  file(APPEND "${RPC_HEADER_OUT}" "protected:\n")
  file(APPEND "${RPC_HEADER_OUT}" "  template<typename F>\n")
  file(APPEND "${RPC_HEADER_OUT}" "  void dispatch_rpc(const Request & request, F & callback) {}\n")
  file(APPEND "${RPC_HEADER_OUT}" "};\n\n")
  file(APPEND "${RPC_HEADER_OUT}" "class ${IF}Sender\n{\npublic:\n")

  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    write_fun_sender("${RPC_HEADER_OUT}" ${IF} ${FUN})
  endforeach(FUN)

  file(APPEND "${RPC_HEADER_OUT}" "};\n\n")
endforeach()

file(APPEND "${RPC_HEADER_OUT}" "}\n")
