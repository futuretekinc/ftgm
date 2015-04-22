#include <signal.h>
#include <stdio.h>
#include <string.h>

#ifndef WIN32
#  include <unistd.h>
#else
#  include <process.h>
#  define snprintf sprintf_s
#endif

#include <mosquitto.h>

#define db_host "localhost"
#define db_username "mqtt_log"
#define db_password "password"
#define db_database "mqtt_log"
#define db_port 3306

#define db_query "INSERT INTO mqtt_log (topic, payload) VALUES (?,?)"

#define mqtt_host "10.0.1.16"
#define mqtt_port 1883

static int run = 1;

void handle_signal(int s)
{
	run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	printf("message->topic = %s\n", message->topic);
	printf("message->payload= %s\n", (char *)message->payload);
}

int main(int argc, char *argv[])
{
	char clientid[24];
	struct mosquitto *mosq;
	int rc = 0;

	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	mosquitto_lib_init();

	memset(clientid, 0, 24);
	snprintf(clientid, 23, "mysql_log_%d", getpid());
	mosq = mosquitto_new(clientid, true, NULL);
	if(mosq){
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_message_callback_set(mosq, message_callback);


	    rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);

		mosquitto_subscribe(mosq, NULL, "#", 0);

		while(run){
			rc = mosquitto_loop(mosq, -1, 1);
			if(run && rc){
				sleep(20);
				mosquitto_reconnect(mosq);
			}
		}
		mosquitto_destroy(mosq);
	}

	mosquitto_lib_cleanup();

	return rc;
}

