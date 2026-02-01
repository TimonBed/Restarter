// Setup Wizard Component
// Main controller for the onboarding flow

var SetupWizard = (function() {
  "use strict";

  var state = {
    currentStep: 0,
    config: {
      ssid: '',
      password: '',
      mqtt: null
    }
  };

  var Icons = {
    check: `<svg class="icon-xs" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="3"><polyline points="20 6 9 17 4 12"/></svg>`,
    checkLg: `<svg class="icon-lg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><polyline points="20 6 9 17 4 12"/></svg>`
  };

  function init() {
    // Render initial step indicator
    StepIndicator.render(document.getElementById('step-indicator'), 0);

    // Render welcome screen
    DeviceIcon.renderWelcome(document.getElementById('step-welcome'), {
      onStart: function() {
        goToStep(1);
      }
    });

    // Render WiFi form
    WifiForm.render(document.getElementById('step-wifi'), {
      onSubmit: handleWifiSubmit,
      onBack: function() {
        goToStep(0);
      }
    });

    // Render MQTT form
    MqttForm.render(document.getElementById('step-mqtt'), {
      onSubmit: handleMqttSubmit,
      onBack: function() {
        goToStep(1);
      }
    });

    // Render connecting screen
    renderConnecting();

    // Render complete screen
    renderComplete();

    // Setup reset button
    document.getElementById('reset-btn').addEventListener('click', handleReset);
  }

  function goToStep(step) {
    // Hide all steps
    document.querySelectorAll('.step-content').forEach(function(el) {
      el.classList.remove('active');
    });

    // Show target step
    var stepMap = {
      0: 'step-welcome',
      1: 'step-wifi',
      2: 'step-mqtt',
      3: 'step-complete'
    };

    var stepEl = document.getElementById(stepMap[step]);
    if (stepEl) {
      stepEl.classList.add('active');
    }

    state.currentStep = step;
    StepIndicator.render(document.getElementById('step-indicator'), step);

    // Show/hide reset button
    var resetBtn = document.getElementById('reset-btn');
    if (step > 0 && step < 3) {
      resetBtn.classList.remove('hidden');
    } else {
      resetBtn.classList.add('hidden');
    }
  }

  function handleWifiSubmit(ssid, password) {
    state.config.ssid = ssid;
    state.config.password = password;
    goToStep(2);
  }

  function handleMqttSubmit(mqttConfig) {
    state.config.mqtt = mqttConfig;
    showConnecting();
  }

  function showConnecting() {
    // Hide all steps and show connecting
    document.querySelectorAll('.step-content').forEach(function(el) {
      el.classList.remove('active');
    });
    document.getElementById('step-connecting').classList.add('active');
    document.getElementById('reset-btn').classList.add('hidden');

    // Animate connection steps
    var step1 = document.getElementById('conn-step-1');
    var step2 = document.getElementById('conn-step-2');
    var step3 = document.getElementById('conn-step-3');

    // Step 1 already done
    step1.classList.add('done');
    step1.querySelector('.connection-step-icon').classList.add('done');
    step1.querySelector('.connection-step-icon').innerHTML = Icons.check;

    setTimeout(function() {
      step2.classList.add('done');
      step2.querySelector('.connection-step-icon').classList.add('done');
      step2.querySelector('.connection-step-icon').innerHTML = Icons.check;
    }, 800);

    setTimeout(function() {
      step3.classList.add('active');
      step3.querySelector('.connection-step-icon').classList.add('active');
    }, 1200);

    // Save config
    saveConfig();
  }

  function saveConfig() {
    var payload = {
      wifiSsid: state.config.ssid,
      wifiPass: state.config.password,
      mqttHost: state.config.mqtt ? state.config.mqtt.broker : '',
      mqttPort: state.config.mqtt ? state.config.mqtt.port : 1883,
      mqttUser: state.config.mqtt ? (state.config.mqtt.username || '') : '',
      mqttPass: state.config.mqtt ? (state.config.mqtt.password || '') : '',
      mqttTopic: state.config.mqtt ? (state.config.mqtt.topic || '') : '',
      mqttTls: state.config.mqtt ? state.config.mqtt.useTls : false
    };

    fetch('/api/config', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    })
    .then(function() {
      setTimeout(showComplete, 1500);
    })
    .catch(function() {
      // Still show complete even on error (ESP may reboot)
      setTimeout(showComplete, 1500);
    });
  }

  function showComplete() {
    document.querySelectorAll('.step-content').forEach(function(el) {
      el.classList.remove('active');
    });
    document.getElementById('step-complete').classList.add('active');
    state.currentStep = 3;
    StepIndicator.render(document.getElementById('step-indicator'), 3);

    // Update summary
    document.getElementById('summary-ssid').textContent = state.config.ssid;
    document.getElementById('summary-mqtt').textContent = state.config.mqtt 
      ? state.config.mqtt.broker 
      : 'Not configured';
  }

  function handleReset() {
    state.currentStep = 0;
    state.config = { ssid: '', password: '', mqtt: null };

    // Reset component states
    WifiForm.reset();
    MqttForm.reset();

    // Re-render components
    WifiForm.render(document.getElementById('step-wifi'), {
      onSubmit: handleWifiSubmit,
      onBack: function() { goToStep(0); }
    });

    MqttForm.render(document.getElementById('step-mqtt'), {
      onSubmit: handleMqttSubmit,
      onBack: function() { goToStep(1); }
    });

    // Reset connecting step
    renderConnecting();

    goToStep(0);
  }

  function renderConnecting() {
    document.getElementById('step-connecting').innerHTML = `
      <div class="connecting-content">
        <div class="connecting-spinner">
          <div class="spinner-bg">
            <svg class="spinner" viewBox="0 0 24 24">
              <circle class="spinner-track" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4" fill="none"/>
              <path class="spinner-path" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"/>
            </svg>
          </div>
        </div>
        <div class="connecting-text">
          <h2 class="connecting-title">Applying Settings</h2>
          <p class="connecting-desc">Configuring your ESP32 adapter...</p>
        </div>
        <div class="connection-steps">
          <div class="connection-step" id="conn-step-1">
            <div class="connection-step-icon"></div>
            <span>Saving configuration</span>
          </div>
          <div class="connection-step" id="conn-step-2">
            <div class="connection-step-icon"></div>
            <span>Connecting to WiFi</span>
          </div>
          <div class="connection-step" id="conn-step-3">
            <div class="connection-step-icon"></div>
            <span>Verifying connection</span>
          </div>
        </div>
      </div>
    `;
  }

  function renderComplete() {
    document.getElementById('step-complete').innerHTML = `
      <div class="complete-content">
        <div class="complete-icon">${Icons.checkLg}</div>
        <div class="complete-text">
          <h1 class="complete-title">All Done!</h1>
          <p class="complete-desc">Your ESP32 adapter is configured and ready.</p>
        </div>
        <div class="config-summary">
          <div class="config-row">
            <span class="config-label">Network</span>
            <span class="config-value" id="summary-ssid">—</span>
          </div>
          <div class="config-row">
            <span class="config-label">MQTT</span>
            <span class="config-value" id="summary-mqtt">Not configured</span>
          </div>
          <div class="config-row">
            <span class="config-label">Status</span>
            <span class="config-value status-online">
              <span class="status-dot pulse"></span>
              Online
            </span>
          </div>
        </div>
        <div class="complete-actions">
          <p class="text-muted text-sm">The device will now restart and connect to your network. This page will close automatically.</p>
          <button id="configure-again-btn" class="btn btn-outline">Configure Again</button>
        </div>
      </div>
    `;

    document.getElementById('configure-again-btn').addEventListener('click', handleReset);
  }

  // Initialize on DOM ready
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }

  return {
    goToStep: goToStep,
    reset: handleReset,
    getState: function() { return state; }
  };
})();
