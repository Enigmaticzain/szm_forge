import { chromium } from '@playwright/test';
import fs from 'fs';

const outDir = '/tmp/baseline';
fs.mkdirSync(outDir, { recursive: true });
const errors = [];
const browser = await chromium.launch({ channel: 'chrome', headless: true, args: ['--no-sandbox'] });
const context = await browser.newContext({ viewport: { width: 1600, height: 900 } });
const page = await context.newPage();

page.on('console', m => { if (m.type() === 'error') errors.push(m.text()); });
page.on('pageerror', e => errors.push('PAGEERROR: ' + e.message));
await page.goto('http://localhost:3000', { waitUntil: 'networkidle', timeout: 30000 });
await page.waitForTimeout(3000);
await page.screenshot({ path: `${outDir}/01_initial.png`, fullPage: false });
console.log('captured 01_initial');

// Try login screen / auth
try {
  await page.click('button:has-text("Login"), button:has-text("Sign In"), input[type=email], input[placeholder*="Email"], button:has-text("Create")').catch(()=>{});
  await page.waitForTimeout(500);
} catch {}
await page.screenshot({ path: `${outDir}/02_auth.png`, fullPage: false });
console.log('captured 02_auth attempt');

const text = await page.evaluate(() => document.body.innerText.slice(0, 2600));
fs.writeFileSync(`${outDir}/bodytext.txt`, text);

// Summary of visible UI labels
for (const t of ['Command Center','Digital Twin','Physics','Simulation','Mechanical','Designing','Viewport','Settings','Material','Assembly']) {
  try {
    const el = await page.$eval(`body`, el => {
      const txt = document.body.innerText;
      return txt.includes(t);
    });
    console.log(`label ${t}: ${el}`);
  } catch (e) {}
}

await page.screenshot({ path: `${outDir}/03_summary.png`, fullPage: true });
console.log('errors:', errors);
await context.close();
await browser.close();
console.log('DONE');