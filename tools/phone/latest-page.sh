#!/bin/bash
# A page that sends the phone straight to the newest APK: the stable link handed out (gh-pages: phone/ and phone/<abi>/).
# It asks GitHub for the newest phone release as it opens (a copy of the page kept by the browser still finds it), and
# falls back to the version written in it.
#   tools/phone/latest-page.sh <version> <abi> <out.html>
set -euo pipefail
VER="$1"; ABI="$2"; OUT="$3"; mkdir -p "$(dirname "$OUT")"
URL="https://github.com/PharmaJava/webos/releases/download/phone-v$VER/stremio-claude-phone-$VER-$ABI.apk"
cat > "$OUT" <<HTML
<!doctype html><html lang="es"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<meta http-equiv="Cache-Control" content="no-cache"><title>Stremio Claude</title>
<style>body{font:18px/1.5 system-ui,sans-serif;background:#0c0b11;color:#eee;display:flex;min-height:100vh;align-items:center;justify-content:center;margin:0;text-align:center;padding:16px}a{color:#9b8cff}</style>
</head><body><div><h1 id="t">Stremio Claude</h1><p>Descargando&hellip; / Downloading&hellip;</p><p><a id="a" href="$URL">stremio-claude-phone-$VER-$ABI.apk</a></p></div>
<script>
var ABI="$ABI", url="$URL", ver="$VER", done=false;
function go(u,v){ if(done) return; done=true; var a=document.getElementById("a"); a.href=u; a.textContent="stremio-claude-phone-"+v+"-"+ABI+".apk"; document.getElementById("t").textContent="Stremio Claude "+v; location.replace(u); }
function num(v){ var p=v.split("."); return (+p[0])*1e6+(+p[1])*1e3+(+p[2]); }
try { var x=new XMLHttpRequest(); x.open("GET","https://api.github.com/repos/PharmaJava/webos/releases?per_page=30"); x.timeout=4000;
  x.onload=function(){ try { var best=null, bv=""; JSON.parse(x.responseText).forEach(function(r){ var m=/^phone-v(\d+\.\d+\.\d+)$/.exec(r.tag_name||""); if(!m||r.draft||r.prerelease) return;
      (r.assets||[]).forEach(function(s){ if(s.name==="stremio-claude-phone-"+m[1]+"-"+ABI+".apk" && (!best||num(m[1])>num(bv))){ best=s.browser_download_url; bv=m[1]; } }); });
    if(best && num(bv)>=num(ver)) go(best,bv); else go(url,ver); } catch(e){ go(url,ver); } };
  x.onerror=x.ontimeout=function(){ go(url,ver); }; x.send(); } catch(e){ go(url,ver); }
setTimeout(function(){ go(url,ver); }, 5000);
</script></body></html>
HTML
