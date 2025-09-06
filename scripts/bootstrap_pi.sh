#!/usr/bin/env bash
set -euo pipefail

# 1) Ensure venv tooling (PEP 668 wants isolation)
sudo apt update
sudo apt install -y python3-venv

# 2) Create/refresh repo-local venv
if [ ! -d ".venv" ]; then
  python3 -m venv .venv
fi
. .venv/bin/activate

# 3) Install Python deps pinned by requirements.txt
pip install --upgrade pip
pip install -r requirements.txt

# 4) Serial permissions (udev rules + dialout) for PlatformIO boards
# (Reload rules and re-plug board afterwards)
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules \
| sudo tee /etc/udev/rules.d/99-platformio-udev.rules >/dev/null
sudo udevadm control --reload-rules
sudo udevadm trigger
sudo usermod -aG dialout "$USER" || true

echo "Bootstrap done. Log out/in (or reboot) to apply 'dialout' group. Then re-plug the NUCLEO."
