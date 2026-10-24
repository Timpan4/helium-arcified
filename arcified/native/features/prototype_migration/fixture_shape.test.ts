import { expect, test } from 'bun:test';
import { normalizeStored, STORAGE_KEY } from '../../../extension/src/state.ts';
import snapshots from './fixtures/snapshots.json';

test('synthetic migration fixture survives the real extension snapshot normalizer', () => {
  expect(STORAGE_KEY).toBe('arcified-lab-v1');
  expect(normalizeStored(snapshots)).toEqual(snapshots);
});
