set(TARGET_NAME ${PROJECT_NAME}-json)

set(SOURCES map.cpp parser.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    account_data.json
    account.json
    ack.json
    available_balance_change_data.json
    available_balance_change.json
    contract.json
    contracts.json
    error.json
    execution_data.json
    execution.json
    fair_method.json
    fills_data_item.json
    fills_data.json
    fills.json
    funding_begin_data.json
    funding_begin.json
    funding_end_data.json
    funding_end.json
    funding_rate_data.json
    funding_rate.json
    instance_server.json
    level2_data_item.json
    level2_data.json
    level2.json
    mark_index_price_data.json
    mark_index_price.json
    mark_method.json
    match_data.json
    match.json
    message.json
    order_book_data_item.json
    order_book_data.json
    order_book.json
    order_change_data.json
    order_change.json
    order_margin_change_data.json
    order_margin_change.json
    orders_data_item.json
    orders_data.json
    orders.json
    pong.json
    position_change_data.json
    position_change.json
    position.json
    position_settlement_data.json
    position_settlement.json
    positions.json
    side.json
    snapshot24h_data.json
    snapshot24h.json
    status.json
    subject.json
    ticker_data.json
    ticker.json
    ticker_v2_data.json
    ticker_v2.json
    token_data.json
    token.json
    type.json
    welcome.json
    withdraw_hold_change.json
    withdraw_hold_change_data.json)

roq_autogen_hpp(
  OUTPUT
  AUTOGEN_HEADERS
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen_cpp(
  OUTPUT
  AUTOGEN_SOURCES
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME} ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library(${TARGET_NAME} OBJECT ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies(${TARGET_NAME} autogen-json-headers)

target_link_libraries(${TARGET_NAME} roq-core::roq-core roq-utils::roq-utils roq-logging::roq-logging fmt::fmt)
