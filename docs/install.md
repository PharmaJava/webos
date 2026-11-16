# Install Stremio Origin on a Samsung TV

You switch the TV to Developer Mode once, then install one file from a computer on the same network. About ten minutes.

**You need:** a Samsung TV (Tizen 4.0 or later; see [Compatibility](compatibility.md)), a computer on the same Wi-Fi or cable network, and the file `stremio-origin-tizen-1.1.0.tpk` from this repository's Releases page.

## Steps
1. **On the computer, note its IP address** (in its network settings; it looks like four numbers with dots).
2. **On the TV, open Apps.** With the remote, type **12345**. A Developer Mode box appears.
3. **Switch Developer mode to On**, type the computer's IP address, and choose OK.
4. **Restart the TV** (hold the power button until it restarts, or unplug it for a moment). Apps now shows "Develop Mode" at the top.
5. **On the computer, install [Apps2Samsung](https://github.com/Apps2Samsung/Apps2Samsung)** and open it. It finds the TVs on your network.
6. **Choose your TV, then choose the `.tpk` file** you downloaded, and install. If it asks you to sign in to a Samsung account, do: that is how it makes the certificate your TV asks for.
7. **On the TV, open Stremio Origin** from Apps and sign in with your Stremio account.

## Good to know
- **2023 and later TVs (Tizen 7 and later)** only accept a package signed with a certificate made for that TV. Apps2Samsung makes one when you sign in to Samsung.
- **Updating:** install the new file the same way. If the TV refuses it with "Author certificate not match", remove the app from the TV first and install again; that erases the app's sign-in and settings on the TV.
- **If the app shows a code instead of starting**, scan the QR code on that screen or tell me the code. See [Compatibility](compatibility.md).

## Other way: Samsung's own tools
Samsung's Tizen Studio can install an app too: open Device Manager, add the TV by its IP address under Remote Device Manager, switch its connection on, choose "Permit to install applications" on it, then install the package.
