import React, { useState, useRef, useEffect } from 'react';
import { HelpCircle } from 'lucide-react';
import { useTooltips } from '../../store/TooltipContext';
import { clsx } from 'clsx';

interface HelpHintProps {
  text: string;
  className?: string;
}

/** Inline (?) marker with rich floating tooltip — respects global show-tooltips setting */
export const HelpHint: React.FC<HelpHintProps> = ({ text, className }) => {
  const { showTooltips } = useTooltips();
  const [open, setOpen] = useState(false);
  const ref = useRef<HTMLSpanElement>(null);

  useEffect(() => {
    if (!open) return;
    const onDoc = (e: MouseEvent) => {
      if (ref.current && !ref.current.contains(e.target as Node)) setOpen(false);
    };
    document.addEventListener('mousedown', onDoc);
    return () => document.removeEventListener('mousedown', onDoc);
  }, [open]);

  if (!showTooltips || !text) return null;

  return (
    <span ref={ref} className={clsx('inline-flex align-middle ml-1 relative', className)}>
      <button
        type="button"
        className="text-forge-text-muted hover:text-forge-accent transition-colors p-0.5"
        aria-label={text}
        onMouseEnter={() => setOpen(true)}
        onMouseLeave={() => setOpen(false)}
        onFocus={() => setOpen(true)}
        onBlur={() => setOpen(false)}
      >
        <HelpCircle size={12} />
      </button>
      {open && (
        <span
          role="tooltip"
          className="absolute z-[200] left-1/2 -translate-x-1/2 bottom-full mb-1 w-56 px-2.5 py-2 rounded-md text-[10px] leading-snug text-forge-text bg-forge-panel border border-forge-border shadow-xl pointer-events-none"
        >
          {text}
        </span>
      )}
    </span>
  );
};

interface TipButtonProps extends React.ButtonHTMLAttributes<HTMLButtonElement> {
  tip?: string;
  children: React.ReactNode;
}

/** Button that applies title + aria when tooltips enabled */
export const TipButton: React.FC<TipButtonProps> = ({ tip, children, className, ...rest }) => {
  const { t } = useTooltips();
  const title = t(tip ?? '');
  return (
    <button type="button" className={className} title={title} aria-label={title ?? undefined} {...rest}>
      {children}
    </button>
  );
};
