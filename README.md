![Stremio Origin](docs/header-rounded.png)

<p align="center">
  <img src="docs/logo-pill.png" width="200" alt="Stremio Origin" align="middle">
  &nbsp;&nbsp;&nbsp;&nbsp;
  <img src="docs/platform-lg.svg" height="72" alt="LG" align="middle">
</p>

<h1 align="center">Stremio Claude</h1>

<p align="center"><strong>A fork of <a href="https://github.com/stremio-origin/webos">Stremio Origin</a> that follows it release by release (now 1.1.1, with every improvement of the original app) and adds, on top (same app id: it installs over Stremio Origin and keeps its data):</strong></p>

- **Films and Series tabs**, each with today's **Top 10** (big rank numbers), popular, top rated, this year's, last year's and every genre, from Cinemeta;
  and an **Anime tab** with the anime catalogs of your addons. Tabs: Discover, Home, Films, Series, Library, Anime.
- **Subtitles that stay in time.** Two "unsure" references from one release (the same timing in two languages) no longer count as two opinions in the
  automatic sync.
- **The pointer when you want it:** moving the remote, turning or pressing the wheel brings it out, and only Back puts it away.
- **Back leaves the video at once**, with the controls up too.
- Our fix for the torrent engine closing the app mid-film (libtorrent `reject_piece`) is now part of Stremio Origin itself.
- **Straight to the point:** no tour after signing in, no news page after an update, no tip or GitHub cards (the guide stays in Settings).
- **No LG screensaver inside the app**, in the menus or over a video: the app answers the TV from the start (before, only once a video had played).
- **Picked from Stremio Origin's open PRs** (1.2.5): sharper Continue Watching cards (#55), long release names that wrap at their dots and dashes (#67), and **Similar from TMDB's recommendations** in your language (#80, opened by IMDb id so the addons find streams; Cinemeta's genre without a key).
- **Back asks before leaving** (1.2.6): at the top, a card asks "Leave the app?" with Stay focused; Back again stays, so nobody leaves by pressing Back again and again.
- **Viewer profiles** (1.3.0, Stremio Origin's PR #111, with its faster icons and launch splash): each person in the house has their own history, Continue Watching and Trakt, picked on "Who's watching?" (with one profile the app goes straight to Home, as before). The profile keyboard works with the Magic Remote's pointer and starts each word with a capital, a new profile goes on to its avatar, and there are **18 avatars**, superheroes among them (all drawn for this app).
- **Kids profiles** (1.4.0): hold OK on a profile, *Kids profile*. That profile sees only children's titles in every tab, Discover, its spotlight and Search: new on Disney+, Disney+ series, Pixar, Walt Disney Animation, DreamWorks, Illumination, Studio Ghibli, children's films and series (films rated G or PG, series in TMDB's Kids genre, never horror, thriller, crime or war). Needs a TMDB key; without one, Cinemeta's family and animation lists.
- **Platforms** (1.4.0): Home starts with a row of the platforms of your country, each with its own icon (Netflix, Disney+, HBO Max, Prime Video, Apple TV, Movistar Plus+, SkyShowtime, Filmin, Crunchyroll...), then Continue Watching and Live TV. Each opens a page of its own, as if its app were open: its Top 10 films and series, new films, series with new episodes, the best rated and its genres. Top 10 numbers as Netflix draws them, beside each poster.
- **Discover in order** (1.4.0, with a TMDB key): films and series in clear categories (trending today, popular, new to stream, new series, new episodes, top rated, hidden gems) with the genres in your language, then your addons' own catalogs.
- **Kids lock** (1.5.0): a parent PIN (four digits, kept on the TV). Lock any profile with it, and a child's profile gets a **daily limit** (30 min to 3 h) and a **bedtime** (19:00 to 22:00, until six in the morning): a notice five minutes before, then the film stops and a card offers another profile or half an hour more with the PIN. The PIN cannot be recovered if it is forgotten.
- **Surprise me** (1.5.0): the first badge among the platforms (and on a child's Home). Pick a mood and film, series or either, and one well-rated title opens (a film already watched is passed over).
- **Premieres** (1.5.0): on Home, films coming to streaming in your country (their digital release day) and new series, each card with its day ("Tomorrow", "In 5 days"); a notice on Home the day one comes out.
- **For you** (1.5.0): recommendations from what each profile watched (finished counts most, then half-seen, then saved; the latest weigh more; what was watched is left out): *For you* and *Because you watched X* on Home, *Films / Series for you* in their tabs. Each profile learns on its own; a child's gets children's titles only.
- **Over the film** (1.5.1): turning the Magic Remote's wheel brings the pointer out, with the controls; pressing the wheel clicks where it points (buttons, the time bar, Skip intro, the sources) and Back puts it away (Back again leaves the film). The sources tray stays on screen (its cards were cut on the left with many addons), an addon with no sources has no tab, and torrents that say they have no seeds are left out.
- **The Library shows what you watched** (1.5.2): a film or series watched without adding it (Stremio keeps it as a temporary entry, with its progress and its watched mark) now shows under *In progress* and *Watched*. *All*, *Films*, *Series* and *Collections* still list only what you added, and nothing is added to your account's library.
- **Add an addon by its link** (1.5.3): *Settings > Addons > Add an addon* shows a code; scan it and paste the addon's link on your phone (its `manifest.json`, or the `stremio://` link of an install button) and the TV adds it to your Stremio account, so it shows on all your devices. Several in a row; Back closes it. On the phone app: *Settings > Addons > Add an addon > Paste the link*.
- **From Stremio Origin's open pull requests** (1.5.4): *Incognito mode* in *Settings > Playback* (#125: nothing kept while it is on, no progress, Continue Watching, last stream or Trakt history; a pill under the clock says so) and the connection measured in the background at launch, with a *Speed test* row; locks around the LG player's tracks, the requests you make ahead of background ones, more debrid cached and uncached marks, the channel keys paging lists, the picture memory set by the TV's RAM, and Arabic text shaping (#121); Greek capitals (#119), Hungarian corrections (#26), and the app's cover in LG's recent apps (#78). On the phone: Incognito, the debrid marks and the translations.
- **Opened from FreeTV:** a title picked on [FreeTV](https://github.com/PharmaJava/FreeTV)'s home opens on its page here (`data/open.json`).
- **Android TV / Google TV** build (under Compatibility). Updates come from this repository's releases; offered in FreeTV's store.

**Install and update with the Homebrew Channel:** in the Channel, *Settings* → *Repositories*, add `https://pharmajava.github.io/webos/apps.json` and remove Stremio Origin's own (`https://stremio-origin.github.io/webos/apps.json`): both list the same app id, and Stremio Origin's older version cannot install over Stremio Claude. Every release updates this list (the `gh-pages` branch: `apps.json` and the package).

<p align="center"><em>Unofficial and independent: not affiliated with, endorsed by or sponsored by Stremio or Anthropic. The names only describe compatibility and how it was made.</em></p>

---

<h1 align="center">Stremio Origin for LG webOS</h1>

<p align="center">A native app for Stremio addons, written in C.</p>

<p align="center"><em>Stremio™ is a trademark of its owner. Stremio Origin is independent and unofficial, and is not affiliated with, endorsed by or sponsored by Stremio. LG is a trademark of its owner; its logo only shows which platform this is for.</em></p>

<p align="center">You bring your own Stremio account and your own addons. The app ships empty and plays only the stream links your addons return.</p>

## Features
- Seek previews: a picture of the film on the seek bar, for debrid and direct links
- Trakt: link with a code, scrobbling, rows on Home, watched marks and Continue Watching from Trakt
- Home, Discover, Library and Search, with the addons from your Stremio account
- Film collections in order, in the Library too, and the next film after the credits
- Live TV and sports from your addons, with All channels, Favorites, Recent and a preview as you browse
- The best stream picked in one press, and a sources tray with a tab for each addon
- Styled subtitles as the film made them, with fonts (Arabic too), size, colour, position, brightness and sync
- Forced-only subtitles; each show remembers its subtitles and audio, original language by default
- Chapters, skip intro and recap, and a next-episode card in two sizes
- Auto-play next episode, and a trailer behind Home
- Spoilers blurred for episodes you haven't watched, and upcoming episodes on Home
- A viewer notice as a film starts: an age and its reasons, from IMDb's Parents Guide
- A row for every catalog, with artwork, logos, trailers, people, reviews and ratings
- Titles and descriptions in your language
- Library, watched state and Continue Watching synced with your Stremio account
- Accent colour, a 12 or 24 hour clock and interface sounds
- Dolby Vision on TVs that support it
- DTS sound converted on the TV
- Torrents, with an optional USB drive
- Picture size: original, fill or zoom
- The app's own screen saver
- The pointer hides when idle, and OK presses what you point at
- 28 languages, Arabic included (right to left); updates and Share logs from the app

## Install
LG TVs on webOS 3 or later (ARM). With the Homebrew Channel, search for Stremio Origin. If it doesn't show up, add the repository `https://stremio-origin.github.io/webos/apps.json`. Or install the `.ipk` from [Releases](https://github.com/stremio-origin/webos/releases) with the TV in Developer Mode.

## Compatibility
Developed and tested on webOS 4.4, and reported working on webOS 4 to 25. [More](docs/compatibility.md).

**Android TV and Google TV (Sony Bravia, TCL, Philips, Chromecast with Google TV…), experimental:** the same app, built for Android
(`tools/build-android.sh`, see `androidtv/`), torrents included (the same engine and patches, built with the NDK: `tools/build-engine-android.sh`).
Install `StremioClaude-AndroidTV-<version>.apk` from the `android-tv-v*` release (with Downloader, a USB drive or `adb install`), or let
[FreeTV](https://github.com/PharmaJava/FreeTV) install it; FreeTV's picks open their title in it, as on webOS. On an emulator in CI it starts, reaches
the network, shows its sign-in screen and loads the torrent engine; video plays through Android's own player (Media3). Not yet on Android: the DTS
conversion. Not tried on a real TV yet.
Releases: `.github/workflows/android.yml` publishes `android-tv-v<version>` from `main`, signed with the `ORIGIN_KEYSTORE` /
`ORIGIN_KEYSTORE_PASSWORD` secrets (without them it builds but publishes nothing: Android updates only an app signed with the same key).

**Android phones and tablets:** the phone app (its own touch interface, from Stremio Origin's phone app) with the same extras as on the TV: profiles and a child's profile, the parental PIN, platforms, Top 10, For you, premieres, Surprise me, creating a Stremio account in the app, adding addons by link, downloads and the torrent engine. Install `stremio-claude-phone-<version>-arm64-v8a.apk` (or `-armeabi-v7a` for older phones) from the `phone-v*` release; it updates itself from there. Built from the `phone` branch (`.github/workflows/phone.yml`).

**Samsung TVs (Tizen):** the same app as on LG, from Stremio Origin's Tizen build (the `samsung` branch, `tools/build-tpk.sh`). Install `stremio-claude-samsung-<version>.tpk` from the `samsung-v*` release with Apps2Samsung (it signs it for your TV); it updates itself from there. Not tried on a real TV yet.

## Privacy
- Sign-in goes directly to Stremio. Your account stays on the TV.
- No analytics. Nothing is sent to the developer.
- Logs stay on the TV. Sharing them is your choice: Settings > About > Share logs.

## Viewer profiles
Choose a profile at launch or select your avatar in the top bar to switch viewers. Hold OK on a profile in the picker to manage it.

Profiles share the TV’s Stremio sign-in and addons. The default profile keeps the existing library and Continue Watching sync with Stremio. Additional profiles keep their library, watched state and playback progress on this TV. Each profile can link its own Trakt account from Settings.

## Trakt
Trakt is optional. You link it with a code and the sign-in stays on the TV. While it is linked, what you watch is added to your Trakt history. The app also reads your Trakt name and stats, history, watchlist, recommendations, watched marks and paused positions, only to show them in the app. Unlink any time: Settings > Trakt > Disconnect Trakt.

Stremio Origin uses Trakt but is not endorsed or certified by Trakt.

## Support
- <img src="docs/heart.svg" height="18" align="top"> Support the project / Request a feature: [Ko-fi](https://ko-fi.com/simussu) <img src="docs/heart.svg" height="18" align="top">
- <img src="docs/discord.svg" height="18" align="top"> [Discord](https://discord.gg/SJEtyV3kW6)
- Report bugs/issues: [stremio-origin/webos/issues](https://github.com/stremio-origin/webos/issues)

## Credits
- Interface sounds: [Interface Sounds](https://kenney.nl/assets/interface-sounds) by Kenney (CC0).
- The glass look: [liquid-glass-react](https://github.com/rdev/liquid-glass-react) by rdev; the app ships a modified version.
- Torrents: [Nuvio Engine](https://github.com/NuvioMedia/nuvio-engine) (GPL-3.0); the app ships a modified version.

## Licence
GPL-3.0, see [LICENSE](LICENSE). Third-party components and their licences: [third_party/licenses/README.txt](third_party/licenses/README.txt).

Provided as is, for personal use on devices you own.
