function enableSidePanel(): void {
  void chrome.sidePanel.setPanelBehavior({ openPanelOnActionClick: true })
    .catch((error: unknown) => console.error('Arcified panel unavailable', error));
}

chrome.runtime.onInstalled.addListener(enableSidePanel);
chrome.runtime.onStartup.addListener(enableSidePanel);
enableSidePanel();
