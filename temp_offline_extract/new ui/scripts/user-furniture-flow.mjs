/**
 * Simulates a normal user through Furniture Workshop UI (any type).
 * Usage: node scripts/user-furniture-flow.mjs [table|chair|ladder]
 * Requires: npm run dev on :3000 (or FORGE_UI_URL)
 */
import { chromium } from 'playwright';

const BASE = process.env.FORGE_UI_URL ?? 'http://127.0.0.1:3000';
const type = (process.argv[2] ?? 'ladder').toLowerCase();
const label = type.charAt(0).toUpperCase() + type.slice(1);

async function clickByText(page, text, options = {}) {
  await page.getByRole('button', { name: text, ...options }).click({ timeout: 20000 });
}

async function main() {
  const browser = await chromium.launch({
    headless: true,
    channel: process.env.PW_CHANNEL ?? 'chrome',
  });
  const page = await browser.newPage();
  const errors = [];

  page.on('console', msg => {
    if (msg.type() === 'error') errors.push(msg.text());
  });

  console.log('Opening', BASE, '— furniture:', type);
  await page.goto(BASE, { waitUntil: 'networkidle' });
  await page.waitForTimeout(4500);

  console.log(`1. Select ${label}`);
  await clickByText(page, label);

  console.log('2. New project');
  await clickByText(page, /1\. NEW PROJECT/i);

  console.log('3. Build');
  await clickByText(page, /2\. BUILD/i);

  console.log('4. Apply load');
  await clickByText(page, /3\. APPLY LOAD/i);

  console.log('5. Measure limits');
  await clickByText(page, /4\. LIMITS/i);

  console.log('6. AI improve');
  await clickByText(page, /5\. AI IMPROVE/i);

  await page.waitForTimeout(800);

  const lastAction = await page.locator('text=/Built|Limit sweep|AI improvements/').first().textContent().catch(() => null);
  const limitRow = await page.locator('text=Limit sweep').first().isVisible().catch(() => false);
  const partRow = await page.locator(`text=${type === 'ladder' ? 'Rail' : type === 'chair' ? 'Seat' : 'Tabletop'}`).first().isVisible().catch(() => false);
  const aiLine = await page.getByTestId('ai-insight').textContent().catch(() => null);

  const shot = `${type}-workflow-result.png`;
  await page.screenshot({ path: shot, fullPage: true });

  console.log('Last action:', lastAction ?? '(none)');
  console.log('Parts visible:', partRow);
  console.log('Limit sweep:', limitRow);
  console.log('AI:', aiLine ?? '(none)');
  console.log('Screenshot:', shot);
  if (errors.length) console.warn('Console errors:', errors.slice(0, 5));

  await browser.close();

  if (!partRow) {
    throw new Error(`Expected ${type} parts in workshop panel after BUILD`);
  }
  if (!limitRow) {
    throw new Error('Limit sweep section did not appear after LIMITS');
  }
  console.log(`Done — ${type} workflow completed through UI.`);
}

main().catch(err => {
  console.error(err);
  process.exit(1);
});
