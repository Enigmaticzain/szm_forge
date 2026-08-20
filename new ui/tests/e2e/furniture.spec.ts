import { test, expect } from '@playwright/test';

test.describe('Furniture Workshop Workflow', () => {
  test('Completes Ladder Flow', async ({ page }) => {
    await page.goto('/');
    
    // Select Ladder
    await page.getByRole('button', { name: 'Ladder' }).click();
    
    // New project
    await page.getByRole('button', { name: /1\. NEW PROJECT/i }).click();
    
    // Build
    await page.getByRole('button', { name: /2\. BUILD/i }).click();
    
    // Apply load
    await page.getByRole('button', { name: /3\. APPLY LOAD/i }).click();
    
    // Limits
    await page.getByRole('button', { name: /4\. LIMITS/i }).click();
    
    // AI improve
    await page.getByRole('button', { name: /5\. AI IMPROVE/i }).click();
    
    // Verify results
    await expect(page.locator('text=Limit sweep').first()).toBeVisible();
    await expect(page.locator('text=Rail').first()).toBeVisible();
  });
});
