export type Result<T> = 
  | { readonly success: true; readonly data: T } 
  | { readonly success: false; readonly error: string };

export const success = <T>(data: T): Result<T> => ({ success: true, data });
export const failure = <T = never>(error: string): Result<T> => ({ success: false, error });
