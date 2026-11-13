![Stremio Origin](docs/header-rounded.png)

<p align="center">
  <img src="docs/logo-pill.png" width="200" alt="Stremio Origin">
</p>

<h1 align="center">Stremio Claude</h1>

<p align="center"><strong>A fork of <a href="https://github.com/stremio-origin/webos">Stremio Origin</a> (same app id, installs over it and keeps its data), with fixes found on a real LG OLED (webOS 6.5):</strong></p>

- **No more closing mid-film.** libtorrent's `peer_connection::reject_piece` kept a stale end while erasing (`tools/engine/libtorrent.patch`); the engine's
  streaming window calls it whenever it frees far pieces, so the app crashed every few minutes (SIGSEGV in `memmove`, always the same address).
- **Subtitles that stay in time.** Two "unsure" references from one release (the same timing in two languages) no longer count as two opinions in the
  automatic sync (it had moved a film's subtitles by +45 s).
- **No TV screensaver over a film** (upstream PR #15) and **a pointer that really hides** on the arrows, on Back and when the hand is still (upstream PR #14,
  plus Back).
- **Back leaves the video at once**, with the controls up too.
- Updates come from this repository's releases. Offered in [FreeTV](https://github.com/PharmaJava/FreeTV)'s store.

<p align="center"><em>Unofficial and independent: not affiliated with, endorsed by or sponsored by Stremio or Anthropic. The names only describe compatibility and how it was made.</em></p>

---

<h1 align="center">Stremio Origin</h1>

<p align="center">A native TV app for Stremio addons, written in C.</p>

<p align="center"><em>Stremio™ is a trademark of its owner. Stremio Origin is independent and unofficial, and is not affiliated with, endorsed by or sponsored by Stremio. The name only describes compatibility.</em></p>

<p align="center">You bring your own Stremio account and your own addons. The app ships empty, hosts and indexes nothing, and plays only the stream links your addons return.</p>

## Features
- Home, Discover, Library and Search from your addons, with artwork, logos and trailers
- The best stream for your TV and connection, picked in one press
- Buttons and highlights take their colour from each title's logo or artwork
- People, reviews and a spotlight of what's new on every title
- A player with episodes, sources, subtitles and audio, skip intro and recap, and a next-episode card
- Styled subtitles as the film made them, subtitle sync, and optional forced-only subtitles in a chosen language
- DTS sound converted on the TV, and Dolby Vision on TVs that support it
- Torrent playback with a buffer sized to your TV, and an optional USB drive
- Cast to the TV from your phone (the Android app is almost done)
- Library, watched state and Continue Watching synced with your Stremio account
- 11 languages, including Arabic (right to left)
- Updates from inside the app

## Install
For LG TVs running webOS 3 and later (ARM). It should work on all of them, but it has only been tested on webOS 4.4. [Compatibility](docs/compatibility.md)

- With the Homebrew Channel: add the repository `https://stremio-origin.github.io/webos/apps.json`,
  then install Stremio Origin.
- Or download the `.ipk` from Releases and install it on a TV in Developer Mode.

## Privacy
- Sign-in goes directly to Stremio. Your account stays on the TV.
- No analytics. Nothing is sent to the developer.
- Logs stay on the TV. Sharing them is your choice: the app seals them and shows a short code.
- Trakt is optional. You link it on the TV with a code, and the sign-in stays on the TV.
- While it is linked, what you watch is added to your Trakt history, and the app reads your Trakt name and viewing stats to show them in Settings.

## Trakt
Stremio Origin uses Trakt but is not endorsed or certified by Trakt.

## Support
- Issues: https://github.com/stremio-origin/webos/issues
- Support: https://ko-fi.com/simussu

## Credits
- Interface sounds: [Interface Sounds](https://kenney.nl/assets/interface-sounds) by Kenney (CC0).
- The glass look: [liquid-glass-react](https://github.com/rdev/liquid-glass-react) by rdev; the app ships a modified version.
- Torrents: [Nuvio Engine](https://github.com/NuvioMedia/nuvio-engine) (GPL-3.0); the app ships a modified version.

## Licence
GPL-3.0, see [LICENSE](LICENSE). Third-party components and their licences: [third_party/licenses/README.txt](third_party/licenses/README.txt).
