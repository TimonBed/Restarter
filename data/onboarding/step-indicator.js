// Step Indicator Component
// Displays progress through wizard steps

var StepIndicator = (function() {
  "use strict";

  var STEPS = ["Welcome", "WiFi", "MQTT", "Done"];

  var Icons = {
    check: `<svg class="icon-xs" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="3"><polyline points="20 6 9 17 4 12"/></svg>`
  };

  function render(container, currentStep) {
    var html = STEPS.map(function(step, idx) {
      var isCompleted = idx < currentStep;
      var isCurrent = idx === currentStep;
      var circleClass = isCompleted ? "completed" : isCurrent ? "current" : "pending";
      var labelClass = isCurrent ? "current" : "pending";

      var content = '<div class="step-item">';
      content += '<div style="display:flex;flex-direction:column;align-items:center;">';
      content += '<div class="step-circle ' + circleClass + '">';
      content += isCompleted ? Icons.check : (idx + 1);
      content += '</div>';
      content += '<span class="step-label ' + labelClass + '">' + step + '</span>';
      content += '</div>';

      if (idx < STEPS.length - 1) {
        content += '<div class="step-line ' + (isCompleted ? 'completed' : 'pending') + '"></div>';
      }

      content += '</div>';
      return content;
    }).join('');

    container.innerHTML = html;
  }

  function getSteps() {
    return STEPS.slice();
  }

  return {
    render: render,
    getSteps: getSteps
  };
})();
