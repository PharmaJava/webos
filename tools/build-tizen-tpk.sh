#!/bin/bash
# The formatting is rough and I'm not going to bother. Pull requests that fix it are welcome; complaints without one help neither me nor the end user.
# Builds Stremio Origin as a native shared library for Samsung Tizen 4/5 (ARMv7 softfp)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ORIGIN_BUILD="${ORIGIN_BUILD:-$HOME/origin-build}"   # the private build folder outside the repo: toolchain, SDK, signer, keys, dev tools
B="$ROOT/build/tizen"; TP="$ROOT/third_party"; OBJ="$B/obj${SB_NETLOG:+-dev}"; mkdir -p "$B/tp" "$OBJ" # (a development build keeps its own objects)
ZIG="${ZIG:-$ORIGIN_BUILD/zig-aarch64-macos-0.17.0/zig}"
SDK="${SDK:-$ORIGIN_BUILD/sdk/arm-webos-linux-gnueabi_sdk-buildroot/arm-webos-linux-gnueabi/sysroot}"

CC="$ZIG cc -target arm-linux-gnueabi.2.24 -mcpu=cortex_a9 -mfloat-abi=softfp"
WEBPCC="$ZIG cc -target arm-linux-gnueabi.2.24 -mcpu=cortex_a9-neon -mfloat-abi=softfp"
CXX="$ZIG c++ -target arm-linux-gnueabi.2.24 -mcpu=cortex_a9 -mfloat-abi=softfp"
AR="$ZIG ar"

# No path of the build machine in what is built: file names in asserts, logs and debug data are written as /src/...,
# /toolchain/... (the same in tools/build-tizen-deps.sh; the package is checked for it at the end, tools/private-check.py)
PMAP="-ffile-prefix-map=$HOME=/home -ffile-prefix-map=$ORIGIN_BUILD=/toolchain -ffile-prefix-map=$(cd "$ORIGIN_BUILD" && pwd -P)=/toolchain -ffile-prefix-map=$ROOT=/src"
OPT="-O2 -fno-strict-aliasing -ffunction-sections -fdata-sections -fPIC $PMAP"
APPID="${APPID:-StrOrigTV1.StremioOrigin}"
TFLAGS="${SB_NETLOG:+-DSB_TPK_NETLOG=\"$SB_NETLOG\"} -DSB_OWNER=\"PharmaJava\" -DSB_REPO=\"webos\" -DSB_PKG_PREFIX=\"stremio-claude-samsung-\" -DSB_PKG_SUFFIX=\".tpk\" -DSB_TPK=1 -DSB_TIZEN=1 -DSTBI_NO_THREAD_LOCALS -DSTBI_NO_THREAD_LOCAL -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -I$ROOT/build/tizen/deps/include -idirafter $SDK/usr/include" # (SDL and zlib: our own builds, linked in; tools/build-tizen-deps.sh)
CFLAGS="$OPT -std=c11 -DSB_APP_ID=\"$APPID\" -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers $TFLAGS -I$ROOT/src -I$TP/freetype/include -I$TP/harfbuzz/src -I$TP/fribidi/lib -I$TP/config/fribidi -I$TP/cjson -I$TP/stb -I$TP/qrcodegen -I$TP/webp -I$TP/crypto -I$TP/libass -include $ROOT/src/platform/tizen/host_bridge.h"

"$ROOT/tools/build-tizen-deps.sh" # SDL and zlib for the TV, built once

# Generate sbkeys.h if not present
mkdir -p "$B"
export B="$ROOT/build/tizen" ORIGIN_BUILD
python3 - << 'PY'
import os, hashlib
src = os.path.join(os.environ['ORIGIN_BUILD'], 'keys.env')
out = os.path.join(os.environ['B'], 'sbkeys.h')
kv = {}
if os.path.isfile(src):
    for ln in open(src):
        ln = ln.strip()
        if ln and not ln.startswith('#') and '=' in ln: k, v = ln.split('=', 1); kv[k.strip()] = v.strip().strip('"\'')
def arr(name):
    v = kv.get(name, '').encode(); pad = hashlib.sha256(b'sb:' + name.encode()).digest()
    return 'static const unsigned char SBK_%s[] = { %s0 }, SBK_%s_PAD[32] = { %s };\n#define SBK_%s_LEN %d\n' % (name, ''.join('%d,' % (c ^ pad[i % 32]) for i, c in enumerate(v)), name, ','.join(str(x) for x in pad), name, len(v))
body = '/* generated keys */\n' + arr('TRAKT_CLIENT_ID') + arr('TMDB_API_KEY')
if not os.path.isfile(out) or open(out).read() != body: open(out, 'w').write(body)
PY
CFLAGS="$CFLAGS -I$B"

# Build third-party (built again when the compiler or its flags change: objects kept from other flags are not linked)
printf '%s\n' "$CC $OPT" > "$B/tp/.flags.new"; if ! cmp -s "$B/tp/.flags.new" "$B/tp/.flags"; then rm -f "$B/tp/libtp.a" "$B/tp/libass.a" "$B"/tp/*.o; rm -rf "$B/tp/ass"; mv "$B/tp/.flags.new" "$B/tp/.flags"; else rm -f "$B/tp/.flags.new"; fi
if [ ! -f "$B/tp/libtp.a" ]; then
  echo "== Building third-party libraries (ARMv7 softfp)..."
  TPQ="-w $OPT"
  FT="-DFT2_BUILD_LIBRARY -DFT_CONFIG_MODULES_H=<ftmodule-sb.h> -I$TP/config -I$TP/freetype/include"
  for f in base/ftbase base/ftinit base/ftsystem base/ftdebug base/ftbitmap base/ftglyph base/ftmm base/ftbbox base/ftsynth base/ftstroke \
           gzip/ftgzip truetype/truetype sfnt/sfnt smooth/smooth autofit/autofit psnames/psnames pshinter/pshinter cff/cff psaux/psaux; do
    $CC $TPQ $FT -c "$TP/freetype/src/$f.c" -o "$B/tp/ft_$(basename $f).o"
  done
  $CXX -w $OPT -std=c++11 -fno-exceptions -fno-rtti -DHAVE_FREETYPE=1 -DHAVE_PTHREAD=1 -I$TP/freetype/include -I$TP/harfbuzz/src \
    -c "$TP/harfbuzz/src/harfbuzz.cc" -o "$B/tp/harfbuzz.o"
  for f in "$TP"/fribidi/lib/fribidi*.c; do
    $CC $TPQ -DHAVE_CONFIG_H -I$TP/config/fribidi -I$TP/fribidi/lib -c "$f" -o "$B/tp/fb_$(basename "${f%.c}").o"
  done
  $CC $TPQ -c "$TP/cjson/cJSON.c" -o "$B/tp/cJSON.o"
  $CC $TPQ -c "$TP/qrcodegen/qrcodegen.c" -o "$B/tp/qrcodegen.o"
  for f in "$TP"/webp/src/dec/*.c "$TP"/webp/src/dsp/*.c "$TP"/webp/src/utils/*.c; do
    case "$(basename "$f")" in *enc*|*writer*|*quant_levels_utils*|*_sse*|*_msa*|*_mips*) continue;; esac
    $WEBPCC $TPQ -DWEBP_REDUCE_CSP -I$TP/webp -c "$f" -o "$B/tp/webp_$(basename "${f%.c}").o"
  done
  $AR rcs "$B/tp/libtp.a" "$B"/tp/*.o
fi

if [ ! -f "$B/tp/libass.a" ]; then
  echo "== Building libass..."
  mkdir -p "$B/tp/ass"
  for f in "$TP"/libass/libass/*.c "$TP"/libass/libass/c/*.c; do
    $CC -w $OPT -std=gnu11 -DHAVE_CONFIG_H -I$TP/config/libass -I$TP/libass/libass -I$TP/freetype/include -I$TP/harfbuzz/src -I$TP/fribidi/lib -I$TP/config/fribidi \
      -c "$f" -o "$B/tp/ass/$(basename "${f%.c}").o"
  done
  $CC -w $OPT -DFT2_BUILD_LIBRARY -DFT_CONFIG_MODULES_H="<ftmodule-sb.h>" -I$TP/config -I$TP/freetype/include -c "$TP/freetype/src/base/fttype1.c" -o "$B/tp/ass/ft_fttype1.o"
  $AR rcs "$B/tp/libass.a" "$B"/tp/ass/*.o
fi

# Build crypto
OBJS=()
for f in "$TP"/crypto/*.c; do
  o="$B/tp/crypto_$(basename "${f%.c}").o"
  if [ ! -f "$o" ] || [ "$f" -nt "$o" ]; then $CC -w $OPT -I$TP/crypto -c "$f" -o "$o"; fi
  OBJS+=("$o")
done

# Build app source files
echo "== Compiling Stremio Origin native C sources..."
DEVSRC=""; [ -n "${SB_NETLOG:-}" ] && DEVSRC="$ORIGIN_BUILD/tizen/dev/tpk_devnet.c" # (development tools: private, never in the repo)
# An object is built again when its source, ANY header of ours, or the compile flags are newer than it. Objects kept from
# before a header changed, or copied in from another folder with a fresh date, were linked as they were: a library that
# still called functions since removed, which the TV then refused to load.
NEWEST_H="$( (ls -t "$ROOT"/src/*.h "$ROOT"/src/*/*.h "$ROOT"/src/*/*/*.h "$B/sbkeys.h" 2>/dev/null || true) | head -1)"
printf '%s\n' "$CC $CFLAGS" > "$OBJ/.flags.new"; if ! cmp -s "$OBJ/.flags.new" "$OBJ/.flags"; then mv "$OBJ/.flags.new" "$OBJ/.flags"; else rm -f "$OBJ/.flags.new"; fi
for o in "$OBJ"/*.o; do [ -f "$o" ] || continue; n="$(basename "$o" .o)"; case "$n" in tpk_devnet*) continue;; esac # an object whose source is gone is removed
  [ -f "$ROOT/src/$(echo "$n" | sed 's#^core_#core/#; s#^ui_#ui/#; s#^screens_#screens/#; s#^platform_tizen_#platform/tizen/#')" ] || rm -f "$o"; done
for f in "$ROOT"/src/core/*.c "$ROOT"/src/ui/*.c "$ROOT"/src/screens/*.c "$ROOT"/src/platform/tizen/*.c "$ROOT"/src/main.c $DEVSRC; do
  [ -f "$f" ] || continue
  o="$OBJ/$(echo "${f#$ROOT/src/}" | tr / _ | sed "s#^.*tpk_devnet#tpk_devnet#").o"
  if [ ! -f "$o" ] || [ "$f" -nt "$o" ] || [ "$NEWEST_H" -nt "$o" ] || [ "$OBJ/.flags" -nt "$o" ]; then # (any header newer, or other flags: built again)
    $CC $CFLAGS -c "$f" -o "$o"
  fi
  OBJS+=("$o")
done

echo "== Linking libstremioorigin.so..."
# (--no-undefined: a call to anything neither in the library nor in the TV's C library or GLES stops the build here.
#  EGL is not linked: the library opens it by name at init, host_bridge.c, so a TV that has it under another file name
#  still loads the library and can say what is missing)
$CXX $OPT -shared -Wl,--hash-style=both -Wl,--gc-sections -Wl,--no-undefined \
  -o "$B/libstremioorigin.so" "${OBJS[@]}" \
  "$B/tp/libass.a" "$B/tp/libtp.a" \
  "$ROOT/build/tizen/deps/lib/libSDL2.a" "$ROOT/build/tizen/deps/lib/libz.a" -Wl,--version-script="$ROOT/src/platform/tizen/exports.map" \
  "$SDK/usr/lib/libGLESv2.so" -ldl -lpthread -lm # (from the TV only the C library and GLES; SDL and zlib inside, their symbols kept to themselves)

echo "== Checking ELF compliance for Tizen 4/5..."
python3 "$ORIGIN_BUILD/tizen/tools/elfcheck.py" "$B/libstremioorigin.so" --mem

echo "== Built successfully: $B/libstremioorigin.so"
ls -lh "$B/libstremioorigin.so"
