import React, { useEffect, useRef } from 'react';

export const ActionObserver: React.FC = () => {
  const isObserving = useRef(false);

  useEffect(() => {
    // Check if we are actively observing
    const checkStatus = async () => {
      try {
        const res = await fetch('http://localhost:8000/api/ai/observe/stats');
        if (res.ok) {
          isObserving.current = true; // For now we assume if we can reach it, we'll try sending.
        }
      } catch (e) {
        isObserving.current = false;
      }
    };
    checkStatus();

    const handleClick = (e: MouseEvent) => {
      if (!isObserving.current) return;
      
      const target = e.target as HTMLElement;
      if (!target) return;

      // Extract semantic context
      let text = target.innerText?.substring(0, 50).trim() || '';
      let type = target.tagName;
      let id = target.id || '';

      // Skip empty clicks on backgrounds
      if (type === 'DIV' && !text && !id) return;

      fetch('http://localhost:8000/api/ai/observe/action', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          type: 'MOUSE_CLICK',
          x: e.clientX,
          y: e.clientY,
          context: {
            element_id: id,
            element_type: type,
            element_text: text
          }
        })
      }).catch(() => {});
    };

    const handleKeyDown = (e: KeyboardEvent) => {
      if (!isObserving.current) return;
      
      if (e.ctrlKey || e.metaKey || e.altKey) {
        const keys = [];
        if (e.ctrlKey) keys.push('Ctrl');
        if (e.shiftKey) keys.push('Shift');
        if (e.altKey) keys.push('Alt');
        if (e.metaKey) keys.push('Meta');
        keys.push(e.key.toUpperCase());

        fetch('http://localhost:8000/api/ai/observe/action', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            type: 'KEYBOARD_SHORTCUT',
            keys: keys
          })
        }).catch(() => {});
      }
    };

    document.addEventListener('click', handleClick, true);
    document.addEventListener('keydown', handleKeyDown, true);

    return () => {
      document.removeEventListener('click', handleClick, true);
      document.removeEventListener('keydown', handleKeyDown, true);
    };
  }, []);

  return null; // Hidden component
};
