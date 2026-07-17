import { useEffect, useRef, useCallback } from 'react';

export function useAnimationFrame(callback: (deltaTime: number) => void, active = true) {
  const requestRef = useRef<number>(0);
  const previousTimeRef = useRef<number>(0);

  const animate = useCallback((time: number) => {
    if (previousTimeRef.current !== 0) {
      const deltaTime = time - previousTimeRef.current;
      callback(deltaTime);
    }
    previousTimeRef.current = time;
    requestRef.current = requestAnimationFrame(animate);
  }, [callback]);

  useEffect(() => {
    if (active) {
      requestRef.current = requestAnimationFrame(animate);
      return () => cancelAnimationFrame(requestRef.current);
    }
  }, [active, animate]);
}

export function useInterval(callback: () => void, delay: number) {
  const savedCallback = useRef(callback);
  
  useEffect(() => {
    savedCallback.current = callback;
  }, [callback]);

  useEffect(() => {
    const tick = () => savedCallback.current();
    const id = setInterval(tick, delay);
    return () => clearInterval(id);
  }, [delay]);
}
