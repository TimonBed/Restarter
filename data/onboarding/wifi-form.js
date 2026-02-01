// WiFi Form Component
// Handles WiFi network selection and configuration

var WifiForm = (function() {
  "use strict";

  var state = {
    networks: [],
    selectedNetwork: null,
    isManualEntry: false,
    isScanning: false
  };

  var scanTimer = null;
  var callbacks = {};

  var Icons = {
    wifi: `<svg class="icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <path d="M5 12.55a11 11 0 0 1 14.08 0"/><path d="M1.42 9a16 16 0 0 1 21.16 0"/>
      <path d="M8.53 16.11a6 6 0 0 1 6.95 0"/><circle cx="12" cy="20" r="1"/>
    </svg>`,
    wifiSmall: `<svg class="icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <path d="M5 12.55a11 11 0 0 1 14.08 0"/><path d="M1.42 9a16 16 0 0 1 21.16 0"/>
      <path d="M8.53 16.11a6 6 0 0 1 6.95 0"/><circle cx="12" cy="20" r="1"/>
    </svg>`,
    lock: `<svg class="lock-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <rect x="3" y="11" width="18" height="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/>
    </svg>`,
    refresh: `<svg class="icon-sm" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <path d="M1 4v6h6"/><path d="M23 20v-6h-6"/>
      <path d="M20.49 9A9 9 0 0 0 5.64 5.64L1 10m22 4l-4.64 4.36A9 9 0 0 1 3.51 15"/>
    </svg>`,
    chevronRight: `<svg class="chevron-right" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <polyline points="9 18 15 12 9 6"/>
    </svg>`,
    eyeOpen: `<svg class="eye-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/><circle cx="12" cy="12" r="3"/>
    </svg>`,
    eyeClosed: `<svg class="eye-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
      <path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"/>
      <line x1="1" y1="1" x2="23" y2="23"/>
    </svg>`
  };

  function escapeHtml(str) {
    var div = document.createElement("div");
    div.textContent = str;
    return div.innerHTML;
  }

  function render(container, cbs) {
    callbacks = cbs || {};

    container.innerHTML = `
      <div class="step-header">
        <div class="step-icon">${Icons.wifi}</div>
        <h1 class="step-title">WiFi Network</h1>
        <p class="step-desc">Connect to your home network</p>
      </div>
      <form id="wifi-form" class="form-content">
        <div class="form-field">
          <div class="field-header">
            <label class="field-label" id="network-label">Available Networks</label>
            <div class="field-actions">
              <button type="button" id="manual-toggle" class="text-link">Manual</button>
            </div>
          </div>
          <div id="network-list" class="network-list"></div>
          <div id="manual-ssid" class="input-group hidden">
            <svg class="input-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M5 12.55a11 11 0 0 1 14.08 0"/><path d="M1.42 9a16 16 0 0 1 21.16 0"/>
              <path d="M8.53 16.11a6 6 0 0 1 6.95 0"/><circle cx="12" cy="20" r="1"/>
            </svg>
            <input type="text" id="ssid-input" class="input" placeholder="Enter network name (SSID)" autocomplete="off" autocapitalize="off"/>
          </div>
        </div>
        <div id="password-field" class="form-field hidden">
          <label class="field-label text-muted">Password</label>
          <div class="input-group">
            <svg class="input-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <rect x="3" y="11" width="18" height="11" rx="2" ry="2"/>
              <path d="M7 11V7a5 5 0 0 1 10 0v4"/>
            </svg>
            <input type="password" id="wifi-password" class="input" placeholder="Enter WiFi password" autocomplete="off"/>
            <button type="button" id="toggle-password" class="input-action">${Icons.eyeOpen}</button>
          </div>
        </div>
        <div class="form-buttons">
          <button type="button" id="back-btn" class="btn btn-outline">Back</button>
          <button type="submit" id="wifi-submit" class="btn btn-primary" disabled>Continue</button>
        </div>
      </form>
    `;

    // Bind events
    document.getElementById('wifi-form').addEventListener('submit', handleSubmit);
    document.getElementById('manual-toggle').addEventListener('click', toggleManualEntry);
    document.getElementById('back-btn').addEventListener('click', function() {
      if (callbacks.onBack) callbacks.onBack();
    });
    document.getElementById('toggle-password').addEventListener('click', togglePasswordVisibility);
    document.getElementById('wifi-password').addEventListener('input', updateSubmitButton);
    document.getElementById('ssid-input').addEventListener('input', updateSubmitButton);

    // Initial scan
    scanWifi();
  }

  function scanWifi() {
    // Cancel any pending scan poll
    if (scanTimer) {
      clearTimeout(scanTimer);
      scanTimer = null;
    }
    
    // Don't start a new scan if we're in manual mode
    if (state.isManualEntry) {
      return;
    }
    
    state.isScanning = true;
    var networkList = document.getElementById('network-list');
    
    if (!networkList) return;
    
    networkList.innerHTML = `
      <div class="network-scanning">
        <svg class="icon spinning" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <path d="M1 4v6h6"/><path d="M23 20v-6h-6"/>
          <path d="M20.49 9A9 9 0 0 0 5.64 5.64L1 10m22 4l-4.64 4.36A9 9 0 0 1 3.51 15"/>
        </svg>
        <span>Scanning...</span>
      </div>
    `;

    fetch("/api/wifi/scan")
      .then(function(r) { return r.json(); })
      .then(function(data) {
        // If we switched to manual mode while waiting, abort
        if (state.isManualEntry) {
          state.isScanning = false;
          return;
        }
        
        if (data.scanning) {
          scanTimer = setTimeout(scanWifi, 1500);
          return;
        }
        state.networks = data.networks || [];
        state.isScanning = false;
        renderNetworkList();
      })
      .catch(function() {
        state.isScanning = false;
        if (networkList) {
          networkList.innerHTML = '<div class="network-scanning"><span>Failed to scan</span></div>';
        }
      });
  }

  function renderNetworkList() {
    var networkList = document.getElementById('network-list');
    
    if (state.networks.length === 0) {
      networkList.innerHTML = '<div class="network-scanning"><span>No networks found</span></div>';
      return;
    }

    var seen = {};
    var uniqueNetworks = state.networks
      .slice()
      .sort(function(a, b) { return b.rssi - a.rssi; })
      .filter(function(net) {
        if (!net.ssid || seen[net.ssid]) return false;
        seen[net.ssid] = true;
        return true;
      });

    networkList.innerHTML = uniqueNetworks.map(function(net) {
      var isSelected = state.selectedNetwork === net.ssid;
      var signalLevel = net.rssi > -50 ? 4 : net.rssi > -60 ? 3 : net.rssi > -70 ? 2 : 1;

      return `
        <button type="button" class="network-item ${isSelected ? 'selected' : ''}" data-ssid="${escapeHtml(net.ssid)}" data-secure="${net.secure}">
          <div class="network-info">
            ${Icons.wifiSmall}
            <span class="network-name">${escapeHtml(net.ssid)}</span>
          </div>
          <div class="network-meta">
            ${net.secure ? Icons.lock : ''}
            <div class="signal-bars">
              <div class="signal-bar ${signalLevel >= 1 ? 'active' : ''}"></div>
              <div class="signal-bar ${signalLevel >= 2 ? 'active' : ''}"></div>
              <div class="signal-bar ${signalLevel >= 3 ? 'active' : ''}"></div>
              <div class="signal-bar ${signalLevel >= 4 ? 'active' : ''}"></div>
            </div>
            ${isSelected ? Icons.chevronRight : ''}
          </div>
        </button>
      `;
    }).join('');

    // Add click handlers
    networkList.querySelectorAll('.network-item').forEach(function(item) {
      item.addEventListener('click', function() {
        selectNetwork(item.dataset.ssid, item.dataset.secure === 'true');
      });
    });
  }

  function selectNetwork(ssid, secure) {
    state.selectedNetwork = ssid;
    renderNetworkList();

    var passwordField = document.getElementById('password-field');
    if (secure) {
      passwordField.classList.remove('hidden');
    } else {
      passwordField.classList.add('hidden');
      document.getElementById('wifi-password').value = '';
    }

    updateSubmitButton();
  }

  function toggleManualEntry() {
    state.isManualEntry = !state.isManualEntry;
    var toggle = document.getElementById('manual-toggle');
    var networkList = document.getElementById('network-list');
    var manualSsid = document.getElementById('manual-ssid');
    var passwordField = document.getElementById('password-field');
    var label = document.getElementById('network-label');

    if (state.isManualEntry) {
      // Cancel any pending scan when switching to manual
      if (scanTimer) {
        clearTimeout(scanTimer);
        scanTimer = null;
      }
      state.isScanning = false;
      
      toggle.textContent = 'Scan';
      label.textContent = 'Network Name';
      networkList.classList.add('hidden');
      manualSsid.classList.remove('hidden');
      passwordField.classList.remove('hidden');
      state.selectedNetwork = null;
    } else {
      toggle.textContent = 'Manual';
      label.textContent = 'Available Networks';
      networkList.classList.remove('hidden');
      manualSsid.classList.add('hidden');
      document.getElementById('ssid-input').value = '';
      
      // Restore password field visibility if a network was previously selected
      if (state.selectedNetwork) {
        var selectedNet = state.networks.find(function(n) { return n.ssid === state.selectedNetwork; });
        if (selectedNet && selectedNet.secure) {
          passwordField.classList.remove('hidden');
        } else {
          passwordField.classList.add('hidden');
        }
      } else {
        passwordField.classList.add('hidden');
      }
      
      // Trigger a new scan when switching back (after state is updated)
      scanWifi();
    }

    updateSubmitButton();
  }

  function togglePasswordVisibility() {
    var input = document.getElementById('wifi-password');
    var btn = document.getElementById('toggle-password');

    if (input.type === 'password') {
      input.type = 'text';
      btn.innerHTML = Icons.eyeClosed;
    } else {
      input.type = 'password';
      btn.innerHTML = Icons.eyeOpen;
    }
  }

  function updateSubmitButton() {
    var submitBtn = document.getElementById('wifi-submit');
    var password = document.getElementById('wifi-password').value;

    if (state.isManualEntry) {
      var ssid = document.getElementById('ssid-input').value.trim();
      submitBtn.disabled = !ssid || !password;
    } else {
      var selectedNet = state.networks.find(function(n) { return n.ssid === state.selectedNetwork; });
      var needsPassword = selectedNet ? selectedNet.secure : false;
      submitBtn.disabled = !state.selectedNetwork || (needsPassword && !password);
    }
  }

  function handleSubmit(e) {
    e.preventDefault();
    
    var ssid = state.isManualEntry 
      ? document.getElementById('ssid-input').value.trim() 
      : state.selectedNetwork;
    var password = document.getElementById('wifi-password').value;

    if (callbacks.onSubmit) {
      callbacks.onSubmit(ssid, password);
    }
  }

  function getState() {
    return {
      networks: state.networks.slice(),
      selectedNetwork: state.selectedNetwork,
      isManualEntry: state.isManualEntry
    };
  }

  function reset() {
    state.selectedNetwork = null;
    state.isManualEntry = false;
  }

  return {
    render: render,
    scanWifi: scanWifi,
    getState: getState,
    reset: reset
  };
})();
