#!/bin/sh

set -eu

extension_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
host_path="$extension_dir/native-host/ytdlp_host.py"
manifest_dir="$HOME/.mozilla/native-messaging-hosts"
manifest_path="$manifest_dir/musicplayer.ytdlp.json"

mkdir -p "$manifest_dir"
chmod 755 "$host_path"

sed "s#__HOST_PATH__#$host_path#g" \
  "$extension_dir/native-host/musicplayer.ytdlp.json.in" > "$manifest_path"

printf '%s\n' "Installed native host: $manifest_path"
printf '%s\n' "Load $extension_dir/manifest.json from about:debugging in Firefox."
