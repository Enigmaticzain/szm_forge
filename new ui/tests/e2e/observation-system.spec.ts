import { test, expect } from '@playwright/test';

test.describe('Action Observation System', () => {
  test('Toggles Passive Observation', async ({ page }) => {
    await page.goto('/');
    
    // Check if PASSIVE OBSERVATION is visible
    const passiveObs = page.locator('text=PASSIVE OBSERVATION');
    if (await passiveObs.isVisible()) {
      const parent = passiveObs.locator('..');
      const toggleBtn = parent.locator('button');
      
      // Toggle ON
      await toggleBtn.click();
      
      // Perform a click to generate telemetry
      await page.click('body');
      
      // Check if ACTIONS text is there
      await expect(page.locator('text=ACTIONS:')).toBeVisible();
    }
  });
});
