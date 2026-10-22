interface ArcTab {
  id?: number;
  windowId: number;
  url?: string;
  title?: string;
  active?: boolean;
}

declare namespace chrome {
  namespace runtime {
    const onInstalled: { addListener(callback: () => void): void };
    const onStartup: { addListener(callback: () => void): void };
  }
  namespace sidePanel {
    function setPanelBehavior(settings: { openPanelOnActionClick: boolean }): Promise<void>;
  }
  namespace tabs {
    function query(queryInfo: { active?: boolean; currentWindow?: boolean; windowId?: number }): Promise<ArcTab[]>;
    function update(tabId: number, updateInfo: { active: boolean }): Promise<ArcTab>;
  }
  namespace windows {
    function getCurrent(): Promise<{ id?: number }>;
    function create(createInfo: { url: string[]; focused?: boolean }): Promise<unknown>;
  }
  namespace storage {
    const local: {
      get(key: string): Promise<Record<string, unknown>>;
      set(items: Record<string, unknown>): Promise<void>;
    };
  }
  namespace permissions {
    function request(request: { origins: string[] }): Promise<boolean>;
  }
  namespace scripting {
    function executeScript<T>(injection: {
      target: { tabId: number };
      func: () => T;
    }): Promise<Array<{ result: T }>>;
  }
}
