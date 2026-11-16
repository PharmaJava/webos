# Compatibility

One file serves every Samsung TV: at start, a picker chooses the part made for the TV's Tizen version.

**The picker is very experimental.** I don't even know if recent Tizen versions accept a package built at the Tizen 4 level. On anything other than Tizen 4.0 to 5.5, an error page with a code or a black screen is very likely.

| Tizen | Status |
|---|---|
| 4.0 to 5.5 | Supported. Developed and tested on Tizen 5.5. |
| 6.0 and later | **Experimental.** An error page or a black screen is very likely. |
| 3.0 and earlier | Can't run it. |

I'm testing whether one package can serve many Tizen versions. If the reports say it can't, I may release one package per version.

## If a start fails
The TV shows a code, a few lines of text and a QR code. Scan it with a phone: it opens an issue with the same text filled in. Nothing is sent from the TV. Please send the code, it is how I learn which Tizen versions the picker can handle. A black screen with no code can still happen.

## Known limits
- Samsung TVs show no Dolby Vision.
- The app shows a new version's address as a QR code and you install it from a computer like the first time; it doesn't install itself.
