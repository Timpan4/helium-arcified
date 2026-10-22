import { describe, expect, test } from 'bun:test';
import { normalizeName, normalizeStored, safeWebUrl, snapshotUrls, upsertSpace } from '../src/state.ts';

describe('workspace snapshot helpers', () => {
  test('rejects privileged URLs and credentials', () => {
    expect(snapshotUrls([
      { url: 'https://example.com' },
      { url: 'chrome://extensions' },
      { url: 'https://user:pass@example.com' },
      { url: 'http://localhost:3000' }
    ])).toEqual(['https://example.com/', 'http://localhost:3000/']);
    expect(safeWebUrl('javascript:alert(1)')).toBeNull();
  });

  test('normalizes untrusted persisted data', () => {
    expect(normalizeStored({
      spaces: [{ id: '1', name: '  Work  Space ', urls: ['https://example.com', 'file:///a'] }]
    })).toMatchObject({ spaces: [{ id: '1', name: 'Work Space', urls: ['https://example.com/'] }] });
    expect(normalizeStored(null)).toEqual({ spaces: [] });
  });

  test('replaces an existing space without affecting others', () => {
    const one = { id: '1', name: 'One', urls: [], savedAt: 'a' };
    const two = { id: '2', name: 'Two', urls: [], savedAt: 'b' };
    const next = upsertSpace({ spaces: [one, two] }, { ...one, name: 'Updated' });
    expect(next.spaces.map((x) => x.name)).toEqual(['Updated', 'Two']);
    expect(normalizeName('  My    Space ')).toBe('My Space');
  });
});
