#!/bin/bash
# A page that sends the phone straight to the newest APK: the stable link handed out (gh-pages: phone/ and phone/<abi>/).
#   tools/phone/latest-page.sh <version> <abi> <out.html>
set -euo pipefail
VER="$1"; ABI="$2"; OUT="$3"; mkdir -p "$(dirname "$OUT")"
URL="https://github.com/PharmaJava/webos/releases/download/phone-v$VER/stremio-claude-phone-$VER-$ABI.apk"
cat > "$OUT" <<HTML
<!doctype html><html lang="es"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Stremio Claude $VER</title><meta http-equiv="refresh" content="0; url=$URL">
<style>body{font:18px/1.5 system-ui,sans-serif;background:#0c0b11;color:#eee;display:flex;min-height:100vh;align-items:center;justify-content:center;margin:0;text-align:center;padding:16px}a{color:#9b8cff}</style>
</head><body><div><h1>Stremio Claude $VER</h1><p>Descargando&hellip; / Downloading&hellip;</p><p><a href="$URL">stremio-claude-phone-$VER-$ABI.apk</a></p></div>
<script>location.replace("$URL")</script></body></html>
HTML
