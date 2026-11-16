#!/usr/bin/env python3
"""Nothing of the build machine or its owner in a package: fails (exit 1) when a package holds a private string.
Usage: private-check.py <package.tpk|.zip> [more packages]

Looked for in every file of the package, as plain text and as the two-byte text .NET assemblies keep their strings in:
  - paths of a build machine: /Users/, /home/<name>/, /private/tmp, /var/folders/
  - this machine's own: the user's name, the home folder's name, the computer's name, the build folders' names
    (the repository's folder, $ORIGIN_BUILD and what it links to)
  - a local network address (192.168.x.y, but for the two stock ranges a torrent library names)
  - what a development build carries (SB_TPK_NETLOG)
  - every line of $ORIGIN_BUILD/private-strings.txt, when there is one (names kept out of this repository)
Also: no debug or key file (.pdb, .dbg, keys.env, .p12, .jks), and no user or group id stored with the files.
The signing certificates inside the signature files are read too; a private word in one is said (warn), not failed:
changing a certificate is the owner's decision.
Nothing private is written here: what is looked for comes from the machine it runs on."""
import getpass, os, re, socket, struct, sys, zipfile

def words():
    w = set()
    def add(x):
        x = (x or "").strip()
        if len(x) >= 4: w.add(x.lower())
    add(getpass.getuser()); add(os.path.basename(os.path.expanduser("~")))
    add(socket.gethostname().split(".")[0])
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__))); add(os.path.basename(root))
    ob = os.environ.get("ORIGIN_BUILD") or os.path.expanduser("~/origin-build")
    add(os.path.basename(ob.rstrip("/"))); add(os.path.basename(os.path.realpath(ob)))
    extra = os.path.join(ob, "private-strings.txt")
    if os.path.isfile(extra):
        for ln in open(extra, encoding="utf-8", errors="replace"):
            ln = ln.strip()
            if ln and not ln.startswith("#"): w.add(ln.lower())
    return sorted(w)

PATHS = [rb"/Users/", rb"/private/tmp", rb"/var/folders/", rb"/home/[a-z][a-z0-9_-]*/"]   # as written, capitals and all
LAN = re.compile(rb"192\.168\.(\d{1,3})\.(\d{1,3})")
DEV = [b"SB_TPK_NETLOG"]
BAD_FILES = (".pdb", ".dbg", ".p12", ".jks")

def wide(b):  # the same text as .NET keeps it: a zero byte after each letter
    return b"".join(bytes([c, 0]) for c in b)

def scan(name, data, ws):
    found = []
    low = data.lower()
    for w in ws:
        b = w.encode("utf-8", "replace")
        for form, label in ((b, ""), (wide(b), " (two-byte text)")):
            i = low.find(form)
            if i >= 0: found.append((w + label, data[max(0, i - 40):i + len(form) + 40]))
    for p in PATHS:
        for form, label in ((p, ""), (None, " (two-byte text)")):
            if form is None:
                if b"[" in p: continue
                m = re.search(re.escape(wide(p)), data)
            else: m = re.search(form, data)
            if m: found.append((p.decode() + label, data[max(0, m.start() - 20):m.end() + 60]))
    for m in LAN.finditer(data):
        if (m.group(1), m.group(2)) not in ((b"0", b"0"), (b"255", b"255")): found.append(("a local network address", m.group(0))); break
    for d in DEV:
        if d in data: found.append((d.decode(), d))
    return found

def show(b):
    return "".join(chr(c) if 32 <= c < 127 else "." for c in b)[:110]

def main(args):
    if not args: print(__doc__); return 2
    ws = words(); bad = 0
    for pkg in args:
        try: z = zipfile.ZipFile(pkg)
        except Exception as e: print("FAIL %s: cannot be opened (%s)" % (pkg, e)); bad += 1; continue
        n = 0
        for info in z.infolist():
            base = info.filename.rsplit("/", 1)[-1].lower()
            if base.endswith(BAD_FILES) or base == "keys.env": print("FAIL %s: a debug or key file inside: %s" % (os.path.basename(pkg), info.filename)); bad += 1
            ex = info.extra; i = 0
            while i + 4 <= len(ex):   # a user or group id stored with the file (the "ux" and older "Ux" fields)
                hid, ln = struct.unpack_from("<HH", ex, i)
                if hid in (0x7875, 0x7855): print("FAIL %s: a user id stored with %s" % (os.path.basename(pkg), info.filename)); bad += 1; break
                i += 4 + ln
            if info.is_dir(): continue
            n += 1
            if base.endswith(".xml") and "signature" in base:   # the signing certificates sit encoded in these: read too
                import base64
                for c in re.findall(rb"<X509Certificate>\s*([A-Za-z0-9+/=\s]+?)\s*</X509Certificate>", z.read(info)):
                    try: der = base64.b64decode(c)
                    except Exception: continue
                    for what, where in scan(info.filename, der, ws):
                        # (said, not failed: the certificate is the owner's to change, a package is still made)
                        print("warn %s: %s in a signing certificate of %s: ...%s..." % (os.path.basename(pkg), what, info.filename, show(where)))
            for what, where in scan(info.filename, z.read(info), ws):
                print("FAIL %s: %s in %s: ...%s..." % (os.path.basename(pkg), what, info.filename, show(where))); bad += 1
            for what, where in scan(info.filename, info.filename.encode(), ws): print("FAIL %s: %s in a file's name: %s" % (os.path.basename(pkg), what, info.filename)); bad += 1
        if z.comment and scan("comment", z.comment, ws): print("FAIL %s: private text in the archive's comment" % os.path.basename(pkg)); bad += 1
        if not bad: print("ok   %s: %d files, no private strings (%d words of this machine and its list, the path forms, local addresses)" % (os.path.basename(pkg), n, len(ws)))
    return 1 if bad else 0

if __name__ == "__main__": sys.exit(main(sys.argv[1:]))
