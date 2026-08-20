import React from 'react';
import { Resizer } from './Resizer';
import { useForgeStore } from '../store/ForgeStoreContext';

interface WorkspaceLayoutProps {
  left?: React.ReactNode;
  center: React.ReactNode;
  right?: React.ReactNode;
  showLeft?: boolean;
  showRight?: boolean;
}

/** Resizable left / center / right used by every workspace tab */
export const WorkspaceLayout: React.FC<WorkspaceLayoutProps> = ({
  left,
  center,
  right,
  showLeft = true,
  showRight = true,
}) => {
  const store = useForgeStore();

  return (
    <div className="flex-1 flex overflow-hidden min-h-0">
      {showLeft && left && (
        <>
          <div
            className="flex-shrink-0 flex flex-col overflow-hidden min-h-0"
            style={{ width: store.leftPanelWidth }}
          >
            {left}
          </div>
          <Resizer
            onResize={delta =>
              store.setLeftPanelWidth(prev => Math.max(200, Math.min(720, prev + delta)))
            }
          />
        </>
      )}

      <div className="flex-1 relative min-w-0 min-h-0">{center}</div>

      {showRight && right && store.showAIPanel && (
        <>
          <Resizer
            onResize={delta =>
              store.setRightPanelWidth(prev => Math.max(200, Math.min(900, prev - delta)))
            }
          />
          <div
            className="flex-shrink-0 overflow-hidden min-h-0"
            style={{ width: store.rightPanelWidth }}
          >
            {right}
          </div>
        </>
      )}
    </div>
  );
};
