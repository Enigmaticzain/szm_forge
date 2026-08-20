import React, { createContext, useContext, useState, useCallback, ReactNode } from 'react';

export type ThemeMode = 'theme1' | 'theme2';

interface ThemeContextType {
  activeTheme: ThemeMode;
  setTheme: (theme: ThemeMode) => void;
}

const ThemeContext = createContext<ThemeContextType>({
  activeTheme: 'theme2',
  setTheme: () => {},
});

export const useTheme = () => useContext(ThemeContext);

export const ThemeProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [activeTheme, setActiveTheme] = useState<ThemeMode>(() => {
    return (localStorage.getItem('szm-theme') as ThemeMode) || 'theme2';
  });

  const setTheme = useCallback((theme: ThemeMode) => {
    localStorage.setItem('szm-theme', theme);
    setActiveTheme(theme);
  }, []);

  return (
    <ThemeContext.Provider value={{ activeTheme, setTheme }}>
      {children}
    </ThemeContext.Provider>
  );
};
