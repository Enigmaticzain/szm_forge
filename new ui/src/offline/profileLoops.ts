/**
 * profileLoops.ts — order-agnostic loop assembly for 2D sketch polylines.
 *
 * The existing extrusion fast path requires line entities in exact draw order.
 * Real sketches aren't like that (DXF imports, multi-stroke drawing, undo).
 * This module assembles closed loops from an UNORDERED segment soup with
 * endpoint tolerance snapping, so extrusion/revolve "just work".
 *
 * Honest behaviour:
 *   - Gaps larger than `tolerance` leave an OPEN CHAIN, counted and reported —
 *     never silently bridged.
 *   - Multi-loop sketches keep every loop; callers typically take the largest
 *     while reporting the others were left out.
 *   - Dangling spurs (degree-1 stubs inside a component) trim out as open
 *     chains rather than corrupting the loop.
 */

export interface Segment2 {
  p1: [number, number];
  p2: [number, number];
}

export interface LoopAssemblyResult {
  /** Ordered point loops (repeat-free — the closure edge is implicit). */
  loops: [number, number][][];
  /** How many open polylines remain (not part of any loop). */
  openChains: number;
  /** Connected components found with no closed loop at all. */
  unclosedComponents: number;
}

export function assembleLoops(segments: Segment2[], tolerance = 0.05): LoopAssemblyResult {
  if (segments.length === 0) return { loops: [], openChains: 0, unclosedComponents: 0 };

  // Node snapping: canonical-key map, first point wins per tolerance cell.
  const nodeOf = new Map<string, number>();
  const points: [number, number][] = [];
  const key = (x: number, y: number) => `${Math.round(x / tolerance)}:${Math.round(y / tolerance)}`;
  const getNode = (p: [number, number]) => {
    const k = key(p[0], p[1]);
    let n = nodeOf.get(k);
    if (n === undefined) {
      n = points.length;
      points.push(p);
      nodeOf.set(k, n);
    }
    return n;
  };

  // Build undirected adjacency with per-edge segment index.
  const adj = new Map<number, { to: number; seg: number }[]>();
  let lastSeg = 0;
  for (const s of segments) {
    const a = getNode(s.p1);
    const b = getNode(s.p2);
    if (a === b) continue; // zero-length degenerate
    (adj.get(a) ?? adj.set(a, []).get(a)!).push({ to: b, seg: lastSeg });
    (adj.get(b) ?? adj.set(b, []).get(b)!).push({ to: a, seg: lastSeg });
    lastSeg++;
  }

  const visitedSeg = new Set<number>();
  const visitedNodeGlobal = new Set<number>();
  const loops: [number, number][][] = [];
  let openChains = 0;
  let unclosedComponents = 0;

  const nodes = [...adj.keys()];
  for (const start of nodes) {
    if (visitedNodeGlobal.has(start)) continue;
    // collect component (BFS)
    const stack = [start];
    const comp: number[] = [];
    const compMarked = new Set<number>();
    while (stack.length) {
      const n = stack.pop()!;
      if (compMarked.has(n)) continue;
      compMarked.add(n);
      comp.push(n);
      for (const e of adj.get(n) ?? []) if (!compMarked.has(e.to)) stack.push(e.to);
    }
    comp.forEach(n => visitedNodeGlobal.add(n));

    // closed-loop extraction via repeated left-turn-free walking: simplest
    // robust approach for planar sketch graphs where loops are simple cycles —
    // walk edges; when we return to the walk start we found a loop.
    const compSet = new Set(comp);
    const edgesUsed = new Set<number>();

    const walkFrom = (s: number, firstEdge: { to: number; seg: number }): [number, number][] | null => {
      // walk: start → first edge → keep going; at junctions prefer the FIRST
      // unused edge (graph is small; backtracking not needed for simple loops)
      const path: number[] = [s];
      const localUsed = new Set<number>([firstEdge.seg]);
      let cur = firstEdge.to;
      let guard = 0;
      while (guard++ < 10000) {
        path.push(cur);
        if (cur === s) {
          // closed. strip if path looks like A→B→A (spike)
          if (path.length >= 4) {
            localUsed.forEach(seg => edgesUsed.add(seg));
            return path.map(n => points[n]);
          }
          return null;
        }
        const next = (adj.get(cur) ?? []).find(e => !localUsed.has(e.seg) && compSet.has(e.to));
        if (!next) {
          // dead end — reopen: if we can still walk back to s via visited edges it's a spike
          return null;
        }
        localUsed.add(next.seg);
        cur = next.to;
      }
      return null;
    };

    let foundLoopHere = false;
    for (const n of comp) {
      for (const e of adj.get(n) ?? []) {
        if (edgesUsed.has(e.seg) || visitedSeg.has(e.seg)) continue;
        const loop = walkFrom(n, e);
        if (loop) {
          foundLoopHere = true;
          visitedSeg.add(e.seg);
          // drop the duplicated start point (walkFrom returns a closed ring)
          const ring = loop.slice(0, -1);
          if (ring.length >= 3) loops.push(ring);
          break; // restart scan for remaining edges in this component
        }
        // probe returned dead end — do not mark the edge: another probe may
        // close through it; leftovers fall out as open-chain accounting below.
      }
    }

    // any component edges not in loops → open chain accounting
    const compSegments = new Set<number>();
    for (const n of comp) for (const e of adj.get(n) ?? []) compSegments.add(e.seg);
    const unused = [...compSegments].filter(seg => !visitedSeg.has(seg) && !edgesUsed.has(seg));
    if (unused.length > 0) {
      openChains++;
      if (!foundLoopHere) unclosedComponents++;
    }
  }

  // Largest first — stable, deterministic for callers that take loops[0].
  loops.sort((a, b) => Math.abs(signedArea(b)) - Math.abs(signedArea(a)));
  return { loops, openChains, unclosedComponents };
}

function signedArea(loop: [number, number][]): number {
  let a = 0;
  for (let i = 0; i < loop.length; i++) {
    const [x1, y1] = loop[i];
    const [x2, y2] = loop[(i + 1) % loop.length];
    a += x1 * y2 - x2 * y1;
  }
  return a / 2;
}
