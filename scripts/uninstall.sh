#!/usr/bin/env bash

set -euo pipefail

APP_NAME="usb-logger"
DEFAULT_USER_HOME="${SUDO_USER:-$USER}"
DEFAULT_HOME_DIR="$(getent passwd "${DEFAULT_USER_HOME}" | cut -d: -f6)"
APP_DIR="${DEFAULT_HOME_DIR:-$HOME}/${APP_NAME}"

if [[ $EUID -ne 0 ]]; then
  echo "Run as root (sudo)." >&2
  exit 1
fi

systemctl stop ${APP_NAME}.service || true
systemctl disable ${APP_NAME}.service || true
rm -f /etc/systemd/system/${APP_NAME}.service
systemctl daemon-reload

rm -rf "${APP_DIR}"

echo "Uninstalled systemd unit and removed app directory: ${APP_DIR}"
