#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

// Notification types
typedef enum {
    NOTIFY_INFO,
    NOTIFY_WARNING,
    NOTIFY_ERROR,
    NOTIFY_SUCCESS
} NotificationType;

// Notification channels
typedef enum {
    CHANNEL_WEBHOOK,
    CHANNEL_EMAIL,
    CHANNEL_SLACK,
    CHANNEL_DISCORD,
    CHANNEL_CUSTOM
} NotificationChannel;

// Notification configuration
typedef struct {
    NotificationChannel channel;
    char endpoint[512];
    char auth_token[256];
    int enabled;
} NotificationConfig;

// Initialize notifications
int notifications_init(const char *config_file);

// Send notification
int notify_send(NotificationType type, const char *title, const char *message);

// Channel-specific functions
int notify_webhook(const char *url, const char *payload);
int notify_slack(const char *webhook_url, const char *message);
int notify_discord(const char *webhook_url, const char *message);

// Task notifications
void notify_task_complete(const char *task_name, int success, double duration_ms);
void notify_pipeline_complete(int total_tasks, int successful, int failed);
void notify_error(const char *error_message);

// Cleanup
void notifications_cleanup(void);

#endif // NOTIFICATIONS_H
