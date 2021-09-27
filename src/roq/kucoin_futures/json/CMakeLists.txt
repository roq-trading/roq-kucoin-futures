set(TARGET_NAME "${PROJECT_NAME}-json")

set(SOURCES parser.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    ack.json
    currencies.json
    currency.json
    error.json
    instance_server.json
    level2.json
    level2_data.json
    level2_data_changes.json
    level2_data_changes_item.json
    message.json
    order_book.json
    order_book_item.json
    pong.json
    snapshot.json
    snapshot_data.json
    snapshot_data_item.json
    subject.json
    symbol.json
    symbols.json
    ticker.json
    ticker_data.json
    token.json
    token_data.json
    type.json
    welcome.json)

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  "roq/kucoin_futures/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "h"
  SOURCES
  ${AUTOGEN_SCHEMAS})

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen(
  OUTPUT
  AUTOGEN_SOURCES
  NAMESPACE
  "roq/kucoin_futures/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "cpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

set(GITIGNORE "${CMAKE_CURRENT_SOURCE_DIR}/.gitignore")

file(WRITE "${GITIGNORE}" "")

foreach(path ${AUTOGEN_SCHEMAS})
  get_filename_component(name ${path} NAME_WE)
  file(APPEND "${GITIGNORE}" "${name}.h\n${name}.cpp\n")
endforeach()

add_library("${TARGET_NAME}" STATIC ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies("${TARGET_NAME}" autogen-json-headers)

target_link_libraries("${TARGET_NAME}" roq-core::roq-core roq-logging::roq-logging fmt::fmt)
