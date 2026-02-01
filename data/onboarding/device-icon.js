// Device Icon Component
// Displays animated ESP32 chip icon with LED indicator

var DeviceIcon = (function() {
  "use strict";

  var Icons = {
    chip: `
      <svg viewBox="0 0 48 48" fill="none">
        <rect x="14" y="14" width="20" height="20" rx="3" fill="currentColor" class="chip-center"/>
        <g stroke="currentColor" stroke-width="2" stroke-linecap="round">
          <line x1="18" y1="14" x2="18" y2="8"/>
          <line x1="24" y1="14" x2="24" y2="8"/>
          <line x1="30" y1="14" x2="30" y2="8"/>
          <line x1="18" y1="34" x2="18" y2="40"/>
          <line x1="24" y1="34" x2="24" y2="40"/>
          <line x1="30" y1="34" x2="30" y2="40"/>
          <line x1="14" y1="18" x2="8" y2="18"/>
          <line x1="14" y1="24" x2="8" y2="24"/>
          <line x1="14" y1="30" x2="8" y2="30"/>
          <line x1="34" y1="18" x2="40" y2="18"/>
          <line x1="34" y1="24" x2="40" y2="24"/>
          <line x1="34" y1="30" x2="40" y2="30"/>
        </g>
      </svg>
    `
  };

  function render(container, options) {
    options = options || {};
    var isAnimating = options.isAnimating !== false;

    container.innerHTML = `
      <div class="device-icon-wrapper">
        ${isAnimating ? '<div class="device-glow"></div>' : ''}
        <div class="device-icon">
          ${Icons.chip}
          <div class="device-led ${isAnimating ? 'pulse' : ''}"></div>
        </div>
      </div>
    `;
  }

  function renderWelcome(container, callbacks) {
    callbacks = callbacks || {};

    container.innerHTML = `
      <div class="welcome-content">
        <div id="device-icon-container"></div>
        <div class="welcome-text">
          <h1 class="welcome-title">Welcome to ESP32 Setup</h1>
          <p class="welcome-desc">Let's configure your adapter to connect to your home network and smart home system.</p>
        </div>
        <div class="welcome-actions">
          <button id="start-btn" class="btn btn-primary btn-lg">Get Started</button>
          <p class="text-muted text-xs">This will take about 2 minutes</p>
        </div>
      </div>
    `;

    // Render device icon
    render(document.getElementById('device-icon-container'), { isAnimating: true });

    // Add event listener
    document.getElementById('start-btn').addEventListener('click', function() {
      if (callbacks.onStart) callbacks.onStart();
    });
  }

  return {
    render: render,
    renderWelcome: renderWelcome,
    Icons: Icons
  };
})();
