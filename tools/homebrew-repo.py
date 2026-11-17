"""Writes Stremio Claude's Homebrew Channel repository (the gh-pages branch of PharmaJava/webos), as Stremio Origin's:
apps.json and api/apps.json (the list), api/apps/<id>/manifest.json, releases/latest.json, the description, the icon
and the latest package itself (the Channel downloads it from the same site).
   python3 tools/homebrew-repo.py <gh-pages worktree> dist/<id>_<version>_arm.ipk <version>   (then commit and push gh-pages)"""
import hashlib, io, json, os, shutil, sys

site, ipk, ver = sys.argv[1], sys.argv[2], sys.argv[3]
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
APP = 'tv.stremioorigin.app'
BASE = 'https://pharmajava.github.io/webos'
name = '%s_%s_arm.ipk' % (APP, ver)
assert os.path.basename(ipk) == name, (ipk, name)
sha = hashlib.sha256(open(ipk, 'rb').read()).hexdigest()

for f in os.listdir(site):  # only the latest package stays on the site
    if f.endswith('_arm.ipk') and f != name:
        os.remove(os.path.join(site, f))
shutil.copyfile(ipk, os.path.join(site, name))
shutil.copyfile(ROOT + 'pkg/icon.png', os.path.join(site, 'icon.png'))
short = io.open(ROOT + 'tools/homebrew/shortDescription.txt', encoding='utf-8').read().strip().replace('Stremio Origin', 'Stremio Claude')
full = io.open(ROOT + 'tools/homebrew/full_description.html', encoding='utf-8').read().replace('Stremio Origin', 'Stremio Claude')

man = {"id": APP, "version": ver, "type": "native", "title": "Stremio Claude",
       "appDescription": "Stremio Origin with Films, Series and Anime tabs and a daily Top 10. You bring the account and addons; it ships empty. Unofficial.",
       "iconUri": BASE + "/icon.png", "sourceUrl": "https://github.com/PharmaJava/webos", "rootRequired": False,
       "ipkUrl": BASE + "/" + name, "ipkHash": {"sha256": sha}}
lst = {"paging": {"page": 1, "count": 1, "maxPage": 1, "itemsTotal": 1},
       "packages": [{"id": APP, "title": "Stremio Claude", "iconUri": BASE + "/icon.png", "pool": "main",
                     "manifestUrl": BASE + "/api/apps/%s/manifest.json" % APP, "shortDescription": short,
                     "fullDescriptionUrl": "api/apps/%s/full_description.html" % APP, "manifest": man}]}


def put(rel, obj):
    p = os.path.join(site, rel); os.makedirs(os.path.dirname(p) or site, exist_ok=True)
    with io.open(p, 'w', encoding='utf-8', newline='\n') as f:
        f.write(obj if isinstance(obj, str) else json.dumps(obj, ensure_ascii=False, indent=2) + '\n')


put('apps.json', lst)
put('api/apps.json', lst)
put('api/apps/%s/manifest.json' % APP, man)
put('api/apps/%s/releases/latest.json' % APP, man)
put('api/apps/%s/full_description.html' % APP, full)
put('.nojekyll', '')
put('index.html', '<!doctype html><meta charset="utf-8"><title>Stremio Claude</title><p>Homebrew Channel repository: <code>%s/apps.json</code></p>\n' % BASE)
print(name, sha)
