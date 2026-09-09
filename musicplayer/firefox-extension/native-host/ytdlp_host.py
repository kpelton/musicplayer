#!/usr/bin/env python3
"""Firefox Native Messaging host for the standalone yt-dlp extension."""

import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys


MAX_MESSAGE_SIZE = 1024 * 1024


def send_message(message):
    encoded = json.dumps(message, ensure_ascii=False).encode("utf-8")
    sys.stdout.buffer.write(struct.pack("=I", len(encoded)))
    sys.stdout.buffer.write(encoded)
    sys.stdout.buffer.flush()


def read_message():
    header = sys.stdin.buffer.read(4)
    if len(header) != 4:
        return None

    message_size = struct.unpack("=I", header)[0]
    if message_size > MAX_MESSAGE_SIZE:
        raise ValueError("native message is too large")

    payload = sys.stdin.buffer.read(message_size)
    if len(payload) != message_size:
        raise ValueError("native message was truncated")
    return json.loads(payload.decode("utf-8"))


def is_youtube_url(value):
    from urllib.parse import urlparse

    parsed = urlparse(value)
    hostname = (parsed.hostname or "").lower()
    return parsed.scheme in ("http", "https") and (
        hostname == "youtube.com" or
        hostname.endswith(".youtube.com") or
        hostname == "youtu.be"
    )


def music_directory():
    music_dir = os.environ.get("XDG_MUSIC_DIR")
    if not music_dir:
        try:
            result = subprocess.run(
                ["xdg-user-dir", "MUSIC"],
                check=False,
                capture_output=True,
                text=True,
            )
            if result.returncode == 0:
                music_dir = result.stdout.strip()
        except OSError:
            pass

    if not music_dir:
        music_dir = str(Path.home() / "Music")
    return Path(music_dir) / "MusicPlayer" / "YouTube"


def download(url):
    if not isinstance(url, str) or not is_youtube_url(url):
        return {"ok": False, "error": "Only YouTube URLs are supported."}

    program = shutil.which("yt-dlp") or shutil.which("youtube-dl")
    if not program:
        return {"ok": False, "error": "Install yt-dlp or youtube-dl first."}

    directory = music_directory()
    try:
        directory.mkdir(mode=0o700, parents=True, exist_ok=True)
    except OSError as error:
        return {"ok": False, "error": "Could not create download folder: " + str(error)}

    command = [
        program,
        "--no-playlist",
        "--extract-audio",
        "--audio-format", "mp3",
        "--audio-quality", "0",
        "--newline",
        "--embed-metadata",
        "--parse-metadata", "%(uploader)s:%(meta_artist)s",
        "--print", "after_move:filepath",
        "--output", str(directory / "%(title)s.%(ext)s"),
        "--extractor-args", "youtube:player_client=android",
        url,
    ]

    print("ytdlp-host: executing: " + " ".join(repr(part) for part in command),
          file=sys.stderr,
          flush=True)
    try:
        result = subprocess.run(command, check=False, capture_output=True, text=True)
    except OSError as error:
        return {"ok": False, "error": "Could not start yt-dlp: " + str(error)}

    if result.stdout:
        print("ytdlp-host: stdout:\n" + result.stdout, file=sys.stderr, flush=True)
    if result.stderr:
        print("ytdlp-host: stderr:\n" + result.stderr, file=sys.stderr, flush=True)

    if result.returncode != 0:
        return {
            "ok": False,
            "error": "yt-dlp exited with status " + str(result.returncode),
        }

    downloaded_path = None
    for line in reversed(result.stdout.splitlines()):
        candidate = Path(line.strip())
        if candidate.is_file():
            downloaded_path = candidate
            break

    if downloaded_path is None:
        return {"ok": False, "error": "yt-dlp finished but no MP3 was found."}

    return {"ok": True, "path": str(downloaded_path)}


def main():
    try:
        message = read_message()
        if message is None:
            return
        if not isinstance(message, dict) or message.get("action") != "download":
            send_message({"ok": False, "error": "Unsupported action."})
            return
        send_message(download(message.get("url")))
    except Exception as error:  # Keep native messaging failures visible in Firefox.
        print("ytdlp-host: " + str(error), file=sys.stderr, flush=True)
        send_message({"ok": False, "error": str(error)})


if __name__ == "__main__":
    main()
