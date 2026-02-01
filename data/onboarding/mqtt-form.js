// MQTT Form Component
// Handles MQTT broker configuration

var MqttForm = (function() {
  "use strict";

  var state = {
    enabled: false,
    showAdvanced: false,
    useTls: false
  };

  var callbacks = {};

  var Icons = {
    server: `<svg class="icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <rect x="2" y="2" width="20" height="8" rx="2" ry="2"/>
      <rect x="2" y="14" width="20" height="8" rx="2" ry="2"/>
      <line x1="6" y1="6" x2="6.01" y2="6"/>
      <line x1="6" y1="18" x2="6.01" y2="18"/>
    </svg>`,
    hash: `<svg class="input-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <line x1="4" y1="9" x2="20" y2="9"/><line x1="4" y1="15" x2="20" y2="15"/>
      <line x1="10" y1="3" x2="8" y2="21"/><line x1="16" y1="3" x2="14" y2="21"/>
    </svg>`,
    user: `<svg class="input-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"/>
      <circle cx="12" cy="7" r="4"/>
    </svg>`,
    lock: `<svg class="input-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <rect x="3" y="11" width="18" height="11" rx="2" ry="2"/>
      <path d="M7 11V7a5 5 0 0 1 10 0v4"/>
    </svg>`,
    shield: `<svg class="icon-sm" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/>
    </svg>`,
    chevronDown: `<svg class="chevron-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <polyline points="6 9 12 15 18 9"/>
    </svg>`,
    eyeOpen: `<svg class="eye-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/><circle cx="12" cy="12" r="3"/>
    </svg>`,
    eyeClosed: `<svg class="eye-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"/>
      <line x1="1" y1="1" x2="23" y2="23"/>
    </svg>`
  };

  function render(container, cbs) {
    callbacks = cbs || {};

    container.innerHTML = `
      <div class="step-header">
        <div class="step-icon">${Icons.server}</div>
        <h1 class="step-title">MQTT Broker</h1>
        <p class="step-desc">Optional: Connect to your smart home</p>
      </div>
      <form id="mqtt-form" class="form-content">
        <button type="button" id="mqtt-toggle" class="mqtt-toggle">
          <div class="mqtt-toggle-content">
            <div class="mqtt-toggle-icon">${Icons.server}</div>
            <div class="mqtt-toggle-text">
              <p class="mqtt-toggle-title">Enable MQTT</p>
              <p class="mqtt-toggle-desc">Home Assistant, Node-RED, etc.</p>
            </div>
          </div>
          <div id="mqtt-switch" class="switch"></div>
        </button>

        <div id="mqtt-fields" class="mqtt-fields hidden">
          <div class="form-field">
            <label class="field-label text-muted">Broker Address</label>
            <div class="input-group">
              <svg class="input-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <rect x="2" y="2" width="20" height="8" rx="2" ry="2"/>
                <rect x="2" y="14" width="20" height="8" rx="2" ry="2"/>
                <line x1="6" y1="6" x2="6.01" y2="6"/>
                <line x1="6" y1="18" x2="6.01" y2="18"/>
              </svg>
              <input type="text" id="mqtt-broker" class="input" placeholder="mqtt.example.com" autocomplete="off" autocapitalize="off"/>
            </div>
          </div>
          <div class="form-field">
            <label class="field-label text-muted">Port</label>
            <div class="input-group">
              ${Icons.hash}
              <input type="number" id="mqtt-port" class="input" placeholder="1883" value="1883" inputmode="numeric"/>
            </div>
          </div>

          <button type="button" id="advanced-toggle" class="advanced-toggle">
            <span>Advanced Options</span>
            ${Icons.chevronDown}
          </button>

          <div id="advanced-fields" class="advanced-fields hidden">
            <div class="form-field">
              <label class="field-label text-muted">Username (optional)</label>
              <div class="input-group">
                ${Icons.user}
                <input type="text" id="mqtt-user" class="input" placeholder="mqtt_user" autocomplete="off" autocapitalize="off"/>
              </div>
            </div>
            <div class="form-field">
              <label class="field-label text-muted">Password (optional)</label>
              <div class="input-group">
                ${Icons.lock}
                <input type="password" id="mqtt-password" class="input" placeholder="Enter password" autocomplete="off"/>
                <button type="button" id="toggle-mqtt-password" class="input-action">${Icons.eyeOpen}</button>
              </div>
            </div>
            <div class="form-field">
              <label class="field-label text-muted">Base Topic (optional)</label>
              <div class="input-group">
                ${Icons.hash}
                <input type="text" id="mqtt-topic" class="input" placeholder="home/esp32" autocomplete="off" autocapitalize="off"/>
              </div>
            </div>
            <button type="button" id="tls-toggle" class="tls-toggle">
              <div class="tls-toggle-content">
                ${Icons.shield}
                <span>Use TLS/SSL</span>
              </div>
              <div id="tls-switch" class="switch"></div>
            </button>
          </div>
        </div>

        <div class="form-buttons">
          <button type="button" id="back-btn" class="btn btn-outline">Back</button>
          <button type="submit" id="mqtt-submit" class="btn btn-primary">Skip MQTT</button>
        </div>
      </form>
    `;

    // Bind events
    document.getElementById('mqtt-form').addEventListener('submit', handleSubmit);
    document.getElementById('mqtt-toggle').addEventListener('click', toggleMqtt);
    document.getElementById('advanced-toggle').addEventListener('click', toggleAdvanced);
    document.getElementById('tls-toggle').addEventListener('click', toggleTls);
    document.getElementById('back-btn').addEventListener('click', function() {
      if (callbacks.onBack) callbacks.onBack();
    });
    document.getElementById('toggle-mqtt-password').addEventListener('click', togglePasswordVisibility);
    document.getElementById('mqtt-broker').addEventListener('input', updateSubmitButton);
  }

  function toggleMqtt() {
    state.enabled = !state.enabled;
    var toggle = document.getElementById('mqtt-toggle');
    var switchEl = document.getElementById('mqtt-switch');
    var fields = document.getElementById('mqtt-fields');

    if (state.enabled) {
      toggle.classList.add('active');
      switchEl.classList.add('active');
      fields.classList.remove('hidden');
    } else {
      toggle.classList.remove('active');
      switchEl.classList.remove('active');
      fields.classList.add('hidden');
    }

    updateSubmitButton();
  }

  function toggleAdvanced() {
    state.showAdvanced = !state.showAdvanced;
    var toggle = document.getElementById('advanced-toggle');
    var fields = document.getElementById('advanced-fields');

    if (state.showAdvanced) {
      toggle.classList.add('open');
      fields.classList.remove('hidden');
    } else {
      toggle.classList.remove('open');
      fields.classList.add('hidden');
    }
  }

  function toggleTls() {
    state.useTls = !state.useTls;
    var toggle = document.getElementById('tls-toggle');
    var switchEl = document.getElementById('tls-switch');

    if (state.useTls) {
      toggle.classList.add('active');
      switchEl.classList.add('active');
    } else {
      toggle.classList.remove('active');
      switchEl.classList.remove('active');
    }
  }

  function togglePasswordVisibility() {
    var input = document.getElementById('mqtt-password');
    var btn = document.getElementById('toggle-mqtt-password');

    if (input.type === 'password') {
      input.type = 'text';
      btn.innerHTML = Icons.eyeClosed;
    } else {
      input.type = 'password';
      btn.innerHTML = Icons.eyeOpen;
    }
  }

  function updateSubmitButton() {
    var submitBtn = document.getElementById('mqtt-submit');
    var broker = document.getElementById('mqtt-broker').value.trim();

    submitBtn.textContent = state.enabled && broker ? 'Apply Settings' : 'Skip MQTT';
  }

  function handleSubmit(e) {
    e.preventDefault();

    var config = null;

    if (state.enabled) {
      var broker = document.getElementById('mqtt-broker').value.trim();
      if (broker) {
        config = {
          broker: broker,
          port: parseInt(document.getElementById('mqtt-port').value) || 1883,
          username: document.getElementById('mqtt-user').value.trim() || undefined,
          password: document.getElementById('mqtt-password').value || undefined,
          topic: document.getElementById('mqtt-topic').value.trim() || undefined,
          useTls: state.useTls
        };
      }
    }

    if (callbacks.onSubmit) {
      callbacks.onSubmit(config);
    }
  }

  function getState() {
    return {
      enabled: state.enabled,
      showAdvanced: state.showAdvanced,
      useTls: state.useTls
    };
  }

  function reset() {
    state.enabled = false;
    state.showAdvanced = false;
    state.useTls = false;
  }

  return {
    render: render,
    getState: getState,
    reset: reset
  };
})();
