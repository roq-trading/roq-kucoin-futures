#!/usr/bin/env bash

if [ "$1" == "debug" ]; then
  PREFIX="gdb --args"
else
  PREFIX=
fi

NAME="kucoin-futures"

CONFIG="${CONFIG:-$NAME}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-kucoin-futures/$CONFIG.toml"

FLAG_FILE="../../../share/flags/prod/flags.cfg"

MARGIN_MODE="CROSS"
DOWNLOAD_TRADES_LOOKBACK="24h"

$PREFIX ./roq-kucoin-futures \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --flagfile "$FLAG_FILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink true \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --margin_mode="$MARGIN_MODE" \
  --download_trades_lookback="$DOWNLOAD_TRADES_LOOKBACK" \
  $@
