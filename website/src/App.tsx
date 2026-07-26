import { useEffect, useState } from 'react';
import { Layers, Zap, Cpu, ArrowRight, ChevronDown } from 'lucide-react';
import { motion } from 'framer-motion';

function App() {
  const [scrollY, setScrollY] = useState(0);

  useEffect(() => {
    const handleScroll = () => setScrollY(window.scrollY);
    window.addEventListener('scroll', handleScroll);
    return () => window.removeEventListener('scroll', handleScroll);
  }, []);

  return (
    <div className="app-container">
      {/* Background glow effects */}
      <div className="glow-orb top-left" style={{ transform: `translateY(${scrollY * 0.2}px)` }} />
      <div className="glow-orb bottom-right" style={{ transform: `translateY(-${scrollY * 0.15}px)` }} />
      
      {/* Navbar */}
      <nav className="navbar glass-panel">
        <div className="nav-brand">
          <div className="brand-logo">
            <Layers size={24} color="#f59e0b" />
          </div>
          <span className="brand-text">SZM Forge</span>
        </div>
        <div className="nav-links">
          <a href="#features">Features</a>
          <a href="#about">About</a>
          <button className="nav-btn">Get Started</button>
        </div>
      </nav>

      {/* Hero Section */}
      <header className="hero-section">
        <motion.div 
          className="hero-content"
          initial={{ opacity: 0, y: 30 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.8, ease: "easeOut" }}
        >
          <div className="badge glass-panel">
            <Zap size={14} className="badge-icon" />
            <span>The Next Generation Platform is coming to szmforge.com</span>
          </div>
          
          <h1 className="hero-title">
            Engineer the <span className="text-gradient">Future</span>
          </h1>
          
          <p className="hero-subtitle">
            SZM Forge is an advanced, physics-driven engineering platform. 
            Design, simulate, and analyze complex mechanical assemblies with real-time feedback.
          </p>

          <div className="hero-actions">
            <button className="btn-primary">
              Request Early Access
              <ArrowRight size={18} />
            </button>
            <button className="btn-secondary">
              View Documentation
            </button>
          </div>
        </motion.div>

        <motion.div 
          className="hero-scroll-indicator"
          animate={{ y: [0, 10, 0] }}
          transition={{ repeat: Infinity, duration: 2 }}
        >
          <ChevronDown size={24} />
        </motion.div>
      </header>

      {/* Features Showcase */}
      <section id="features" className="features-section">
        <div className="section-header">
          <h2 className="section-title">Built for Performance</h2>
          <p className="section-desc">Everything you need to design robust mechanical systems.</p>
        </div>

        <div className="features-grid">
          <motion.div 
            className="feature-card glass-panel"
            whileHover={{ y: -5, scale: 1.02 }}
            transition={{ type: "spring", stiffness: 300 }}
          >
            <div className="feature-icon-wrapper">
              <Cpu size={24} className="feature-icon" />
            </div>
            <h3>Real-Time Physics</h3>
            <p>Experience true-to-life stress, thermal, and friction simulations instantly as you design.</p>
          </motion.div>
          
          <motion.div 
            className="feature-card glass-panel"
            whileHover={{ y: -5, scale: 1.02 }}
            transition={{ type: "spring", stiffness: 300 }}
          >
            <div className="feature-icon-wrapper">
              <Layers size={24} className="feature-icon" />
            </div>
            <h3>Procedural Assemblies</h3>
            <p>Generate complex chain and sprocket systems dynamically with advanced CSG algorithms.</p>
          </motion.div>
          
          <motion.div 
            className="feature-card glass-panel"
            whileHover={{ y: -5, scale: 1.02 }}
            transition={{ type: "spring", stiffness: 300 }}
          >
            <div className="feature-icon-wrapper">
              <Zap size={24} className="feature-icon" />
            </div>
            <h3>AI Analytics</h3>
            <p>Predict failure modes and receive intelligent design optimization suggestions.</p>
          </motion.div>
        </div>
      </section>

      {/* Footer */}
      <footer className="footer glass-panel">
        <div className="footer-content">
          <div className="footer-brand">
            <Layers size={20} color="#f59e0b" />
            <span>SZM Forge</span>
          </div>
          <div className="footer-copy">
            &copy; {new Date().getFullYear()} Enigmaticzain. All rights reserved.
          </div>
        </div>
      </footer>
    </div>
  );
}

export default App;
