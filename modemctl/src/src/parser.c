#include <parser.h>

#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <response.h>
#include <stdio.h>

bool is_empty(const char *line, size_t len)
{
        return line == NULL || len == 0;
}


bool is_ok(const char *line, size_t len)
{
        if (is_empty(line, len)) {
                return false;
        }
        char ok[] = "OK";
        return (len == sizeof(ok) - 1) && (strncmp(line, ok, sizeof(ok) - 1) == 0);
}


bool is_error(const char *line, size_t len)
{
        if (is_empty(line, len)) {
                return false;
        }
        char error[] = "ERROR";
        if ((len == sizeof(error) - 1) && (strncmp(line, error, sizeof(error) - 1) == 0)) {
                return true;
        }
        char cme_error[] = "+CME ERROR";
        if ((len >= sizeof(cme_error) - 1) && (strncmp(line, cme_error, sizeof(cme_error) - 1) == 0)) {
                return true;
        }
        char cms_error[] = "+CMS ERROR";
        if ((len >= sizeof(cms_error) - 1) && (strncmp(line, cms_error, sizeof(cms_error) - 1) == 0)) {
                return true;
        }
        return false;
}


bool is_end(const char *line, size_t len)
{
        return is_ok(line, len) || is_error(line, len);
}

bool is_unsolicited_event(const char *line, size_t len) // gal but reikia filtruoti konkrecius pranesimus; turi buti tikrinama po klaidos tikrinimo
{
        if (is_empty(line, len)) {
                return false;
        }
        return line[0] == '+';
}