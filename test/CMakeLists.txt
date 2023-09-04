set(TARGET_NAME ${PROJECT_NAME}-test)

set(SOURCES
    json_account.cpp
    json_available_balance_change.cpp
    json_execution.cpp
    json_fills.cpp
    json_funding_rate.cpp
    json_mark_index_price.cpp
    json_level2.cpp
    json_order_book.cpp
    json_order_change.cpp
    json_order_margin_change.cpp
    json_orders.cpp
    json_position_change.cpp
    json_positions.cpp
    json_position_settlement.cpp
    json_ticker.cpp
    json_token.cpp
    json_withdraw_hold_change.cpp
    main.cpp)

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME})

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(
  ${TARGET_NAME}
  ${PROJECT_NAME}-json
  roq-server::roq-server
  roq-core::roq-core
  roq-io::roq-io
  aklomp::base64
  fmt::fmt
  Catch2::Catch2
  ${RT_LIBRARIES})

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
