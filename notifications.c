#include "notifications.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

static NotificationConfig g_notification_config = {0};

int notifications_init(const char *config_file) {
    // In production, parse config file
    g_notification_config.enabled = 0; // Disabled by default

    // Check environment for webhook URL
    const char *webhook = getenv("REPROVM_WEBHOOK_URL");
    if (webhook) {
        g_notification_config.channel = CHANNEL_WEBHOOK;
        strncpy(g_notification_config.endpoint, webhook,
                sizeof(g_notification_config.endpoint) - 1);
        g_notification_config.enabled = 1;
        LOG_INFO("Notifications enabled: webhook=%s", webhook);
    }

    return 0;
}

// Stub implementation - would use libcurl in production
int notify_webhook(const char *url, const char *payload) {
    if (!g_notification_config.enabled) return 0;

    LOG_DEBUG("Webhook notification: url=%s, payload=%s", url, payload);

    // In production, use libcurl to POST
    // CURL *curl = curl_easy_init();
    // if (curl) {
    //     curl_easy_setopt(curl, CURLOPT_URL, url);
    //     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    //     CURLcode res = curl_easy_perform(curl);
    //     curl_easy_cleanup(curl);
    //     return (res == CURLE_OK) ? 0 : -1;
    // }

    return 0;
}

int notify_send(NotificationType type, const char *title, const char *message) {
    if (!g_notification_config.enabled) return 0;

    const char *type_str;
    switch (type) {
        case NOTIFY_INFO:    type_str = "INFO"; break;
        case NOTIFY_WARNING: type_str = "WARNING"; break;
        case NOTIFY_ERROR:   type_str = "ERROR"; break;
        case NOTIFY_SUCCESS: type_str = "SUCCESS"; break;
        default:             type_str = "UNKNOWN"; break;
    }

    char payload[2048];
    snprintf(payload, sizeof(payload),
             "{\"type\":\"%s\",\"title\":\"%s\",\"message\":\"%s\"}",
             type_str, title, message);

    return notify_webhook(g_notification_config.endpoint, payload);
}

int notify_slack(const char *webhook_url, const char *message) {
    char payload[2048];
    snprintf(payload, sizeof(payload), "{\"text\":\"%s\"}", message);
    return notify_webhook(webhook_url, payload);
}

int notify_discord(const char *webhook_url, const char *message) {
    char payload[2048];
    snprintf(payload, sizeof(payload), "{\"content\":\"%s\"}", message);
    return notify_webhook(webhook_url, payload);
}

void notify_task_complete(const char *task_name, int success, double duration_ms) {
    char message[512];
    snprintf(message, sizeof(message),
             "Task '%s' %s in %.2f ms",
             task_name,
             success ? "completed successfully" : "failed",
             duration_ms);

    notify_send(success ? NOTIFY_SUCCESS : NOTIFY_ERROR,
                "Task Complete", message);
}

void notify_pipeline_complete(int total_tasks, int successful, int failed) {
    char message[512];
    snprintf(message, sizeof(message),
             "Pipeline complete: %d total, %d successful, %d failed",
             total_tasks, successful, failed);

    notify_send(failed > 0 ? NOTIFY_WARNING : NOTIFY_SUCCESS,
                "Pipeline Complete", message);
}

void notify_error(const char *error_message) {
    notify_send(NOTIFY_ERROR, "Error", error_message);
}

void notifications_cleanup(void) {
    // Cleanup
}
