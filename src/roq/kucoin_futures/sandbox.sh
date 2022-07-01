#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
	PREFIX="gdb --args"
else
	PREFIX=
fi

NAME="kucoin-futures"

CONFIG_FILE="$CWD/config/$NAME-sandbox.toml"

URI="api-sandbox-futures.kucoin.com"

REST_URI="https://$URI"

$PREFIX ./roq-kucoin-futures \
  --name "${NAME}" \
  --config_file "$CONFIG_FILE" \
  --event_log_dir "${HOME}/var/lib/roq/data" \
  --event_log_symlink \
  --client_listen_address "${HOME}/run/${NAME}.sock" \
  --metrics_listen_address "${HOME}/run/${NAME}_metrics.sock" \
  --rest_uri "$REST_URI" \
  $@
