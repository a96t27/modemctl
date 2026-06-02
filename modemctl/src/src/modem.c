#include <modem.h>
#include <stdbool.h>

bool is_action_implemented(struct action *action)
{
        return action != NULL && action->at_cmd != NULL && action->parser != NULL && action->print_parser_resp != NULL;
}