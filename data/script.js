// v2 - setup panel isolated from websocket
(function () {
  "use strict";

  // DOM elements
  const $ = (id) => document.getElementById(id);
  const deviceInfo = $("device-info");
  const pcState = $("pc-state");
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

  // Setup panel: determined ONCE on load, never touched by WebSocket
  let setupPanelLocked = false;

  function initSetupPanel() {
    fetch("/api/config")
      .then((r) => r.json())
      .then((cfg) => {
        // Pre-fill form fields
        $("wifi-ssid").value = cfg.wifiSsid || "";
        $("mqtt-host").value = cfg.mqttHost || "";
        $("mqtt-port").value = cfg.mqttPort || 1883;
        $("mqtt-user").value = cfg.mqttUser || "";
        // Timing fields
        $("power-pulse-ms").value = cfg.powerPulseMs || 500;
        $("reset-pulse-ms").value = cfg.resetPulseMs || 250;
        $("boot-grace-ms").value = cfg.bootGraceMs || 60000;

        // Show panel only if no WiFi configured
        if (!cfg.wifiSsid) {
          setupPanel.classList.remove("hidden");
          setupPanel.style.display = "";
        }
        setupPanelLocked = true; // Lock - never change visibility again
      })
      .catch(() => {
        // Network error in AP mode - show setup
        setupPanel.classList.remove("hidden");
        setupPanel.style.display = "";
        setupPanelLocked = true;
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
