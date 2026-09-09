set(TARGET_NAME ${PROJECT_NAME}-protocol-json)

set(SOURCES encoder.cpp map.cpp parser.cpp utils.cpp ws_parser.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    account_ack.json
    account_ack_data.json
    ack.json
    add_order_ack.json
    add_order_ack_data.json
    asset_class.json
    available_balance_change.json
    available_balance_change_data.json
    cancel_all_orders_ack.json
    cancel_all_orders_ack_data.json
    cancel_order_ack.json
    cancel_order_ack_data.json
    channel_type.json
    contracts_ack.json
    contracts_ack_data_item.json
    display_type.json
    error.json
    execution.json
    execution_data.json
    fair_method.json
    fee_type.json
    fills_ack.json
    fills_ack_data_item.json
    funding_begin.json
    funding_begin_data.json
    funding_end.json
    funding_end_data.json
    funding_rate.json
    funding_rate_data.json
    instance_server.json
    level2.json
    level2_data.json
    level2_data_item.json
    liquidity.json
    margin_mode.json
    mark_index_price.json
    mark_index_price_data.json
    mark_method.json
    market_stage.json
    market_type.json
    match.json
    match_data.json
    message.json
    order_book_ack.json
    order_book_ack_data.json
    order_book_ack_data_bids_asks_item.json
    order_change.json
    order_change_data.json
    order_margin_change.json
    order_margin_change_data.json
    order_status.json
    order_type.json
    order_update_type.json
    orders_ack.json
    orders_ack_data.json
    orders_ack_data_items_item.json
    pong.json
    position_adjust_risk_limit.json
    position_adjust_risk_limit_data.json
    position_change.json
    position_change_data.json
    position_settlement.json
    position_settlement_data.json
    position_side.json
    positions_ack.json
    positions_ack_data_item.json
    self_trade_prevention.json
    side.json
    snapshot24h.json
    snapshot24h_data.json
    status.json
    stop.json
    stop_price_type.json
    subject.json
    symbol_order_change.json
    ticker_v2.json
    ticker_v2_data.json
    time_in_force.json
    token.json
    token_data.json
    trade_type.json
    type.json
    wallet_balance_change.json
    wallet_balance_change_data.json
    welcome.json
    withdraw_hold_change.json
    withdraw_hold_change_data.json
    ws_add_order_ack.json
    ws_add_order_ack_data.json
    ws_auth.json
    ws_cancel_order_ack.json
    ws_cancel_order_ack_data.json
    ws_error.json
    ws_message.json
    ws_op.json
    ws_pong.json
    ws_user_rate_limit.json
    ws_welcome.json)

roq_autogen_hpp(
  OUTPUT
  AUTOGEN_HEADERS
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

add_custom_target(protocol-json-autogen-headers ALL DEPENDS ${AUTOGEN_HEADERS})

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

add_dependencies(${TARGET_NAME} protocol-json-autogen-headers)

target_link_libraries(${TARGET_NAME} PRIVATE roq-core::roq-core roq-codec::roq-codec roq-utils::roq-utils roq-logging::roq-logging fmt::fmt)
