
#ifndef _XNET_H_INCLUDED
#define _XNET_H_INCLUDED


#define	SERVER_VERSION		0.90		// a Quake verziója

#define	MAX_MSGLEN		8000		// max length of a reliable message
#define	MAX_DATAGRAM		1024		// max length of unreliable message

#define	NET_NAMELEN		64

#define NET_MAXMESSAGE		8192
#define NET_HEADERSIZE		(2 * sizeof(unsigned int))
#define NET_DATAGRAMSIZE	(MAX_DATAGRAM + NET_HEADERSIZE)

// NetHeader flags
#define NETFLAG_LENGTH_MASK	0x0000ffff
#define NETFLAG_DATA		0x00010000
#define NETFLAG_ACK		0x00020000
#define NETFLAG_NAK		0x00040000
#define NETFLAG_EOM		0x00080000
#define NETFLAG_UNRELIABLE	0x00100000
#define NETFLAG_CTL		0x80000000


typedef enum {
	src_client,		// came in over a net connection as a clc_stringcmd
				// host_client will be valid during this state.
	src_command		// from the command buffer
} cmd_source_e;

XLIBDEF cmd_source_e cmd_source;


#define NET_PROTOCOL_VERSION	1


// This is the network info/connection protocol.  It is used to find Quake
// servers, get info about them, and connect to them.  Once connected, the
// Quake game protocol (documented elsewhere) is used.
//
//
// General notes:
//	game_name is currently always "QUAKE", but is there so this same protocol
//	can be used for future games as well; can you say Quake2?
//
// CCREQ_CONNECT
//		string	game_name		"QUAKE"
//		byte	net_protocol_version	NET_PROTOCOL_VERSION
//
// CCREQ_SERVER_INFO
//		string	game_name		"QUAKE"
//		byte	net_protocol_version	NET_PROTOCOL_VERSION
//
// CCREQ_PLAYER_INFO
//		byte	player_number
//
// CCREQ_RULE_INFO
//		string	rule
//
//
//
// CCREP_ACCEPT
//		long	port
//
// CCREP_REJECT
//		string	reason
//
// CCREP_SERVER_INFO
//		string	server_address
//		string	host_name
//		string	level_name
//		byte	current_players
//		byte	max_players
//		byte	protocol_version	NET_PROTOCOL_VERSION
//
// CCREP_PLAYER_INFO
//		byte	player_number
//		string	name
//		long	colors
//		long	frags
//		long	connect_time
//		string	address
//
// CCREP_RULE_INFO
//		string	rule
//		string	value

//	note:
//		There are two address forms used above.  The short form is just a
//		port number.  The address that goes along with the port is defined as
//		"whatever address you receive this reponse from".  This lets us use
//		the host OS to solve the problem of multiple host addresses (possibly
//		with no routing between them); the host will use the right address
//		when we reply to the inbound connection request.  The long from is
//		a full address and port in a string.  It is used for returning the
//		address of a server that is not running locally.

#define CCREQ_CONNECT		0x01
#define CCREQ_SERVER_INFO	0x02
#define CCREQ_PLAYER_INFO	0x03
#define CCREQ_RULE_INFO		0x04

#define CCREP_ACCEPT		0x81
#define CCREP_REJECT		0x82
#define CCREP_SERVER_INFO	0x83
#define CCREP_PLAYER_INFO	0x84
#define CCREP_RULE_INFO		0x85

#define U_SIGNAL		(1<<7)		// just differentiates from other updates

/*
#pragma pack(push, 1)
struct sockaddr {
  USHORT    sa_family;
  char       sa_data[14];
};
#pragma pack(pop)
*/


//
//
//
typedef struct xsocket_s {

	struct xsocket_s *next;
	double	connecttime;
	double	lastMessageTime;
	double	lastSendTime;

	BOOL	disconnected;
	BOOL	canSend;
	BOOL	sendNext;

	int	driver;
	int	landriver;
	int	socket;
	void	*driverdata;

	unsigned int	ackSequence;
	unsigned int	sendSequence;
	unsigned int	unreliableSendSequence;
	int		sendMessageLength;
	UCHAR		sendMessage[ NET_MAXMESSAGE ];

	unsigned int	receiveSequence;
	unsigned int	unreliableReceiveSequence;
	int		receiveMessageLength;
	UCHAR		receiveMessage[ NET_MAXMESSAGE ];

	struct sockaddr	addr;
	char	address[ NET_NAMELEN ];

} xsocket_t, *xsocket_ptr;



//
//
//
typedef struct sizebuf_s {
	BOOL	allowoverflow;		// if false, do a Sys_Error
	BOOL	overflowed;		// set to true if the buffer size failed
	UCHAR	*data;
	int	maxsize;
	int	cursize;
} sizebuf_t;


XLIBDEF double net_time;

XLIBDEF sizebuf_t net_message;
XLIBDEF int net_activeconnections;

XLIBDEF int messagesSent;
XLIBDEF int messagesReceived;
XLIBDEF int unreliableMessagesSent;
XLIBDEF int unreliableMessagesReceived;

XLIBDEF xsocket_t *net_activeSockets;
XLIBDEF xsocket_t *net_freeSockets;
XLIBDEF int net_numsockets;

XLIBDEF int net_hostport;
XLIBDEF char my_tcpip_address[ NET_NAMELEN ];
XLIBDEF BOOL tcpipAvailable;


#define HOSTCACHESIZE	16


//
//
//
typedef struct {
	char	name[256];
	char	map[256];
	char	cname[256];
	int	users;
	int	maxusers;
	int	driver;
	int	ldriver;
	struct  sockaddr addr;
} hostcache_t;

XLIBDEF int hostCacheCount;
XLIBDEF hostcache_t hostcache[HOSTCACHESIZE];

#define MAXHOSTNAMELEN 256
XLIBDEF char hostname[ MAXHOSTNAMELEN ];


//
//
//
typedef struct _PollProcedure {
	struct _PollProcedure	*next;
	double nextTime;
	void (*procedure)( void );
	void *arg;
} PollProcedure;

XLIBDEF void SchedulePollProcedure( PollProcedure *pp, double timeOffset );


#define	NUM_PING_TIMES 16


//
//
//
typedef struct client_s {
	BOOL		active;			// false = client is free
	BOOL		spawned;		// false = don't send datagrams
	BOOL		dropasap;		// has been told to go to another level
	BOOL		privileged;		// can execute any host command
	BOOL		sendsignon;		// only valid before spawned

	double		last_message;		// reliable messages must be sent
						// periodically
	struct xsocket_s *netconnection;	// communications handle

	// usercmd_t	cmd;			// movement
	// vec3_t	wishdir;		// intended motion calced from cmd

	sizebuf_t	message;		// can be added to at any time,
						// copied and clear once per frame
	char		msgbuf[MAX_MSGLEN];
	// edict_t	*edict;			// EDICT_NUM(clientnum+1)
	char		name[256];		// for printing to other people
	int		colors;

	float		ping_times[NUM_PING_TIMES];
	int		num_pings;		// ping_times[num_pings%NUM_PING_TIMES]

	// spawn parms are carried from level to level
	// float 	spawn_parms[NUM_SPAWN_PARMS];

	// client known data for deltas
	// int		old_frags;
} client_t, *client_ptr;



//
//
//
typedef struct server_static_s {
	int	maxclients;
	int	maxclientslimit;
	struct client_s	*clients;	// [maxclients]
	int	serverflags;		// episode completion information
	BOOL	changelevel_issued;	// cleared when at SV_SpawnServer
} server_static_t;


typedef enum {
	ss_loading,
	ss_active
} server_state_e;



//
//
//
typedef struct server_s {
	BOOL		active;			// false if only a net client
	BOOL		paused;
	BOOL		loadgame;		// handle connections specially

	double		time;

	int		lastcheck;		// used by PF_checkclient
	double		lastchecktime;

	char		name[256];		// map name

	/***
	char		modelname[64];		// maps/<name>.bsp, for model_precache[0]
	struct model_s 	*worldmodel;
	char		*model_precache[MAX_MODELS];	// NULL terminated
	struct model_s	*models[MAX_MODELS];
	char		*sound_precache[MAX_SOUNDS];	// NULL terminated
	char		*lightstyles[MAX_LIGHTSTYLES];
	int		num_edicts;
	int		max_edicts;
	edict_t		*edicts;		// can NOT be array indexed, because
						// edict_t is variable sized, but can
						// be used to reference the world ent
	***/

	server_state_e	state;			// some actions are only valid during load

	sizebuf_t	datagram;
	UCHAR		datagram_buf[MAX_DATAGRAM];

	sizebuf_t	reliable_datagram;	// copied to all clients at end of frame
	UCHAR		reliable_datagram_buf[MAX_DATAGRAM];

	sizebuf_t	signon;
	UCHAR		signon_buf[8192];
} server_t, *server_ptr;


XLIBDEF server_static_t svs;
XLIBDEF server_t	sv;				// local server
XLIBDEF double		host_time;



// #define MAX_CLIENTS 8
// extern client_t clients[MAX_CLIENTS];

XLIBDEF char cl_name[256];

XLIBDEF client_t *host_client;


#define	SIGNONS	 4		// signon messages to receive before connected

typedef enum {
	ca_dedicated, 		// a dedicated server with no ability to start a client
	ca_disconnected, 	// full screen console with no connection
	ca_connected		// valid netcon, talking to a server
} cactive_e;



//
// the client_static_t structure is persistant through an arbitrary number
// of server connections
//
typedef struct client_static_s {

	cactive_e	state;

	// personalization data sent to server
	char		mapstring[256];
	// char		spawnparms[MAX_MAPSTRING];	// to restart a level

	// demo loop control
	// int		demonum;			// -1 = don't play demos
	// char		demos[MAX_DEMOS][MAX_DEMONAME];	// when not playing

	// demo recording info must be here, because record is started before
	// entering a map (and clearing client_state_t)

	/***
	BOOL		demorecording;
	BOOL		demoplayback;
	BOOL		timedemo;
	int		forcetrack;		// -1 = use normal cd track
	FILE		*demofile;
	int		td_lastframe;		// to meter out one message a frame
	int		td_startframe;		// host_framecount at start
	float		td_starttime;		// realtime at second frame of timedemo
	***/

	// connection information
	int		signon;			// 0 to SIGNONS
	struct xsocket_s *netcon;
	sizebuf_t	message;		// writing buffer to send to server

} client_static_t, *client_static_ptr;



//
// the client_state_t structure is wiped completely at every
// server signon
//
typedef struct {
	int movemessages;	// since connecting to this server
				// throw out the first couple, so the player
				// doesn't accidentally do something the
				// first frame
	// usercmd_t	cmd;	// last command sent to the server

	// information for local display
	// int	stats[MAX_CL_STATS];	// health, etc
	// int	items;			// inventory bit flags
	// float item_gettime[32];	// cl.time of aquiring item, for blinking
	// float faceanimtime;		// use anim frame if cl.time < this

	// cshift_t cshifts[NUM_CSHIFTS];	// color shifts for damage, powerups
	// cshift_t prev_cshifts[NUM_CSHIFTS];	// and content types

	// the client maintains its own idea of view angles, which are
	// sent to the server each frame.  The server sets punchangle when
	// the view is temporarliy offset, and an angle reset commands at the start
	// of each level and after teleporting.

	// vec3_t	mviewangles[2];	// during demo playback viewangles is lerped
					// between these
	// vec3_t	viewangles;
	// vec3_t	mvelocity[2];	// update by server, used for lean+bob
					// (0 is newest)
	// vec3_t	velocity;	// lerped between mvelocity[0] and [1]
	// vec3_t	punchangle;	// temporary offset

	// pitch drifting vars

	float	idealpitch;
	float	pitchvel;
	BOOL	nodrift;
	float	driftmove;
	double	laststop;

	float	viewheight;
	float	crouch;		// local amount for smoothing stepups

	BOOL	paused;		// send over by server
	BOOL	onground;
	BOOL	inwater;

	int	intermission;	// don't change view angle, full screen, etc
	int	completed_time;	// latched at intermission start

	double	mtime[2];	// the timestamp of last two messages
	double	time;		// clients view of time, should be between
				// servertime and oldservertime to generate
				// a lerp point for other data
	double	oldtime;	// previous cl.time, time-oldtime is used
				// to decay light values and smooth step ups

	float	last_received_message;	// (realtime) for net trouble icon

	//
	// information that is static for the entire time connected to a server
	//

	// struct model_s *model_precache[MAX_MODELS];
	// struct sfx_s	*sound_precache[MAX_SOUNDS];

	char	levelname[256];	// for display on solo scoreboard
	int	viewentity;	// cl_entitites[cl.viewentity] = player
	int	maxclients;
	int	gametype;

	// refresh related state
	// struct model_s *worldmodel;	// cl_entitites[0].model
	// struct efrag_s *free_efrags;
	int	num_entities;		// held in cl_entities array
	int	num_statics;		// held in cl_staticentities array
	// entity_t viewent;		// the gun model

	int	cdtrack, looptrack;	// cd audio

	// frag scoreboard
	// scoreboard_t	*scores;	// [cl.maxclients]

} client_state_t, *client_state_ptr;


XLIBDEF client_static_t	cls;
XLIBDEF client_state_t	cl;

XLIBDEF double host_frametime;
XLIBDEF double realtime;		// not bounded in any way, changed at
					// start of every frame, never reset

#define NET_MESSAGETIMEOUT (10.0)


//
//
//
typedef struct net_landriver_s {

	char	*name;
	BOOL	initialized;
	int	controlSock;
	int	(*Init)( void );
	void	(*Shutdown)( void );
	void	(*Listen)( BOOL state );
	int 	(*OpenSocket)( int port );
	int 	(*CloseSocket)( int socket );
	int 	(*Connect)( int socket, struct sockaddr *addr );
	int 	(*CheckNewConnections)( void );
	int 	(*Read)( int socket, UCHAR *buf, int len, struct sockaddr *addr );
	int 	(*Write)( int socket, UCHAR *buf, int len, struct sockaddr *addr );
	int 	(*Broadcast)( int socket, UCHAR *buf, int len );
	char	*(*AddrToString)( struct sockaddr *addr );
	int 	(*StringToAddr)( char *string, struct sockaddr *addr );
	int 	(*GetSocketAddr)( int socket, struct sockaddr *addr );
	int 	(*GetNameFromAddr)( struct sockaddr *addr, char *name );
	int 	(*GetAddrFromName)( char *name, struct sockaddr *addr );
	int	(*AddrCompare)( struct sockaddr *addr1, struct sockaddr *addr2 );
	int	(*GetSocketPort)( struct sockaddr *addr );
	int	(*SetSocketPort)( struct sockaddr *addr, int port );

} net_landriver_t, *net_landriver_ptr;

#define	MAX_NET_DRIVERS		8
XLIBDEF int net_numlandrivers;
XLIBDEF net_landriver_t net_landrivers[MAX_NET_DRIVERS];


//
//
//
typedef struct net_driver_s {

	char	*name;
	BOOL	initialized;
	int	(*Init)( void );
	void	(*Listen)( BOOL state );
	void	(*SearchForHosts)( BOOL xmit );
	xsocket_t *(*Connect)( char *host );
	xsocket_t *(*CheckNewConnections)( void );
	int	(*QGetMessage)( xsocket_t *sock );
	int	(*QSendMessage)( xsocket_t *sock, sizebuf_t *data );
	int	(*SendUnreliableMessage)( xsocket_t *sock, sizebuf_t *data );
	BOOL	(*CanSendMessage)( xsocket_t *sock );
	BOOL	(*CanSendUnreliableMessage)( xsocket_t *sock );
	void	(*Close)( xsocket_t *sock );
	void	(*Shutdown)( void );
	int	controlSock;

} net_driver_t, *net_driver_ptr;


#define	MAX_NET_DRIVERS 8

XLIBDEF net_driver_t net_drivers[MAX_NET_DRIVERS];
XLIBDEF int net_numdrivers;

XLIBDEF int net_driverlevel;

XLIBDEF int msg_readcount;
XLIBDEF BOOL msg_badread;


#define	PROTOCOL_VERSION	15

//
// server to client
//
#define	svc_bad		0
#define	svc_nop		1
#define	svc_disconnect	2
#define	svc_updatestat	3	// [byte] [long]
#define	svc_version	4	// [long] server version
#define	svc_setview	5	// [short] entity number
#define	svc_sound	6	// <see code>
#define	svc_time	7	// [float] server time
#define	svc_print	8	// [string] null terminated string
#define	svc_stufftext	9	// [string] stuffed into client's console buffer
				// the string should be \n terminated
#define	svc_setangle	10	// [angle3] set the view angle to this absolute value

#define	svc_serverinfo	11	// [long] version
				// [string] signon string
				// [string]..[0]model cache
				// [string]...[0]sounds cache
#define	svc_lightstyle	12	// [byte] [string]
#define	svc_updatename	13	// [byte] [string]
#define	svc_updatefrags	14	// [byte] [short]
#define	svc_clientdata	15	// <shortbits + data>
#define	svc_stopsound	16	// <see code>
#define	svc_updatecolors 17	// [byte] [byte]
#define	svc_particle	18	// [vec3] <variable>
#define	svc_damage	19

#define	svc_spawnstatic	20
// #define svc_spawnbinary	21
#define	svc_spawnbaseline 22

#define	svc_temp_entity	23

#define	svc_setpause	24	// [byte] on / off
#define	svc_signonnum	25	// [byte]  used for the signon sequence

#define	svc_centerprint	26	// [string] to put in center of the screen

#define	svc_killedmonster	27
#define	svc_foundsecret		28

#define	svc_spawnstaticsound 29	// [coord3] [byte] samp [byte] vol [byte] aten

#define	svc_intermission 30	// [string] music
#define	svc_finale	31	// [string] music [string] text

#define	svc_cdtrack	32	// [byte] track [byte] looptrack
#define svc_sellscreen	33

#define svc_cutscene	34

//
// client to server
//
#define	clc_bad		0
#define	clc_nop 	1
#define	clc_disconnect	2
#define	clc_move	3	// [usercmd_t]
#define	clc_stringcmd	4	// [string] message




XLIBDEF int UDP_Init( void );
XLIBDEF void UDP_Shutdown( void );
XLIBDEF void UDP_Listen( BOOL state );
XLIBDEF int UDP_OpenSocket( int port );
XLIBDEF int UDP_CloseSocket( int socket );
XLIBDEF int UDP_Connect( int socket, struct sockaddr *addr );
XLIBDEF int UDP_CheckNewConnections( void );
XLIBDEF int UDP_Read( int socket, UCHAR *buf, int len, struct sockaddr *addr );
XLIBDEF int UDP_Write( int socket, UCHAR *buf, int len, struct sockaddr *addr );
XLIBDEF int UDP_Broadcast( int socket, UCHAR *buf, int len );
XLIBDEF char *UDP_AddrToString( struct sockaddr *addr );
XLIBDEF int UDP_StringToAddr( char *string, struct sockaddr *addr );
XLIBDEF int UDP_GetSocketAddr( int socket, struct sockaddr *addr );
XLIBDEF int UDP_GetNameFromAddr( struct sockaddr *addr, char *name );
XLIBDEF int UDP_GetAddrFromName( char *name, struct sockaddr *addr );
XLIBDEF int UDP_AddrCompare( struct sockaddr *addr1, struct sockaddr *addr2 );
XLIBDEF int UDP_GetSocketPort( struct sockaddr *addr );
XLIBDEF int UDP_SetSocketPort( struct sockaddr *addr, int port );


XLIBDEF int Datagram_Init( void );
XLIBDEF void Datagram_Listen( BOOL state );
XLIBDEF void Datagram_SearchForHosts( BOOL xmit );
XLIBDEF xsocket_t *Datagram_Connect( char *host );
XLIBDEF xsocket_t *Datagram_CheckNewConnections( void );
XLIBDEF int Datagram_GetMessage( xsocket_t *sock );
XLIBDEF int Datagram_SendMessage( xsocket_t *sock, sizebuf_t *data );
XLIBDEF int Datagram_SendUnreliableMessage( xsocket_t *sock, sizebuf_t *data );
XLIBDEF BOOL Datagram_CanSendMessage( xsocket_t *sock );
XLIBDEF BOOL Datagram_CanSendUnreliableMessage( xsocket_t *sock );
XLIBDEF void Datagram_Close( xsocket_t *sock);
XLIBDEF void Datagram_Shutdown (void);

XLIBDEF int Loop_Init( void );
XLIBDEF void Loop_Listen( BOOL state );
XLIBDEF void Loop_SearchForHosts( BOOL xmit );
XLIBDEF xsocket_t *Loop_Connect( char *host );
XLIBDEF xsocket_t *Loop_CheckNewConnections( void );
XLIBDEF int Loop_GetMessage( xsocket_t *sock );
XLIBDEF int Loop_SendMessage( xsocket_t *sock, sizebuf_t *data );
XLIBDEF int Loop_SendUnreliableMessage( xsocket_t *sock, sizebuf_t *data );
XLIBDEF BOOL Loop_CanSendMessage( xsocket_t *sock );
XLIBDEF BOOL Loop_CanSendUnreliableMessage( xsocket_t *sock );
XLIBDEF void Loop_Close( xsocket_t *sock );
XLIBDEF void Loop_Shutdown( void );



XLIBDEF void SZ_Alloc( sizebuf_t *buf, int startsize );
XLIBDEF void SZ_Free( sizebuf_t *buf );
XLIBDEF void SZ_Clear( sizebuf_t *buf );
XLIBDEF void *SZ_GetSpace( sizebuf_t *buf, int length );
XLIBDEF void SZ_Write( sizebuf_t *buf, void *data, int length );
XLIBDEF void SZ_Print( sizebuf_t *buf, char *data );


XLIBDEF void MSG_WriteChar( sizebuf_t *sb, int c );
XLIBDEF void MSG_WriteByte( sizebuf_t *sb, int c );
XLIBDEF void MSG_WriteShort( sizebuf_t *sb, int c );
XLIBDEF void MSG_WriteLong( sizebuf_t *sb, int c );
XLIBDEF void MSG_WriteFloat( sizebuf_t *sb, float f );
XLIBDEF void MSG_WriteString( sizebuf_t *sb, char *s );
XLIBDEF void MSG_WriteCoord( sizebuf_t *sb, float f );
XLIBDEF void MSG_WriteAngle( sizebuf_t *sb, float f );
XLIBDEF void MSG_BeginReading( void );
XLIBDEF int MSG_ReadChar( void );
XLIBDEF int MSG_ReadByte( void );
XLIBDEF int MSG_ReadShort( void );
XLIBDEF int MSG_ReadLong( void );
XLIBDEF float MSG_ReadFloat( void );
XLIBDEF char *MSG_ReadString( void );
XLIBDEF float MSG_ReadCoord( void );
XLIBDEF float MSG_ReadAngle( void );


XLIBDEF double SetNetTime( void );

XLIBDEF BOOL NET_CanSendMessage( xsocket_t *sock );
XLIBDEF int NET_SendToAll( sizebuf_t *data, int blocktime );
XLIBDEF BOOL NET_Init( void );
XLIBDEF void NET_Shutdown( void );
XLIBDEF void NET_Poll( void );
XLIBDEF int NET_SendUnreliableMessage( xsocket_t *sock, sizebuf_t *data );
XLIBDEF int NET_SendMessage( xsocket_t *sock, sizebuf_t *data );
XLIBDEF int NET_GetMessage( xsocket_t *sock );
XLIBDEF void NET_Close( xsocket_t *sock );
XLIBDEF xsocket_t *NET_CheckNewConnections( void );
XLIBDEF xsocket_t *NET_Connect( char *host );
XLIBDEF void NET_FreeQSocket( xsocket_t *sock );
XLIBDEF xsocket_t *NET_NewQSocket( void );
XLIBDEF void NET_Slist( void );
XLIBDEF void NET_Listen( BOOL state );


XLIBDEF void SV_Init( void );
XLIBDEF void SV_SendServerinfo( client_t *client );
XLIBDEF void SV_ConnectClient( int clientnum );
XLIBDEF void SV_CheckForNewClients( void );
XLIBDEF void SV_ClearDatagram( void );
XLIBDEF void SV_WriteEntitiesToClient( client_t *client, sizebuf_t *msg );
XLIBDEF void SV_WriteClientdataToMessage( client_t *client, sizebuf_t *msg );
XLIBDEF BOOL SV_SendClientDatagram( client_t *client );
XLIBDEF void SV_UpdateToReliableMessages( void );
XLIBDEF void SV_SendNop( client_t *client );
XLIBDEF void SV_SendClientMessages( void );
XLIBDEF void SV_SendReconnect( void );
XLIBDEF void SV_SpawnServer( char *server );


XLIBDEF void SV_ClientThink( void );
XLIBDEF void SV_ReadClientMove( void );
XLIBDEF BOOL SV_ReadClientMessage( void );
XLIBDEF void SV_RunClients( void );
XLIBDEF void SV_Physics( void );


XLIBDEF void CL_ClearState( void );
XLIBDEF void CL_Disconnect( void );
XLIBDEF void CL_EstablishConnection( char *host );
XLIBDEF void CL_SignonReply( void );
XLIBDEF int CL_ReadFromServer( void );
XLIBDEF void CL_SendCmd( void );
XLIBDEF void CL_Init( void );


XLIBDEF int CL_GetMessage( void );
XLIBDEF void CL_SendMove( void );

// cl_parse.cpp

XLIBDEF void CL_KeepaliveMessage( void );
XLIBDEF void CL_ParseServerInfo( void );
XLIBDEF void CL_ParseUpdate( int bits );
XLIBDEF void CL_ParseClientdata( int bits );
XLIBDEF void CL_ParseServerMessage( void );



// host.cpp

XLIBDEF void Host_EndGame( char *message, ... );
XLIBDEF void Host_Error( char *error, ... );
XLIBDEF void Host_InitLocal( void );
XLIBDEF void SV_ClientPrintf( char *fmt, ... );
XLIBDEF void SV_BroadcastPrintf( char *fmt, ... );
XLIBDEF void Host_ClientCommands( char *fmt, ... );
XLIBDEF void SV_DropClient( BOOL crash );
XLIBDEF void Host_ShutdownServer( BOOL crash );
XLIBDEF void Host_ClearMemory( void );
XLIBDEF BOOL Host_FilterTime( float time );
XLIBDEF void Host_ServerFrame( void );
XLIBDEF void Host_Frame( float time );
XLIBDEF void Host_Init( void );
XLIBDEF void Host_Shutdown( void );

XLIBDEF void Host_Ping( void );
XLIBDEF void Host_Disconnect( void );
XLIBDEF void Host_Reconnect( void );
XLIBDEF void Host_Connect( char *name );
XLIBDEF void Host_PreSpawn( void );
XLIBDEF void Host_Spawn( void );
XLIBDEF void Host_Begin( void );
XLIBDEF void Host_Name( char *newName );
XLIBDEF void Host_Test( char *host );



// cmd.cpp

XLIBDEF void Cmd_Init( void );
XLIBDEF void Cmd_ExecuteString( char *text, cmd_source_e src );
XLIBDEF void Cbuf_AddText( char *str );
XLIBDEF void Cbuf_Execute( void );
XLIBDEF void Cmd_ForwardToServer( char *cmd );



XLIBDEF double Sys_FloatTime( void );
XLIBDEF void Sys_Init( void );


#endif






