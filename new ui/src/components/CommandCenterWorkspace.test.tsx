// @vitest-environment jsdom
import React from 'react';
import { render, screen } from '@testing-library/react';
import '@testing-library/jest-dom/vitest';
import { describe, expect, it } from 'vitest';
import { CommandCenterWorkspace } from './CommandCenterWorkspace';

describe('CommandCenterWorkspace', () => {
  it('renders the SZM Forge command center overview', () => {
    render(<CommandCenterWorkspace />);

    expect(screen.getByText('Engineering Command Center')).toBeInTheDocument();
    expect(screen.getByText('SZM AI memory')).toBeInTheDocument();
    expect(screen.getByText('Launch critical workflows')).toBeInTheDocument();
  });
});
