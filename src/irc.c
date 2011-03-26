#include "inari.h"

#include "command.h"

irc_server_t connect_to_server(char* server, int port, char* nick) {
  irc_server_t irc;
  LOG("Connecting to server: %s:%d as %s", server, port, nick);

  struct hostent* host;
  struct in_addr ip_addr;
  int sockfd = -1;
  struct sockaddr_in dest;


  if(!(host = gethostbyname(server))) {
    LOG("Connect to '%s' failed!", server);
    irc.status = CLOSED;
    return irc;
  }

  /* TODO: should probably check more than just the first IP result */
  ip_addr = *(struct in_addr*)(host->h_addr_list[0]);

  LOG("Resolved '%s' to %s", server, inet_ntoa(ip_addr));

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_addr = ip_addr;
  dest.sin_port = htons(port);

  if(connect(sockfd, (struct sockaddr*)&dest, sizeof(struct sockaddr))) {
    LOG("Connect to %s failed!", inet_ntoa(ip_addr));
    irc.status = CLOSED;
    return irc;
  }

  LOG("Connected to %s", inet_ntoa(ip_addr));
  
  irc.socketfd = sockfd;
  irc.status = AUTH;
  irc.nick = nick;
  irc.num_admins = 0;
  irc.admins = malloc(0);

  return irc;
}

void irc_destroy(irc_server_t *irc) {
  /* close socket */
  if(irc->socketfd)
    close(irc->socketfd);

  /* free all malloc'd admin nicks */
  unsigned i;
  for(i = 0; i < irc->num_admins; ++i) {
    free(irc->admins[i]);
  }
  free(irc->admins);
  free(irc);
}

void irc_add_admin(irc_server_t* irc, char* nick) {
  LOG("Adding admin '%s'", nick);

  if(irc_is_admin(*irc, nick)) {
    LOG("'%s' is already an admin, ignoring.", nick);
    return;
  }

  unsigned num = irc->num_admins += 1;

  /* allow variables that will go out of scope or be overwritten to be set as admins */
  char* nick_cpy = malloc(strlen(nick));
  nick_cpy = strcpy(nick_cpy, nick);

  irc->admins  = realloc(irc->admins, sizeof(char*) * num);
  irc->admins[num - 1] = nick_cpy;
}

int irc_is_admin(irc_server_t irc, char* nick) {
  if(!irc.num_admins) {
    return 0;
  }

  unsigned i;
  for(i = 0; i < irc.num_admins; ++i) {
    if(!strcmp(nick, irc.admins[i])) {
      return 1;
    }
  }

  return 0;
}

void irc_handle(irc_server_t* irc) {
  char buffer[MESSAGE_SIZE];
  memset(buffer, 0, MESSAGE_SIZE);
  int len = recv(irc->socketfd, buffer, MESSAGE_SIZE, 0);

  /* error occured, close */
  if(len <= 0) {
    LOG("ERROR: Closing");
    irc->status = CLOSED;
    return;
  }

  /* get rid of \r\n bit */
  buffer[len - 2] = '\0';

  if(irc->echo) {
    LOG("<< %s", buffer);
  }

  switch(irc->status) {
  case AUTH:
    /* Search for authentication success code */
    if(strstr(buffer, "001")) {
      irc->status = CONN;
      LOG("Authentication successful");
    }
    break;
  case CONN:
    irc_handle_msg(irc, buffer);
    break;
  case CLOSED:
    return;
  }
}

void irc_handle_msg(irc_server_t* irc, char* msg) {
  
  /* server ping */
  if(strstr(msg, "PING") == msg) {
    msg[1] = 'O'; /* PONG */
    irc_send(*irc, msg);
    return;
  }

  command_handle_msg(irc, msg);
}

void irc_authenticate(irc_server_t irc, char* pass) {
  LOG("Authenticating with server");

  irc_sendf(irc, "NICK %s", irc.nick);
  irc_sendf(irc, "USER %s * * :inari", irc.nick);

  if(pass) {
    irc_privmsg(irc, "NickServ IDENTIFY", pass);
  }
}

int irc_send(irc_server_t irc, char* msg) {
  if(irc.echo) {
    LOG(">> %s", msg);
  }
  
  char* nl = "\r\n";

  int size = send(irc.socketfd, msg, strlen(msg), 0);
  size += send(irc.socketfd, nl, strlen(nl), 0);

  return size;
}

int irc_sendf(irc_server_t irc, char* fmt, ...) {
  char buf[0x1000];
  va_list args;
  va_start(args, fmt);

  vsnprintf(buf, 0x1000, fmt, args);

  va_end(args);
  return irc_send(irc, buf);
}

void irc_join(irc_server_t irc, char* chan) {
  irc_sendf(irc, "JOIN %s", chan);
}

void irc_part(irc_server_t irc, char* chan) {
  irc_sendf(irc, "PART %s", chan);
}

int irc_privmsg(irc_server_t irc, char* where, char* msg) {
  return irc_sendf(irc, "PRIVMSG %s :%s", where, msg);
}

int irc_privmsgf(irc_server_t irc, char* chan, char* fmt, ...) {
  char buf[0x1000];
  va_list args;
  va_start(args, fmt);
  
  vsnprintf(buf, 0x1000, fmt, args);
  
  va_end(args);
  return irc_privmsg(irc, chan, buf);
}
