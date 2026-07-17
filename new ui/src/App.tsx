import React, { useEffect } from 'react';
import { useTheme } from './store/ThemeContext';
import ModernApp from './ModernApp';
import LegacyApp from './LegacyApp';

const App: React.FC = () => {
  const { activeTheme } = useTheme();

  // Apply a global class to the body to scope theme-specific CSS
  useEffect(() => {
    document.body.className = activeTheme === 'theme1' ? 'legacy-theme' : 'modern-theme';
  }, [activeTheme]);

  if (activeTheme === 'theme1') {
    return <LegacyApp />;
  }

  return <ModernApp />;
};

export default App;
