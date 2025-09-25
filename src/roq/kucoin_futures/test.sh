#!/usr/bin/env bash

if [ "$1" == "debug" ]; then
  PREFIX="gdb --args"
else
  PREFIX=
fi

NAME="kucoin-futures"

CONFIG="${CONFIG:-$NAME-test}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-kucoin-futures/$CONFIG.toml"

API="v2"

FLAGFILE="../../../share/flags/test/flags.cfg"

$PREFIX ./roq-kucoin-futures \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --flagfile "$FLAGFILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --api="$API" \
  $@
