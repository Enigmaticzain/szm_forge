import React, { Component } from 'react';

interface Props {
  children: React.ReactNode;
}

interface State {
  failed: boolean;
}

export class GeometryModeErrorBoundary extends Component<Props, State> {
  state: State = { failed: false };

  static getDerivedStateFromError(): State {
    return { failed: true };
  }

  render() {
    if (this.state.failed) {
      return (
        <div role="alert" className="h-full flex items-center justify-center p-6 text-center">
          <div className="max-w-md">
            <div className="text-sm font-semibold text-forge-text">This geometry mode could not start.</div>
            <p className="mt-2 text-xs text-forge-text-muted">
              A 3D viewport may require WebGL support. Switch to Sketch to continue creating without it.
            </p>
          </div>
        </div>
      );
    }

    return this.props.children;
  }
}
