#ifndef IRC_IB1_H
#define IRC_IB1_H

typedef struct {
	char *nick, *chan, *host, *port;
	int conn;
} IRC_Connection;

typedef struct {
	char *from, *command, *where, *message, *sep, *target;
} IRC_Input;

void write_raw(char *fmt, ...);
void privmsg(char *channel, char *fmt, ...);

#endif
