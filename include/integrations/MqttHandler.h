/**
 * =============================================================================
 * MqttHandler.h - MQTT & Home Assistant Integration
 * =============================================================================
 */

#pragma once

/**
 * Initialize MQTT handler.
 * Call once in setup().
 */
void MqttHandler_setup();

/**
 * Handle MQTT connection and messages.
 * Call continuously in loop().
 */
void MqttHandler_loop();

/**
 * Publish current state to MQTT.
 * Call periodically (e.g., every 1-2 seconds).
 */
void MqttHandler_publishState();
