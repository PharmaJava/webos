#!/bin/bash
# The formatting is rough and I'm not going to bother. Pull requests that fix it are welcome; complaints without one help neither me nor the end user.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ORIGIN_BUILD="${ORIGIN_BUILD:-$HOME/origin-build}"   # the private build folder outside the repo: toolchain, SDK, signer, keys, dev tools

echo "=== [1/5] Building native C library for Tizen 4/5 ARMv7 ==="
"$ROOT/tools/build-tizen-tpk.sh"

echo "=== [2/5] Stripping debug symbols ==="
python3 "$ROOT/tools/elfstrip.py" "$ROOT/build/tizen/libstremioorigin.so" "$ROOT/build/tizen/libstremioorigin.so.stripped"
python3 "$ORIGIN_BUILD/tizen/tools/elfcheck.py" "$ROOT/build/tizen/libstremioorigin.so.stripped" --mem
python3 "$ROOT/tools/tizen-libcheck.py" "$ROOT/build/tizen/libstremioorigin.so.stripped" # (what it asks of the TV: every Tizen from 4.0)

echo "=== [3/5] Staging assets and native binaries for TPK packaging ==="
mkdir -p "$ROOT/tizentv/lib" "$ROOT/tizentv/res/art" "$ROOT/tizentv/shared/res" "$ROOT/dist"
cp "$ROOT/build/tizen/libstremioorigin.so.stripped" "$ROOT/tizentv/lib/libstremioorigin.so"
cp "$ROOT/build/tizen/libstremioorigin.so.stripped" "$ROOT/tizentv/res/libstremioorigin.so"
cp -R "$ROOT/assets"/* "$ROOT/tizentv/res/art/"
# Samsung's own What's new (its items and its repo), over the shared one; with no items yet, no card at all rather than another platform's
if python3 -c 'import json,sys; sys.exit(0 if json.load(open(sys.argv[1])).get("items") else 1)' "$ROOT/tizentv/whatsnew.json" 2>/dev/null; then cp "$ROOT/tizentv/whatsnew.json" "$ROOT/tizentv/res/art/whatsnew.json"
else rm -f "$ROOT/tizentv/res/art/whatsnew.json"; echo "note: tizentv/whatsnew.json has no items: the package goes without a What's new card"; fi
# the libraries loaded on demand, the same builds as webOS (ARMv7 soft-float, glibc 2.17): the torrent engine
# (tools/build-engine.sh webos) and the sound converter (tools/build-ffmpeg.sh webos)
for l in libsbengine.so libsbdts.so; do if [ -f "$ROOT/build/webos/$l" ]; then cp "$ROOT/build/webos/$l" "$ROOT/tizentv/res/$l"; else rm -f "$ROOT/tizentv/res/$l"; echo "note: no build/webos/$l: the package goes without it"; fi; done
rm -f "$ROOT/tizentv/shared/res/StremioOrigin.png"; cp "$ROOT/tizentv/launcher-icon.png" "$ROOT/tizentv/shared/res/StremioOriginTile.png" # (on its own dark tile: Samsung's launcher lights a chosen app white, and the white ORIGIN under the mark disappeared on it)
VER=$(sed -n 's/.*package="[^"]*" version="\([0-9.]*\)".*/\1/p' "$ROOT/tizentv/tizen-manifest.xml")
printf '%s' "$VER" > "$ROOT/tizentv/res/versao.txt"
cp "$ROOT/tizentv/silence.wav" "$ROOT/tizentv/res/silence.wav" # (one second of silence: host D plays it shortly after the start, tizentv/hosts/HostNui.cs)
printf '{"id":"StrOrigTV1.StremioOrigin","version":"%s"}\n' "$VER" > "$ROOT/tizentv/res/appinfo.json"   # the running version, where the app reads it (its art folder's parent): About, the log's header, the update check

echo "=== [4/5] Building the .NET side: the hosts, then the picker that packs them ==="
cd "$ROOT/tizentv"
# One host per Samsung generation, each built against its own reference level (A: Tizen 4, B: 6.0, C: 6.5, D: 7 and
# later); the picker (StremioOrigin.csproj, the package's entry) starts the one this TV needs. SB_REFS: a folder of
# Samsung's reference packages for the newer levels, when they are not in the package cache yet.
rm -rf hostbin; mkdir -p hostbin
for h in a/HostA b/HostB c/HostC d/HostD; do
  "$HOME/.dotnet/dotnet" build "hosts/$h.csproj" -c Release -nologo -v q ${SB_REFS:+--source "$SB_REFS" --source "$HOME/.nuget/packages"} ${SB_NETLOG:+-p:SbNetLog=true -p:SbDevDir=$ORIGIN_BUILD/tizen/dev} -o "hostbin/.$(basename "$h")"
  cp "hostbin/.$(basename "$h")/StremioOrigin.$(basename "$h").dll" hostbin/
done
rm -rf hostbin/.Host*
for l in A B C D; do [ -f "hostbin/StremioOrigin.Host$l.dll" ] || { echo "Error: host $l was not built" >&2; exit 1; }; done
"$HOME/.dotnet/dotnet" build StremioOrigin.csproj -c Release -nologo -v q

RAW_TPK=$(find "$ROOT/tizentv/bin/Release" -name '*.tpk' | head -1)
if [ -z "$RAW_TPK" ]; then
    echo "Error: dotnet did not generate a .tpk file!" >&2
    exit 1
fi

echo "=== [5/5] Signing TPK with Developer Certificate ==="
OUT_TPK="$ROOT/dist/stremio-claude-samsung-$VER.tpk"
# (Stremio Claude) with the author's signer when there is one; else the package as Tizen.NET.Sdk signed it: Apps2Samsung signs it again for each TV anyway
if [ -f "$ORIGIN_BUILD/tizen/signer/sign.js" ]; then node "$ORIGIN_BUILD/tizen/signer/sign.js" "$RAW_TPK" "$OUT_TPK"; else cp "$RAW_TPK" "$OUT_TPK"; fi

# the library with its debug data stays out of the package: kept beside it for reading a crash, never shipped
mkdir -p "$ROOT/dist/dbg"; cp "$ROOT/build/tizen/libstremioorigin.so" "$ROOT/dist/dbg/libstremioorigin-$VER${SB_NETLOG:+-dev}.so"
# nothing of the build machine or its owner in a package that can leave it: the build stops here if there is
if [ -z "${SB_NETLOG:-}" ]; then python3 "$ROOT/tools/private-check.py" "$OUT_TPK" || { echo "Error: private strings in the package (above): not a package to hand out" >&2; mv "$OUT_TPK" "$OUT_TPK.private-strings"; exit 1; }
else echo "note: a development build (SB_NETLOG): not checked for private strings, never to be handed out"; fi

echo ""
echo "=== BUILD COMPLETE ==="
echo "Target: $OUT_TPK ($(ls -lh "$OUT_TPK" | awk '{print $5}'))"
