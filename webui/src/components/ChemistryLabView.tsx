import { useMemo, useState } from 'react';
import { periodicElements, type ElementData } from '../data/periodicTable';

const MAX_RECIPE_ITEMS = 6;

export default function ChemistryLabView() {
  const [query, setQuery] = useState('');
  const [selectedElement, setSelectedElement] = useState<ElementData | null>(periodicElements[0]);
  const [recipe, setRecipe] = useState<ElementData[]>([]);
  const [appliedMessage, setAppliedMessage] = useState('Select elements to build a custom material recipe.');

  const filteredElements = useMemo(() => {
    const normalized = query.trim().toLowerCase();
    if (!normalized) return periodicElements;

    return periodicElements.filter((element) => {
      return [
        element.name,
        element.symbol,
        element.category,
        element.commonUse,
        element.atomicNumber.toString()
      ].some((value) => value.toLowerCase().includes(normalized));
    });
  }, [query]);

  const addToRecipe = (element: ElementData) => {
    setRecipe((current) => {
      if (current.some((item) => item.atomicNumber === element.atomicNumber)) {
        return current;
      }
      const nextRecipe = [...current, element].slice(-MAX_RECIPE_ITEMS);
      setAppliedMessage(`Added ${element.symbol} to the material recipe.`);
      return nextRecipe;
    });
  };

  const removeFromRecipe = (elementNumber: number) => {
    setRecipe((current) => current.filter((item) => item.atomicNumber !== elementNumber));
    setAppliedMessage('Removed one element from the material recipe.');
  };

  const applyToSolids = () => {
    if (recipe.length === 0) {
      setAppliedMessage('Choose at least one element to form a material recipe for solids.');
      return;
    }

    const recipeSummary = recipe.map((item) => item.symbol).join(', ');
    setAppliedMessage(`Material recipe ready for solids: ${recipeSummary}.`);
  };

  const materialSummary = recipe.length > 0
    ? recipe.map((item) => item.symbol).join(' + ')
    : 'No elements selected yet';

  return (
    <div className="h-full w-full flex flex-col lg:flex-row bg-forge-bg overflow-hidden">
      <div className="flex-1 p-4 sm:p-5 flex flex-col overflow-hidden">
        <div className="flex flex-col gap-3 md:flex-row md:items-end md:justify-between">
          <div>
            <div className="text-[10px] uppercase tracking-[0.35em] text-forge-accent">Chemistry Lab</div>
            <h2 className="text-xl font-semibold text-forge-text">Periodic table explorer for materials design</h2>
            <p className="text-sm text-forge-text-dim mt-1">Inspect the full element set, review its core properties, and compose a candidate material recipe for solids or other engineering applications.</p>
          </div>
          <div className="rounded-xl border border-forge-border bg-forge-surface/70 px-3 py-2 text-[11px] text-forge-text-dim">
            <span className="font-semibold text-forge-text">{filteredElements.length}</span> elements available
          </div>
        </div>

        <div className="mt-4 flex flex-col gap-2 sm:flex-row sm:items-center">
          <input
            value={query}
            onChange={(event) => setQuery(event.target.value)}
            placeholder="Search by name, symbol, use case or category"
            className="w-full rounded-lg border border-forge-border bg-forge-surface px-3 py-2 text-sm text-forge-text outline-none ring-0"
          />
          <div className="rounded-lg border border-forge-border bg-forge-panel/80 px-3 py-2 text-[11px] text-forge-text-dim">
            Tip: build up to 6 elements per material recipe.
          </div>
        </div>

        <div className="mt-4 grid flex-1 gap-2 overflow-y-auto pr-1 sm:grid-cols-2 xl:grid-cols-3 2xl:grid-cols-4">
          {filteredElements.map((element) => {
            const isSelected = selectedElement?.atomicNumber === element.atomicNumber;
            const isInRecipe = recipe.some((item) => item.atomicNumber === element.atomicNumber);

            return (
              <button
                key={element.atomicNumber}
                type="button"
                onClick={() => {
                  setSelectedElement(element);
                  setAppliedMessage(`${element.symbol} selected for inspection.`);
                }}
                className={`rounded-2xl border p-3 text-left transition-all ${
                  isSelected
                    ? 'border-forge-accent bg-forge-accent/10 shadow-[0_0_0_1px_rgba(0,238,255,0.2)]'
                    : 'border-forge-border bg-forge-surface/80 hover:border-forge-accent/40 hover:bg-forge-panel'
                }`}
              >
                <div className="flex items-center justify-between text-[10px] uppercase tracking-[0.25em] text-forge-text-dim">
                  <span>{element.atomicNumber}</span>
                  <span>{element.category}</span>
                </div>
                <div className="mt-3 flex items-center justify-between gap-2">
                  <div>
                    <div className="text-xl font-semibold text-forge-text">{element.symbol}</div>
                    <div className="text-[11px] text-forge-text-muted">{element.name}</div>
                  </div>
                  {isInRecipe ? (
                    <span className="rounded-full border border-forge-accent/30 bg-forge-accent/10 px-2 py-0.5 text-[9px] font-semibold uppercase tracking-[0.2em] text-forge-accent">
                      In recipe
                    </span>
                  ) : null}
                </div>
                <div className="mt-3 text-[11px] text-forge-text-dim">{element.properties}</div>
              </button>
            );
          })}
        </div>
      </div>

      <aside className="w-full lg:w-[360px] border-t lg:border-t-0 lg:border-l border-forge-border bg-forge-surface/70 p-4 sm:p-5 overflow-y-auto">
        {selectedElement ? (
          <div className="space-y-4">
            <div className="rounded-2xl border border-forge-border bg-forge-panel/80 p-4">
              <div className="flex items-center justify-between">
                <div>
                  <div className="text-[10px] uppercase tracking-[0.35em] text-forge-accent">Selected element</div>
                  <div className="mt-1 text-2xl font-semibold text-forge-text">{selectedElement.symbol} · {selectedElement.name}</div>
                </div>
                <div className="rounded-full border border-forge-border px-3 py-1 text-[10px] uppercase tracking-[0.2em] text-forge-text-dim">
                  Z = {selectedElement.atomicNumber}
                </div>
              </div>

              <div className="mt-4 grid gap-3 text-sm text-forge-text-dim">
                <div className="flex items-center justify-between rounded-lg border border-forge-border/70 bg-forge-surface/70 px-3 py-2">
                  <span>Category</span>
                  <span className="font-medium text-forge-text">{selectedElement.category}</span>
                </div>
                <div className="flex items-center justify-between rounded-lg border border-forge-border/70 bg-forge-surface/70 px-3 py-2">
                  <span>State</span>
                  <span className="font-medium text-forge-text">{selectedElement.state}</span>
                </div>
                <div className="flex items-center justify-between rounded-lg border border-forge-border/70 bg-forge-surface/70 px-3 py-2">
                  <span>Atomic weight</span>
                  <span className="font-medium text-forge-text">{selectedElement.atomicWeight}</span>
                </div>
                <div className="flex items-center justify-between rounded-lg border border-forge-border/70 bg-forge-surface/70 px-3 py-2">
                  <span>Electronegativity</span>
                  <span className="font-medium text-forge-text">{selectedElement.electronegativity}</span>
                </div>
              </div>

              <div className="mt-4 rounded-xl border border-forge-border/70 bg-forge-surface/70 p-3 text-sm text-forge-text-dim">
                <div className="text-[10px] uppercase tracking-[0.25em] text-forge-text-muted">Key property</div>
                <div className="mt-2 text-forge-text">{selectedElement.properties}</div>
              </div>

              <div className="mt-4 rounded-xl border border-forge-border/70 bg-forge-surface/70 p-3 text-sm text-forge-text-dim">
                <div className="text-[10px] uppercase tracking-[0.25em] text-forge-text-muted">Common use</div>
                <div className="mt-2 text-forge-text">{selectedElement.commonUse}</div>
              </div>

              <button
                type="button"
                onClick={() => addToRecipe(selectedElement)}
                className="mt-4 w-full rounded-xl bg-forge-accent/90 px-3 py-2 text-sm font-semibold text-slate-950 transition hover:bg-forge-accent"
              >
                Add to material recipe
              </button>
            </div>

            <div className="rounded-2xl border border-forge-border bg-forge-panel/80 p-4">
              <div className="flex items-center justify-between">
                <div className="text-[10px] uppercase tracking-[0.35em] text-forge-accent">Material recipe</div>
                <div className="text-[10px] text-forge-text-dim">{recipe.length}/{MAX_RECIPE_ITEMS}</div>
              </div>

              <div className="mt-3 flex min-h-[56px] flex-wrap gap-2">
                {recipe.length === 0 ? (
                  <div className="w-full rounded-xl border border-dashed border-forge-border/70 bg-forge-surface/60 p-3 text-sm text-forge-text-dim">
                    Add elements here to create a candidate material composition.
                  </div>
                ) : (
                  recipe.map((item) => (
                    <div key={item.atomicNumber} className="flex items-center gap-2 rounded-full border border-forge-accent/30 bg-forge-accent/10 px-2.5 py-1.5 text-sm text-forge-text">
                      <span className="font-semibold">{item.symbol}</span>
                      <button
                        type="button"
                        onClick={() => removeFromRecipe(item.atomicNumber)}
                        className="text-[10px] uppercase tracking-[0.2em] text-forge-text-dim"
                      >
                        Remove
                      </button>
                    </div>
                  ))
                )}
              </div>

              <div className="mt-4 rounded-xl border border-forge-border/70 bg-forge-surface/70 p-3 text-sm text-forge-text-dim">
                <div className="text-[10px] uppercase tracking-[0.25em] text-forge-text-muted">Recipe summary</div>
                <div className="mt-2 text-forge-text">{materialSummary}</div>
              </div>

              <button
                type="button"
                onClick={applyToSolids}
                className="mt-4 w-full rounded-xl border border-forge-accent/30 px-3 py-2 text-sm font-semibold text-forge-accent transition hover:bg-forge-accent/10"
              >
                Use in solids or components
              </button>

              <div className="mt-3 rounded-xl border border-forge-border/70 bg-forge-surface/70 p-3 text-sm text-forge-text-dim">
                {appliedMessage}
              </div>
            </div>
          </div>
        ) : null}
      </aside>
    </div>
  );
}
