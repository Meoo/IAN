
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

  list(APPEND RPC_D_${RPC_CURRENT_IF}_FUNCTIONS ${ARG_NAME})
endmacro()


#

include(${RPC_DESCRIPTOR})

#


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
  file(APPEND "${FILE}" "  virtual void ${FUN}() = 0;\n")
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

  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    write_fun_receiver("${RPC_HEADER_OUT}" ${IF} ${FUN})
  endforeach(FUN)

  file(APPEND "${RPC_HEADER_OUT}" "protected:\n")
  file(APPEND "${RPC_HEADER_OUT}" "  template<typename F>\n")
  file(APPEND "${RPC_HEADER_OUT}" "  void dispatch_rpc(const ${IF}Request & request, F & callback) {}\n")
  file(APPEND "${RPC_HEADER_OUT}" "};\n\n")
  file(APPEND "${RPC_HEADER_OUT}" "class ${IF}Sender\n{\npublic:\n")

  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    write_fun_sender("${RPC_HEADER_OUT}" ${IF} ${FUN})
  endforeach(FUN)

  file(APPEND "${RPC_HEADER_OUT}" "};\n\n")
endforeach()

file(APPEND "${RPC_HEADER_OUT}" "}\n")
