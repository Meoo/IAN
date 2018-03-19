#
# This file is part of the IAN project - https://github.com/Meoo/IAN
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Input:
# RPC_DESCRIPTOR : descriptor .cmake file
# RPC_HEADER_OUT : output header
# RPC_FBS_OUT : output flatbuffers file


# IDL

set(RPC_IF_LIST)

macro(rpc_namespace NS)
  set(RPC_NAMESPACE ${NS})
endmacro()

set(RPC_FB_INCLUDES)
macro(rpc_fb_include)
  foreach(INC ${ARGN})
    list(APPEND RPC_FB_INCLUDES ${INC})
  endforeach()
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

  set(RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_IN_FB ${ARG_IN})
  set(RPC_D_${RPC_CURRENT_IF}_${ARG_NAME}_OUT_FB ${ARG_OUT})
endmacro()


#

include(${RPC_DESCRIPTOR})

#


if(NOT RPC_NAMESPACE)
  message(FATAL_ERROR "A namespace must be defined")
endif()


# Generate .fbs
macro(write_fun_proto FILE IF FUN)
  file(APPEND "${FILE}" "table ${IF}_${FUN}_Req {\n")
  foreach(FUN_ARG ${RPC_D_${IF}_${FUN}_IN_FB})
    file(APPEND "${FILE}" "  ${FUN_ARG};\n")
  endforeach()
  file(APPEND "${FILE}" "}\n")
  list(APPEND MSG_TYPES_REQ "${IF}_${FUN}_Req")

  if(RPC_D_${IF}_${FUN}_OUT_FB)
    file(APPEND "${FILE}" "table ${IF}_${FUN}_Rep {\n")
    foreach(FUN_ARG ${RPC_D_${IF}_${FUN}_OUT_FB})
      file(APPEND "${FILE}" "  ${FUN_ARG};\n")
    endforeach()
    file(APPEND "${FILE}" "}\n\n")
    list(APPEND MSG_TYPES_REP "${IF}_${FUN}_Rep")
  else()
    file(APPEND "${FILE}" "\n")
  endif()
endmacro()

file(WRITE "${RPC_FBS_OUT}" "// Generated file\n")

foreach(INC ${RPC_FB_INCLUDES})
file(APPEND "${RPC_FBS_OUT}" "include \"${INC}\";")
endforeach()

file(APPEND "${RPC_FBS_OUT}" "namespace ${RPC_NAMESPACE};\n\n")
file(APPEND "${RPC_FBS_OUT}" "table RpcError {\n")
file(APPEND "${RPC_FBS_OUT}" "  code:uint32;\n")
file(APPEND "${RPC_FBS_OUT}" "  message:string;\n")
file(APPEND "${RPC_FBS_OUT}" "}\n\n")

set(MSG_TYPES_REQ)
set(MSG_TYPES_REP "RpcError")

# Loop on interfaces
foreach(IF ${RPC_IF_LIST})
  file(APPEND "${RPC_FBS_OUT}" "// ${IF}\n")
  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    write_fun_proto("${RPC_FBS_OUT}" ${IF} ${FUN})
  endforeach()
endforeach()

string(REPLACE ";" ", " UNION_LIST_REQ "${MSG_TYPES_REQ}")
string(REPLACE ";" ", " UNION_LIST_REP "${MSG_TYPES_REP}")
file(APPEND "${RPC_FBS_OUT}" "// Final message\n")
file(APPEND "${RPC_FBS_OUT}" "union RpcRequestU {\n  ${UNION_LIST_REQ}\n}\n")
file(APPEND "${RPC_FBS_OUT}" "table RpcRequest {\n")
file(APPEND "${RPC_FBS_OUT}" "  id:uint32;\n")
file(APPEND "${RPC_FBS_OUT}" "  data:RpcRequestU;\n")
file(APPEND "${RPC_FBS_OUT}" "}\n\n")
file(APPEND "${RPC_FBS_OUT}" "union RpcReplyU {\n  ${UNION_LIST_REP}\n}\n")
file(APPEND "${RPC_FBS_OUT}" "table RpcReply {\n")
file(APPEND "${RPC_FBS_OUT}" "  id:uint32;\n")
file(APPEND "${RPC_FBS_OUT}" "  data:RpcReplyU;\n")
file(APPEND "${RPC_FBS_OUT}" "}\n\n")

#

# Generate .hpp
macro(write_fun_receiver VAR IF FUN)
  set(FUN_ARGS "const ${IF}_${FUN}_Req & args")
  set(${VAR} "${${VAR}}  using ${IF}_${FUN}_Req = ${RPC_NAMESPACE}::${IF}_${FUN}_Req;\n")
  if(RPC_D_${IF}_${FUN}_OUT_FB)
    set(${VAR} "${${VAR}}  using ${IF}_${FUN}_Return = ${RPC_NAMESPACE}::RpcReturn<Base, ${IF}<Base, true>, ${IF}_${FUN}_Rep>;\n")
    set(${VAR} "${${VAR}}  friend class ${IF}_${FUN}_Return;\n")
    list(APPEND FUN_ARGS "${IF}_${FUN}_Return ret")
  endif()
  string(REPLACE ";" ", " ARG_LIST "${FUN_ARGS}")
  set(${VAR} "${${VAR}}  virtual void ${FUN}(${ARG_LIST}) = 0;\n")
endmacro()

macro(write_fun_dispatcher VAR IF FUN)
  set(${VAR} "${${VAR}}  template<typename If = ${IF}<Base, true> >\n")
  set(${VAR} "${${VAR}}  inline typename std::enable_if<!impl::ContainsIf<If, Interfaces<Base, true>...>::value, bool>::type\n")
  set(${VAR} "${${VAR}}    dispatch_${IF}_${FUN}(const RpcRequest & request) { return false; }\n")
  set(${VAR} "${${VAR}}  template<typename If = ${IF}<Base, true> >\n")
  set(${VAR} "${${VAR}}  inline typename std::enable_if<impl::ContainsIf<If, Interfaces<Base, true>...>::value, bool>::type\n")
  set(${VAR} "${${VAR}}    dispatch_${IF}_${FUN}(const RpcRequest & request)\n")
  set(${VAR} "${${VAR}}  {\n")
  if(RPC_D_${IF}_${FUN}_OUT_FB)
    set(${VAR} "${${VAR}}    static_cast<${IF}<Base, true>*>(this)->${FUN}(*request.data_as_${IF}_${FUN}_Req(), ${IF}_${FUN}_Return(static_cast<Base*>(this)));\n")
  else()
    set(${VAR} "${${VAR}}    static_cast<${IF}<Base, true>*>(this)->${FUN}(*request.data_as_${IF}_${FUN}_Req());\n")
  endif()
  set(${VAR} "${${VAR}}    return true;\n")
  set(${VAR} "${${VAR}}  }\n")
endmacro()

macro(write_fun_sender VAR IF FUN)
  if(RPC_D_${IF}_${FUN}_OUT_FB)
    set(${VAR} "${${VAR}}  template<typename F, typename R>\n")
    set(${VAR} "${${VAR}}  inline void ${FUN}(F & args_builder, R && return_callback)\n")
  else()
    set(${VAR} "${${VAR}}  template<typename F>\n")
    set(${VAR} "${${VAR}}  inline void ${FUN}(F & args_builder)\n")
  endif()
  set(${VAR} "${${VAR}}  {\n")
  set(${VAR} "${${VAR}}    using T = ${IF}_${FUN}_Req;\n")
  set(${VAR} "${${VAR}}    flatbuffers::FlatBufferBuilder builder;\n")
  set(${VAR} "${${VAR}}    flatbuffers::Offset<T> data = args_builder(builder);\n")
  if(RPC_D_${IF}_${FUN}_OUT_FB)
    set(${VAR} "${${VAR}}    std::uint32_t rpc_id = register_rpc_return_callback(RpcResultInvoker(RpcRequestUTraits<T>::enum_value, std::forward<R>(return_callback)));\n")
  else()
    set(${VAR} "${${VAR}}    std::uint32_t rpc_id = 0;\n")
  endif()
  set(${VAR} "${${VAR}}    auto rpc_req = CreateRpcRequest(builder, rpc_id, RpcRequestUTraits<T>::enum_value, data.Union());\n")
  set(${VAR} "${${VAR}}    emit_rpc_request(builder, rpc_req);\n")
  set(${VAR} "${${VAR}}  }\n")
endmacro()

set(HPP_IF)

# Loop on interfaces
foreach(IF ${RPC_IF_LIST})
  set(HPP_IF "${HPP_IF}template<class Base, bool>\n")
  set(HPP_IF "${HPP_IF}class ${IF} // ${IF}Receiver\n{\n")
  set(HPP_IF "${HPP_IF}protected:\n")
  set(HPP_IF "${HPP_IF}  virtual void emit_rpc_reply(flatbuffers::FlatBufferBuilder & builder, flatbuffers::Offset<RpcReply> reply) = 0;\n")
  set(HPP_IF "${HPP_IF}public:\n")

  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    write_fun_receiver(HPP_IF ${IF} ${FUN})
  endforeach(FUN)

  set(HPP_IF "${HPP_IF}};\n\n")
  set(HPP_IF "${HPP_IF}template<class Base>\n")
  set(HPP_IF "${HPP_IF}class ${IF}<Base, false> // ${IF}Sender\n{\n")
  set(HPP_IF "${HPP_IF}protected:\n")
  set(HPP_IF "${HPP_IF}  virtual void emit_rpc_request(flatbuffers::FlatBufferBuilder & builder, flatbuffers::Offset<RpcRequest> request) = 0;\n")
  set(HPP_IF "${HPP_IF}  virtual std::uint32_t register_rpc_return_callback(RpcResultInvoker callback) = 0;\n")
  set(HPP_IF "${HPP_IF}public:\n")

  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    write_fun_sender(HPP_IF ${IF} ${FUN})
  endforeach(FUN)

  set(HPP_IF "${HPP_IF}};\n\n")
endforeach()

set(RPC_HPP_INTERFACES "${HPP_IF}")

set(HPP_SWITCH)

# Loop on interfaces
foreach(IF ${RPC_IF_LIST})
  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    set(HPP_SWITCH "${HPP_SWITCH}    case RpcRequestU_${IF}_${FUN}_Req: return dispatch_${IF}_${FUN}(request);\n")
  endforeach(FUN)
endforeach()

set(RPC_HPP_REQUEST_SWITCH "${HPP_SWITCH}")

set(HPP_DISP)

# Loop on interfaces
foreach(IF ${RPC_IF_LIST})
  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    write_fun_dispatcher(HPP_DISP ${IF} ${FUN})
  endforeach(FUN)
endforeach()

set(RPC_HPP_REQUEST_DISPATCHERS "${HPP_DISP}")

configure_file(${CMAKE_CURRENT_LIST_DIR}/protoRpc_template.hpp ${RPC_HEADER_OUT} @ONLY)
