/**
 * =============================================================================
 * LokiHandler.h - Grafana Loki Log Shipping
 * =============================================================================
 */

#pragma once

/**
 * Initialize Loki handler.
 * Call once in setup().
 */
void LokiHandler_setup();

/**
 * Push buffered logs to Loki periodically.
 * Call continuously in loop().
 */
void LokiHandler_loop();

/**
 * Log a message to Loki (and Serial).
 * @param level Log level: "DEBUG", "INFO", "WARN", "ERROR"
 * @param message Log message
 */
void Loki_log(const char *level, const char *message);

/**
 * Printf-style logging to Loki.
 * @param level Log level
 * @param format Printf format string
 * @param ... Format arguments
 */
void Loki_logf(const char *level, const char *format, ...);

// Convenience macros
void Loki_debug(const char *message);
void Loki_info(const char *message);
void Loki_warn(const char *message);
void Loki_error(const char *message);
