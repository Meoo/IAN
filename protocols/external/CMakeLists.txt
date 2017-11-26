
project(protocols-external)


ian_add_protocol(TARGET ${PROJECT_NAME} INCLUDE "proto-ext" GEN_JS FILES test.fbs test2.fbs)
