import { chromium } from '@playwright/test';
import fs from 'fs';

const outDir = '/tmp/baseline';
fs.mkdirSync(outDir, { recursive: true });

const browser = await chromium.launch({ channel: 'chrome', headless: true, args: ['--no-sandbox'] });
const context = await browser.newContext({ viewport: { width: 1600, height: 900 } });
const page = await context.newPage();

page.on('console', m => { if (m.type() === 'error') console.log('CONSOLE ERROR:', m.text()); });

async function testWorkspaceSwitch() {
  await page.goto('http://localhost:3000', { waitUntil: 'networkidle', timeout: 30000 });
  await page.waitForTimeout(3000);
  
  // Check initial state
  const initialWorkspace = await page.evaluate(() => {
    const store = window.localStorage.getItem('szm-workspace');
    return store ? store : 'command-center'; // fallback
  });
  console.log('Initial workspace:', initialWorkspace);
  
  // Try clicking GEOMETRY workspace
  const geometryBtn = 'button:text("GEOMETRY")';
  await page.click(geometryBtn);
  await page.waitForTimeout(2000);
  
  // Check new state
  const newWorkspace = await page.evaluate(() => {
    const store = window.localStorage.getItem('szm-workspace');
    return store ? store : 'command-center';
  });
  console.log('After clicking GEOMETRY:', newWorkspace);
  
  // Check if content changed - look for specific elements
  const contentChanged = await page.evaluate(() => {
    const hasGeometrySection = document.querySelector('.design-review-section') !== null;
    const hasCommandCenterSection = document.querySelector('.command-center-section') !== null;
    return hasGeometrySection && !hasCommandCenterSection;
  });
  console.log('Content changed to geometry sections:', contentChanged);
  
  // Capture screenshots
  await page.screenshot({ path: `${outDir}/01_initial.png` });
  await page.click(geometryBtn);
  await page.waitForTimeout(2000);
  await page.screenshot({ path: `${outDir}/02_after_geometry.png` });
  
  await context.close();
  await browser.close();
}

testWorkspaceSwitch().catch(console.error);