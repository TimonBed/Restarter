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
  const forcePowerBtn = $("force-power-btn");
  const forcePowerProgress = $("force-power-progress");
  const actionLog = $("action-log");
  const timingForm = $("timing-form");
  const otaCurrentVersion = $("ota-current-version");
  const otaRemoteWrap = $("ota-remote-wrap");
  const otaRemoteVersion = $("ota-remote-version");
  const otaNotes = $("ota-notes");
  const otaError = $("ota-error");
  const otaProgressWrap = $("ota-progress-wrap");
  const otaProgress = $("ota-progress");
  const otaCheckBtn = $("ota-check-btn");
  const otaUpdateBtn = $("ota-update-btn");

  let csrfToken = "";
  let otaPollTimer = null;

  function initConfig() {
    fetch("/api/config")
      .then((r) => r.json())
      .then((cfg) => {
        // Timing fields
        $("power-pulse-ms").value = cfg.powerPulseMs || 500;
        $("reset-pulse-ms").value = cfg.resetPulseMs || 500;
        $("boot-grace-ms").value = cfg.bootGraceMs || 60000;
        // Integrations: MQTT
        if (cfg.mqtt) {
          $("mqtt-host").value = cfg.mqtt.host || "";
          $("mqtt-port").value = cfg.mqtt.port || 1883;
          $("mqtt-user").value = cfg.mqtt.user || "";
          $("mqtt-pass").value = "";
          $("mqtt-tls").checked = !!cfg.mqtt.tls;
        }
        // Integrations: Loki
        if (cfg.loki) {
          $("loki-host").value = cfg.loki.host || "";
          $("loki-user").value = cfg.loki.user || "";
          $("loki-pass").value = "";
        }
      })
      .catch(() => {});
  }

  // Status display - simple direct updates
  function updateStatus(data) {
    if (data.hostname) {
      deviceInfo.textContent = data.hostname + (data.deviceId ? " • " + data.deviceId : "");
    }
    if (data.fwVersion && otaCurrentVersion) {
      otaCurrentVersion.textContent = data.fwVersion;
    }
    if (data.csrfToken) {
      csrfToken = data.csrfToken;
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
    // ESP32 system stats
    if (typeof data.cpuLoad === "number") {
      $("esp-cpu").textContent = data.cpuLoad + "%";
      $("esp-cpu-bar").style.width = data.cpuLoad + "%";
    }
    if (typeof data.freeHeap === "number" && typeof data.totalHeap === "number") {
      const usedHeap = data.totalHeap - data.freeHeap;
      const usedPct = Math.round((usedHeap / data.totalHeap) * 100);
      const freeKb = Math.round(data.freeHeap / 1024);
      const totalKb = Math.round(data.totalHeap / 1024);
      $("esp-ram").textContent = freeKb + " / " + totalKb + " KB";
      $("esp-ram-used").style.width = usedPct + "%";
    }
    if (data.ota) {
      renderOtaStatus(data.ota);
    }
  }

  function renderOtaStatus(ota) {
    if (!otaCurrentVersion) return;

    if (ota.currentVersion) {
      otaCurrentVersion.textContent = ota.currentVersion;
    }

    const hasRemote = !!ota.remoteVersion;
    otaRemoteWrap.classList.toggle("hidden", !hasRemote);
    otaRemoteVersion.textContent = hasRemote ? ota.remoteVersion : "—";

    const hasNotes = !!ota.notes;
    otaNotes.classList.toggle("hidden", !hasNotes);
    otaNotes.textContent = hasNotes ? ota.notes : "";

    const hasError = !!ota.error;
    otaError.classList.toggle("hidden", !hasError);
    otaError.textContent = hasError ? ota.error : "";

    const progress = Number(ota.progress || 0);
    const showProgress = !!ota.updateInProgress || progress > 0;
    otaProgressWrap.classList.toggle("hidden", !showProgress);
    otaProgress.style.width = Math.max(0, Math.min(100, progress)) + "%";

    const canUpdate = !!ota.available && !ota.updateInProgress;
    otaUpdateBtn.classList.toggle("hidden", !canUpdate);
    otaUpdateBtn.disabled = !canUpdate;
    otaCheckBtn.disabled = !!ota.checking || !!ota.updateInProgress;
    if (ota.updateInProgress) {
      otaCheckBtn.textContent = "Updating...";
      otaUpdateBtn.textContent = "Updating...";
    } else {
      otaCheckBtn.textContent = "Check for Updates";
      otaUpdateBtn.textContent = "Update Now";
    }
  }

  function fetchOtaStatus() {
    return fetch("/api/ota/status")
      .then(function (r) { return r.json(); })
      .then(function (ota) {
        renderOtaStatus(ota || {});
        return ota;
      })
      .catch(function () { return null; });
  }

  function setOtaPolling(enabled) {
    if (otaPollTimer) {
      clearInterval(otaPollTimer);
      otaPollTimer = null;
    }
    if (enabled) {
      otaPollTimer = setInterval(fetchOtaStatus, 1200);
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
        const pct = Math.min(100, (elapsed / 2000) * 100);
        progressBar.style.width = pct + "%";
        if (elapsed >= 2000) {
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
  setupHoldButton(forcePowerBtn, forcePowerProgress, "/api/action/force-power");

  // Timing form submit
  timingForm.addEventListener("submit", function (e) {
    e.preventDefault();
    fetch("/api/config")
      .then((r) => r.json())
      .then((cfg) => {
        const payload = {
          wifiSsid: cfg.wifiSsid || "",
          wifiPass: "",
          mqtt: { host: cfg.mqtt?.host || "", port: cfg.mqtt?.port || 1883, user: cfg.mqtt?.user || "", pass: "", tls: !!cfg.mqtt?.tls },
          loki: { host: cfg.loki?.host || "", user: cfg.loki?.user || "", pass: "" },
          powerPulseMs: parseInt($("power-pulse-ms").value, 10) || 500,
          resetPulseMs: parseInt($("reset-pulse-ms").value, 10) || 500,
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

  // Integrations form submit
  const integrationsForm = $("integrations-form");
  if (integrationsForm) {
    integrationsForm.addEventListener("submit", function (e) {
      e.preventDefault();
      fetch("/api/config")
        .then((r) => r.json())
        .then((cfg) => {
          const mqttPass = $("mqtt-pass").value;
          const lokiPass = $("loki-pass").value;
          const payload = {
            wifiSsid: cfg.wifiSsid || "",
            wifiPass: "",
            mqtt: {
              host: ($("mqtt-host").value || "").trim(),
              port: parseInt($("mqtt-port").value, 10) || 1883,
              user: ($("mqtt-user").value || "").trim(),
              pass: mqttPass,
              tls: $("mqtt-tls").checked
            },
            loki: {
              host: ($("loki-host").value || "").trim(),
              user: ($("loki-user").value || "").trim(),
              pass: lokiPass
            },
            powerPulseMs: cfg.powerPulseMs || 500,
            resetPulseMs: cfg.resetPulseMs || 500,
            bootGraceMs: cfg.bootGraceMs || 60000
          };
          return fetch("/api/config", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(payload)
          });
        })
        .then(function () {
          addLog("Integrations saved - rebooting...");
        })
        .catch(function () {});
    });
  }

  // Factory reset button
  const factoryResetBtn = $("factory-reset-btn");
  if (factoryResetBtn) {
    factoryResetBtn.addEventListener("click", function() {
      if (!confirm("This will clear all settings and restart in setup mode.\n\nAfter reset, connect to the ESP32's WiFi network to configure.")) {
        return;
      }
      factoryResetBtn.disabled = true;
      factoryResetBtn.textContent = "Resetting...";
      
      fetch("/api/factory-reset", { method: "POST" })
        .then(function() {
          addLog("Factory reset initiated - device will restart in AP mode");
          factoryResetBtn.textContent = "Restarting...";
          // Show instructions
          var section = $("device-section");
          if (section) {
            section.innerHTML = '<h2 class="text-base font-semibold">Device Restarting</h2>' +
              '<p class="text-sm text-slate-300">The device is restarting in setup mode.</p>' +
              '<p class="text-sm text-slate-400 mt-2">To complete setup:</p>' +
              '<ol class="text-sm text-slate-400 mt-1 space-y-1" style="list-style:decimal;padding-left:1.5rem;">' +
              '<li>Look for WiFi network starting with <strong class="text-slate-200">Restarter-</strong></li>' +
              '<li>Connect to that network</li>' +
              '<li>Open <strong class="text-slate-200">192.168.4.1</strong> in your browser</li>' +
              '</ol>';
          }
        })
        .catch(function() {
          factoryResetBtn.disabled = false;
          factoryResetBtn.textContent = "Factory Reset & Reconfigure";
          addLog("Factory reset failed");
        });
    });
  }

  if (otaCheckBtn) {
    otaCheckBtn.addEventListener("click", function () {
      fetch("/api/ota/check")
        .then(function (r) { return r.json(); })
        .then(function (ota) {
          renderOtaStatus(ota || {});
          if (ota && ota.available) {
            addLog("Firmware update available: " + (ota.remoteVersion || "new version"));
          } else {
            addLog("No firmware update available");
          }
        })
        .catch(function () {
          addLog("Failed to check for updates");
        });
    });
  }

  if (otaUpdateBtn) {
    otaUpdateBtn.addEventListener("click", function () {
      if (!csrfToken) {
        addLog("Update blocked: missing CSRF token");
        return;
      }
      if (!confirm("Install firmware update now? The device will reboot after update.")) {
        return;
      }

      fetch("/api/ota/update", {
        method: "POST",
        headers: { "X-CSRF-Token": csrfToken }
      })
        .then(function (r) { return r.json(); })
        .then(function (ota) {
          renderOtaStatus(ota || {});
          addLog("Firmware update started");
          setOtaPolling(true);
        })
        .catch(function () {
          addLog("Failed to start firmware update");
        });
    });
  }

  // Tabs: Status = status+controls, Settings = timing+device
  (function () {
    const tabBtns = document.querySelectorAll(".tab-btn[data-tab]");
    const panels = document.querySelectorAll("[data-tab-panel]");
    const tabToPanels = { status: ["status", "controls"], integrations: ["integrations"], log: ["log"], settings: ["timing", "settings"] };
    function showTab(tabId) {
      const show = tabToPanels[tabId] || [];
      tabBtns.forEach(function (btn) {
        btn.classList.toggle("tab-active", btn.getAttribute("data-tab") === tabId);
      });
      panels.forEach(function (panel) {
        const id = panel.getAttribute("data-tab-panel");
        panel.classList.toggle("hidden", show.indexOf(id) === -1);
      });
    }
    tabBtns.forEach(function (btn) {
      btn.addEventListener("click", function () {
        showTab(btn.getAttribute("data-tab"));
      });
    });
  })();

  // Initialize
  initConfig();
  connectWs();
  fetchOtaStatus().then(function (ota) {
    setOtaPolling(!!(ota && ota.updateInProgress));
  });
})();
