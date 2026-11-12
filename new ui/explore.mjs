import { chromium } from '@playwright/test';
import fs from 'fs';

const outDir = '/tmp/baseline';
fs.mkdirSync(outDir, { recursive: true });
const errors = [];
const screenshots = [];

const browser = await chromium.launch({ channel: 'chrome', headless: true, args: ['--no-sandbox'] });
const context = await browser.newContext({ viewport: { width: 1600, height: 900 } });
const page = await context.newPage();

async function capture(label) {
  await page.screenshot({ path: `${outDir}/${label}.png`, fullPage: true });
  screenshots.push(label);
  console.log(`captured ${label}`);
}

async function clickSelector(selector, label) {
  try {
    await page.click(selector);
    console.log(`clicked ${label}`);
    return true;
  } catch (e) {
    console.log(`failed to click ${label}: ${e.message}`);
    return false;
  }
}

async function explore() {
  await page.goto('http://localhost:3000', { waitUntil: 'networkidle', timeout: 30000 });
  await page.waitForTimeout(3000);
  await capture('01_initial');
  
  // Try to login
  try {
    await clickSelector('button:text("SIGN IN")', 'sign in button');
    await page.waitForTimeout(2000);
    await capture('02_after_login_attempt');
  } catch (e) {
    console.log('login attempt failed, continuing...');
  }
  
  // Explore main navigation
  const workspaces = ['digital-twin', 'manufacturing', 'ai-assistant', 'fleet-command', 'physics', 'simulation', 'geometry', 'node-programming', 'text-to-cad', 'command-center', 'knowledge-base', 'components', 'mechanical', 'optics', 'robotics', 'combined-assembly', 'testing-lab', 'engineering'];
  
  for (const ws of workspaces) {
    try {
      await clickSelector(`button:text("${ws.toUpperCase()}")`, `workspace ${ws}`);
      await page.waitForTimeout(1500);
      await capture(`02_${ws}_workspace`);
    } catch (e) {
      console.log(`could not access workspace ${ws}: ${e.message}`);
    }
  }
  
  // Check settings
  try {
    await clickSelector('button:text("Settings")', 'settings button');
    await page.waitForTimeout(2000);
    await capture('03_settings');
  } catch (e) {
    console.log('could not access settings: ' + e.message);
  }
  
  // Check notifications
  try {
    await clickSelector('button:text("Notifications")', 'notifications button');
    await page.waitForTimeout(1500);
    await capture('04_notifications');
  } catch (e) {
    console.log('could not access notifications: ' + e.message);
  }
  
  // Check import functionality
  try {
    await clickSelector('button:text("ADD")', 'import button');
    await page.waitForTimeout(2000);
    await capture('05_import_attempt');
  } catch (e) {
    console.log('could not access import: ' + e.message);
  }
  
  // Check AI panel
  try {
    await clickSelector('button:text("AI")', 'ai panel toggle');
    await page.waitForTimeout(1500);
    await capture('06_ai_panel');
  } catch (e) {
    console.log('could not access ai panel: ' + e.message);
  }
  
  // Final summary
  await capture('07_final');
  console.log('exploration complete');
  console.log('screenshots:', screenshots);
  console.log('errors:', errors);
  
  await context.close();
  await browser.close();
}

explore().catch(console.error);