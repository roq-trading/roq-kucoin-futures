include(RoqAutogen)

set(TARGET_NAME ${PROJECT_NAME}-test)

set(SOURCES
    json_account_ack.cpp
    # json_add_order_ack.cpp
    json_available_balance_change.cpp
    json_cancel_order_ack.cpp
    json_contracts_ack.cpp
    json_encoder.cpp
    json_execution.cpp
    json_fills_ack.cpp
    json_funding_begin.cpp
    json_funding_end.cpp
    json_funding_rate.cpp
    json_level2.cpp
    json_mark_index_price.cpp
    json_match.cpp
    json_order_book_ack.cpp
    json_order_change.cpp
    json_order_margin_change.cpp
    json_orders_ack.cpp
    json_position_adjust_risk_limit.cpp
    json_position_change.cpp
    json_positions_ack.cpp
    json_position_settlement.cpp
    json_snapshot24h.cpp
    json_ticker_v2.cpp
    json_token.cpp
    json_wallet_balance_change.cpp
    json_withdraw_hold_change.cpp
    json_ws_add_order_ack.cpp
    json_ws_cancel_order_ack.cpp
    json_ws_error.cpp
    main.cpp)

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME})

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-json Catch2::Catch2)

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
