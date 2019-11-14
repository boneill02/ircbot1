#define _POSIX_C_SOURCE 200112L

#include <netdb.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"

typedef struct {
	char *nick, *chan, *host, *port;
	int conn;
} IRC_Connection;

typedef struct {
	char *from, *command, *where, *message, *sep, *target;
} IRC_Input;

IRC_Connection connection;

int i, j, l, sl, o = -1;
char buf[513];
char sbuf[512];

void connect_to_network();
void handle_input(IRC_Input *input);
void start_bot();
void usage(const char *argv0);
void write_raw(char *fmt, ...);


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
	if (!strncmp(input->command, "001", 3) && connection.chan != NULL)
		write_raw("JOIN %s\r\n", connection.chan);

	if (!strncmp(input->command, "PRIVMSG", 7) || !strncmp(input->command, "NOTICE", 6)) {
		if (input->where == NULL || input->message == NULL) return;
		if ((input->sep = strchr(input->from, '!')) != NULL)
			input->from[input->sep - input->from] = '\0';

		if (input->where[0] == '#' || input->where[0] == '&' || input->where[0] == '+'
				|| input->where[0] == '!')
			input->target = input->where;
		else
			input->target = input->from;
		
		if (!strncmp(input->where, connection.chan, strlen(connection.chan))) {
			if (!strncmp(input->message, "hello", 5)) {
				write_raw("PRIVMSG %s :Hello, world!\r\n", connection.chan);
			}
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

void usage(const char *argv0)
{
	printf("usage: %s [-h host] [-p port] [-n nick] [-c chan]\n", argv0);
}

int main(int argc, char *argv[])
{
	connection.nick = BOT_NICK;
	connection.chan = BOT_CHAN;
	connection.host = BOT_HOST;
	connection.port = BOT_PORT;
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			switch (argv[i][0]) {
				case '-':
					switch (argv[i][1]) {
						case 'h': connection.host = argv[++i]; break;
						case 'p': connection.port = argv[++i]; break;
						case 'n': connection.nick = argv[++i]; break;
						case 'c': connection.chan = argv[++i]; break;
						default: usage(argv[0]);
					}
					break;
				default: usage(argv[0]);
			}
		}
	}

	connect_to_network();
	start_bot();

	return 0;
}
