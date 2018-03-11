
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
  list(APPEND MSG_TYPES "${IF}_${FUN}_Req")

  if(RPC_D_${IF}_${FUN}_OUT_FB)
    file(APPEND "${FILE}" "table ${IF}_${FUN}_Rep {\n")
    foreach(FUN_ARG ${RPC_D_${IF}_${FUN}_OUT_FB})
      file(APPEND "${FILE}" "  ${FUN_ARG};\n")
    endforeach()
    file(APPEND "${FILE}" "}\n\n")
    list(APPEND MSG_TYPES "${IF}_${FUN}_Rep")
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

set(MSG_TYPES "RpcError")

# Loop on interfaces
foreach(IF ${RPC_IF_LIST})
  file(APPEND "${RPC_FBS_OUT}" "// ${IF}\n")
  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    write_fun_proto("${RPC_FBS_OUT}" ${IF} ${FUN})
  endforeach()
endforeach()

string(REPLACE ";" ", " UNION_LIST "${MSG_TYPES}")
file(APPEND "${RPC_FBS_OUT}" "// Final message\n")
file(APPEND "${RPC_FBS_OUT}" "union RpcMessageU {\n  ${UNION_LIST}\n}\n")
file(APPEND "${RPC_FBS_OUT}" "table RpcMessage {\n")
file(APPEND "${RPC_FBS_OUT}" "  id:uint32;\n")
file(APPEND "${RPC_FBS_OUT}" "  data:RpcMessageU;\n")
file(APPEND "${RPC_FBS_OUT}" "}\n\n")

#

# Generate .hpp
macro(write_fun_receiver FILE IF FUN)
  set(FUN_ARGS "const ${IF}_${FUN}_Req & args")
  if(RPC_D_${IF}_${FUN}_OUT_FB)
    list(APPEND FUN_ARGS "ReturnCallback<${IF}_${FUN}_Rep> && callback")
  endif()
  string(REPLACE ";" ", " ARG_LIST "${FUN_ARGS}")
  file(APPEND "${FILE}" "  virtual void ${FUN}(${ARG_LIST}) = 0;\n")
endmacro()

macro(write_fun_dispatcher FILE IF FUN)
  file(APPEND "${FILE}" "  template<typename If = ${IF}<Base, true> >\n")
  file(APPEND "${FILE}" "  inline typename std::enable_if<!impl::ContainsIf<If, Interfaces<Base, true>...>::value, bool>::type\n")
  file(APPEND "${FILE}" "    dispatch_${IF}_${FUN}(const RpcMessage & message) { return false; }\n")
  file(APPEND "${FILE}" "  template<typename If = ${IF}<Base, true> >\n")
  file(APPEND "${FILE}" "  inline typename std::enable_if<impl::ContainsIf<If, Interfaces<Base, true>...>::value, bool>::type\n")
  file(APPEND "${FILE}" "    dispatch_${IF}_${FUN}(const RpcMessage & message)\n")
  file(APPEND "${FILE}" "  {\n")
  file(APPEND "${FILE}" "    static_cast<${IF}<Base, true>*>(this)->${FUN}(message.data_as_${IF}_${FUN}_Req());\n")
  file(APPEND "${FILE}" "    return true;\n")
  file(APPEND "${FILE}" "  }\n")
endmacro()

macro(write_fun_sender FILE IF FUN)
  if(RPC_D_${IF}_${FUN}_OUT_FB)
    file(APPEND "${FILE}" "  template<typename F, typename R>\n")
    file(APPEND "${FILE}" "  inline void ${FUN}(F & args_builder, R && return_callback)\n")
  else()
    file(APPEND "${FILE}" "  template<typename F>\n")
    file(APPEND "${FILE}" "  inline void ${FUN}(F & args_builder)\n")
  endif()
  file(APPEND "${FILE}" "  {\n")
  file(APPEND "${FILE}" "    using T = ${IF}_${FUN}_Req;\n")
  file(APPEND "${FILE}" "    flatbuffers::FlatBufferBuilder builder;\n")
  file(APPEND "${FILE}" "    flatbuffers::Offset<T> data = args_builder(builder);\n")
  file(APPEND "${FILE}" "    uint32_t rpc_id = 0;\n")
  file(APPEND "${FILE}" "    auto rpc_msg = CreateRpcMessage(builder, rpc_id, RpcMessageUTraits<T>::enum_value, data.Union());\n")
  file(APPEND "${FILE}" "  }\n")
endmacro()

file(WRITE "${RPC_HEADER_OUT}" "// Generated file\n")
file(APPEND "${RPC_HEADER_OUT}" "#pragma once\n\n")
file(APPEND "${RPC_HEADER_OUT}" "#include <type_traits>\n\n")
file(APPEND "${RPC_HEADER_OUT}" "namespace ${RPC_NAMESPACE}\n{\n\n")
file(APPEND "${RPC_HEADER_OUT}" "namespace impl\n{\n")
# http://en.cppreference.com/w/cpp/types/disjunction
# Remove when C++17
file(APPEND "${RPC_HEADER_OUT}" "template<class...> struct Disjunction : std::false_type {};\n")
file(APPEND "${RPC_HEADER_OUT}" "template<class B1> struct Disjunction<B1> : B1 {};\n")
file(APPEND "${RPC_HEADER_OUT}" "template<class B1, class... Bn>\n")
file(APPEND "${RPC_HEADER_OUT}" "struct Disjunction<B1, Bn...> \n")
file(APPEND "${RPC_HEADER_OUT}" "    : std::conditional_t<bool(B1::value), B1, Disjunction<Bn...>> {};\n")
file(APPEND "${RPC_HEADER_OUT}" "template<class If, class... Interfaces>\n")
file(APPEND "${RPC_HEADER_OUT}" "using ContainsIf = impl::Disjunction<std::is_same<If, Interfaces>...>;\n")
file(APPEND "${RPC_HEADER_OUT}" "}\n\n")

# Loop on interfaces
foreach(IF ${RPC_IF_LIST})
  file(APPEND "${RPC_HEADER_OUT}" "template<class Base, bool>\n")
  file(APPEND "${RPC_HEADER_OUT}" "class ${IF} // ${IF}Receiver\n{\n")
  file(APPEND "${RPC_HEADER_OUT}" "public:\n")

  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    write_fun_receiver("${RPC_HEADER_OUT}" ${IF} ${FUN})
  endforeach(FUN)

  file(APPEND "${RPC_HEADER_OUT}" "};\n\n")
  file(APPEND "${RPC_HEADER_OUT}" "template<class Base>\n")
  file(APPEND "${RPC_HEADER_OUT}" "class ${IF}<Base, false> // ${IF}Sender\n{\n")
  file(APPEND "${RPC_HEADER_OUT}" "public:\n")

  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    write_fun_sender("${RPC_HEADER_OUT}" ${IF} ${FUN})
  endforeach(FUN)

  file(APPEND "${RPC_HEADER_OUT}" "};\n\n")
endforeach()

file(APPEND "${RPC_HEADER_OUT}" "template<class Base, template<class, bool> class... Interfaces>\n")
file(APPEND "${RPC_HEADER_OUT}" "class RpcReceiver : public Interfaces<Base, true>...\n{\n")
file(APPEND "${RPC_HEADER_OUT}" "protected:\n")
file(APPEND "${RPC_HEADER_OUT}" "  inline bool dispatch_rpc_request(const RpcMessage & message)\n")
file(APPEND "${RPC_HEADER_OUT}" "  {\n")
file(APPEND "${RPC_HEADER_OUT}" "    switch (message.data_type())\n")
file(APPEND "${RPC_HEADER_OUT}" "    {\n")
# Loop on interfaces
foreach(IF ${RPC_IF_LIST})
  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    file(APPEND "${RPC_HEADER_OUT}" "    case RpcMessageU_${IF}_${FUN}_Req:\n")
    file(APPEND "${RPC_HEADER_OUT}" "      return dispatch_${IF}_${FUN}(message);\n")
  endforeach(FUN)
endforeach()
file(APPEND "${RPC_HEADER_OUT}" "    default: return false;\n")
file(APPEND "${RPC_HEADER_OUT}" "    }\n")
file(APPEND "${RPC_HEADER_OUT}" "  }\n")
file(APPEND "${RPC_HEADER_OUT}" "private:\n")
# Loop on interfaces
foreach(IF ${RPC_IF_LIST})
  # Loop on functions
  foreach(FUN ${RPC_D_${IF}_FUNCTIONS})
    write_fun_dispatcher("${RPC_HEADER_OUT}" ${IF} ${FUN})
  endforeach(FUN)
endforeach()
file(APPEND "${RPC_HEADER_OUT}" "};\n\n")

file(APPEND "${RPC_HEADER_OUT}" "template<class Base, template<class, bool> class... Interfaces>\n")
file(APPEND "${RPC_HEADER_OUT}" "class RpcSender : public Interfaces<Base, false>...\n{\n")
file(APPEND "${RPC_HEADER_OUT}" "public:\n")
file(APPEND "${RPC_HEADER_OUT}" "};\n\n")

file(APPEND "${RPC_HEADER_OUT}" "} // namespace ${RPC_NAMESPACE}\n")
