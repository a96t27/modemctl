#include <context.h>
#include <stdbool.h>

bool is_valid_context(struct context *ctx)
{
        return ctx != NULL && ctx->modem != NULL && is_valid_at_port(ctx->port);
}


bool is_valid_at_port(struct at_port *port)
{
        return port != NULL && port->fd >= 0 && port->timeout_seconds >= 0;
}