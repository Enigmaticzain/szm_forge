import { chromium } from '@playwright/test';
import fs from 'fs';

const outDir = '/tmp/baseline';
fs.mkdirSync(outDir, { recursive: true });

const browser = await chromium.launch({ channel: 'chrome', headless: true, args: ['--no-sandbox'] });
const context = await browser.newContext({ viewport: { width: 1600, height: 900 } });
const page = await context.newPage();

async function inspect() {
  await page.goto('http://localhost:3000', { waitUntil: 'networkidle', timeout: 30000 });
  await page.waitForTimeout(3000);
  
  // List all buttons on the page
  const buttons = await page.evaluate(() => {
    return Array.from(document.querySelectorAll('button')).map(b => ({
      text: b.textContent?.trim().substring(0, 50),
      className: b.className?.substring(0, 100),
      visible: b.offsetParent !== null
    }));
  });
  
  console.log('=== All buttons ===');
  buttons.forEach((b, i) => console.log(`${i}: text="${b.text}" class="${b.className}" visible=${b.visible}`));
  
  // Check what workspace dropdowns contain
  const dropdownText = await page.evaluate(() => {
    const labels = ['CREATION MODULES', 'EXECUTION', 'TOOLS & AI'];
    return labels.map(l => {
      const btn = Array.from(document.querySelectorAll('button')).find(b => b.textContent?.includes(l));
      return { label: l, found: !!btn, text: btn?.textContent?.trim().substring(0, 200) };
    });
  });
  console.log('=== Dropdown labels ===');
  dropdownText.forEach(d => console.log(`${d.label}: found=${d.found} text="${d.text}"`));
  
  await page.screenshot({ path: `${outDir}/01_initial.png`, fullPage: true });
  console.log('captured screenshot');
  
  // Click CREATION MODULES dropdown
  await page.click('button:text("CREATION MODULES")');
  await page.waitForTimeout(1000);
  
  // List dropdown items
  const dropdownItems = await page.evaluate(() => {
    return Array.from(document.querySelectorAll('button')).map(b => ({
      text: b.textContent?.trim().substring(0, 100),
      visible: b.offsetParent !== null,
      className: b.className?.substring(0, 50)
    })).filter(b => b.visible);
  });
  console.log('=== All visible buttons after dropdown open ===');
  dropdownItems.forEach((b, i) => console.log(`${i}: text="${b.text}" class="${b.className}"`));
  
  await page.screenshot({ path: `${outDir}/02_dropdown_open.png`, fullPage: true });
  
  await context.close();
  await browser.close();
}

inspect().catch(console.error);