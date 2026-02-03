/**
 * =============================================================================
 * MetricsHandler.h - Prometheus Metrics Endpoint
 * =============================================================================
 */

#pragma once

/**
 * Initialize Prometheus metrics endpoint.
 * Registers GET /metrics route.
 * Call once in setup().
 */
void MetricsHandler_setup();

/**
 * Metrics handler loop (no-op, metrics are pull-based).
 * Call in loop() for consistency.
 */
void MetricsHandler_loop();
