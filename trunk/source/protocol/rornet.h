/*
This file is part of "Rigs of Rods Server" (Relay mode)
Copyright 2007 Pierre-Michel Ricordel
Contact: pricorde@rigsofrods.com
"Rigs of Rods Server" is distributed under the terms of the GNU General Public License.

"Rigs of Rods Server" is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

"Rigs of Rods Server" is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef RORNETPROTOCOL_H__
#define RORNETPROTOCOL_H__

#define BITMASK(x) (1 << (x-1))

// protocol settings
static const int   MAX_PEERS = 64;             //!< maximum clients connected at the same time
static const int   MAX_MESSAGE_LENGTH = 32768; //!< maximum size of a RoR message. 32768Bytes = 32kB

// protocol version
static const char *RORNET_VERSION = "RoRnet_2.33"; //!< the protocol version information

// REGISTRY STUFF
static const char *REPO_SERVER = "api.rigsofrods.com"; //!< the web API URL
static const char *REPO_URLPREFIX = "";                //!< prefix for the API

// used by configurator
static const char *REPO_HTML_SERVERLIST = "http://api.rigsofrods.com/serverlist/"; //!< server list URL
static const char *NEWS_HTML_PAGE = "http://api.rigsofrods.com/news/"; //!< news html page URL

// ENUMs

/*
 * commands
 */
enum {
	MSG2_HELLO  = 1000,                //!< client sends its version as first message
	// hello responses
	MSG2_FULL,                         //!< no more slots for us
	MSG2_WRONG_PW,                     //!< server send that on wrong pw
	MSG2_WRONG_VER,                    //!< wrong version
	MSG2_BANNED,                       //!< client not allowed to join
	MSG2_WELCOME,                      //!< we can proceed

	MSG2_VERSION,                      //!< server responds with its version

	MSG2_CHAT,                         //!< chat line

	MSG2_SERVER_SETTINGS,              //!< server send client the terrain name: server_info_t

	MSG2_GAME_CMD,                     //!< send to client from server only

	MSG2_USER_INFO,                    //!< user data that is sent from the server to the clients
	MSG2_PRIVCHAT,                     //!< sent from client to server to send private chat messages

	// stream functions
	MSG2_STREAM_REGISTER,              //!< create new stream
	MSG2_STREAM_REGISTER_RESULT,       //!< result of a stream creation
	//MSG2_STREAM_REGISTER_RESP,         //!< reply from server to registering client
	//MSG2_STREAM_CONTROL_FLOW,          //!< suspend/unsuspend streams
	//MSG2_STREAM_CONTROL_FLOW_RESP,     //!< reply from server to requesting client
	//MSG2_STREAM_UNREGISTER,            //!< remove stream
	//MSG2_STREAM_UNREGISTER_RESP,       //!< remove stream response from server to requsting client
	//MSG2_STREAM_TAKEOVER,              //!< stream takeover
	//MSG2_STREAM_TAKEOVER_RESP,         //!< stream takeover response from server
	MSG2_STREAM_DATA,                  //!< stream data
	MSG2_USER_JOIN,                    //!< new user joined
	MSG2_USER_LEAVE,                   //!< user leaves

	// master server interaction
	MSG2_MASTERINFO,                   //!< master information response
};

/*
 * user authentication flags on the server
 */
enum {
	AUTH_NONE   = 0,                   //!< no authentication
	AUTH_ADMIN  = BITMASK(1),          //!< admin on the server
	AUTH_RANKED = BITMASK(2),          //!< ranked status
	AUTH_MOD    = BITMASK(3),          //!< moderator status
	AUTH_BOT    = BITMASK(4),          //!< bot status
	AUTH_BANNED = BITMASK(5),          //!< banned
};


/*
 * used to transport truck states across
 */
enum {
	NETMASK_HORN        = BITMASK(1),  //!< horn is in use
	NETMASK_LIGHTS      = BITMASK(2),  //!< lights on
	NETMASK_BRAKES      = BITMASK(3),  //!< brake lights on
	NETMASK_REVERSE     = BITMASK(4),  //!< reverse light on
	NETMASK_BEACONS     = BITMASK(5),  //!< beacons on
	NETMASK_BLINK_LEFT  = BITMASK(6),  //!< left blinker on
	NETMASK_BLINK_RIGHT = BITMASK(7),  //!< right blinker on
	NETMASK_BLINK_WARN  = BITMASK(8),  //!< warn blinker on
	NETMASK_CLIGHT1     = BITMASK(9),  //!< custom light 1 on
	NETMASK_CLIGHT2     = BITMASK(10), //!< custom light 2 on
	NETMASK_CLIGHT3     = BITMASK(11), //!< custom light 3 on
	NETMASK_CLIGHT4     = BITMASK(12), //!< custom light 4 on
	NETMASK_POLICEAUDIO = BITMASK(13), //!< police siren on
	NETMASK_PARTICLE    = BITMASK(14), //!< custom particles on
};

// structs

/*
 * this structure defines the header every RoR paket uses
 */
typedef struct
{
	unsigned int command;     //!< the command of this packet: MSG2_*
	int source;               //!< source of this command: 0 = server
	unsigned int streamid;    //!< streamid for this command
	unsigned int size;        //!< size of the attached data block
} header_t;

/*
 * structure that is send from the cleint to server and vice versa, to broadcast a new stream
 */
typedef struct
{
	char name[128];           //!< the truck filename
	int type;                 //!< stream type
	int status;               //!< initial stream status
	int origin_sourceid;      //!< origin sourceid
	int origin_streamid;      //!< origin streamid
	char data[8000];          //!< data used for stream setup
} stream_register_t;

/*
 * specialization of stream_register_t for truck registrations
 */
typedef struct
{
	char name[128];            //!< the truck filename
	int type;                  //!< stream type
	int status;                //!< initial stream status
	int origin_sourceid;       //!< origin sourceid
	int origin_streamid;       //!< origin streamid
	int bufferSize;            //!< initial stream status
	char truckconfig[10][60];  //!< truck section configuration
} stream_register_trucks_t;

/*
 * structure sent to remove a stream
 */
typedef struct
{
	int sid;                   //!< the unique id of the stream
} stream_unregister_t;

/*
 * general user information structure
 */
typedef struct
{
	unsigned int uniqueid;     //!< user unique id
	char username[20];         //!< the nickname of the user
	char usertoken[40];        //!< user token
	char serverpassword[40];   //!< server password
	char language[10];         //!< user's language. For example "de-DE" or "en-US"
	char clientname[10];       //!< the name and version of the client. For exmaple: "ror" or "gamebot"
	char clientversion[25];    //!< a version number of the client. For example 1 for RoR 0.35
	char clientGUID[40];       //!< the clients GUID
	char sessiontype[10];      //!< the requested session type. For example "normal", "bot", "rcon"
	char sessionoptions[128];  //!< reserved for future options

	int authstatus;            //!< auth status set by server: AUTH_*
	int slotnum;               //!< slot number set by server
	int colournum;             //!< colour set by server
} user_info_t;


/*
 * net force structure
 */
typedef struct
{
	unsigned int target_uid;   //!< target UID
	unsigned int node_id;      //!< node of target
	float fx;                  //!< force x
	float fy;                  //!< force y
	float fz;                  //!< force z
} netforce_t;

/*
 * truck property structure
 */
typedef struct
{
	int time;                  //!< time data
	float engine_speed;        //!< engine RPM
	float engine_force;        //!< engine acceleration
	unsigned int flagmask;     //!< flagmask: NETMASK_*
} oob_t;


/*
 * server settings struct
 */
typedef struct
{
	char protocolversion[20];  //!< protocol version being used
	char terrain[128];         //!< terrain name
	char servername[128];      //!< name of the server
	bool password;             //!< passworded server?
	char info[9046];           //!< info text
} server_info_t;


#endif //RORNETPROTOCOL_H__
