#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fnmatch.h>
#include <cJSON.h>

#define PORT 3000
#define EP_ROOT "/"
#define EP_FILES "/files"
#define EP_FILES_WILDCARD "/files/*"
#define EP_PARAM "/param"
#define INDEX "/files/index.html"
#define STATIC_DIRECTORY "./static"
#define CONFIG_JSON "./config.json"
#define MAX_PATH_LENGTH 256
#define MAX_POST_SIZE 65535

static int redirect(struct MHD_Connection *connection, const char *message, const char *target_url)
{
    struct MHD_Response *response;
    int ret;
    response = MHD_create_response_from_buffer(strlen(message), (void *)message, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Location", target_url);
    ret = MHD_queue_response(connection, MHD_HTTP_TEMPORARY_REDIRECT, response);
    MHD_destroy_response(response);
    return ret;
}
static int respond(struct MHD_Connection *connection, const char *message, int response_code)
{
    struct MHD_Response *response;
    int ret;
    response = MHD_create_response_from_buffer(strlen(message), (void *)message, MHD_RESPMEM_MUST_COPY);
    ret = MHD_queue_response(connection, response_code, response);
    MHD_destroy_response(response);
    return ret;
}

static int root_handler(struct MHD_Connection *connection)
{
    // redirect to static files
    return redirect(connection, "Redirecting...", INDEX);
}

static int files_handler(struct MHD_Connection *connection, const char *url)
{
    struct MHD_Response *response;
    int ret;

    // strip prefix
    const char *stripped_url = url + strlen(EP_FILES);
    if (strcmp(stripped_url, "/") == 0 || strcmp(stripped_url, "") == 0)
    {
        return redirect(connection, "Redirecting...", INDEX);
    }
    else
    {
        char local_path[MAX_PATH_LENGTH];
        snprintf(local_path, MAX_PATH_LENGTH, "%s%s", STATIC_DIRECTORY, stripped_url);
        FILE *fp = fopen(local_path, "rb");

        if (fp == NULL)
        {
            return respond(connection, "file not found", MHD_HTTP_NOT_FOUND);
        }

        fseek(fp, 0L, SEEK_END);
        int sz = ftell(fp);
        rewind(fp);
        response = MHD_create_response_from_fd(sz, fileno(fp));
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }
}
int KeyValueIterator(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
    cJSON_AddStringToObject((cJSON *)cls, key, value);
}
static int param_handler(struct MHD_Connection *connection, const char *method)
{
    struct MHD_Response *response;
    int ret;
    FILE *fp = fopen(CONFIG_JSON, "rb+");
    if (fp == NULL)
    {
        return respond(connection, "Config file not found", MHD_HTTP_NOT_FOUND);
    }
    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);
    rewind(fp);

    if (strcmp(method, "GET") == 0)
    {
        // get all param
        response = MHD_create_response_from_fd(sz, fileno(fp));
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }
    else if (strcmp(method, "POST") == 0)
    {
        struct MHD_HTTP_Header *pos;
        char *data = malloc(sz + 1);
        size_t copied_sz = fread(data, 1, sz, fp);

        cJSON *json = cJSON_Parse(data);
        free(data);
        MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, KeyValueIterator, (void *)json);

        fp = freopen(CONFIG_JSON, "wb", fp);

        fputs(cJSON_Print(json), fp);
        fclose(fp);
        return respond(connection, "Success", MHD_HTTP_OK);
    }
    else
    {
        return respond(connection, "Invalid method", MHD_HTTP_BAD_REQUEST);
    }
}
static int ahc_echo(void *cls,
                    struct MHD_Connection *connection,
                    const char *url,
                    const char *method,
                    const char *version,
                    const char *upload_data,
                    size_t *upload_data_size,
                    void **ptr)
{
    int ret;
    if (strcmp(url, EP_ROOT) == 0)
    {
        printf("Request root endpoint\n");
        ret = root_handler(connection);
    }
    else if (strcmp(url, EP_PARAM) == 0)
    {
        printf("Request param endpoint\n");
        ret = param_handler(connection, method);
    }

    else if (fnmatch(EP_FILES_WILDCARD, url, 0) == 0 || strcmp(url, EP_FILES) == 0)
    {
        printf("Request files endpoint\n");
        ret = files_handler(connection, url);
    }
    return ret;
}

int main(int argc, char **argv)
{
    struct MHD_Daemon *d;
    d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                         PORT,
                         NULL,
                         NULL,
                         &ahc_echo,
                         NULL,
                         MHD_OPTION_END);
    if (d == NULL)
        return 1;
    (void)getc(stdin);
    MHD_stop_daemon(d);
    return 0;
}