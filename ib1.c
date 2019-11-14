#define _POSIX_C_SOURCE 200112L

#define DEBUG 1

#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
	char *nick, *channel, *host, *port;
	int conn;
} IRC_Connection;

typedef struct {
	char *from, *command, *where, *message, *sep, *target;
} IRC_Input;

IRC_Connection connection;

int i, j, l, sl, o = -1;
char buf[513];
char sbuf[512];

void write_raw(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(sbuf, 512, fmt, ap);
	va_end(ap);
	write(connection.conn, sbuf, strlen(sbuf));
#if DEBUG
	printf("<< %s", sbuf);
#endif
}

void connect_to_network()
{
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(connection.host, connection.port, &hints, &res);
	connection.conn = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	connect(connection.conn, res->ai_addr, res->ai_addrlen);

	write_raw("USER %s 0 0 :%s\r\n", connection.nick, connection.nick);
	write_raw("NICK %s\r\n", connection.nick);
}

void handle_input(IRC_Input *input)
{
	if (!strncmp(input->command, "001", 3) && connection.channel != NULL)
		write_raw("JOIN %s\r\n", connection.channel);

	if (!strncmp(input->command, "PRIVMSG", 7) || !strncmp(input->command, "NOTICE", 6)) {
		if (input->where == NULL || input->message == NULL) return;
		if ((input->sep = strchr(input->from, '!')) != NULL)
			input->from[input->sep - input->from] = '\0';

		if (input->where[0] == '#' || input->where[0] == '&' || input->where[0] == '+'
				|| input->where[0] == '!')
			input->target = input->where;
		else
			input->target = input->from;

		if (!strncmp(input->message, "hi", 2) && !strncmp(input->where, "#botchan", 8)) {
			write_raw("PRIVMSG #botchan :Hello, world!\r\n");
		}
	}
}

void start_bot()
{
	int start = 0, wordcount = 0;
	IRC_Input input;

	while ((sl = read(connection.conn, sbuf, 512))) {
		start = wordcount = 0;
		memset(&input, 0, sizeof(IRC_Input));
		for (i = 0; i < sl; i++) {
			o++;
			buf[o] = sbuf[i];
			if ((i > 0 && sbuf[i] == '\n' && sbuf[i - 1] == '\r') || o == 512) {
#if DEBUG
				printf(">> %s\n", buf);
#endif
				buf[o + 1] = '\0';
				l = o;
				o = -1;

				if (!strncmp(buf, "PING", 4)) {
					/* PONG */
					buf[1] = 'O';
					write_raw(buf);
				} else if (buf[0] == ':') {
					for (j = 1; j < l; j++) {
						if (buf[j] == ' ') {
							buf[j] = '\0';
							wordcount++;
							switch(wordcount) {
								case 1: input.from = buf + 1; break;
								case 2: input.command = buf + start; break;
								case 3: input.where = buf + start; break;
							}
							if (j == l - 1) continue;
								start = j + 1;
							} else if (buf[j] == ':' && wordcount == 3) {
							if (j < l - 1) input.message = buf + j + 1;
							break;
						}
					}
				}
				if (wordcount < 2) continue;

				handle_input(&input);
				wordcount = 0;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	connection.nick = "bot";
	connection.channel = "#botchan"; 
	connection.host = "localhost";
	connection.port = "6667";

	connect_to_network();
	start_bot();

	return 0;
}
