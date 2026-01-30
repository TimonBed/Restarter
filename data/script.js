// v5 - WiFi SSID dropdown with scan
(function () {
  "use strict";

  // DOM elements
  const $ = (id) => document.getElementById(id);
  const deviceInfo = $("device-info");
  const pcState = $("pc-state");
  const tempDisplay = $("temperature");
  const hddLastActive = $("hdd-last-active");
  const wifiState = $("wifi-state");
  const wifiSsidDisplay = $("wifi-ssid-display");
  const wifiIp = $("wifi-ip");
  const wifiRssi = $("wifi-rssi");
  const signalBars = $("signal-bars");
  const powerBtn = $("power-btn");
  const powerProgress = $("power-progress");
  const resetHoldBtn = $("reset-hold-btn");
  const resetProgress = $("reset-progress");
  const actionLog = $("action-log");
  const setupPanel = $("setup-panel");
  const setupForm = $("setup-form");
  const timingForm = $("timing-form");
  const wifiSsidSelect = $("wifi-ssid");
  const scanBtn = $("scan-btn");

  // Tab switching
  function initTabs() {
    const tabBtns = document.querySelectorAll(".tab-btn");
    tabBtns.forEach(function(btn) {
      btn.addEventListener("click", function() {
        const tabId = btn.dataset.tab;
        // Update button states
        tabBtns.forEach(function(b) { b.classList.remove("tab-active"); });
        btn.classList.add("tab-active");
        // Show/hide tab content
        document.querySelectorAll(".tab-content").forEach(function(content) {
          content.style.display = content.id === "tab-" + tabId ? "" : "none";
        });
      });
    });
  }
  initTabs();

  // WiFi network scanning
  let scanPollTimer = null;
  
  function scanWifi() {
    scanBtn.disabled = true;
    scanBtn.textContent = "...";
    
    fetch("/api/wifi/scan")
      .then(function(r) { return r.json(); })
      .then(function(data) {
        if (data.scanning) {
          // Scan in progress, poll again
          scanPollTimer = setTimeout(scanWifi, 1500);
          return;
        }
        // Got results
        populateSsidDropdown(data.networks || []);
        scanBtn.disabled = false;
        scanBtn.textContent = "Scan";
      })
      .catch(function() {
        scanBtn.disabled = false;
        scanBtn.textContent = "Scan";
      });
  }
  
  function populateSsidDropdown(networks) {
    const currentVal = wifiSsidSelect.value;
    wifiSsidSelect.innerHTML = '<option value="">Select Network...</option>';
    
    // Sort by signal strength (strongest first)
    networks.sort(function(a, b) { return b.rssi - a.rssi; });
    
    // Remove duplicates (keep strongest)
    const seen = {};
    networks.forEach(function(net) {
      if (net.ssid && !seen[net.ssid]) {
        seen[net.ssid] = true;
        const opt = document.createElement("option");
        opt.value = net.ssid;
        const signal = net.rssi > -50 ? "●●●●" : net.rssi > -60 ? "●●●○" : net.rssi > -70 ? "●●○○" : "●○○○";
        const lock = net.secure ? "🔒" : "";
        opt.textContent = net.ssid + " " + signal + " " + lock;
        wifiSsidSelect.appendChild(opt);
      }
    });
    
    // Restore previous selection if still available
    if (currentVal) {
      wifiSsidSelect.value = currentVal;
    }
  }
  
  scanBtn.addEventListener("click", scanWifi);
  
  // Auto-scan on page load when setup panel is shown
  function triggerInitialScan() {
    if (setupPanel.style.display !== "none") {
      scanWifi();
    }
  }

  // Track AP mode state
  let isApMode = false;
  let modeLocked = false;

  // Hide STA-only sections and show setup panel in AP mode
  function applyApMode(apMode) {
    if (modeLocked) return;
    isApMode = apMode;
    
    const staOnlySections = document.querySelectorAll("[data-sta-only]");
    staOnlySections.forEach(function(section) {
      section.style.display = apMode ? "none" : "";
    });
    
    if (apMode) {
      setupPanel.classList.remove("hidden");
      setupPanel.style.display = "";
    }
  }

  function initSetupPanel() {
    fetch("/api/config")
      .then((r) => r.json())
      .then((cfg) => {
        // Store configured SSID to restore after scan
        if (cfg.wifiSsid) {
          const opt = document.createElement("option");
          opt.value = cfg.wifiSsid;
          opt.textContent = cfg.wifiSsid + " (configured)";
          wifiSsidSelect.appendChild(opt);
          wifiSsidSelect.value = cfg.wifiSsid;
        }
        // Pre-fill other form fields
        $("mqtt-host").value = cfg.mqttHost || "";
        $("mqtt-port").value = cfg.mqttPort || 1883;
        $("mqtt-user").value = cfg.mqttUser || "";
        // Timing fields
        $("power-pulse-ms").value = cfg.powerPulseMs || 500;
        $("reset-pulse-ms").value = cfg.resetPulseMs || 250;
        $("boot-grace-ms").value = cfg.bootGraceMs || 60000;

        // Check if in AP mode (no WiFi configured = AP mode)
        if (!cfg.wifiSsid) {
          applyApMode(true);
          triggerInitialScan();
        }
        modeLocked = true; // Lock - never change visibility again
      })
      .catch(() => {
        // Network error likely means AP mode - show setup only
        applyApMode(true);
        triggerInitialScan();
        modeLocked = true;
      });
  }

  // Status display - simple direct updates
  function updateStatus(data) {
    if (data.hostname) {
      deviceInfo.textContent = data.hostname + (data.deviceId ? " • " + data.deviceId : "");
    }
    if (data.pcState) {
      pcState.textContent = data.pcState;
    }
    if (typeof data.temperature === "number") {
      tempDisplay.textContent = data.temperature.toFixed(1) + " °C";
    }
    if (typeof data.hddLastActiveSec === "number") {
      if (data.hddLastActiveSec < 0) {
        hddLastActive.textContent = "Never";
      } else if (data.hddLastActiveSec < 5) {
        hddLastActive.textContent = "Active";
      } else if (data.hddLastActiveSec < 60) {
        hddLastActive.textContent = data.hddLastActiveSec + "s ago";
      } else if (data.hddLastActiveSec < 3600) {
        hddLastActive.textContent = Math.floor(data.hddLastActiveSec / 60) + "m ago";
      } else {
        hddLastActive.textContent = Math.floor(data.hddLastActiveSec / 3600) + "h ago";
      }
    }
    if (typeof data.apMode === "boolean" || typeof data.wifiConnected === "boolean") {
      if (data.apMode) {
        wifiState.textContent = "AP Mode";
      } else if (data.wifiConnected) {
        wifiState.textContent = "Connected";
      } else {
        wifiState.textContent = "Disconnected";
      }
    }
    if (data.ssid) {
      wifiSsidDisplay.textContent = data.ssid;
    }
    if (data.ip !== undefined) {
      wifiIp.textContent = data.ip || "—";
    }
    if (typeof data.rssi === "number") {
      wifiRssi.textContent = data.rssi ? data.rssi + " dBm" : "—";
      updateSignalBars(data.rssi);
    }
  }

  // Signal strength bars
  function updateSignalBars(rssi) {
    const bars = signalBars.querySelectorAll("[data-bar]");
    let level = 0;
    if (rssi <= -80 || rssi === 0) level = 0;
    else if (rssi <= -70) level = 1;
    else if (rssi <= -60) level = 2;
    else if (rssi <= -50) level = 3;
    else level = 4;
    bars.forEach(function (bar) {
      const n = parseInt(bar.dataset.bar, 10);
      bar.classList.toggle("bg-emerald-500", n <= level);
      bar.classList.toggle("bg-slate-600", n > level);
    });
  }

  // Action log
  function addLog(msg) {
    const entry = document.createElement("div");
    entry.textContent = "[" + new Date().toLocaleTimeString() + "] " + msg;
    actionLog.prepend(entry);
    while (actionLog.children.length > 20) {
      actionLog.lastChild.remove();
    }
  }

  // WebSocket - only for live status and logs
  let ws = null;
  let retryDelay = 500;

  function connectWs() {
    const proto = location.protocol === "https:" ? "wss:" : "ws:";
    ws = new WebSocket(proto + "//" + location.host + "/ws");

    ws.onopen = function () {
      retryDelay = 500;
    };

    ws.onmessage = function (e) {
      try {
        const msg = JSON.parse(e.data);
        if (msg.type === "log") {
          addLog(msg.message || "Action");
        } else {
          updateStatus(msg);
        }
      } catch (err) {
        // Ignore parse errors
      }
    };

    ws.onclose = function () {
      setTimeout(connectWs, retryDelay + Math.random() * 200);
      retryDelay = Math.min(retryDelay * 2, 8000);
    };
  }

  // API actions
  function postAction(endpoint) {
    fetch(endpoint, { method: "POST" }).catch(function () {});
  }

  // Hold button factory
  function setupHoldButton(btn, progressBar, endpoint) {
    let timer = null;
    let start = 0;

    function beginHold(e) {
      e.preventDefault();
      start = Date.now();
      timer = setInterval(function () {
        const elapsed = Date.now() - start;
        const pct = Math.min(100, (elapsed / 3000) * 100);
        progressBar.style.width = pct + "%";
        if (elapsed >= 3000) {
          endHold(true);
        }
      }, 50);
    }

    function endHold(trigger) {
      if (timer) {
        clearInterval(timer);
        timer = null;
      }
      progressBar.style.width = "0%";
      if (trigger) {
        postAction(endpoint);
      }
    }

    btn.addEventListener("mousedown", beginHold);
    btn.addEventListener("touchstart", beginHold);
    btn.addEventListener("mouseup", function () { endHold(false); });
    btn.addEventListener("mouseleave", function () { endHold(false); });
    btn.addEventListener("touchend", function () { endHold(false); });
    btn.addEventListener("touchcancel", function () { endHold(false); });
  }

  setupHoldButton(powerBtn, powerProgress, "/api/action/power");
  setupHoldButton(resetHoldBtn, resetProgress, "/api/action/reset");

  // Setup form submit
  setupForm.addEventListener("submit", function (e) {
    e.preventDefault();
    const payload = {
      wifiSsid: $("wifi-ssid").value.trim(),
      wifiPass: $("wifi-pass").value,
      mqttHost: $("mqtt-host").value.trim(),
      mqttPort: parseInt($("mqtt-port").value, 10) || 1883,
      mqttUser: $("mqtt-user").value.trim(),
      mqttPass: $("mqtt-pass").value,
      powerPulseMs: parseInt($("power-pulse-ms").value, 10) || 500,
      resetPulseMs: parseInt($("reset-pulse-ms").value, 10) || 250,
      bootGraceMs: parseInt($("boot-grace-ms").value, 10) || 60000
    };
    fetch("/api/config", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    }).then(function () {
      setupPanel.classList.add("hidden");
      setupPanel.style.display = "none";
    }).catch(function () {});
  });

  // Timing form submit (saves timing without reboot)
  timingForm.addEventListener("submit", function (e) {
    e.preventDefault();
    fetch("/api/config")
      .then((r) => r.json())
      .then((cfg) => {
        const payload = {
          wifiSsid: cfg.wifiSsid || "",
          wifiPass: "",
          mqttHost: cfg.mqttHost || "",
          mqttPort: cfg.mqttPort || 1883,
          mqttUser: cfg.mqttUser || "",
          mqttPass: "",
          powerPulseMs: parseInt($("power-pulse-ms").value, 10) || 500,
          resetPulseMs: parseInt($("reset-pulse-ms").value, 10) || 250,
          bootGraceMs: parseInt($("boot-grace-ms").value, 10) || 60000
        };
        return fetch("/api/config", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify(payload)
        });
      })
      .then(function () {
        addLog("Timing saved - rebooting...");
      })
      .catch(function () {});
  });

  // Initialize
  initSetupPanel();
  connectWs();
})();
