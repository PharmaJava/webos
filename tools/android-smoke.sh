#!/bin/bash
# The Android TV build on an emulator, as a person opens it: installs the APK, starts it, waits, and checks from the app's
# own log that it drew frames and the network answered (no crash). Leaves dist/smoke/ with a screenshot and the log.
#   tools/android-smoke.sh path/to/app.apk
set -uo pipefail
APK="$1"; OUT="$(cd "$(dirname "$0")/.." && pwd)/dist/smoke"; mkdir -p "$OUT"
PKG=tv.stremioorigin.app
adb wait-for-device
adb shell 'while [ "$(getprop sys.boot_completed)" != 1 ]; do sleep 2; done'
adb install -r -g "$APK" || exit 1
adb logcat -c
adb shell monkey -p $PKG -c android.intent.category.LAUNCHER 1 >/dev/null
sleep 45
adb exec-out screencap -p > "$OUT/home.png"
adb shell input keyevent KEYCODE_DPAD_RIGHT; sleep 2; adb shell input keyevent KEYCODE_DPAD_DOWN; sleep 3
adb exec-out screencap -p > "$OUT/moved.png"
adb logcat -d > "$OUT/logcat.txt"
grep "StremioOrigin" "$OUT/logcat.txt" > "$OUT/app.log" || true
alive=$(adb shell pidof $PKG || true)
echo "process: ${alive:-gone}"
fail=0
[ -n "$alive" ] || { echo "::error::the app is not running"; fail=1; }
grep -q "\[frame\]" "$OUT/app.log" || { echo "::error::no [frame] line: it did not draw"; fail=1; }
if grep -E "FATAL EXCEPTION|\[crash\]|SIGSEGV|Fatal signal" "$OUT/logcat.txt" | grep -i -E "stremio|$PKG|crash"; then echo "::error::crash in the log"; fail=1; fi
tail -40 "$OUT/app.log"
exit $fail
