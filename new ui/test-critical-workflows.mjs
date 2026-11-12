import { chromium } from '@playwright/test';
import fs from 'fs';

const outDir = '/tmp/baseline';
fs.mkdirSync(outDir, { recursive: true });

const browser = await chromium.launch({ channel: 'chrome', headless: true, args: ['--no-sandbox', '--disable-dev-shm-usage'] });
const context = await browser.newContext({ viewport: { width: 1600, height: 900 } });
const page = await context.newPage();

page.on('console', m => { if (m.type() === 'error') console.log('CONSOLE ERROR:', m.text()); });

async function testCriticalWorkflows() {
  await page.goto('http://localhost:3000', { waitUntil: 'networkidle', timeout: 30000 });
  await page.waitForTimeout(3000);
  
  // 1. Test Import/Add functionality
  console.log('=== Testing Import/Add ===');
  await page.click('button:text("IMPORT / ADD")');
  await page.waitForTimeout(2000);
  await page.screenshot({ path: `${outDir}/01_import_add.png`, fullPage: true });
  
  // 2. Test Simulation Controls
  console.log('=== Testing Simulation Controls ===');
  await page.click('button:text("RUN")');
  await page.waitForTimeout(3000);
  await page.screenshot({ path: `${outDir}/02_simulation_run.png`, fullPage: true });
  
  // 3. Test USD Export
  console.log('=== Testing USD Export ===');
  await page.click('button:text("EXPORT USD")');
  await page.waitForTimeout(3000);
  await page.screenshot({ path: `${outDir}/03_export_usd.png`, fullPage: true });
  
  // 4. Test Settings Panel
  console.log('=== Testing Settings ===');
  await page.click('button:text("PANEL")');
  await page.waitForTimeout(2000);
  await page.screenshot({ path: `${outDir}/04_settings.png`, fullPage: true });
  
  // 5. Test AI Panel
  console.log('=== Testing AI Panel ===');
  await page.click('button:text("PANEL")');
  await page.waitForTimeout(1500);
  await page.screenshot({ path: `${outDir}/05_ai_panel.png`, fullPage: true });
  
  // 6. Test Command Center Workspace
  console.log('=== Testing Command Center ===');
  await page.click('button:text("CMD CENTER")');
  await page.waitForTimeout(2000);
  await page.screenshot({ path: `${outDir}/06_command_center.png`, fullPage: true });
  
  await context.close();
  await browser.close();
  console.log('All critical workflows tested');
}

inspect().catch(console.error);