/**
 * Simulates a normal user: ladder project via Furniture Workshop UI buttons.
 * Run: node scripts/user-ladder-flow.mjs  (with `npm run dev` on :3000)
 */
import { chromium } from 'playwright';

const BASE = process.env.FORGE_UI_URL ?? 'http://127.0.0.1:3000';

async function clickByText(page, text, options = {}) {
  await page.getByRole('button', { name: text, ...options }).click({ timeout: 15000 });
}

async function main() {
  const browser = await chromium.launch({
    headless: true,
    channel: process.env.PW_CHANNEL ?? 'chrome',
  });
  const page = await browser.newPage();
  console.log('Opening', BASE);
  await page.goto(BASE, { waitUntil: 'networkidle' });

  // Wait past loading screen (~3s)
  await page.waitForTimeout(4500);

  console.log('1. Select Ladder');
  await clickByText(page, 'Ladder');

  console.log('2. New project');
  await clickByText(page, /1\. NEW PROJECT/i);

  console.log('3. Build ladder');
  await clickByText(page, /2\. BUILD/i);

  console.log('4. Apply load (100 kg default for ladder)');
  await clickByText(page, /3\. APPLY LOAD/i);

  console.log('5. Measure limits');
  await clickByText(page, /4\. LIMITS/i);

  console.log('6. AI improve');
  await clickByText(page, /5\. AI IMPROVE/i);

  const status = await page.locator('text=Limit sweep').first().textContent().catch(() => null);
  const ai = await page.locator('text=AI:').first().textContent().catch(() => null);

  console.log('UI status:', status ?? '(limits panel)');
  console.log('AI line:', ai ?? '(see workshop panel)');

  await page.screenshot({ path: 'ladder-workflow-result.png', fullPage: true });
  console.log('Screenshot: new ui/ladder-workflow-result.png');

  await browser.close();
  console.log('Done — ladder workflow completed through UI buttons.');
}

main().catch(err => {
  console.error(err);
  process.exit(1);
});
