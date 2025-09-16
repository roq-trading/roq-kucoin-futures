set(TARGET_NAME ${PROJECT_NAME}-json)

set(SOURCES encoder.cpp map.cpp parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    account_data.json
    account.json
    ack.json
    add_order_ack_data.json
    add_order_ack.json
    available_balance_change_data.json
    available_balance_change.json
    cancel_all_orders_ack_data.json
    cancel_all_orders_ack.json
    cancel_order_ack_data.json
    cancel_order_ack.json
    contracts_item.json
    contracts.json
    display_type.json
    error.json
    execution_data.json
    execution.json
    fair_method.json
    fee_type.json
    fills_item.json
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
    liquidity.json
    margin_mode.json
    market_stage.json
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
    order_status.json
    order_type.json
    order_update_type.json
    pong.json
    position_adjust_risk_limit_data.json
    position_adjust_risk_limit.json
    position_change_data.json
    position_change.json
    positions_data.json
    position_settlement_data.json
    position_settlement.json
    position_side.json
    positions.json
    self_trade_prevention.json
    side.json
    snapshot24h_data.json
    snapshot24h.json
    status.json
    stop.json
    stop_price_type.json
    subject.json
    symbol_order_change.json
    ticker_v2_data.json
    ticker_v2.json
    time_in_force.json
    token_data.json
    token.json
    trade_type.json
    type.json
    wallet_balance_change_data.json
    wallet_balance_change.json
    welcome.json
    withdraw_hold_change_data.json
    withdraw_hold_change.json)

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

target_link_libraries(${TARGET_NAME} PRIVATE roq-core::roq-core roq-codec::roq-codec roq-utils::roq-utils roq-logging::roq-logging fmt::fmt)
