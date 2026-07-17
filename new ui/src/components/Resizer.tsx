import React, { useCallback, useEffect, useState } from 'react';

interface ResizerProps {
  onResize: (delta: number) => void;
  orientation?: 'vertical' | 'horizontal';
  className?: string;
}

export const Resizer: React.FC<ResizerProps> = ({ onResize, orientation = 'vertical', className = '' }) => {
  const [isResizing, setIsResizing] = useState(false);

  const handleMouseDown = useCallback((e: React.MouseEvent) => {
    e.preventDefault();
    setIsResizing(true);
  }, []);

  useEffect(() => {
    const handleMouseMove = (e: MouseEvent) => {
      if (!isResizing) return;
      onResize(orientation === 'vertical' ? e.movementX : e.movementY);
    };

    const handleMouseUp = () => {
      setIsResizing(false);
    };

    if (isResizing) {
      document.addEventListener('mousemove', handleMouseMove);
      document.addEventListener('mouseup', handleMouseUp);
      // Change cursor globally while resizing
      document.body.style.cursor = orientation === 'vertical' ? 'col-resize' : 'row-resize';
      document.body.style.userSelect = 'none';
    } else {
      document.body.style.cursor = '';
      document.body.style.userSelect = '';
    }

    return () => {
      document.removeEventListener('mousemove', handleMouseMove);
      document.removeEventListener('mouseup', handleMouseUp);
    };
  }, [isResizing, onResize, orientation]);

  return (
    <div
      onMouseDown={handleMouseDown}
      className={`
        ${orientation === 'vertical' ? 'w-1.5 cursor-col-resize h-full' : 'h-1.5 cursor-row-resize w-full'}
        bg-forge-border/20 hover:bg-forge-accent/50 transition-colors z-20 flex-shrink-0 flex items-center justify-center
        ${className}
      `}
    >
      {orientation === 'vertical' && (
        <div className="w-[1px] h-8 bg-forge-text-muted/30" />
      )}
    </div>
  );
};
