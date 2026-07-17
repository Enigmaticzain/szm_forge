import React, { createContext, useCallback, useContext, useEffect, useState, ReactNode } from 'react';

interface TooltipContextType {
  showTooltips: boolean;
  setShowTooltips: (value: boolean) => void;
  /** Returns title string for native tooltip, or undefined when disabled */
  t: (text: string) => string | undefined;
}

const TooltipContext = createContext<TooltipContextType>({
  showTooltips: true,
  setShowTooltips: () => {},
  t: (text) => text,
});

const STORAGE_KEY = 'szm-show-tooltips';

export const TooltipProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [showTooltips, setShowTooltipsState] = useState(() => {
    const stored = localStorage.getItem(STORAGE_KEY);
    if (stored === '0') return false;
    if (stored === '1') return true;
    return true;
  });

  const setShowTooltips = useCallback((value: boolean) => {
    localStorage.setItem(STORAGE_KEY, value ? '1' : '0');
    setShowTooltipsState(value);
  }, []);

  const t = useCallback(
    (text: string) => (showTooltips && text ? text : undefined),
    [showTooltips]
  );

  useEffect(() => {
    document.documentElement.toggleAttribute('data-tooltips-off', !showTooltips);
  }, [showTooltips]);

  return (
    <TooltipContext.Provider value={{ showTooltips, setShowTooltips, t }}>
      {children}
    </TooltipContext.Provider>
  );
};

export const useTooltips = () => useContext(TooltipContext);
