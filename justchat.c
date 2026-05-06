#include <libwebsockets.h>
#include <string.h>
#include <signal.h>

static int interrupted;

struct client {
        char channel_name[32];
        char msg[900];
        int msg_len;
        int has_msg;
        struct lws *id;
        struct client *next;
};
static struct client *clients = NULL;

static struct client *find_client(struct lws* wsi) {
        struct client *client_pointer_my = clients;

        while (client_pointer_my != NULL) {
                if (client_pointer_my->id == wsi) {
                        return client_pointer_my;
                }
                client_pointer_my = client_pointer_my->next;
        }
        return NULL;
}

static int 
callback_chat(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) 
{
        uint8_t buf[LWS_PRE + 1024], *start = &buf[LWS_PRE], *p = start, *end = &buf[sizeof(buf) -1];
        const char *val;
        int n;

        switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
                // ADD CLIENT
                struct client *new_client_p = NULL;
                new_client_p = malloc(sizeof(struct client));
                if (!new_client_p) return -1;

                memset(new_client_p, 0, sizeof(struct client));
                new_client_p->id = wsi; 
                new_client_p->next = clients;
                clients = new_client_p;
                // ADD CLIENT

                lwsl_user("New client connected!");
                break;
        case LWS_CALLBACK_RECEIVE:
                struct client * client_p = find_client(wsi);
                if (client_p == NULL) {
                        lwsl_user("Getting client failed");
                        return 0;
                }
                if (len > sizeof(client_p->msg))
                    len = sizeof(client_p->msg);


                memcpy(client_p->msg, in, len);
                client_p->msg_len = len;


                // TOKENIZING
                char raw_msg[900];               
                if (len >= sizeof(raw_msg))
                    len = sizeof(raw_msg) - 1;

                memcpy(raw_msg, in, len);
                raw_msg[len] = '\0';
                char *msg_tokenize = strtok(raw_msg, ",");
                char channel_name[32];
                int channel_yes = 0; 
                char username[32];
                int username_yes = 0;
                char message[900];
                while (msg_tokenize != NULL) {
                        if (channel_yes == 0) {
                                strncpy(channel_name, msg_tokenize, sizeof(channel_name));
                                channel_name[sizeof(channel_name)-1] = '\0';
                                channel_yes = 1;
                        } else if (username_yes == 0) {
                                strncpy(username, msg_tokenize, sizeof(username));
                                username[sizeof(username)-1] = '\0';
                                username_yes = 1;
                        } else {
                                strncpy(message, msg_tokenize, sizeof(message) - 1);
                                message[sizeof(message) - 1] = '\0';
                        }
                        msg_tokenize = strtok(NULL, ",");
                }

                // TOKENIZING
                strncpy(client_p->channel_name, channel_name, sizeof(client_p->channel_name)-1);
                client_p->channel_name[sizeof(client_p->channel_name)-1] = '\0';

                lwsl_user("Channel: %s | User: %s | Message: %s", channel_name, username, message);
                struct client *client_loop = clients;
                while (client_loop != NULL) {
                        if (strcmp(client_loop->channel_name, channel_name) == 0) {
                                memcpy(client_loop->msg, in, len);
                                client_loop->msg_len = len;
                                client_loop->has_msg = 1;
                                lws_callback_on_writable(client_loop->id);
                        }
                        client_loop = client_loop->next;
                } 
                break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
                char *send_message = &buf[LWS_PRE];
                
                struct client *client_msg = find_client(wsi);
                if (client_msg == NULL) return 0;

                if (!client_msg->has_msg)
                        return 0;

                memcpy(send_message, client_msg->msg, client_msg->msg_len);
                lws_write(wsi, send_message, client_msg->msg_len, LWS_WRITE_TEXT);
                client_msg->has_msg = 0;
                break;
        case LWS_CALLBACK_CLOSED:
                struct client *client_rem = find_client(wsi);
                if (client_p == NULL) return 0; 
                if (clients == client_rem) 
                        clients = client_rem->next;
                else {
                        struct client *cur_p = clients;
                        while (cur_p != NULL && cur_p->next != client_rem) {
                                cur_p = cur_p->next;
                        } 
                        if (cur_p != NULL) {
                                cur_p->next = client_rem->next;
                        }
                }
                free(client_rem);
                lwsl_user("Client disconnected!");
                break;
        default:
                break;
        }
        return 0;
} 


static struct lws_protocols protocols[] = {
        { "chat", callback_chat, 0, 1024 }, 
        { NULL, NULL, 0, 0 }
};

static const struct lws_http_mount mount = {
        .mountpoint		= "/",			
        .origin			= "./page",
        .def			= "index.html",	
        .origin_protocol	= LWSMPRO_FILE,	
        .mountpoint_len		= 1,	
};

void sigint_handler(int sig)
{
        interrupted = 1;
}

int main(int argc, const char **argv)
{
        struct lws_context_creation_info info;
        struct lws_context *context;
        const char *p;
        int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

        signal(SIGINT, sigint_handler);

        if ((p = lws_cmdline_option(argc, argv, "-d")))
                logs = atoi(p);

        lws_set_log_level(logs, NULL);
        lwsl_user("justchat! - Websocket based minimal chat server.\n");
        lwsl_user("PORT: 8080\n");
        memset(&info, 0, sizeof info); 
        info.port = 8080;
        info.protocols = protocols;
        info.mounts = &mount;
        info.error_document_404 = "/404.html";

        context = lws_create_context(&info);
        if (!context) {
                lwsl_err("lws init failed\n");
                return 1;
        }

        while (n >= 0 && !interrupted)
                n = lws_service(context, 1000);

        lws_context_destroy(context);

        return 0;
}

