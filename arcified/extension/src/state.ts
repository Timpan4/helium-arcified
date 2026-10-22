export const STORAGE_KEY = 'arcified-lab-v1';

export interface Space {
  id: string;
  name: string;
  urls: string[];
  savedAt: string;
}

export interface AppState {
  spaces: Space[];
}

export function safeWebUrl(value: unknown): string | null {
  if (typeof value !== 'string') return null;
  try {
    const url = new URL(value);
    if (url.protocol !== 'https:' && url.protocol !== 'http:') return null;
    if (url.username || url.password) return null;
    return url.href;
  } catch {
    return null;
  }
}

export function snapshotUrls(tabs: Array<{ url?: string }>): string[] {
  return tabs.map((tab) => safeWebUrl(tab.url))
    .filter((url): url is string => url !== null).slice(0, 120);
}

export function normalizeName(name: string): string {
  return name.trim().replace(/\s+/g, ' ').slice(0, 40);
}

export function normalizeStored(value: unknown): AppState {
  if (!value || typeof value !== 'object' || !('spaces' in value)) return { spaces: [] };
  const items = (value as { spaces?: unknown }).spaces;
  if (!Array.isArray(items)) return { spaces: [] };
  const spaces: Space[] = [];
  for (const raw of items.slice(0, 40)) {
    if (!raw || typeof raw !== 'object') continue;
    const item = raw as Record<string, unknown>;
    if (typeof item.id !== 'string' || typeof item.name !== 'string' || !Array.isArray(item.urls)) continue;
    const name = normalizeName(item.name);
    if (!name) continue;
    spaces.push({
      id: item.id,
      name,
      urls: snapshotUrls(item.urls.map((url: unknown) => ({ url: typeof url === 'string' ? url : undefined }))),
      savedAt: typeof item.savedAt === 'string' ? item.savedAt : ''
    });
  }
  return { spaces };
}

export function upsertSpace(state: AppState, space: Space): AppState {
  return {
    spaces: [space, ...state.spaces.filter((existing) => existing.id !== space.id)].slice(0, 40)
  };
}
