#!/bin/bash
# The phone app on an emulator, as a person opens it: installs the APK, starts it, waits, and checks that the process
# lives, it drew frames and nothing crashed. Leaves dist/smoke-<name>/ with a screenshot and the log.
#   tools/phone/smoke.sh path/to/app.apk <name>
set -uo pipefail
APK="$1"; NAME="${2:-app}"; OUT="$(cd "$(dirname "$0")/../.." && pwd)/dist/smoke-$NAME"; mkdir -p "$OUT"
PKG=app.stremioclaude.phone
adb wait-for-device
adb shell 'while [ "$(getprop sys.boot_completed)" != 1 ]; do sleep 2; done'
adb shell getprop ro.product.cpu.abilist | tee "$OUT/abis.txt"
adb uninstall $PKG >/dev/null 2>&1
adb install -r -g "$APK" || exit 1
adb logcat -c
adb shell monkey -p $PKG -c android.intent.category.LAUNCHER 1 >/dev/null
sleep 40
adb exec-out screencap -p > "$OUT/screen.png"
adb logcat -d > "$OUT/logcat.txt"
grep "StremioOrigin" "$OUT/logcat.txt" > "$OUT/app.log" || true
alive=$(adb shell pidof $PKG || true)
echo "process: ${alive:-gone}"
fail=0
[ -n "$alive" ] || { echo "::error::$NAME: the app is not running"; fail=1; }
grep -q "\[frame\]" "$OUT/app.log" || { echo "::error::$NAME: no [frame] line: it did not draw"; fail=1; }
grep -E "FATAL EXCEPTION|Fatal signal|UnsatisfiedLinkError|AndroidRuntime|DEBUG   :|libc    :" "$OUT/logcat.txt" | head -60 | tee "$OUT/crash.txt"
[ -s "$OUT/crash.txt" ] && grep -qi -E "$PKG|stremio|libmain" "$OUT/crash.txt" && { echo "::error::$NAME: crash in the log"; fail=1; }
tail -40 "$OUT/app.log"
exit $fail
