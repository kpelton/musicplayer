const NATIVE_HOST = "musicplayer.ytdlp";
const MENU_ID = "download-with-ytdlp";

function isYouTubeUrl(value) {
  if (!value) {
    return false;
  }

  try {
    const url = new URL(value);
    const hostname = url.hostname.toLowerCase();

    return (url.protocol === "http:" || url.protocol === "https:") &&
      (hostname === "youtube.com" ||
       hostname.endsWith(".youtube.com") ||
       hostname === "youtu.be");
  } catch (error) {
    return false;
  }
}

function selectedUrl(info, tab) {
  if (info && info.linkUrl && isYouTubeUrl(info.linkUrl)) {
    return info.linkUrl;
  }

  if (tab && isYouTubeUrl(tab.url)) {
    return tab.url;
  }

  return null;
}

function showNotice(title, message) {
  return browser.notifications.create({
    type: "basic",
    title: title,
    message: message
  }).catch(() => undefined);
}

function downloadUrl(url) {
  if (!url) {
    return showNotice("yt-dlp download failed", "This is not a YouTube URL.");
  }

  showNotice("yt-dlp download", "Starting download...");
  return browser.runtime.sendNativeMessage(NATIVE_HOST, {
    action: "download",
    url: url
  }).then((reply) => {
    if (!reply || !reply.ok) {
      throw new Error(reply && reply.error ? reply.error : "yt-dlp failed");
    }

    const filename = reply.path ? reply.path.split("/").pop() : "audio file";
    return showNotice("yt-dlp download complete", filename + " was saved.");
  }).catch((error) => {
    return showNotice("yt-dlp download failed", error.message || String(error));
  });
}

browser.menus.create({
  id: MENU_ID,
  title: "Download with yt-dlp",
  contexts: ["page", "link"],
  targetUrlPatterns: [
    "*://youtube.com/*",
    "*://*.youtube.com/*",
    "*://youtu.be/*"
  ]
});

browser.menus.onClicked.addListener((info, tab) => {
  if (info.menuItemId !== MENU_ID) {
    return;
  }

  const url = selectedUrl(info, tab);
  return downloadUrl(url);
});

browser.browserAction.onClicked.addListener((tab) => {
  downloadUrl(tab && isYouTubeUrl(tab.url) ? tab.url : null);
});
