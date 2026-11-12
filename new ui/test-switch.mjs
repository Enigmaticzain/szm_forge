import { chromium } from '@playwright/test';
import fs from 'fs';

const outDir = '/tmp/baseline';
fs.mkdirSync(outDir, { recursive: true });

const browser = await chromium.launch({ channel: 'chrome', headless: true, args: ['--no-sandbox'] });
const context = await browser.newContext({ viewport: { width: 1600, height: 900 } });
const page = await context.newPage();

async function testSwitch() {
  await page.goto('http://localhost:3000', { waitUntil: 'networkidle', timeout: 30000 });
  await page.waitForTimeout(3000);
  
  // Open CREATION MODULES dropdown
  await page.click('button:text("CREATION MODULES")');
  await page.waitForTimeout(800);
  
  // Click GEOMETRY
  await page.click('button:text("GEOMETRY")');
  await page.waitForTimeout(2000);
  
  // Check content
  const content = await page.evaluate(() => {
    return document.body.innerText.substring(0, 2000);
  });
  console.log('=== After clicking GEOMETRY ===');
  console.log(content.substring(0, 1000));
  
  await page.screenshot({ path: `${outDir}/01_after_geometry.png`, fullPage: true });
  
  // Try SIMULATION
  await page.click('button:text("EXECUTION")');
  await page.waitForTimeout(800);
  await page.click('button:text("SIMULATION")');
  await page.waitForTimeout(2000);
  
  const simContent = await page.evaluate(() => document.body.innerText.substring(0, 1500));
  console.log('=== After clicking SIMULATION ===');
  console.log(simContent.substring(0, 800));
  
  await page.screenshot({ path: `${outDir}/02_after_simulation.png`, fullPage: true });
  
  // Try AI ASSISTANT
  await page.click('button:text("TOOLS & AI")');
  await page.waitForTimeout(800);
  await page.click('button:text("SZM AI")');
  await page.waitForTimeout(2000);
  
  const aiContent = await page.evaluate(() => document.body.innerText.substring(0, 1500));
  console.log('=== After clicking SZM AI ===');
  console.log(aiContent.substring(0, 800));
  
  await page.screenshot({ path: `${outDir}/03_after_ai.png`, fullPage: true });
  
  await context.close();
  await browser.close();
}

testSwitch().catch(console.error);