# Change Log

All notable changes will be documented in this file.

## Head

### Fixed

* Refresh private token #563

## 1.1.2 &ndash; 2026-02-08

### Changed

* Renamed flag: `--ws_api` (from: `--test_wsapi`)

### Fixed

* Fixing market data timestamps (#558)
* postOnly was missing from new order (#544)


## 1.1.1 &ndash; 2025-12-14

## 1.1.0 &ndash; 2025-11-22

## 1.0.9 &ndash; 2025-09-26

### Fixed

* HTTP response with status code 503 (service unavailable) should map to `RequestStatus::REJECTED` (#522)

## 1.0.8 &ndash; 2025-08-16

## 1.0.7 &ndash; 2025-07-02

## 1.0.6 &ndash; 2025-05-16

## 1.0.5 &ndash; 2025-03-26

## 1.0.4 &ndash; 2024-12-30

## 1.0.3 &ndash; 2024-11-26

## 1.0.2 &ndash; 2024-07-14

## 1.0.1 &ndash; 2024-04-14

## 1.0.0 &ndash; 2024-03-16

## 0.9.9 &ndash; 2024-01-28

## 0.9.8 &ndash; 2023-11-20

## 0.9.7 &ndash; 2023-09-18

## 0.9.6 &ndash; 2023-07-22

## 0.9.5 &ndash; 2023-06-12

## 0.9.4 &ndash; 2023-05-04

## 0.9.3 &ndash; 2023-03-20

## 0.9.2 &ndash; 2023-02-22

## 0.9.1 &ndash; 2023-01-12

## 0.9.0 &ndash; 2022-12-22

## 0.8.9 &ndash; 2022-11-14

## 0.8.8 &ndash; 2022-10-04

## 0.8.7 &ndash; 2022-08-22

## 0.8.6 &ndash; 2022-07-18

### Changed

* Added `--api` (flag) to select between v1 and v2 (#228)

## 0.8.5 &ndash; 2022-06-06

### Changed

* Market data support for `--net_disconnect_on_idle_timeout`.

## 0.8.4 &ndash; 2022-05-14

## 0.8.3 &ndash; 2022-03-22

## 0.8.2 &ndash; 2022-02-18

## 0.8.1 &ndash; 2022-01-16

## 0.8.0 &ndash; 2022-01-12

## 0.7.9 &ndash; 2021-12-08

## 0.7.8 &ndash; 2021-11-02

### Added

* Add exchange sequence number to `MarketByPrice` and `MarketByOrder` (#101)
* Add `max_trade_vol` and `trade_vol_step_size` to ReferenceData (#100)

### Changed

* Move cache utilities to API (#111)
* Interface to support binary data from web::socket
* ReferenceData currencies should follow FX conventions (#99)
* Replace `snapshot` (bool) with `update_type` (UpdateType) (#97)
* Moved signature handling to tools library (chore)

### Removed

* Remove custom literals (#110)
* Remove external rate-limiter mirroring from the REST connection (#83)

## 0.7.7 &ndash; 2021-09-20

### Changed

* Added HTTP `request_id` (#55)

## 0.7.6 &ndash; 2021-09-02

### Changed

* New order management interface (#25)

## 0.7.5 &ndash; 2021-08-08

## 0.7.4 &ndash; 2021-07-20

## 0.7.3 &ndash; 2021-07-06

## 0.7.2 &ndash; 2021-06-20

## 0.7.1 &ndash; 2021-05-30
