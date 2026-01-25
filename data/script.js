// Cache DOM elements once.
const deviceInfo = document.getElementById("device-info");
const pcState = document.getElementById("pc-state");
const wifiState = document.getElementById("wifi-state");
const powerBtn = document.getElementById("power-btn");
const resetHoldBtn = document.getElementById("reset-hold-btn");
const holdProgress = document.getElementById("hold-progress");
const resetSlider = document.getElementById("reset-slider");
const actionLog = document.getElementById("action-log");
const setupPanel = document.getElementById("setup-panel");
const setupForm = document.getElementById("setup-form");

let holdTimer = null; // Interval for hold-to-reset progress
let holdStart = null; // Timestamp when hold started

function setStatus(data) {
  // Update UI labels from the status payload.
  deviceInfo.textContent = `${data.hostname || "unknown"} • ${data.deviceId || ""}`;
  pcState.textContent = data.pcState || "—";
  wifiState.textContent = data.apMode ? "AP Mode" : (data.wifiConnected ? "Connected" : "Disconnected");
  const hasConfig = data.hasConfig === true;
  const needsOnboarding = !hasConfig;
  setupPanel.classList.toggle("hidden", !(data.apMode || needsOnboarding));
}

function fetchStatus() {
  // Fallback polling if WebSocket drops.
  fetch("/api/status")
    .then((res) => res.json())
    .then(setStatus)
    .catch(() => {});
}

function setupWebSocket() {
  // Live updates via WebSocket.
  const protocol = location.protocol === "https:" ? "wss" : "ws";
  const ws = new WebSocket(`${protocol}://${location.host}/ws`);
  ws.onmessage = (evt) => {
    try {
      setStatus(JSON.parse(evt.data));
    } catch {}
  };
  ws.onclose = () => setTimeout(setupWebSocket, 2000);
}

function postAction(path) {
  // Small helper for POST actions.
  return fetch(path, { method: "POST" });
}

function logAction(message) {
  if (!actionLog) {
    return;
  }
  const time = new Date().toLocaleTimeString();
  const line = document.createElement("div");
  line.textContent = `[${time}] ${message}`;
  actionLog.prepend(line);
  const entries = actionLog.querySelectorAll("div");
  if (entries.length > 20) {
    entries[entries.length - 1].remove();
  }
}

powerBtn.addEventListener("click", () => {
  postAction("/api/action/power");
  logAction("Power pulse requested");
});

function resetHoldStart() {
  // Start 3-second hold timer with visual progress.
  holdStart = Date.now();
  holdTimer = setInterval(() => {
    const elapsed = Date.now() - holdStart;
    const pct = Math.min(100, (elapsed / 3000) * 100);
    holdProgress.style.width = `${pct}%`;
    if (elapsed >= 3000) {
      resetHoldStop(true);
    }
  }, 50);
}

function resetHoldStop(trigger) {
  // Stop hold timer; trigger reset if completed.
  if (holdTimer) {
    clearInterval(holdTimer);
    holdTimer = null;
  }
  holdProgress.style.width = "0%";
  if (trigger) {
    postAction("/api/action/reset");
    logAction("Reset pulse requested (hold)");
  }
}

resetHoldBtn.addEventListener("mousedown", resetHoldStart);
resetHoldBtn.addEventListener("touchstart", resetHoldStart);
["mouseup", "mouseleave", "touchend", "touchcancel"].forEach((evt) => {
  resetHoldBtn.addEventListener(evt, () => resetHoldStop(false));
});

resetSlider.addEventListener("input", () => {
  // Slider unlocks reset at 100%.
  if (Number(resetSlider.value) >= 100) {
    resetSlider.value = 0;
    postAction("/api/action/reset");
    logAction("Reset pulse requested (slider)");
  }
});

setupForm.addEventListener("submit", (evt) => {
  // Save config to the device.
  evt.preventDefault();
  const payload = {
    wifiSsid: document.getElementById("wifi-ssid").value.trim(),
    wifiPass: document.getElementById("wifi-pass").value,
    mqttHost: document.getElementById("mqtt-host").value.trim(),
    mqttPort: Number(document.getElementById("mqtt-port").value || 1883),
    mqttUser: document.getElementById("mqtt-user").value.trim(),
    mqttPass: document.getElementById("mqtt-pass").value,
  };
  fetch("/api/config", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload),
  });
});

// Pre-fill setup form and show onboarding if WiFi missing.
fetch("/api/config")
  .then((res) => res.json())
  .then((cfg) => {
    document.getElementById("wifi-ssid").value = cfg.wifiSsid || "";
    document.getElementById("mqtt-host").value = cfg.mqttHost || "";
    document.getElementById("mqtt-port").value = cfg.mqttPort || 1883;
    document.getElementById("mqtt-user").value = cfg.mqttUser || "";
    if (!cfg.wifiSsid) {
      setupPanel.classList.remove("hidden");
    }
  })
  .catch(() => {
    setupPanel.classList.remove("hidden");
  });

// Start UI updates.
fetchStatus();
setupWebSocket();
