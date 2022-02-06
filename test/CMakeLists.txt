set(TARGET_NAME "${PROJECT_NAME}-test")

set(SOURCES
    json_account.cpp
    json_available_balance_change.cpp
    json_fills.cpp
    json_order_change.cpp
    json_order_margin_change.cpp
    json_orders.cpp
    json_position_change.cpp
    json_position_settlement.cpp
    json_positions.cpp
    json_token.cpp
    json_withdraw_hold_change.cpp
    main.cpp)

roq_gitignore(OUTPUT ".gitignore" SOURCES "${TARGET_NAME}")

add_executable("${TARGET_NAME}" ${SOURCES})

target_link_libraries(
  "${TARGET_NAME}"
  "${PROJECT_NAME}-json"
  roq-server::roq-server
  roq-core::roq-core
  fmt::fmt
  gtest_main
  ${RT_LIBRARIES})

target_compile_features("${TARGET_NAME}" PUBLIC cxx_std_20)

set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS_RELEASE -s)

# run

add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")
