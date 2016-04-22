#include <syslog.h>
#include <time.h>

int main(int argc, char *argv[])
{
	time_t	xTime;
	syslog(LOG_INFO, "%s", asctime(gmtime(&xTime)));
}
