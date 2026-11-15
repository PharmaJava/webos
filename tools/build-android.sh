#!/bin/bash
# Builds Stremio Origin for Android TV: dist/StremioOrigin-AndroidTV-<version>.apk (armeabi-v7a, runs on every Android TV / Google TV).
#   tools/build-android.sh            # needs the Android SDK (ANDROID_HOME) with NDK 27.2.12479018, JDK 17+, Gradle 8.9+
#   KEYS=$ORIGIN_BUILD/keys.env ...   # the app's own Trakt/TMDB keys, as tools/build.sh
#   ORIGIN_KEYSTORE=... ORIGIN_KEYSTORE_PASSWORD=...   # sign with your own key (else the debug key: fine for trying it)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
"$ROOT/tools/android-deps.sh"
: "${ANDROID_HOME:=${ANDROID_SDK_ROOT:-}}"; [ -n "$ANDROID_HOME" ] || { echo "Set ANDROID_HOME to the Android SDK"; exit 1; }
export ANDROID_HOME
GRADLE="${GRADLE:-gradle}"
(cd "$ROOT/androidtv" && "$GRADLE" --no-daemon -q :app:assembleRelease)
VER=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["version"])' "$ROOT/pkg/appinfo.json")
mkdir -p "$ROOT/dist"
cp "$ROOT/androidtv/app/build/outputs/apk/release/app-release.apk" "$ROOT/dist/StremioOrigin-AndroidTV-$VER.apk"
ls -l "$ROOT/dist/StremioOrigin-AndroidTV-$VER.apk"
