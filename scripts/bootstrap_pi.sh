#!/usr/bin/env bash
set -euo pipefail

# Make venv
python3 -m venv .venv
. .venv/bin/activate
pip install --upgrade pip
pip install -r bridge/requirements.txt

# PlatformIO udev rules (Linux)
if [ -w /etc/udev/rules.d ]; then
  sudo curl -fsSL \
    https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules \
    -o /etc/udev/rules.d/99-platformio-udev.rules
  sudo udevadm control --reload-rules
  sudo udevadm trigger
fi

# Serial access group
if id -nG "$USER" | grep -qw dialout; then
  echo "User already in dialout"
else
  echo "Adding $USER to dialout (relog/reboot required)"
  sudo usermod -aG dialout "$USER"
fi

echo "Bootstrap done. Replug the NUCLEO and use VS Code tasks."
