#include <context.h>
#include <stdbool.h>

bool is_valid_context(struct context *ctx)
{
        return ctx != NULL && ctx->modem != NULL && ctx->port != NULL; // TODO: is valid modem and port
}
