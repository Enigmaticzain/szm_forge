const puppeteer = require('puppeteer');

(async () => {
  console.log("Launching browser for live demonstration...");
  const browser = await puppeteer.launch({
    executablePath: '/usr/bin/google-chrome',
    headless: false,
    defaultViewport: null,
    args: ['--start-maximized']
  });

  const page = await browser.newPage();
  
  console.log("Navigating to http://localhost:3000...");
  await page.goto('http://localhost:3000', { waitUntil: 'networkidle2' });

  // Function to click elements by text
  const clickByText = async (selector, text) => {
    await page.evaluate((sel, t) => {
      const elements = [...document.querySelectorAll(sel)];
      const target = elements.find(el => el.textContent.trim().includes(t));
      if (target) {
        target.click();
      }
    }, selector, text);
  };

  const delay = ms => new Promise(res => setTimeout(res, ms));

  console.log("Switching to Chemistry Lab...");
  await clickByText('button', 'CHEMISTRY LAB');
  await delay(1000);

  // Synthesize Materials
  const synthesizeMaterial = async (name) => {
    console.log(`Synthesizing ${name}...`);
    // Find the input field near 'MATERIAL NAME'
    await page.evaluate((nameText) => {
      const inputs = document.querySelectorAll('input[type="text"]');
      if (inputs.length > 0) {
        // The first text input in the chemistry lab is the material name
        const input = inputs[0];
        // Trigger React change event
        const nativeInputValueSetter = Object.getOwnPropertyDescriptor(window.HTMLInputElement.prototype, 'value').set;
        nativeInputValueSetter.call(input, nameText);
        const ev2 = new Event('input', { bubbles: true});
        input.dispatchEvent(ev2);
      }
    }, name);
    await delay(500);
    
    // Click SYNTHESIZE
    await clickByText('button', 'SYNTHESIZE');
    await delay(500);
  };

  await synthesizeMaterial('Custom Copper');
  await synthesizeMaterial('Custom Neodymium');
  await synthesizeMaterial('Custom Silicon Steel');
  await synthesizeMaterial('Custom Synthetic Oil');

  console.log("Switching to Circuit Design...");
  await clickByText('button', 'CIRCUIT DESIGN');
  await delay(1000);

  console.log("Opening 🔩 PARTS category...");
  await clickByText('button', '🔩 PARTS');
  await delay(1000);

  console.log("Executing Custom Motor Assembly Preset...");
  await clickByText('button', 'Custom Motor Assembly');
  await delay(1000);

  // We need to click on the canvas to select parts.
  // We can't easily guess canvas coordinates for specific parts in puppeteer without math, 
  // but since the preset places stator at 250, 150, we can click there.
  console.log("Assigning materials...");
  const canvasElement = await page.$('canvas');
  const box = await canvasElement.boundingBox();
  
  const clickCanvasAt = async (x, y) => {
    await page.mouse.click(box.x + x, box.y + y);
    await delay(500);
  };

  // Click Stator (center is 250, 150, outer ring is ~50px out, so let's click 250, 100)
  await clickCanvasAt(250, 100);
  
  // Select material for Stator
  await page.evaluate(() => {
    const selects = document.querySelectorAll('select');
    if (selects.length > 0) {
      selects[0].value = 'Custom Silicon Steel';
      selects[0].dispatchEvent(new Event('change', { bubbles: true }));
    }
  });
  await delay(1000);

  // Click Coil (preset places it at 250, 120)
  await clickCanvasAt(250, 120);
  await page.evaluate(() => {
    const selects = document.querySelectorAll('select');
    if (selects.length > 0) {
      selects[0].value = 'Custom Copper';
      selects[0].dispatchEvent(new Event('change', { bubbles: true }));
    }
  });
  await delay(1000);

  // Click Magnet (preset places it at 250, 135)
  await clickCanvasAt(250, 135);
  await page.evaluate(() => {
    const selects = document.querySelectorAll('select');
    if (selects.length > 0) {
      selects[0].value = 'Custom Neodymium';
      selects[0].dispatchEvent(new Event('change', { bubbles: true }));
    }
  });
  await delay(1000);
  
  // Click Bearing (preset places it at 250, 150)
  await clickCanvasAt(250, 150);
  await page.evaluate(() => {
    const selects = document.querySelectorAll('select');
    if (selects.length > 0) {
      // Find the second select (lubricant) or first if material isn't there
      const select = selects.length > 1 ? selects[1] : selects[0];
      select.value = 'Custom Synthetic Oil';
      select.dispatchEvent(new Event('change', { bubbles: true }));
    }
  });
  await delay(1000);

  console.log("Starting Simulation...");
  await clickByText('button', '▶ START');

  console.log("Simulation running. Watching for 10 seconds...");
  await delay(10000);

  console.log("Closing browser.");
  await browser.close();
})();
