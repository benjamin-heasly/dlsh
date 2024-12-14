/*
 * NAME
 *   stimctrl.c
 *
 * DESCRIPTION
 *   basic socket funcs for socket communication with stim program
 * bound to Tcl functions
 *
 * AUTHOR
 *   DLS, 8/99, 2/11
 *
 * ADAPTED FROM
 *   sockprog.cpp
 */

/*
  SOCKPROG.CPP
  Copyright 1995 by Ken Coviak
  Prepared for CS 671, Grand Valley State University
*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define DllEntryPoint DllMain
#define EXPORT(a,b) __declspec(dllexport) a b


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning (disable: 4996)

#ifdef ORIGINAL
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <windows.h>
#include <winsock.h>
#include "wsa_xtra.h"
#endif


#include <tcl.h>

/*
 * Local tables for holding imgs
 */
static Tcl_HashTable sockTable;
static int sockCount = 0;		/* increasing count of images  */


#define NUL             '\0' /* ASCII NUL character */ 
#define MAXLINE (65536)

/* function prototypes */
int send_command(Tcl_Interp *interp, char *cmd, SOCKET sock );
void pWSAerror(char *text );           /* error message text */


typedef struct _sockEntry {
  SOCKET socket;
} SOCKET_ENTRY;

static int sockFindSocket(Tcl_Interp *interp, char *name, SOCKET_ENTRY **sock)
{
  SOCKET_ENTRY *socket;
  Tcl_HashEntry *entryPtr;
  
  if ((entryPtr = Tcl_FindHashEntry(&sockTable, name))) {
    socket = Tcl_GetHashValue(entryPtr);
    if (!socket) {
      Tcl_SetResult(interp, "bad socket ptr in hash table", TCL_STATIC);
      return TCL_ERROR;
    }
    if (sock) *sock = socket;
    return TCL_OK;
  }
  else {
    if (sock) {			/* If sock was null, then don't set error */
      Tcl_AppendResult(interp, "socket \"", name, "\" not found", 
		       (char *) NULL);
    }
    return TCL_ERROR;
  }
}


static int sockAddSocket(Tcl_Interp *interp, SOCKET_ENTRY *socket, 
			 char *sockname)
{
  Tcl_HashEntry *entryPtr;
  int newentry;

  entryPtr = Tcl_CreateHashEntry(&sockTable, sockname, &newentry);
  Tcl_SetHashValue(entryPtr, socket);
  Tcl_SetResult(interp, sockname, TCL_STATIC);
  return TCL_OK;
}


static int sockDirCmd (ClientData data, Tcl_Interp *interp,
		       int argc, char *argv[])
{
  Tcl_HashEntry *entryPtr;
  Tcl_HashSearch searchEntry;
  Tcl_DString dirList;
  Tcl_DStringInit(&dirList);
  entryPtr = Tcl_FirstHashEntry(&sockTable, &searchEntry);
  if (entryPtr) {
    Tcl_DStringAppendElement(&dirList, Tcl_GetHashKey(&sockTable, entryPtr));
    while ((entryPtr = Tcl_NextHashEntry(&searchEntry))) {
      Tcl_DStringAppendElement(&dirList, Tcl_GetHashKey(&sockTable, entryPtr));
    }
  }
  Tcl_DStringResult(interp, &dirList);
  return TCL_OK;
}

static int freesocket(SOCKET_ENTRY *sockentry)
{
  if (sockentry) {
    shutdown(sockentry->socket, 2);
    closesocket(sockentry->socket);
    free(sockentry);
    WSACleanup();
    return 1;
  }
  return 0;
}

static int sockCloseCmd (ClientData data, Tcl_Interp *interp,
			  int argc, char *argv[])
{
  SOCKET_ENTRY *socket ;
  Tcl_HashEntry *entryPtr;
  int i;
  
  if (argc < 2) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " socket(s)", (char *) NULL);
    return TCL_ERROR;
  }
  
  for (i = 1; i < argc; i++) {
    if ((entryPtr = Tcl_FindHashEntry(&sockTable, argv[i]))) {
      if ((socket = Tcl_GetHashValue(entryPtr)))
	freesocket(socket);
      Tcl_DeleteHashEntry(entryPtr);
    }
    
    /* 
     * If user has specified "delete ALL" iterate through the hash table
     * and recursively call this function to close all open dyngroups
     */
    else if (!strcmp(argv[i],"ALL") || !strcmp(argv[1],"all")) {
      Tcl_HashSearch search;
      for (entryPtr = Tcl_FirstHashEntry(&sockTable, &search);
	   entryPtr != NULL;
	   entryPtr = Tcl_NextHashEntry(&search)) {
	Tcl_VarEval(interp, argv[0], " ", Tcl_GetHashKey(&sockTable, entryPtr),
		    (char *) NULL);
      }
      sockCount = 0;
    }
    else {
      Tcl_AppendResult(interp, argv[0], ": socket \"", argv[i], 
		       "\" not found", (char *) NULL);
      return TCL_ERROR;
    }
  }
  return TCL_OK;
}


static int sockOpenCmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
  int    port = (int) clientData;     /* port to connect to */
  WSADATA wsadata;        /* Windows Sockets API data */
  struct hostent FAR *hp; /* ptr to host info struct */
  struct sockaddr_in server; /* socket address and port */
  int    stat;            /* status return value */
  static char sockname[128];
  SOCKET_ENTRY *newsock;

  char *hoststr, *colon;
  char host[128];
  
  if (port != 0) {
    if (argc < 2) {
      sprintf(interp->result, "usage: %s host", argv[0]);
      return TCL_ERROR;
    }
  }
  else {
    if (argc < 3) {
      sprintf(interp->result, "usage: %s host port", argv[0]);
      return TCL_ERROR;
    }
    port = atoi(argv[2]);
  }  

  stat = WSAStartup( MAKEWORD(2,2), &wsadata );
  if (stat != 0) {
    sprintf(interp->result, "error loading winsock");
    return TCL_ERROR;
  }
  
  /*------------------------------------------------------*/
  /* Create the socket                                    */
  /*------------------------------------------------------*/

  newsock = (SOCKET_ENTRY *) calloc(1, sizeof(SOCKET_ENTRY));
  
  newsock->socket = socket( AF_INET, SOCK_STREAM, 0 );
  if (newsock->socket == INVALID_SOCKET) {
    sprintf(interp->result, "error creating socket");
    return TCL_ERROR;
  }
  
  /*------------------------------------------------------*/
  /* Connect (and bind) the socket to the service         */
  /* and provider.                                        */
  /*------------------------------------------------------*/

  hoststr = argv[1];
  if (!(colon = strchr(hoststr,':'))) {
    strcpy(host, hoststr);
  }
  else {
    strncpy(host, hoststr, colon-hoststr);
    host[colon-hoststr] = NULL;
    port = atoi(colon+1);	/* could have error checking here */
  }
  
  hp = gethostbyname( host );
  if (hp == NULL) {
    sprintf(interp->result, "%s: host %s not found", argv[0], argv[1]);
    return TCL_ERROR;
  }
  
  memset( &server, 0, sizeof(server) );
  memcpy( &server.sin_addr, hp->h_addr, hp->h_length );
  server.sin_family = PF_INET;
  server.sin_port = (u_short) htons((u_short) port);
  
  stat = connect( newsock->socket, (const struct sockaddr FAR *)&server,
		  sizeof(server) );
  if (stat != 0) {
    sprintf(interp->result, "%s: error connecting to host %s", 
	    argv[0], argv[1]);
    return TCL_ERROR;
  }
  
  sprintf(sockname, "sock%d", sockCount++);
  return sockAddSocket(interp, newsock, sockname);
}

int sockSendCmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
  SOCKET_ENTRY *sockentry;
  int cmdarg = 2;
  char *cmd;
  int i, len = 0;
  
  if (argc < 3) {
    sprintf(interp->result, "usage: %s socket command", argv[0]);
    return TCL_ERROR;
  }
  if (sockFindSocket(interp, argv[1], &sockentry) != TCL_OK)
    return TCL_ERROR;
  
  
  /* Multi args */
  if (argc > cmdarg) {
    /* Loop through and figure out how long the command line is */
    for (i = cmdarg; i < argc; i++) {
      len += (strlen(argv[i])+1);
    }
    cmd = (char *) calloc(len+1, sizeof(char));
    for (i = cmdarg; i < argc; i++) {
      strcat(cmd, argv[i]);
      strcat(cmd, " ");
    }
    send_command(interp, cmd,  sockentry->socket );
    free(cmd);
  }
  else {
    send_command(interp, argv[cmdarg],  sockentry->socket );
  }
  return TCL_OK;
}

int rmtSendCmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
  
  int    port = (int) clientData;     /* port to connect to */
  WSADATA wsadata;        /* Windows Sockets API data */
  SOCKET sock;            /* socket descriptor */
  struct hostent FAR *hp; /* ptr to host info struct */
  struct sockaddr_in server; /* socket address and port */
  int    stat;            /* status return value */
  int cmdarg = 2;

  char *hoststr, *colon;
  char host[128];

  if (port != 0) {
    if (argc < 3) {
      sprintf(interp->result, "usage: %s host command", argv[0]);
      return TCL_ERROR;
    }
  }
  else {
    if (argc < 4) {
      sprintf(interp->result, "usage: %s host port command", argv[0]);
      return TCL_ERROR;
    }
    port = atoi(argv[2]);
    cmdarg = 3;
  }  

  stat = WSAStartup( MAKEWORD(2,2), &wsadata );
  if (stat != 0) {
    sprintf(interp->result, "error loading winsock");
    return TCL_ERROR;
  }
  
  /*------------------------------------------------------*/
  /* Create the socket                                    */
  /*------------------------------------------------------*/
  
  sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if (sock == INVALID_SOCKET) {
    sprintf(interp->result, "error creating socket");
    WSACleanup();
    return TCL_ERROR;
  }
  
  /*------------------------------------------------------*/
  /* Connect (and bind) the socket to the service         */
  /* and provider.                                        */
  /*------------------------------------------------------*/


  hoststr = argv[1];
  if (!(colon = strchr(hoststr,':'))) {
    strcpy(host, hoststr);
  }
  else {
    strncpy(host, hoststr, colon-hoststr);
    host[colon-hoststr] = NULL;
    port = atoi(colon+1);	/* could have error checking here */
  }

  
  hp = gethostbyname( host );
  if (hp == NULL) {
    sprintf(interp->result, "%s: host %s not found", argv[0], argv[1]);
    WSACleanup();
    return TCL_ERROR;
  }
  
  memset( &server, 0, sizeof(server) );
  memcpy( &server.sin_addr, hp->h_addr, hp->h_length );
  server.sin_family = PF_INET;
  server.sin_port = (u_short) htons((u_short) port);
  
  stat = connect( sock, (const struct sockaddr FAR *)&server,
		  sizeof(server) );
  if (stat != 0) {
    sprintf(interp->result, "%s: error connecting to host %s", 
	    argv[0], argv[1]);
    WSACleanup();
    return TCL_ERROR;
  }

  /*------------------------------------------------------*/
  /* Read from stdin and write to socket, and             */
  /* read from socket and write to stdout.                */
  /*------------------------------------------------------*/


  /* Multi args */
  if (argc > cmdarg)
  {
    char *cmd;
    int i, len = 0;
    
    /* Loop through and figure out how long the command line is */
    for (i = cmdarg; i < argc; i++) {
      len += (strlen(argv[i])+1);
    }
    cmd = (char *) calloc(len+1, sizeof(char));
    for (i = cmdarg; i < argc; i++) {
      strcat(cmd, argv[i]);
      strcat(cmd, " ");
    }
    send_command(interp, cmd,  sock );
    free(cmd);
  }
  else {
    send_command(interp, argv[cmdarg],  sock );
  }

  /*------------------------------------------------------*/
  /* Clean up and go home.                                */
  /*------------------------------------------------------*/
  
  shutdown( sock, 2 );
  closesocket( sock );
  WSACleanup();

  return TCL_OK;
}

int send_command(Tcl_Interp *interp, char *command, SOCKET sock )
{
  int nb;                     /* number of bytes */
  char buffer[MAXLINE];       /* line read from stdin */
  int bufcnt;                 /* no. of chars in buffer */
  static char sockbuf[MAXLINE];      /* chars read from socket */
  int sockcnt;                /* no. of chars in sockbuf */

  strcpy(buffer, command);

  bufcnt = strlen( buffer );

  /*
   * add newline
   */
  if (buffer[bufcnt-1] != '\n') {
    buffer[bufcnt-1] = '\n';
    buffer[bufcnt] = '\0';
    bufcnt++;
  }
  
  /*----------------------------------------------*/
  /* Send the command                             */
  /*----------------------------------------------*/

  nb = send( sock, buffer, bufcnt, 0 );

  if (nb != bufcnt)
    {
      Tcl_SetResult(interp, "error sending on socket", TCL_STATIC);
      return TCL_ERROR;
    }
  
  /*----------------------------------------------*/
  /* Read the response from the stim server.      */
  /* Note that if we do not get enough characters */
  /* back, we will wait here indefinitely.        */
  /*----------------------------------------------*/
  
  sockcnt = 0;
  nb = recv( sock, &sockbuf[sockcnt], MAXLINE-sockcnt-1, 0 );
  if (nb == SOCKET_ERROR) {
    Tcl_SetResult(interp, "error reading from socket", TCL_STATIC);
    return TCL_ERROR;
  }
  else if (nb == 0) /* socket closed by server */
    return TCL_OK;
  
  /*------------------------------------------*/
  /* Put response in interp result            */
  /*------------------------------------------*/
  sockbuf[nb-2] = NUL;
  Tcl_SetResult(interp, sockbuf, TCL_STATIC);
  
  return TCL_OK;
}


/* The function pWSAerror is used to print a Windows Sockets*/
/* API error message.  This function is used instead of     */
/* perror because the WinSock API does not return its error */
/* codes via errno.                                         */
 
void pWSAerror(char *text)
{
  int err;                /* Winsock error code */
  
  err = WSAGetLastError();
  printf( "%s, error %d\n", text, err );
  
  /* As a future enhancement, this routine could         */
  /* implement a switch on the error code and print a    */
  /* specific text message for each type of error.       */
  
  return;
}



EXPORT(int,Stimctrl_Init) _ANSI_ARGS_((Tcl_Interp *interp))
{
#ifdef USE_TCL_STUBS
  Tcl_InitStubs(interp, "8.5", 0);
#else
  Tcl_PkgRequire(interp, "Tcl", "8.5", 0);
#endif
  
  if (Tcl_PkgProvide(interp, "stimctrl", "1.1") != TCL_OK) {
    return TCL_ERROR;
  }

  Tcl_CreateCommand(interp, "rmt_send", (Tcl_CmdProc *) rmtSendCmd,
		    (ClientData) 4610, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "ess_send", (Tcl_CmdProc *) rmtSendCmd,
		    (ClientData) 4620, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "socket_send", (Tcl_CmdProc *) rmtSendCmd,
		    (ClientData) 0, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "sock_open", (Tcl_CmdProc *) sockOpenCmd,
		    (ClientData) 0, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "sock_send", (Tcl_CmdProc *) sockSendCmd,
		    (ClientData) 0, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "sock_close", (Tcl_CmdProc *) sockCloseCmd,
		    (ClientData) 0, (Tcl_CmdDeleteProc *) NULL);

  Tcl_InitHashTable(&sockTable, TCL_STRING_KEYS);

  return TCL_OK;
}
