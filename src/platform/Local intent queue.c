#include "local_intent_queue.h"

#include <stdlib.h>
#include <string.h>

struct client_queue_node {
    struct platform_client_command command;
    struct client_queue_node *next;
};

struct server_queue_node {
    struct platform_server_command command;
    struct server_queue_node *next;
};

struct platform {
    struct client_queue_node *client_to_server_head;
    struct client_queue_node *client_to_server_tail;

    struct server_queue_node *server_to_client_head;
    struct server_queue_node *server_to_client_tail;
};

static void destroy_client_queue(struct platform *platform)
{
    struct client_queue_node *node = platform->client_to_server_head;

    while (node != NULL) {
        struct client_queue_node *next = node->next;

        if (node->command.type == PLATFORM_COMMAND_NODE_FLASHED) {
            free((void *)node->command.data.flashed.code);
        }

        free(node);
        node = next;
    }
}

static void destroy_server_queue(struct platform *platform)
{
    struct server_queue_node *node = platform->server_to_client_head;

    while (node != NULL) {
        struct server_queue_node *next = node->next;

        free(node);
        node = next;
    }
}

struct platform *platform_create(void)
{
    struct platform *platform = calloc(1, sizeof(*platform));

    return platform;
}

void platform_destroy(struct platform *platform)
{
    if (platform == NULL) {
        return;
    }

    destroy_client_queue(platform);
    destroy_server_queue(platform);

    free(platform);
}

bool platform_send(
    struct platform *platform,
    struct platform_client_command command
)
{
    struct client_queue_node *node;

    if (platform == NULL) {
        return false;
    }

    node = malloc(sizeof(*node));

    if (node == NULL) {
        return false;
    }

    node->command = command;
    node->next = NULL;

    if (command.type == PLATFORM_COMMAND_NODE_FLASHED) {
        if (command.data.flashed.code == NULL) {
            free(node);
            return false;
        }

        node->command.data.flashed.code =
            strdup(command.data.flashed.code);

        if (node->command.data.flashed.code == NULL) {
            free(node);
            return false;
        }
    }

    if (platform->client_to_server_tail == NULL) {
        platform->client_to_server_head = node;
        platform->client_to_server_tail = node;
    } else {
        platform->client_to_server_tail->next = node;
        platform->client_to_server_tail = node;
    }

    return true;
}

bool platform_has_client_command(
    const struct platform *platform
)
{
    if (platform == NULL) {
        return false;
    }

    return platform->client_to_server_head != NULL;
}

bool platform_receive_client_command(
    struct platform *platform,
    struct platform_client_command *command
)
{
    struct client_queue_node *node;

    if (platform == NULL ||
        command == NULL ||
        platform->client_to_server_head == NULL) {
        return false;
    }

    node = platform->client_to_server_head;

    *command = node->command;

    platform->client_to_server_head = node->next;

    if (platform->client_to_server_head == NULL) {
        platform->client_to_server_tail = NULL;
    }

    free(node);

    return true;
}

bool platform_has_server_command(
    const struct platform *platform
)
{
    if (platform == NULL) {
        return false;
    }

    return platform->server_to_client_head != NULL;
}

bool platform_receive_server_command(
    struct platform *platform,
    struct platform_server_command *command
)
{
    struct server_queue_node *node;

    if (platform == NULL ||
        command == NULL ||
        platform->server_to_client_head == NULL) {
        return false;
    }

    node = platform->server_to_client_head;

    *command = node->command;

    platform->server_to_client_head = node->next;

    if (platform->server_to_client_head == NULL) {
        platform->server_to_client_tail = NULL;
    }

    free(node);

    return true;
}
