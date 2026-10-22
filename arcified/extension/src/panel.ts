import { askLocalModel, type PageContext } from './ai.js';
import { STORAGE_KEY, type AppState, type Space, normalizeName, normalizeStored, safeWebUrl, snapshotUrls, upsertSpace } from './state.js';

function get<T extends HTMLElement>(id: string): T {
  const node = document.getElementById(id);
  if (!node) throw new Error('Missing element ' + id);
  return node as T;
}

const spaces = get<HTMLDivElement>('spaces');
const tabs = get<HTMLDivElement>('tabs');
const status = get<HTMLParagraphElement>('status');
const contextStatus = get<HTMLParagraphElement>('context-status');
const answer = get<HTMLPreElement>('answer');
const nameInput = get<HTMLInputElement>('space-name');
const questionInput = get<HTMLTextAreaElement>('question');
const askButton = get<HTMLButtonElement>('ask');
let data: AppState = { spaces: [] };
let page: PageContext | null = null;

function message(error: unknown): string {
  return error instanceof Error ? error.message : String(error);
}

async function persist(): Promise<void> {
  await chrome.storage.local.set({ [STORAGE_KEY]: data });
}

function action(label: string, callback: () => Promise<void>): HTMLButtonElement {
  const button = document.createElement('button');
  button.type = 'button';
  button.textContent = label;
  button.addEventListener('click', () => { void callback().catch((error: unknown) => { status.textContent = message(error); }); });
  return button;
}

async function currentTabs(): Promise<ArcTab[]> {
  const window = await chrome.windows.getCurrent();
  if (window.id === undefined) throw new Error('No current window');
  return chrome.tabs.query({ windowId: window.id });
}

async function renderTabs(): Promise<void> {
  tabs.replaceChildren();
  for (const tab of await currentTabs()) {
    if (tab.id === undefined) continue;
    const id = tab.id;
    const item = action(tab.title || tab.url || 'Untitled tab', async () => {
      await chrome.tabs.update(id, { active: true });
      await renderTabs();
    });
    item.className = tab.active ? 'tab current' : 'tab';
    item.title = tab.url || '';
    tabs.append(item);
  }
}

function renderSpaces(): void {
  spaces.replaceChildren();
  if (!data.spaces.length) spaces.textContent = 'No saved snapshots yet.';
  for (const space of data.spaces) {
    const row = document.createElement('div');
    row.className = 'space';
    const name = document.createElement('strong');
    name.textContent = space.name;
    const open = action('Open ' + space.urls.length + ' tabs', async () => {
      if (!space.urls.length) throw new Error('Empty space snapshot');
      await chrome.windows.create({ url: space.urls, focused: true });
    });
    const remove = action('Delete', async () => {
      if (!confirm('Delete saved space ' + space.name + '?')) return;
      data = { spaces: data.spaces.filter((item) => item.id !== space.id) };
      await persist();
      renderSpaces();
    });
    row.append(name, open, remove);
    spaces.append(row);
  }
}

async function saveSpace(): Promise<void> {
  const name = normalizeName(nameInput.value);
  if (!name) throw new Error('Enter a space name.');
  const urls = snapshotUrls(await currentTabs());
  if (!urls.length) throw new Error('No ordinary website tabs to save.');
  const existing = data.spaces.find((space) => space.name.toLowerCase() === name.toLowerCase());
  const space: Space = { id: existing?.id || crypto.randomUUID(), name, urls, savedAt: new Date().toISOString() };
  data = upsertSpace(data, space);
  await persist();
  renderSpaces();
  nameInput.value = '';
  status.textContent = 'Saved ' + urls.length + ' tab URLs. Existing windows were not changed.';
}

async function readPage(): Promise<void> {
  const active = (await chrome.tabs.query({ active: true, currentWindow: true }))[0];
  const url = safeWebUrl(active?.url);
  if (!url || active?.id === undefined) throw new Error('Cannot read this browser page.');
  const allowed = await chrome.permissions.request({ origins: [new URL(url).origin + '/*'] });
  if (!allowed) {
    status.textContent = 'Page permission denied.';
    return;
  }
  const result = await chrome.scripting.executeScript({
    target: { tabId: active.id },
    func: () => ({
      title: document.title,
      selection: window.getSelection()?.toString().slice(0, 3000) || '',
      content: document.body?.innerText.slice(0, 15000) || ''
    })
  });
  if (!result[0]?.result) throw new Error('No page content.');
  page = { url, ...result[0].result };
  contextStatus.textContent = 'Captured ' + page.content.length + ' characters from ' + page.title + '. Nothing sent yet.';
}

async function ask(): Promise<void> {
  const question = questionInput.value.trim();
  if (!question) throw new Error('Enter a question.');
  askButton.disabled = true;
  answer.textContent = 'Contacting local LM Studio…';
  try {
    answer.textContent = await askLocalModel(question, page);
  } catch (error) {
    answer.textContent = 'AI error: ' + message(error);
  } finally {
    askButton.disabled = false;
  }
}

function bind(id: string, fn: () => Promise<void>): void {
  get<HTMLButtonElement>(id).addEventListener('click', () => {
    void fn().catch((error: unknown) => { status.textContent = message(error); });
  });
}

async function init(): Promise<void> {
  const stored = await chrome.storage.local.get(STORAGE_KEY);
  data = normalizeStored(stored[STORAGE_KEY]);
  renderSpaces();
  await renderTabs();
  status.textContent = 'Snapshots are not live Arc Spaces. Data stays on this device.';
}

bind('save-space', saveSpace);
bind('refresh-tabs', renderTabs);
bind('read-page', readPage);
bind('ask', ask);
void init().catch((error: unknown) => { status.textContent = message(error); });
