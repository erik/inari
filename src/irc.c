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

  /* set as nonblocking */
  int opts = fcntl(irc.socketfd, F_GETFL);
  opts = (opts | O_NONBLOCK);
  fcntl(irc.socketfd, F_SETFL, opts);
  
  irc.socketfd = sockfd;
  irc.status = AUTH;
  irc.nick = nick;

  return irc;
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

  /* TODO: allow user to turn echo on or off for a specific server */
  LOG("<< %s", buffer);

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

  char* nickmsg = malloc(strlen(irc.nick) + 5);
  strcpy(nickmsg, "NICK ");
  strcat(nickmsg, irc.nick);

  irc_send(irc, nickmsg);
  irc_send(irc, "USER inari * * :inari");

  if(pass) {
    irc_privmsg(irc, "NickServ IDENTIFY", pass);
  }

  free(nickmsg);
}

int irc_send(irc_server_t irc, char* msg) {
  LOG(">> %s", msg);
  unsigned len = strlen(msg);
  char* snd = malloc(len + 2);

  strcpy(snd, msg);
  strcat(snd, "\r\n");

  int size = send(irc.socketfd, snd, strlen(snd), 0);
  free(snd);
  return size;
}

void irc_join(irc_server_t irc, char* chan) {
  unsigned len = strlen(chan);
  char* msg = malloc(5 + len);

  strcpy(msg, "JOIN ");
  strcat(msg, chan);

  irc_send(irc, msg);
  free(msg);
  return;
}

int irc_privmsg(irc_server_t irc, char* where, char* msg) {
  /* TODO: this is a mess */
  char* cmd = "PRIVMSG ";
  char* snd = malloc(strlen(where) + strlen(msg) + strlen(cmd));

  strcpy(snd, cmd);
  strcat(snd, where);
  strcat(snd, " :");
  strcat(snd, msg);

  int size = irc_send(irc, snd);
  free(snd);
  return size;
}
