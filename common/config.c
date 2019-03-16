/****************************************************************************************
MIT License

Copyright (c) 2019 Plan C

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/

#include "config.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>


/*********************************************************
 * Function     : load config file
 * Parameters   : filepath - path of config file
 * Return       : config_t* or NULL
*********************************************************/
config_t* config_load(const char* filepath)
{
    GKeyFile* ini = g_key_file_new ();
    GError* error = NULL;

    if(!g_key_file_load_from_file(ini, filepath, G_KEY_FILE_NONE, &error))
    {
        log_printf(LOG_ERROR, "Cannot load config file(%s) : %s.",
                    filepath, error->message);
        
        return NULL;
    }

    return ini;
}


/*********************************************************
 * Function     : get groups name of config
 * Parameters   : config - config_t*
 *                length - return number of groups
 * Return       : array of groups name or NULL
*********************************************************/
char** config_get_groups (config_t* config, gsize* length)
{
    return g_key_file_get_groups(config, length);
}


/*********************************************************
 * Function     : get value of group and key
 * Parameters   : config - config_t*
 *                group  - group name
 *                key    - key
 * Return       : value of group[key] or NULL
*********************************************************/
char* config_get_value(config_t* config, const char* group, const char* key)
{
    GError* error = NULL;
    char* ret = g_key_file_get_value(config, group, key, &error);
    if(ret == NULL)
    {
        log_printf(LOG_ERROR, "Cannot get config value %s[%s] : %s.",
                     group, key, error->message);
        
        return NULL;
    }

    return ret;
}


/*********************************************************
 * Function     : free memory of config
 * Parameters   : config - config_t*
 * Return       : void
*********************************************************/
void config_free(config_t* config)
{
    g_key_file_free(config);
}
