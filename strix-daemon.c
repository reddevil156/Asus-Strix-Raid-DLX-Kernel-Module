#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <pthread.h>
#include <poll.h>
#include <alsa/asoundlib.h>
#include <alsa/control.h>


#define DEFAULT_DEVICE		"/dev/strixdlx"

pthread_t thread_id_read;
pthread_t thread_id_write;
long volume = 0;
pthread_mutex_t lockWriteMutex;
const char *card = "default";
const char *selem_name = "Master";
snd_mixer_t *handle;
snd_mixer_selem_id_t *sid;
snd_mixer_elem_t* elem;
long min, max;


static int running = 0;
static int delay = 1;
static int counter = 0;
static char *conf_file_name = NULL;
static char *pid_file_name = NULL;
static int pid_fd = -1;
static char *app_name = NULL;
static FILE *log_stream;



/**
 * \brief Callback function for handling signals.
 * \param	sig	identifier of signal
 */
void handle_signal(int sig)
{
	if (sig == SIGINT) {
		fprintf(log_stream, "Debug: stopping daemon ...\n");
        if (thread_id_read >=0) {
            pthread_kill(thread_id_read, SIGINT);
            pthread_kill(thread_id_write, SIGINT);
        } 
		/* Unlock and close lockfile */
		if (pid_fd != -1) {
			lockf(pid_fd, F_ULOCK, 0);
			close(pid_fd);
		}
		/* Try to delete lockfile */
		if (pid_file_name != NULL) {
			unlink(pid_file_name);
		}
		running = 0;
		/* Reset signal handling to default behavior */
		signal(SIGINT, SIG_DFL);
	} else if (sig == SIGHUP) {
		fprintf(log_stream, "Debug: reloading daemon config file ...\n");
	} else if (sig == SIGCHLD) {
		fprintf(log_stream, "Debug: received SIGCHLD signal\n");
	}
}

/**
 * \brief This function will daemonize this app
 */
static void daemonize()
{
	pid_t pid = 0;
	int fd;

	/* Fork off the parent process */
	pid = fork();

	/* An error occurred */
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	/* Success: Let the parent terminate */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* On success: The child process becomes session leader */
	if (setsid() < 0) {
		exit(EXIT_FAILURE);
	}

	/* Ignore signal sent from child to parent process */
	signal(SIGCHLD, SIG_IGN);

	/* Fork off for the second time*/
	pid = fork();

	/* An error occurred */
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	/* Success: Let the parent terminate */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* Set new file permissions */
	umask(0);

	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	chdir("/");

	/* Close all open file descriptors */
	for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
		close(fd);
	}

	/* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
	stdin = fopen("/dev/null", "r");
	stdout = fopen("/dev/null", "w+");
	stderr = fopen("/dev/null", "w+");

	/* Try to write PID of daemon to lockfile */
	if (pid_file_name != NULL)
	{
		char str[256];
		pid_fd = open(pid_file_name, O_RDWR|O_CREAT, 0640);
		if (pid_fd < 0) {
			/* Can't open lockfile */
			exit(EXIT_FAILURE);
		}
		if (lockf(pid_fd, F_TLOCK, 0) < 0) {
			/* Can't lock file */
			exit(EXIT_FAILURE);
		}
		/* Get current PID */
		sprintf(str, "%d\n", getpid());
		/* Write PID to lockfile */
		write(pid_fd, str, strlen(str));
	}
}

void send_cmd(int fd, int cmd)
{
	int retval = 0;

	retval = write(fd, &cmd, 1);
	if (retval < 0)
		fprintf(stderr, "could not send command to fd=%d\n", fd);
}

void *readThread(void *vargp) {

	char c;
	int fd, i, n, value;
	
	char *dev = DEFAULT_DEVICE;
	short revents;
	struct pollfd pfd;
	char buf[3];


	printf("Open device %s\n", dev);
	fd = open(dev, O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(1);
	}

	//printf("Send command %i\n", cmd);
	//send_cmd(fd, cmd);

	pfd.fd = fd;
	pfd.events = POLLIN;

	while (1) {
        puts("loop");
        i = poll(&pfd, 1, -1);
        if (i == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        revents = pfd.revents;
        if (revents & POLLIN) {
            n = read(pfd.fd, buf, sizeof(buf));
			pthread_mutex_lock(&lockWriteMutex);
            printf("POLLIN n=%d buf=%.*s\n", n, n, buf);
			sscanf(buf, "%d", &value);
			printf("value %d\n", value);
			snd_mixer_selem_set_playback_volume_all(elem, value * max / 100);
			memset(buf, 0, sizeof(buf));
			volume = value*max /100;
			printf("volume aus read: %d\n", volume);
			pthread_mutex_unlock(&lockWriteMutex);
			
        }
    }
	close(fd);
}

void *writeThread(void *vargs) {

	int i = 100;
	long value = 0;
	int retval = 0;
	int send_buf;

	int fd;
	char *dev = DEFAULT_DEVICE;

	printf("Open device %s\n", dev);
	fd = open(dev, O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(1);
	}

	while(1) {
		pthread_mutex_lock(&lockWriteMutex);
		if (snd_mixer_handle_events(handle) <0) {
			goto next;
		}
		if (snd_mixer_selem_get_playback_volume(elem, 0, &value ) <0) {
			goto next;
		}

		if (value != volume) {
			printf("value direkt %d\n", value);
			volume = value;

			value = value *100 / max;
			printf("value reduziert %d\n", value);
			send_buf = (int)value;
			retval = write(fd, &send_buf, 1);
		if (retval < 0)
			fprintf(stderr, "could not send command to fd=%d\n", fd);
		}
next:
		printf("volume aus mixer %d\n", value);
		printf("volume aus write: %d\n", volume);

		pthread_mutex_unlock(&lockWriteMutex);
		sleep(1);
		
	}

	close(fd);

}

/* Main function */
int main(int argc, char *argv[])
{

    /* Open system log and write message to it */
	openlog(argv[0], LOG_PID|LOG_CONS, LOG_DAEMON);
	syslog(LOG_INFO, "Started %s", app_name);

	/* Daemon will handle two signals */
	signal(SIGINT, handle_signal);
	signal(SIGHUP, handle_signal);

    log_stream = stdout;

/*
*/
    pthread_mutex_init(&lockWriteMutex,0);

	
	int err;


	snd_mixer_open(&handle, 0);
    
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    elem = snd_mixer_find_selem(handle, sid);
	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

	printf("Start Thread");
	pthread_create(&thread_id_read, NULL, readThread, NULL);
	pthread_create(&thread_id_write, NULL, writeThread, NULL);
	
	pthread_join(thread_id_read, NULL);
	pthread_join(thread_id_write, NULL);
	printf("After Thread");
	
	snd_mixer_close(handle);

/*
*/
   	syslog(LOG_INFO, "Stopped %s", app_name);
	closelog();

	/* Free allocated memory */
	if (pid_file_name != NULL) free(pid_file_name);

	return EXIT_SUCCESS;


}