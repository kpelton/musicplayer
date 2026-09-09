# Download with yt-dlp Firefox extension

This is a standalone Firefox extension. It adds **Download with yt-dlp** to
the right-click menu for YouTube pages and YouTube links. The downloaded MP3
is saved in `Music/MusicPlayer/YouTube`; it is not sent to MusicPlayer and it
does not modify the playlist.

## Requirements

- Firefox
- Python 3
- `yt-dlp` (or `youtube-dl`) available in `PATH`
- `ffmpeg` available in `PATH` for MP3 conversion

## Install the native host

From this directory, run:

```sh
./install-native-host.sh
```

Then open `about:debugging#/runtime/this-firefox`, choose **Load Temporary
Add-on**, and select `manifest.json` in this directory. The extension ID is
fixed in the manifest so the native host can authorize it. Firefox will add a
music/download icon to the toolbar; it can be clicked on a YouTube video to
start the same download as the context-menu option.

## Use it

Open a YouTube video, right-click the page, and choose **Download with
yt-dlp**. The same option is available when right-clicking a YouTube video
link. Firefox shows a notification when the download starts, succeeds, or
fails.
