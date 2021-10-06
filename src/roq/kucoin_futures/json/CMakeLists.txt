set(TARGET_NAME "${PROJECT_NAME}-json")

set(SOURCES parser.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    account.json
    account_data.json
    ack.json
    contract.json
    contracts.json
    error.json
    fair_method.json
    funding_begin.json
    funding_begin_data.json
    funding_end.json
    funding_end_data.json
    funding_rate.json
    funding_rate_data.json
    instance_server.json
    level2.json
    level2_data.json
    mark_index_price.json
    mark_index_price_data.json
    mark_method.json
    match.json
    match_data.json
    message.json
    order_book.json
    order_book_data.json
    order_book_data_item.json
    pong.json
    position.json
    positions.json
    side.json
    snapshot24h.json
    snapshot24h_data.json
    status.json
    subject.json
    ticker.json
    ticker_data.json
    ticker_v2.json
    ticker_v2_data.json
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
