#!/bin/bash
# Stremio Claude for Android phones, built on Linux (GitHub Actions): the folders upstream's own scripts expect (tools/phone/*.sh,
# made for a Mac) are set up here, then the libraries, the app, the torrent engine and the download engine for both processors,
# and the APKs (one per processor). Needs ANDROID_HOME with NDK 27.2.12479018, JDK 17, Gradle, cmake, ninja (and Go for downloads).
#   PHONE_KEYSTORE / PHONE_KEYSTORE_PASSWORD: the release key (else Gradle's debug key: fine to try, not to publish)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PRIV="$HOME/stremiobrew-build"; mkdir -p "$PRIV/src" "$PRIV/mac" "$PRIV/android/keys"
ln -sfn "$ANDROID_HOME" "$PRIV/android/sdk"
export NDK="$ANDROID_HOME/ndk/27.2.12479018"
get() { [ -f "$2" ] || curl -fsSL --retry 3 -o "$2" "$1"; }
get https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-3.6.2/mbedtls-3.6.2.tar.bz2 "$PRIV/src/mbedtls-3.6.2.tar.bz2"
get https://curl.se/download/curl-8.11.1.tar.xz "$PRIV/src/curl-8.11.1.tar.xz"
get https://github.com/libsdl-org/SDL/releases/download/release-2.30.12/SDL2-2.30.12.tar.gz "$PRIV/src/SDL2-2.30.12.tar.gz"
[ -d "$PRIV/mac/SDL2-2.30.12" ] || tar -xzf "$PRIV/src/SDL2-2.30.12.tar.gz" -C "$PRIV/mac"
for ABI in arm64-v8a armeabi-v7a; do
  echo "::group::libraries and app ($ABI)"
  "$ROOT/tools/phone/build-deps.sh" "$ABI"
  "$ROOT/tools/phone/build.sh" "$ABI"
  echo "::endgroup::"
done
if [ "${ENGINE:-1}" = 1 ]; then for T in android-arm64 android-armv7; do echo "::group::torrent engine ($T)"; "$ROOT/tools/build-engine.sh" "$T"; echo "::endgroup::"; done; fi
if [ "${DOWNLOADS:-1}" = 1 ] && command -v go >/dev/null; then for T in android-arm64 android-armv7; do echo "::group::download engine ($T)"; "$ROOT/tools/gopeed/build.sh" "$T" || echo "(downloads: not built for $T)"; echo "::endgroup::"; done; fi
JNI="$ROOT/android-phone/app/src/main/jniLibs"
for ABI in arm64-v8a armeabi-v7a; do
  mkdir -p "$JNI/$ABI"; rm -f "$JNI/$ABI"/*.so; B="$ROOT/build/phone-$ABI"
  for f in "$B/deps/lib/libSDL2.so" "$B/deps/lib/libcurl.so" "$B/libmain.so"; do cp "$f" "$JNI/$ABI/"; done
  for f in "$B/libsbengine.so" "$B/libsbdl.so"; do [ -f "$f" ] && cp "$f" "$JNI/$ABI/" || echo "note: no $f"; done
done
if [ -n "${PHONE_KEYSTORE:-}" ] && [ -f "$PHONE_KEYSTORE" ]; then
  ( umask 077; printf 'storeFile=%s\nstorePassword=%s\nkeyAlias=%s\nkeyPassword=%s\n' "$PHONE_KEYSTORE" "$PHONE_KEYSTORE_PASSWORD" "${PHONE_KEY_ALIAS:-origin}" "$PHONE_KEYSTORE_PASSWORD" > "$PRIV/android/keys/phone.properties" )
fi
printf 'sdk.dir=%s\n' "$ANDROID_HOME" > "$ROOT/android-phone/local.properties"
(cd "$ROOT/android-phone" && "${GRADLE:-gradle}" --no-daemon -q :app:assembleRelease)
VER=$(sed -n "s/.*versionName '\([^']*\)'.*/\1/p" "$ROOT/android-phone/app/build.gradle" | head -1)
mkdir -p "$ROOT/dist"; OUT="$ROOT/android-phone/app/build/outputs/apk/release"
for ABI in arm64-v8a armeabi-v7a; do
  f="$OUT/app-$ABI-release.apk"; [ -f "$f" ] || f="$OUT/app-$ABI-release-unsigned.apk"
  cp "$f" "$ROOT/dist/stremio-claude-phone-$VER-$ABI.apk"
done
ls -l "$ROOT/dist"
