/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			 Low-level communication module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>


#include <dirent.h>
#include "mud.h"
#ifdef DNS_SLAVE /* DNS slave stuff, more sections like this follow - Samson 12-21-00 */
#include "dns_slave.h"
#endif


/*
 * Socket and TCP/IP stuff.
 */
#ifdef WIN32
#include <io.h>
#undef EINTR
#undef EMFILE
#define EINTR WSAEINTR
#define EMFILE WSAEMFILE
#define EWOULDBLOCK WSAEWOULDBLOCK
#define MAXHOSTNAMELEN 32

#define  TELOPT_ECHO        '\x01'
#define  GA                 '\xF9'
#define  SE                 '\xF0'
#define  SB                 '\xFA'
#define  WILL               '\xFB'
#define  WONT               '\xFC'
#define  DO                 '\xFD'
#define  DONT               '\xFE'
#define  IAC                '\xFF'
void bailout(void);
void shutdown_checkpoint(void);
#else
#include <sys/socket.h>
#include <netinet/in.h>
  /*#include <netinet/in_systm.h> */
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>
#define closesocket close
#endif

#define  TELOPT_MXP        '\x5B'

#ifdef sun
int gethostname(char *name, int namelen);
#endif

const	unsigned char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	unsigned char	echo_on_str	  [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	unsigned char go_ahead_str	[] = { IAC, GA, '\0' };
const unsigned char will_mxp_str  [] = { IAC, WILL, TELOPT_MXP, '\0' };
const unsigned char start_mxp_str [] = { IAC, SB, TELOPT_MXP, IAC, SE, '\0' };
const unsigned char do_mxp_str    [] = { IAC, DO, TELOPT_MXP, '\0' };
const unsigned char dont_mxp_str  [] = { IAC, DONT, TELOPT_MXP, '\0' };

void save_sysdata args((SYSTEM_DATA sys));

#ifdef MCCP
#define TELOPT_COMPRESS 85
#define TELOPT_COMPRESS2 86
const   char    eor_on_str      [] = { IAC, WILL, TELOPT_EOR, '\0' };
const   char    compress_on_str [] = { IAC, WILL, TELOPT_COMPRESS, '\0' };
const   char    compress2_on_str [] = { IAC, WILL, TELOPT_COMPRESS2, '\0' };

bool    compressStart   args( ( DESCRIPTOR_DATA *d, unsigned char telopt ) );
bool    compressEnd     args( ( DESCRIPTOR_DATA *d ) );
#endif

void auth_maxdesc args((int *md, fd_set * ins, fd_set * outs, fd_set * excs));
void auth_check args((fd_set * ins, fd_set * outs, fd_set * excs));
void set_auth args((DESCRIPTOR_DATA * d));
void kill_auth args((DESCRIPTOR_DATA * d));


/*
 * Global variables.
 */
IMMORTAL_HOST *immortal_host_start; /* Start of Immortal legal domains */
IMMORTAL_HOST *immortal_host_end; /* End of Immortal legal domains */
DESCRIPTOR_DATA *first_descriptor; /* First descriptor  */
DESCRIPTOR_DATA *last_descriptor; /* Last descriptor  */
DESCRIPTOR_DATA *d_next; /* Next descriptor in loop */
int num_descriptors;
FILE *fpReserve; /* Reserved file handle  */
bool mud_down; /* Shutdown   */
bool service_shut_down; /* Shutdown by operator closing down service */
bool wizlock; /* Game is wizlocked  */
time_t boot_time;
HOUR_MIN_SEC set_boot_time_struct;
HOUR_MIN_SEC *set_boot_time;
struct tm *new_boot_time;
struct tm new_boot_struct;
char str_boot_time[MIL];
char lastplayercmd[MIL * 2];
time_t current_time; /* Time of this pulse  */
int control; /* Controlling descriptor */
int control2; /* Controlling descriptor #2 */
int conclient; /* MUDClient controlling desc */
int conjava; /* JavaMUD controlling desc */
int newdesc; /* New descriptor  */
fd_set in_set; /* Set of desc's for reading */
fd_set out_set; /* Set of desc's for writing */
fd_set exc_set; /* Set of desc's with errors */
int maxdesc;
char *alarm_section = "(unknown)";

#ifdef DNS_SLAVE
pid_t slave_pid;
pid_t pastslave_pid;
static int slave_socket = -1;
#endif

/*
 * OS-dependent local functions.
 */
void game_loop args(());
int init_socket args((int port));
void new_descriptor args((int new_desc));
bool read_from_descriptor args((DESCRIPTOR_DATA * d));
bool write_to_descriptor args((int desc, char *txt, int length));


/*
 * Other local functions (OS-independent).
 */
bool check_parse_name args((char *name, bool newchar));
bool check_reconnect args((DESCRIPTOR_DATA * d, char *name, bool fConn));
bool check_playing args((DESCRIPTOR_DATA * d, char *name, bool kick));
int main args((int argc, char **argv));
void nanny args((DESCRIPTOR_DATA * d, char *argument));
bool flush_buffer args((DESCRIPTOR_DATA * d, bool fPrompt));
void read_from_buffer args((DESCRIPTOR_DATA * d));
void stop_idling args((CHAR_DATA * ch));
void free_desc args((DESCRIPTOR_DATA * d));
void display_prompt args((DESCRIPTOR_DATA * d));
int make_color_sequence args((const char *col, char *buf, DESCRIPTOR_DATA * d));
void set_pager_input args((DESCRIPTOR_DATA * d, char *argument));
bool pager_output args((DESCRIPTOR_DATA * d));

void mail_count args((CHAR_DATA * ch));

void tax_player args((CHAR_DATA * ch));

int port;

int base_resis_values args((CHAR_DATA *ch, int type));

#ifdef DNS_SLAVE
#define MAX_HOST 60
#define MAX_OPEN_FILES 128
bool get_slave_result(void);
void slave_timeout(void);

void boot_slave(void)
{
   int i, sv[2];

   if (slave_socket != -1)
   {
      close(slave_socket);
      slave_socket = -1;
   }

   if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) < 0)
   {
      bug("boot_slave: socketpair: %s", strerror(errno));
      return;
   }
   /* set to nonblocking */
   if (fcntl(sv[0], F_SETFL, FNDELAY) == -1)
   {
      bug("boot_slave: fcntl( F_SETFL, FNDELAY ): %s", strerror(errno));
      close(sv[0]);
      close(sv[1]);
      return;
   }

   slave_pid = fork();

   switch (slave_pid)
   {
      case -1:
         bug("boot_slave: fork: %s", strerror(errno));
         close(sv[0]);
         close(sv[1]);
         return;

      case 0: /* child */
         close(sv[0]);
         close(0);
         close(1);
         if (dup2(sv[1], 0) == -1)
         {
            bug("boot_slave: child: unable to dup stdin: %s", strerror(errno));
            _exit(1);
         }
         if (dup2(sv[1], 1) == -1)
         {
            bug("boot_slave: child: unable to dup stdout: %s", strerror(errno));
            _exit(1);
         }
         for (i = 3; i < MAX_OPEN_FILES; ++i)
            close(i);

         execlp("../src/dns_slave", "dns_slave", NULL);
         bug("boot_slave: child: unable to exec: %s", strerror(errno));
         _exit(1);
   }

   close(sv[1]);

   if (fcntl(sv[0], F_SETFL, FNDELAY) == -1)
   {
      bug("boot_slave: fcntl: %s", strerror(errno));
      close(sv[0]);
      return;
   }
   slave_socket = sv[0];
}

char *printhostaddr(char *hostbuf, struct in_addr *addr)
{
   char *tmp;

   tmp = inet_ntoa(*addr);
   strcpy(hostbuf, tmp);

   return (hostbuf + strlen(hostbuf));
}

void make_slave_request(struct sockaddr_in *sock)
{
   char buf[512];
   int num;

   sprintf(buf, "%c%s\n", SLAVE_IPTONAME, inet_ntoa(sock->sin_addr));

   if (slave_socket != -1)
   {
      /* ask slave to do a hostname lookup for us */
      if (write(slave_socket, buf, (num = strlen(buf))) != num)
      {
         bug("[SLAVE] make_slave_request: Losing slave on write: %s", strerror(errno));
         close(slave_socket);
         slave_socket = -1;
         return;
      }
   }
}

void check_desc_state(DESCRIPTOR_DATA * d)
{
   if ((d->site_info & (HostName | HostNameFail)))
   {
      if (d->connected == CON_GETDNS)
      {
         d->wait = 0;
         d->connected = CON_GET_ACCOUNT;
         write_to_buffer(d, "Please enter your account name, or type new for a new account: ", 0);
      }
   }
}

bool get_slave_result(void)
{
   DESCRIPTOR_DATA *d, *n;
   char buf[MSL + 1], token[MAX_STRING_LENGTH];
   int len, octet[4];
   long addr;

   len = read(slave_socket, buf, MSL);
   if (len < 0)
   {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
         return TRUE;

      bug("[SLAVE] get_slave_result: Losing slave on read: %s", strerror(errno));
      close(slave_socket);
      slave_socket = -1;
      return TRUE;
   }
   else if (!len)
      return TRUE;

   buf[len] = 0;

   switch (buf[0])
   {
      case SLAVE_IPTONAME_FAIL:
         if (sscanf(buf + 1, "%d.%d.%d.%d", &octet[0], &octet[1], &octet[2], &octet[3]) >= 4)
         {
            addr = (octet[0] << 24) + (octet[1] << 16) + (octet[2] << 8) + (octet[3]);

            addr = ntohl(addr);

            for (d = first_descriptor; d; d = n)
            {
               n = d->next;
               if (d->site_info & (HostName | HostNameFail))
                  continue;

               if (d->addr != addr)
                  continue;

               /* Do nothing to host name, addr is still there */
               d->site_info |= HostNameFail;
               check_desc_state(d);
            }
         }
         else
            bug("[SLAVE] Invalid: '%s'", buf);

         break;

      case SLAVE_IPTONAME:
         if (sscanf(buf + 1, "%d.%d.%d.%d %s", &octet[0], &octet[1], &octet[2], &octet[3], token) != 5)
         {
            bug("[SLAVE] Invalid: %s", buf);
            return TRUE;
         }
         addr = (octet[0] << 24) + (octet[1] << 16) + (octet[2] << 8) + (octet[3]);

         addr = ntohl(addr);

         for (d = first_descriptor; d; d = n)
         {
            n = d->next;
            if (d->site_info & HostName)
               continue;

            if (d->addr != addr)
               continue;

            strncpy(d->host, token, MAX_HOST);
            d->host[MAX_HOST] = 0;
            d->site_info |= HostName;
            check_desc_state(d);
         }
         break;

      default:
         bug("[SLAVE] Invalid: %s", buf);
         break;
   }
   return FALSE;
}

void slave_timeout(void)
{
   DESCRIPTOR_DATA *d, *n;
   time_t nowtime, tdiff;

   nowtime = time(0);
   for (d = first_descriptor; d; d = n)
   {
      n = d->next;
      if (d->connected == CON_GETDNS)
      {
         /* Worst case, one minute wait */
         tdiff = nowtime - d->contime;

         /* hostname timeout: 60 seconds */
         if ((tdiff > 60) || ((tdiff > 10) && (d->site_info & (HostName | HostNameFail))))
         {
            d->wait = 0;
            d->connected = CON_GET_ACCOUNT;
            write_to_buffer(d, "Please enter your account name, or type new for a new account: ", 0);
         }
      }
   }
}
#endif

#ifdef WIN32
int mainthread(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
   struct timeval now_time;
   char hostn[128];
   bool fCopyOver = !TRUE;

   /*
    * Memory debugging if needed.
    */
#if defined(MALLOC_DEBUG)
   malloc_debug(2);
#endif
/*#ifdef MTRACE
  setenv( "MALLOC_TRACE", "mtrace_log.log", 0 );
  mtrace();
#endif */

   num_descriptors = 0;
   first_descriptor = NULL;
   last_descriptor = NULL;
   sysdata.NO_NAME_RESOLVING = TRUE;
   sysdata.WAIT_FOR_AUTH = TRUE;

   /*
    * Init time.
    */
   gettimeofday(&now_time, NULL);
   current_time = (time_t) now_time.tv_sec;
/*  gettimeofday( &boot_time, NULL);   okay, so it's kludgy, sue me :) */
   boot_time = time(0); /*  <-- I think this is what you wanted */
   strcpy(str_boot_time, ctime(&current_time));

   /*
    * Init boot time.
    */
   set_boot_time = &set_boot_time_struct;
   set_boot_time->manual = 0;

   new_boot_time = update_time(localtime(&current_time));
   /* Copies *new_boot_time to new_boot_struct, and then points
      new_boot_time to new_boot_struct again. -- Alty */
   new_boot_struct = *new_boot_time;
   new_boot_time = &new_boot_struct;
   new_boot_time->tm_mday += 1;
   if (new_boot_time->tm_hour > 12)
      new_boot_time->tm_mday += 1;
   new_boot_time->tm_sec = 0;
   new_boot_time->tm_min = 0;
   new_boot_time->tm_hour = 6;

   /* Update new_boot_time (due to day increment) */
   new_boot_time = update_time(new_boot_time);
   new_boot_struct = *new_boot_time;
   new_boot_time = &new_boot_struct;
   /* Bug fix submitted by Gabe Yoder */
   new_boot_time_t = mktime(new_boot_time);
   reboot_check(mktime(new_boot_time));
   /* Set reboot time string for do_time */
   get_reboot_string();
   init_pfile_scan_time(); /* Pfile autocleanup initializer - Samson 5-8-99 */

   /*
    * Reserve two channels for our use.
    */
   if ((fpReserve = fopen(NULL_FILE, "r")) == NULL)
   {
      perror(NULL_FILE);
      exit(1);
   }
   if ((fpLOG = fopen(NULL_FILE, "r")) == NULL)
   {
      perror(NULL_FILE);
      exit(1);
   }

   /*
    * Get the port number.
    */
   port = 4000;
   if (argc > 1)
   {
      if (!is_number(argv[1]))
      {
         fprintf(stderr, "Usage: %s [port #]\n", argv[0]);
         exit(1);
      }
      else if ((port = atoi(argv[1])) <= 1024)
      {
         fprintf(stderr, "Port number must be above 1024.\n");
         exit(1);
      }
   }
   if (argv[2] && argv[2][0])
   {
      fCopyOver = TRUE;
      control = atoi(argv[3]);
      control2 = atoi(argv[4]);
   }
   else
      fCopyOver = FALSE;
   /*
    * Run the game.
    */
#ifdef WIN32
   {
      /* Initialise Windows sockets library */

      unsigned short wVersionRequested = MAKEWORD(1, 1);
      WSADATA wsadata;
      int err;

      /* Need to include library: wsock32.lib for Windows Sockets */
      err = WSAStartup(wVersionRequested, &wsadata);
      if (err)
      {
         fprintf(stderr, "Error %i on WSAStartup\n", err);
         exit(1);
      }

      /* standard termination signals */
      signal(SIGINT, (void *) bailout);
      signal(SIGTERM, (void *) bailout);
   }
#endif /* WIN32 */

#ifdef DNS_SLAVE
   log_string("Booting DNS Slave process");
   boot_slave();
#endif
   log_string("Booting Database");
   boot_db(fCopyOver);
   log_string("Initializing socket");
   if (!fCopyOver) /* We have already the port if copyover'ed */
   {
      control = init_socket(port);
   }
//    control  = init_socket( port   );
/*    control2 = init_socket( port+1 );
    conclient= init_socket( port+10);
    conjava  = init_socket( port+20);
 */

   /* I don't know how well this will work on an unnamed machine as I don't
      have one handy, and the man pages are ever-so-helpful.. -- Alty */
   if (gethostname(hostn, sizeof(hostn)) < 0)
   {
      perror("main: gethostname");
      strcpy(hostn, "unresolved");
   }
   sprintf(log_buf, "%s ready at address %s on port %d.", sysdata.mud_name, hostn, port);
/*
    sprintf( log_buf, "Realms of Despair ready at address %s on port %d.",
	hostn, port );
*/
   log_string(log_buf);

   game_loop();

   closesocket(control);
   closesocket(control2);
   closesocket(conclient);
   closesocket(conjava);

#ifdef WIN32
   if (service_shut_down)
   {
      CHAR_DATA *vch;

      /* Save all characters before booting. */
      for (vch = first_char; vch; vch = vch->next)
         if (!IS_NPC(vch))
         {
            shutdown_checkpoint();
            save_char_obj(vch);
         }
   }
   /* Shut down Windows sockets */

   WSACleanup(); /* clean up */
   kill_timer(); /* stop timer thread */
#endif

#ifdef DNS_SLAVE
   if (slave_socket != -1)
   {
      log_string("Terminating DNS Slave process.");
      kill(slave_pid, SIGKILL);
   }
#endif
   /*
    * That's all, folks.
    */
   log_string("Normal termination of game.");
   exit(0);
   return 0;
}


int init_socket(int port)
{
   char hostname[64];
   struct sockaddr_in sa;
   int x = 1;
   int fd;

   gethostname(hostname, sizeof(hostname));


   if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("Init_socket: socket");
      exit(1);
   }

   if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &x, sizeof(x)) < 0)
   {
      perror("Init_socket: SO_REUSEADDR");
      closesocket(fd);
      exit(1);
   }

#if defined(SO_DONTLINGER) && !defined(SYSV)
   {
      struct linger ld;

      ld.l_onoff = 1;
      ld.l_linger = 1000;

      if (setsockopt(fd, SOL_SOCKET, SO_DONTLINGER, (void *) &ld, sizeof(ld)) < 0)
      {
         perror("Init_socket: SO_DONTLINGER");
         closesocket(fd);
         exit(1);
      }
   }
#endif

   memset(&sa, '\0', sizeof(sa));
   sa.sin_family = AF_INET;
   sa.sin_port = htons(port);

   if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) == -1)
   {
      perror("Init_socket: bind");
      closesocket(fd);
      exit(1);
   }

   if (listen(fd, 50) < 0)
   {
      perror("Init_socket: listen");
      closesocket(fd);
      exit(1);
   }

   return fd;
}

/* set up MXP */
void turn_on_mxp (DESCRIPTOR_DATA *d)
{
   char buf[MSL];
   char line[MSL];
   SLAB_DATA *slab;
   int x;
   
   d->mxp = TRUE;  /* turn it on now */
   write_to_buffer( d, (char *) start_mxp_str, 0 );
   write_to_buffer( d, MXPTAG ("!-- Set up MXP elements --"), 0);
   /* Exit tag */
   write_to_buffer( d, MXPTAG ("!ELEMENT Ex '<send>' FLAG=RoomExit"), 0);
   /* Room description tag */
   write_to_buffer( d, MXPTAG ("!ELEMENT rdesc '<p>' FLAG=RoomDesc"), 0);
   /* Get an item tag (for things on the ground) */
   write_to_buffer( d, MXPTAG 
      ("!ELEMENT Get \"<send href='"
            "get &#39;&name;&#39;|"
            "examine &#39;&name;&#39;|"
            "drink &#39;&name;&#39;|"
            "sacrifice &#39;&name;&#39;"
        "' "
        "hint='RH mouse click to use this object|"
            "Get &name;|"
            "Examine &name;|"
            "Drink from &desc;|"
            "Sacrifice &desc;"
        "'>\" ATT='name'"), 
       0);
   write_to_buffer( d, MXPTAG
      ("!ELEMENT Learn \"<send href='"
            "learn &#39;&name;&#39; &#39;&skill;&#39; 1|"
            "learn &#39;&name;&#39; &#39;&skill;&#39; 2|"
            "learn &#39;&name;&#39; &#39;&skill;&#39; 3|"
            "learn &#39;&name;&#39; &#39;&skill;&#39; 4|"
            "learn &#39;&name;&#39; &#39;&skill;&#39; 5|"
            "learn &#39;&name;&#39; &#39;&skill;&#39; 6"
        "' "
        "hint='RH mouse click to choose mastery level|"
            "Learn &skill; from &name; at Beginner|"
            "Learn &skill; from &name; at Novice|"
            "Learn &skill; from &name; at Expert|"
            "Learn &skill; from &name; at Master|"
            "Learn &skill; from &name; at Elite|"
            "Learn &skill; from &name; at Flawless"
        "'>\" ATT='name skill'"),
       0);
   sprintf(buf, "%s!ELEMENT Forge \"<send href='", MXP_BEG);
   x = 1;
   for (slab = first_slab; slab ; slab = slab->next)  
   {
      if (!slab->next)
         sprintf(line, "forge create %d &name;", x);
      else
         sprintf(line, "forge create %d &name;|", x);
      strcat(buf, line);
      x++;
   }
   strcat(buf, "' hint='RH mouse click to forge this item with the ore in the list|");
   for (slab = first_slab; slab ; slab = slab->next)  
   {
      if (!slab->next)
         sprintf(line, "Forge &name; out of %s", slab->adj);
      else
         sprintf(line, "Forge &name; out of %s|", slab->adj);
      strcat(buf, line);
   } 
   sprintf(line, "'>\" ATT='name'%s", MXP_END);
   strcat(buf, line);
   write_to_buffer(d, buf, 0);
   /*write_to_buffer(d, MXPTAG 
             ("!ELEMENT Forge \"<send href='"
            "forge create 1 &name;;"
            "' "
            "hint='RH mouse click to use this object|"
            "forge &name; out of copper;"
            "'>\" ATT='name desc'"), 0); */
            
   //write_to_buffer(d, MXPTAG(buf), 0);
   // Output to a Mob
   write_to_buffer( d, MXPTAG
       ("!ELEMENT Mobile \"<send href='"
            "attack &#39;&name;&#39; body|" 
            "examine &#39;&name;&#39;|"
            "consider &#39;&name;&#39;|"
            "steal coins &#39;&name;&#39;|"
            "backstab &#39;&name;&#39;|"
            "fire &#39;&name;&#39;"
        "' "
        "hint='RH mouse click to use this object|"
            "Attack &desc;|"
            "Examine &desc;|"
            "Consider &desc;|"
            "Steal Coins from &desc;|"
            "Backstab &desc;|"
            "Fire on &desc;"
        "'>\" ATT='name desc'"),
       0);
   // Output to a Player on look
   write_to_buffer( d, MXPTAG
       ("!ELEMENT Look \"<send href='"
            "glance &#39;&name;&#39;|"
            "look &#39;&name;&#39;|"
            "introduce self &name;|"
            "attack &#39;&name;&#39;|"
            "steal coins &#39;&name;&#39;|"
            "backstab &#39;&name;&#39;|"
            "fire &#39;&name;&#39;"
        "' "
        "hint='RH mouse click to use this object|"
           "Glance at &desc;|"
           "Look at &desc;|"
           "Introduce yourself to &desc;|"
           "Attack &desc;|"
           "Steal coins from &desc;|"
           "Backstab &desc;|"
           "Fire on &desc;"
        "'>\" ATT='name desc'"),
      0);
   /* Drop an item tag (for things in the inventory) */
   write_to_buffer( d, MXPTAG 
       ("!ELEMENT Drop \"<send href='"
            "drop &#39;&name;&#39;|"
            "examine &#39;&name;&#39;|"
            "look in &#39;&name;&#39;|"
            "wear &#39;&name;&#39;|"
            "eat &#39;&name;&#39;|"
            "drink &#39;&name;&#39;|"
            "repair &name;|"
            "c &#39;identify&#39; &name;"
        "' "
        "hint='RH mouse click to use this object|"
            "Drop &name;|"
            "Examine &name;|"
            "Look inside &name;|"
            "Wear &name;|"
            "Eat &name;|"
            "Drink &name;|"
            "Repair &name;|"
            "Identify &name;"
        "'>\" ATT='name'"), 
       0);
   write_to_buffer( d, MXPTAG 
       ("!ELEMENT Listgroups \"<send href='"
            "listgroups &#39;&name;&#39;|"
            "listgroups toadvance &name;"
        "' "
        "hint='Default is listgroups <group>, right click for more options|"
            "Listgroups &name;|"
            "Listgroups toadvance &name;"
        "'>\" ATT='name'"), 
       0);
   // Used in listgroups on the groups
   /* Bid an item tag (for things in the auction) */
   write_to_buffer( d, MXPTAG 
       ("!ELEMENT Bid \"<send href='bid &#39;&name;&#39;' "
        "hint='Bid for &name;'>\" "
        "ATT='name'"), 
       0);
   // Click on a helpfile to view it
   write_to_buffer( d, MXPTAG
       ("!ELEMENT Help \"<send href='help &#39;&name;&#39;' "
        "hint='Help &name;'>\" "
        "ATT='name'"),
        0);
   // For any generic command where you don't really need any options but one that is passed
   write_to_buffer( d, MXPTAG
       ("!ELEMENT Command \"<send href='&command;' "
        "hint='&desc;'>\" "
        "ATT='command desc'"),
        0);
   // Same as command put sends the text to the prompt
   write_to_buffer( d, MXPTAG
       ("!ELEMENT PCommand \"<send href='&command;' "
        "hint='&desc;' prompt>\" "
        "ATT='command desc'"),
        0);
   // Click on a help index to view it
   write_to_buffer( d, MXPTAG
       ("!ELEMENT Hindex \"<send href='hindex &#39;&name;&#39;' "
        "hint='hindex &name;'>\" "
        "ATT='name'"),
        0);
   /* List an item tag (for things in a shop) */
   write_to_buffer( d, MXPTAG 
       ("!ELEMENT List \"<send href='buy &#39;&name;&#39;' "
        "hint='Buy &desc;'>\" "
        "ATT='name desc'"), 
       0);
   /* Player tag (for who lists, tells etc.) */
   write_to_buffer( d, MXPTAG 
       ("!ELEMENT Player \"<send href='tell &#39;&name;&#39; ' "
        "hint='Send a message to &name; (No Menu)' prompt>\" "
        "ATT='name'"), 
       0);
} /* end of turn_on_mxp */ 

/*
static void SegVio()
{
  CHAR_DATA *ch;
  char buf[MSL];

  log_string( "SEGMENTATION VIOLATION" );
  log_string( lastplayercmd );
  for ( ch = first_char; ch; ch = ch->next )
  {
    sprintf( buf, "%cPC: %-20s room: %d", IS_NPC(ch) ? 'N' : ' ',
    		ch->name, ch->in_room->vnum );
    log_string( buf );  
  }
  exit(0);
}
*/

/*
 * LAG alarm!							-Thoric
 */
void caught_alarm()
{
   char buf[MSL];

   sprintf(buf, "ALARM CLOCK!  In section %s", alarm_section);
   bug(buf);
   strcpy(buf, "Alas, the hideous malevalent entity known only as 'Lag' rises once more!\n\r");
   echo_to_all(AT_IMMORT, buf, ECHOTAR_ALL);
   if (newdesc)
   {
      FD_CLR(newdesc, &in_set);
      FD_CLR(newdesc, &out_set);
      FD_CLR(newdesc, &exc_set);
      log_string("clearing newdesc");
   }
}

bool check_bad_desc(int desc)
{
   if (FD_ISSET(desc, &exc_set))
   {
      FD_CLR(desc, &in_set);
      FD_CLR(desc, &out_set);
      log_string("Bad FD caught and disposed.");
      return TRUE;
   }
   return FALSE;
}

/*
 * Determine whether this player is to be watched  --Gorog
 */
bool chk_watch(sh_int player_level, char *player_name, char *player_site)
{
   WATCH_DATA *pw;

/*
    char buf[MIL];
    sprintf( buf, "che_watch entry: plev=%d pname=%s psite=%s",
                  player_level, player_name, player_site);
    log_string(buf);
*/
   if (!first_watch)
      return FALSE;

   for (pw = first_watch; pw; pw = pw->next)
   {
      if (pw->target_name)
      {
         if (!str_cmp(pw->target_name, player_name) && player_level < pw->imm_level)
            return TRUE;
      }
      else if (pw->player_site)
      {
         if (!str_prefix(pw->player_site, player_site) && player_level < pw->imm_level)
            return TRUE;
      }
   }
   return FALSE;
}


void accept_new(int ctrl)
{
   static struct timeval null_time;
   DESCRIPTOR_DATA *d;

   /* int maxdesc; Moved up for use with id.c as extern */

#if defined(MALLOC_DEBUG)
   if (malloc_verify() != 1)
      abort();
#endif

   /*
    * Poll all active descriptors.
    */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( ctrl, &in_set );

	maxdesc = ctrl;
	newdesc = 0;
	for ( d = first_descriptor; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	    if( d->ifd != -1 && d->ipid != -1 )
	    {
	    	maxdesc = UMAX( maxdesc, d->ifd );
	    	FD_SET( d->ifd, &in_set );
	    }
	    if ( d == last_descriptor )
	      break;
	}
   auth_maxdesc(&maxdesc, &in_set, &out_set, &exc_set);

   //maxdesc = imc_fill_fdsets(maxdesc, &in_set, &out_set, &exc_set);

   if (select(maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0)
   {
      perror("accept_new: select: poll");
      exit(1);
   }
#ifdef DNS_SLAVE
   /* slave result? */
   if (slave_socket != -1)
   {
      if (FD_ISSET(slave_socket, &in_set))
      {
         {
            while (!get_slave_result())
               ;
         }
      }
   }
   else
      slave_timeout();
#endif
   if (FD_ISSET(ctrl, &exc_set))
   {
      bug("Exception raise on controlling descriptor %d", ctrl);
      FD_CLR(ctrl, &in_set);
      FD_CLR(ctrl, &out_set);
   }
   else if (FD_ISSET(ctrl, &in_set))
   {
      newdesc = ctrl;
      new_descriptor(newdesc);
   }
}

void game_loop()
{
   struct timeval last_time;
   char cmdline[MIL];
   DESCRIPTOR_DATA *d;

/*  time_t	last_check = 0;  */

#ifndef WIN32
   signal(SIGPIPE, SIG_IGN);
   signal(SIGALRM, caught_alarm);
#endif

   /* signal( SIGSEGV, SegVio ); */
   gettimeofday(&last_time, NULL);
   current_time = (time_t) last_time.tv_sec;

   /* Main loop */
   while (!mud_down)
   {
      accept_new(control);
      accept_new(control2);
      accept_new(conclient);
      accept_new(conjava);

      auth_check(&in_set, &out_set, &exc_set);

      /*
       * Kick out descriptors with raised exceptions
       * or have been idle, then check for input.
       */
      for (d = first_descriptor; d; d = d_next)
      {
         int tlogin, tnotes, tidle;
         
         if (d == d->next)
         {
            bug("descriptor_loop: loop found & fixed");
            d->next = NULL;
         }
         d_next = d->next;

         d->idle++; /* make it so a descriptor can idle out */
         
         if (d && d->character && d->character->pcdata)
         {
            tlogin = d->character->pcdata->timeout_login;
            tnotes = d->character->pcdata->timeout_notes;
            tidle = d->character->pcdata->timeout_idle;
         }
         else
         {
            tlogin = sysdata.timeout_login;
            tnotes = sysdata.timeout_notes;
            tidle = sysdata.timeout_idle;
         }
         if (tlogin == 0)
            tlogin = 720;
         if (tnotes == 0)
            tnotes = 2000000000; //Not forever, but damn close;
         if (tidle == 0)
            tidle = 2000000000; //Not forever, but damn close;
            
         if (tlogin < 60)
            tlogin = 60;
         if (FD_ISSET(d->descriptor, &exc_set))
         {
            FD_CLR(d->descriptor, &in_set);
            FD_CLR(d->descriptor, &out_set);
            if (d->character && (d->connected == CON_PLAYING || d->connected == CON_EDITING))
               save_char_obj(d->character);
            d->outtop = 0;
            close_socket(d, TRUE);
            continue;
         }
         else if ((d->connected < CON_SHOW_ACCOUNT_MENU && d->connected > CON_LAST_MENU_OPTION && !d->character && d->idle > tlogin) /* 3 mins */
            || (d->connected != CON_PLAYING && d->idle > tnotes) /* 10 mins */ //buffer and menu
            || d->idle > tidle) /* 20 min  */
         {
            write_to_descriptor(d->descriptor, "Idle timeout... disconnecting.\n\r", 0);
            d->outtop = 0;
            close_socket(d, TRUE);
            continue;
         }
         else
         {
            d->fcommand = FALSE;

            if (FD_ISSET(d->descriptor, &in_set))
            {
               d->idle = 0;
               if (d->character)
                  d->character->timer = 0;
               if (!read_from_descriptor(d))
               {
                  FD_CLR(d->descriptor, &out_set);
                  if (d->character && (d->connected == CON_PLAYING || d->connected == CON_EDITING))
                     save_char_obj(d->character);
                  d->outtop = 0;
                  close_socket(d, FALSE);
                  continue;
               }
            }
            /* check for input from the dns */
	        if( ( d->connected == CON_PLAYING || d->character != NULL ) && d->ifd != -1 && FD_ISSET( d->ifd, &in_set ) )
	           process_dns( d );
            if (d->character && d->character->wait > 0)
            {
               --d->character->wait;
               continue;
            }

            read_from_buffer(d);
#ifdef DNS_SLAVE /* Fix here provided by Senir */
            if (d->incomm[0] != '\0' && d->connected != CON_GETDNS)
#else
            if (d->incomm[0] != '\0')
#endif
            {
               d->fcommand = TRUE;
               stop_idling(d->character);

               strcpy(cmdline, d->incomm);
               d->incomm[0] = '\0';

               if (d->character)
                  set_cur_char(d->character);

               if (d->pagepoint)
                  set_pager_input(d, cmdline);
               else
                  switch (d->connected)
                  {
                     default:
                        nanny(d, cmdline);
                        break;
                     case CON_PLAYING:
                        interpret(d->character, cmdline);
                        break;
                     case CON_EDITING:
                        edit_buffer(d->character, cmdline);
                        break;
                  }
            }
         }
         if (d == last_descriptor)
            break;
      }

      /*
       * Autonomous game motion.
       */
      update_handler();

      /*
       * Check REQUESTS pipe
       */
      check_requests();

      /*
       * Output.
       */
      for (d = first_descriptor; d; d = d_next)
      {
         d_next = d->next;

         if ((d->fcommand || d->outtop > 0) && FD_ISSET(d->descriptor, &out_set))
         {
            if (d->pagepoint)
            {
               if (!pager_output(d))
               {
                  if (d->character && (d->connected == CON_PLAYING || d->connected == CON_EDITING))
                     save_char_obj(d->character);
                  d->outtop = 0;
                  close_socket(d, FALSE);
               }
            }
            else if (!flush_buffer(d, TRUE))
            {
               if (d->character && (d->connected == CON_PLAYING || d->connected == CON_EDITING))
                  save_char_obj(d->character);
               d->outtop = 0;
               close_socket(d, FALSE);
            }
         }
         if (d == last_descriptor)
            break;
      }

      /*
       * Synchronize to a clock.
       * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
       * Careful here of signed versus unsigned arithmetic.
       */
      {
         struct timeval now_time;
         long secDelta;
         long usecDelta;

         gettimeofday(&now_time, NULL);
         usecDelta = ((int) last_time.tv_usec) - ((int) now_time.tv_usec) + 1000000 / PULSE_PER_SECOND;
         secDelta = ((int) last_time.tv_sec) - ((int) now_time.tv_sec);
         while (usecDelta < 0)
         {
            usecDelta += 1000000;
            secDelta -= 1;
         }

         while (usecDelta >= 1000000)
         {
            usecDelta -= 1000000;
            secDelta += 1;
         }

         if (secDelta > 0 || (secDelta == 0 && usecDelta > 0))
         {
            struct timeval stall_time;

            stall_time.tv_usec = usecDelta;
            stall_time.tv_sec = secDelta;
#ifdef WIN32
            Sleep((stall_time.tv_sec * 1000L) + (stall_time.tv_usec / 1000L));
#else
            if (select(0, NULL, NULL, NULL, &stall_time) < 0 && errno != EINTR)
            {
               perror("game_loop: select: stall");
               exit(1);
            }
#endif
         }
      }

      gettimeofday(&last_time, NULL);
      current_time = (time_t) last_time.tv_sec;

      /* Check every 5 seconds...  (don't need it right now)
         if ( last_check+5 < current_time )
         {
         CHECK_LINKS(first_descriptor, last_descriptor, next, prev,
         DESCRIPTOR_DATA);
         last_check = current_time;
         }
       */
   }
   /*  Save morphs so can sort later. --Shaddai */
   if (sysdata.morph_opt)
      save_morphs();

   fflush(stderr); /* make sure strerr is flushed */
   return;
}

void init_descriptor(DESCRIPTOR_DATA * dnew, int desc)
{
   dnew->next = NULL;
   dnew->descriptor = desc;
   dnew->connected = CON_GET_ACCOUNT;
   dnew->outsize = 2000;
   dnew->idle = 0;
   dnew->lines = 0;
   dnew->scrlen = 24;
   dnew->user = STRALLOC("unknown");
   dnew->ifd           = -1;    /* Descriptor pipes, used for DNS resolution and such */
   dnew->ipid          = -1;
   dnew->newstate = 0;
   dnew->prevcolor = 0x07;
   dnew->mxp = FALSE;   /* NJG - initially MXP is off */

   CREATE(dnew->outbuf, char, dnew->outsize);
}

void new_descriptor(int new_desc)
{
   char buf[MSL];
   DESCRIPTOR_DATA *dnew;
   struct sockaddr_in sock;
   int desc;
   int size;

   /*    char bugbuf[MSL]; */
#ifdef WIN32
   unsigned long arg = 1;
#endif

   size = sizeof(sock);
   if (check_bad_desc(new_desc))
   {
      set_alarm(0);
      return;
   }
   set_alarm(20);
   alarm_section = "new_descriptor::accept";
   if ((desc = accept(new_desc, (struct sockaddr *) &sock, (socklen_t *) &size)) < 0)
   {
      set_alarm(0);
      return;
   }
   if ( check_bad_desc( new_desc ) )
    {
       set_alarm( 0 );
       return;
    }
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

   set_alarm(20);
   alarm_section = "new_descriptor: after accept";

#ifdef WIN32
   if (ioctlsocket(desc, FIONBIO, &arg) == -1)
#else
   if (fcntl(desc, F_SETFL, FNDELAY) == -1)
#endif
   {
      perror("New_descriptor: fcntl: FNDELAY");
      set_alarm(0);
      return;
   }
   if (check_bad_desc(new_desc))
      return;

   CREATE(dnew, DESCRIPTOR_DATA, 1);
   init_descriptor(dnew, desc);
   dnew->port = ntohs(sock.sin_port);
   strcpy(buf, inet_ntoa(sock.sin_addr));
   sprintf(log_buf, "Sock.sinaddr:  %s, port %hd.", buf, dnew->port);
   log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
   CREATE( dnew->outbuf, char, dnew->outsize );
   strcpy( log_buf, inet_ntoa( sock.sin_addr ) );

   dnew->host = STRALLOC( log_buf );

   if ( !sysdata.NO_NAME_RESOLVING )
   {
	  strcpy( buf, in_dns_cache( log_buf ) );
 
      if( buf[0] == '\0' )
	     resolve_dns( dnew, sock.sin_addr.s_addr );
      else
	  {
	     STRFREE( dnew->host );
	     dnew->host = STRALLOC( buf );
      }
   }
   if (check_total_bans(dnew))
   {
      write_to_descriptor(desc, "Your site has been banned from this Mud.\n\r", 0);
      free_desc(dnew);
      set_alarm(0);
      return;
   }
   /*
    * Init descriptor data.
    */

   if (!last_descriptor && first_descriptor)
   {
      DESCRIPTOR_DATA *d;

      bug("New_descriptor: last_desc is NULL, but first_desc is not! ...fixing");
      for (d = first_descriptor; d; d = d->next)
         if (!d->next)
            last_descriptor = d;
   }
   #ifdef MCCP
    write_to_buffer(dnew, eor_on_str, 0);
    write_to_buffer(dnew, compress2_on_str, 0);
    write_to_buffer(dnew, compress_on_str, 0);
   #endif
   LINK(dnew, first_descriptor, last_descriptor, next, prev);
   /*
      * Send the greeting.
    */
   {
      extern char *help_greeting;

      if (help_greeting[0] == '.')
         write_to_buffer(dnew, help_greeting + 1, 0);
      else
         write_to_buffer(dnew, help_greeting, 0);
   }
#ifdef DNS_SLAVE
   if (!sysdata.NO_NAME_RESOLVING)
   {
      make_slave_request(&sock);
      write_to_buffer(dnew, "Please wait - looking up DNS information.\n\r", 0);
      /* Make 'em wait, don't wanna lose site info */
      dnew->wait = 1 << 24;
      dnew->connected = CON_GETDNS;
   }
   else
   {
      write_to_buffer(dnew, "Please enter your account name, or type new for a new account: ", 0);
   }
#else
   write_to_buffer(dnew, "Please enter your account name, or type new for a new account: ", 0);
#endif

   alarm_section = "new_descriptor: set_auth";
   set_auth(dnew);
   alarm_section = "new_descriptor: after set_auth";

   if (++num_descriptors > sysdata.maxplayers)
      sysdata.maxplayers = num_descriptors;
   if (sysdata.maxplayers > sysdata.alltimemax)
   {
      if (sysdata.time_of_max)
         DISPOSE(sysdata.time_of_max);
      sprintf(buf, "%24.24s", ctime(&current_time));
      sysdata.time_of_max = str_dup(buf);
      sysdata.alltimemax = sysdata.maxplayers;
      sprintf(log_buf, "Broke all-time maximum player record: %d", sysdata.alltimemax);
      log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
      to_channel(log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);
      save_sysdata(sysdata);
   }
   set_alarm(0);
   return;
}

void free_desc(DESCRIPTOR_DATA * d)
{
   kill_auth(d);
   closesocket(d->descriptor);
#ifndef DNS_SLAVE
   STRFREE(d->host);
#endif
   DISPOSE(d->outbuf);
   if (d->account)
   {
      ACCOUNT_NAME *aname;
      ACCOUNT_NAME *anext;
      
      //Don't want to lock them out of their account by getting disconnected, laugh
      if (d->account->editing == 1)
      {
         d->account->editing = 0;
         save_account(d, 0);
      }
      
      for (aname = d->account->first_player; aname; aname = anext)
      {
         anext = aname->next;
         if (aname->name)
            STRFREE(aname->name);
         DISPOSE(aname);
      }
      if (d->account->name)
         STRFREE(d->account->name);
      if (d->account->passwd)
         STRFREE(d->account->passwd);
      if (d->account->email)
         STRFREE(d->account->email);
      DISPOSE(d->account);
   }
   STRFREE(d->user); /* identd */
   if (d->pagebuf)
      DISPOSE(d->pagebuf);
   #ifdef MCCP
     compressEnd(d);
   #endif
   DISPOSE(d);
/*    --num_descriptors;  This is called from more than close_socket -- Alty */
   return;
}

void close_socket(DESCRIPTOR_DATA * dclose, bool force)
{
   CHAR_DATA *ch;
   DESCRIPTOR_DATA *d;
   bool DoNotUnlink = FALSE;
   
   if( dclose->ipid != -1 ) 
   {
	  int status;

	  kill( dclose->ipid, SIGKILL );
      waitpid( dclose->ipid, &status, 0 );
   }
   if( dclose->ifd != -1 )
      close( dclose->ifd );

   /* flush outbuf */
   if (!force && dclose->outtop > 0)
      flush_buffer(dclose, FALSE);

   /* say bye to whoever's snooping this descriptor */
   if (dclose->snoop_by)
      write_to_buffer(dclose->snoop_by, "Your victim has left the game.\n\r", 0);

   /* stop snooping everyone else */
   for (d = first_descriptor; d; d = d->next)
      if (d->snoop_by == dclose)
         d->snoop_by = NULL;

   /* Check for switched people who go link-dead. -- Altrag */
   if (dclose->original)
   {
      if ((ch = dclose->character) != NULL)
         do_return(ch, "");
      else
      {
         bug("Close_socket: dclose->original without character %s", (dclose->original->name ? dclose->original->name : "unknown"));
         dclose->character = dclose->original;
         dclose->original = NULL;
      }
   }

   ch = dclose->character;

   /* sanity check :( */
   if (!dclose->prev && dclose != first_descriptor)
   {
      DESCRIPTOR_DATA *dp, *dn;

      bug("Close_socket: %s desc:%p != first_desc:%p and desc->prev = NULL!", ch ? ch->name : d->host, dclose, first_descriptor);
      dp = NULL;
      for (d = first_descriptor; d; d = dn)
      {
         dn = d->next;
         if (d == dclose)
         {
            bug("Close_socket: %s desc:%p found, prev should be:%p, fixing.", ch ? ch->name : d->host, dclose, dp);
            dclose->prev = dp;
            break;
         }
         dp = d;
      }
      if (!dclose->prev)
      {
         bug("Close_socket: %s desc:%p could not be found!.", ch ? ch->name : dclose->host, dclose);
         DoNotUnlink = TRUE;
      }
   }
   if (!dclose->next && dclose != last_descriptor)
   {
      DESCRIPTOR_DATA *dp, *dn;

      bug("Close_socket: %s desc:%p != last_desc:%p and desc->next = NULL!", ch ? ch->name : d->host, dclose, last_descriptor);
      dn = NULL;
      for (d = last_descriptor; d; d = dp)
      {
         dp = d->prev;
         if (d == dclose)
         {
            bug("Close_socket: %s desc:%p found, next should be:%p, fixing.", ch ? ch->name : d->host, dclose, dn);
            dclose->next = dn;
            break;
         }
         dn = d;
      }
      if (!dclose->next)
      {
         bug("Close_socket: %s desc:%p could not be found!.", ch ? ch->name : dclose->host, dclose);
         DoNotUnlink = TRUE;
      }
   }

   if (dclose->character)
   {
      if ((dclose->connected == CON_PLAYING
            || dclose->connected == CON_ROLL_STATS
            || dclose->connected == CON_EDITING) || (dclose->connected >= CON_NOTE_TO && dclose->connected <= CON_NOTE_FINISH))
      {
         if (xIS_SET(ch->act, PLR_GAMBLER))
         {
            act(AT_ACTION, "Because $n lost $s link, $e forfeits $s bet.", ch, NULL, NULL, TO_CANSEE);
            xREMOVE_BIT(ch->act, PLR_GAMBLER); /* Removes gambler bit in player */
         }
         act(AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_CANSEE);
         ch->desc = NULL;
      }
/*    else if ( dclose->connected == CON_PLAYING )
	{
         if ( dclose->character->position == POS_FIGHTING
         || dclose->character->position ==  POS_EVASIVE
         || dclose->character->position ==  POS_DEFENSIVE
         || dclose->character->position ==  POS_AGGRESSIVE
         || dclose->character->position ==  POS_BERSERK )
         {
            act( AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_CANSEE );
	      ch->desc = NULL;
         }
         else
         {
	          ch->desc = NULL;
                ld_punt( ch );
         }

      } */
      else
      {
         sprintf(log_buf, "Closing link to %s.", ch->pcdata->filename);
         log_string_plus(log_buf, LOG_COMM, UMAX(sysdata.log_level, ch->level));

         /* clear descriptor pointer to get rid of bug message in log */
         dclose->character->desc = NULL;
         free_char(dclose->character);
      }
   }


   if (!DoNotUnlink)
   {
      free_runbuf(dclose);
      /* make sure loop doesn't get messed up */
      if (d_next == dclose)
         d_next = d_next->next;
      UNLINK(dclose, first_descriptor, last_descriptor, next, prev);
   }
   if (dclose->mxpclient)
      STRFREE(dclose->mxpclient);

   #ifdef MCCP
     compressEnd(dclose);
   #endif

   if (dclose->descriptor == maxdesc)
      --maxdesc;

   free_desc(dclose);
   --num_descriptors;
   return;
}


bool read_from_descriptor(DESCRIPTOR_DATA * d)
{
   int iStart, iErr;

   /* Hold horses if pending command already. */
   if (d->incomm[0] != '\0')
      return TRUE;

   /* Check for overflow. */
   iStart = strlen(d->inbuf);
   if (iStart >= sizeof(d->inbuf) - 10)
   {
      sprintf(log_buf, "%s input overflow!", d->host);
      log_string(log_buf);
      write_to_descriptor(d->descriptor,
         "\n\r*** PUT A LID ON IT!!! ***\n\rYou cannot enter the same command more than 20 consecutive times!\n\r", 0);
      return FALSE;
   }

   for (;;)
   {
      int nRead;

      nRead = recv(d->descriptor, d->inbuf + iStart, sizeof(d->inbuf) - 10 - iStart, 0);
#ifdef WIN32
      iErr = WSAGetLastError();
#else
      iErr = errno;
#endif
      if (nRead > 0)
      {
         iStart += nRead;
         if (d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r')
            break;
      }
      else if (nRead == 0)
      {
         log_string_plus("EOF encountered on read.", LOG_COMM, sysdata.log_level);
         return FALSE;
      }
      else if (iErr == EWOULDBLOCK)
         break;
      else
      {
         perror("Read_from_descriptor");
         return FALSE;
      }
   }

   d->inbuf[iStart] = '\0';
   return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer(DESCRIPTOR_DATA * d)
{
   CHAR_DATA *ch;
   char buf[MSL];
   char *c;
   int i, j, k;
   #ifdef MCCP
    int iac = 0;
   #endif
   unsigned char *p;

   buf[0] = '\0';

   ch = d->original ? d->original : d->character; /* Defined pointer ch -- Xerves 4/16/99 */

   /*
    * Hold horses if pending command already.
    */
   if (d->incomm[0] != '\0')
      return;

   if (d->run_buf)
   {
      while (isdigit(*d->run_head) && *d->run_head != '\0')
      {
         char *s, *e;

         s = d->run_head;
         while (isdigit(*s))
            s++;
         e = s;
         while (*(--s) == '0' && s != d->run_head) ;
         if (isdigit(*s) && *s != '0' && *e != 'o')
         {
            if (*(e + 1) == '+')
            {
               c = e;
               e++;
               switch (*(++e))
               {
                  case 'e':
                  case 'w':
                  case 's':
                  case 'n':
                     sprintf(buf, "%c%c", *c, *e);
                     break;

                  default:
                     return;
               }
               strcpy(d->incomm, buf);
            }
            else
            {
               d->incomm[0] = *e;
               d->incomm[1] = '\0';
            }
            s[0]--;
            while (isdigit(*(++s)))
               *s = '9';
            return;
         }
         if (*e == 'o')
            d->run_head = e;
         else
         {
            if (*(++e) == '+')
            {
               e++;
               e++;
            }
            d->run_head = e;
         }
      }
      if (*d->run_head != '\0')
      {
         if (*d->run_head != 'o')
         {
            if (*(d->run_head + 1) == '+')
            {
               c = d->run_head;
               d->run_head++;
               switch (*(++d->run_head))
               {
                  case 'e':
                  case 'w':
                  case 's':
                  case 'n':
                     sprintf(buf, "%c%c", *c, *d->run_head++);
                     break;

                  default:
                     return;
               }
               strcpy(d->incomm, buf);
            }
            else
            {
               d->incomm[0] = *d->run_head++;
               d->incomm[1] = '\0';
            }
            return;
         }
         else
         {
            char buf2[MIL];

            d->run_head++;
            c = d->run_head;

            sprintf(buf, "open ");
            if (*(d->run_head + 1) == '+')
            {
               d->run_head++;
               switch (*(++d->run_head))
               {
                  case 'e':
                  case 'w':
                  case 's':
                  case 'n':
                     sprintf(buf2, "%c%c", *c, *d->run_head);
                     switch (buf2[0])
                     {
                        case 'n':
                           if ((buf2[1]) == 'w')
                           {
                              sprintf(buf + strlen(buf), "northwest");
                              break;
                           }
                           if ((buf2[1]) == 'e')
                           {
                              sprintf(buf + strlen(buf), "northeast");
                              break;
                           }
                        case 'e':
                           if ((buf2[1]) == 'n')
                           {
                              sprintf(buf + strlen(buf), "northeast");
                              break;
                           }
                           if ((buf2[1]) == 's')
                           {
                              sprintf(buf + strlen(buf), "southeast");
                              break;
                           }
                        case 'w':
                           if ((buf2[1]) == 'n')
                           {
                              sprintf(buf + strlen(buf), "northwest");
                              break;
                           }
                           if ((buf2[1]) == 's')
                           {
                              sprintf(buf + strlen(buf), "southwest");
                              break;
                           }
                        case 's':
                           if ((buf2[1]) == 'e')
                           {
                              sprintf(buf + strlen(buf), "southeast");
                              break;
                           }
                           if ((buf2[1]) == 'w')
                           {
                              sprintf(buf + strlen(buf), "southwest");
                              break;
                           }
                        default:
                           return;
                     }
                     break;

                  default:
                     return;
               }
            }
            else
            {
               switch (*d->run_head)
               {
                  case 'n':
                     sprintf(buf + strlen(buf), "north");
                     break;
                  case 's':
                     sprintf(buf + strlen(buf), "south");
                     break;
                  case 'e':
                     sprintf(buf + strlen(buf), "east");
                     break;
                  case 'w':
                     sprintf(buf + strlen(buf), "west");
                     break;
                  case 'u':
                     sprintf(buf + strlen(buf), "up");
                     break;
                  case 'd':
                     sprintf(buf + strlen(buf), "down");
                     break;
                  default:
                     return;
               }
            }
            strcpy(d->incomm, buf);
            d->run_head++;
            return;
         }
      }
      free_runbuf(d);
   }
   /*  
      Look for incoming telnet negotiation
   */
 
 
   for (p = d->inbuf; *p; p++)
      if (*p == IAC)
      {
         if (memcmp (p, do_mxp_str, strlen ((char *) do_mxp_str)) == 0)
         {
            turn_on_mxp (d);
            /* remove string from input buffer */
            memmove (p, &p [strlen ((char *) do_mxp_str)], strlen ((char *) &p [strlen ((char *) do_mxp_str)]) + 1);
            p--; /* adjust to allow for discarded bytes */
         } /* end of turning on MXP */
         else  if (memcmp (p, dont_mxp_str, strlen ((char *) dont_mxp_str)) == 0)
         {
            d->mxp = FALSE;
            /* remove string from input buffer */
            memmove (p, &p [strlen ((char *) dont_mxp_str)], strlen ((char *) &p [strlen ((char *) dont_mxp_str)]) + 1);
            p--; /* adjust to allow for discarded bytes */
         } /* end of turning off MXP */
      } /* end of finding an IAC */

   /*
    * Look for at least one new line.
    */
   for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r' && i < MAX_INBUF_SIZE; i++)
   {
      if (d->inbuf[i] == '\0')
         return;
   }

   /*
    * Canonical input processing.
    */
   for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
   {
      if (k >= 254)
      {
         write_to_descriptor(d->descriptor, "Line too long.\n\r", 0);

         /* skip the rest of the line */
         /*
            for ( ; d->inbuf[i] != '\0' || i>= MAX_INBUF_SIZE ; i++ )
            {
            if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
            break;
            }
          */
         d->inbuf[i] = '\n';
         d->inbuf[i + 1] = '\0';
         break;
      }
      #ifdef MCCP
      if ( d->inbuf[i] == (signed char)IAC )
           iac=1;
        else if ( iac==1 && (d->inbuf[i] == (signed char)DO || d->inbuf[i] == (signed char)DONT) )
            iac=2;
        else if ( iac==2 )
        {
            iac = 0;
            /*if ( d->inbuf[i] == (signed char)TELOPT_COMPRESS )
            {
                if ( d->inbuf[i-1] == (signed char)DO )
                    compressStart(d, TELOPT_COMPRESS);
                else if ( d->inbuf[i-1] == (signed char)DONT )
                    compressEnd(d);
            }
            else */if ( d->inbuf[i] == (signed char)TELOPT_COMPRESS2 )
            {
                if ( d->inbuf[i-1] == (signed char)DO )
                    compressStart(d, TELOPT_COMPRESS2);
                else if ( d->inbuf[i-1] == (signed char)DONT )
                    compressEnd(d);
            }
        }
      else
      #endif
      if (d->inbuf[i] == '\b' && k > 0)
         --k;
      else if (isascii(d->inbuf[i]) && isprint(d->inbuf[i]))
         d->incomm[k++] = d->inbuf[i];
   }

   /*
    * Finish off the line.
    */
   if (k == 0)
      d->incomm[k++] = ' ';
   d->incomm[k] = '\0';

   /*
    * Deal with bozos with #repeat 1000 ...
    */
   if (k > 1 || d->incomm[0] == '!')
   {
      if (d->incomm[0] != '!' && strcmp(d->incomm, d->inlast))
      {
         d->repeat = 0;
      }
      else
      {
         if (++d->repeat >= 50 && ch && ch->level < LEVEL_IMMORTAL)
         {
            write_to_descriptor(d->descriptor, "\n\r*** PUT A LID ON IT!!! ***\n\rYou cannot enter the same command more than 50 consecutive times!\n\r", 0);
            strcpy(d->incomm, "quit");
         }
      }
   }

   /*
    * Do '!' substitution.
    */
   if (d->incomm[0] == '!')
      strcpy(d->incomm, d->inlast);
   else
      strcpy(d->inlast, d->incomm);

   /*
    * Shift the input buffer.
    */
   while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
      i++;
   for (j = 0; (d->inbuf[j] = d->inbuf[i + j]) != '\0'; j++)
      ;
   return;
}

sh_int client_speed( sh_int speed)
{
 switch ( speed )
 {
   default:
    break;
   case 1:
    return 512;
   case 2:
    return 1024;
   case 3:
    return 2048;
   case 4:
    return 3584;
   case 5:
    return 5120;
   case 6:
    return 7680;
   case 7:
    return 10240;

 }
 return 512; // Better than a mere default case.
}

void do_speed( CHAR_DATA *ch, char *argument)
{
  sh_int speed=atoi(argument);

  if (!ch->desc)
   return;   // Don't send messages to people who don't exist. duh.

  if (argument[0] == '\0')
  {
     ch_printf( ch, "Your present speed is a %d, which equates to %d bytes per second.\n\r", ch->desc->speed, client_speed(ch->desc->speed) );
     return;
  }

  if (speed > 6 || speed < 0)
  {
     send_to_char("Speed is between 0 and 6.\n\r", ch);
     return;
  }
  ch->speed = speed;
  ch->desc->speed = speed;
  ch_printf( ch, "The MUD will now send output to you at %d bytes per second.\n\r", client_speed( speed) );
  if ( client_speed(speed) > 2048)
   ch_printf( ch, "You should be aware %d is fast enough to lag you if you have a slow connection.\n\r", client_speed( speed) );
  return;
}

/*
 * Low level output function.
 */
bool flush_buffer(DESCRIPTOR_DATA * d, bool fPrompt)
{
   char buf[MIL * UMAX( 1, d->speed)];
   extern bool mud_down;

   /*
     * If buffer has more than their max, send max every second.
     */
   if (!mud_down && d->outtop > UMAX(MAX_INBUF_SIZE*4,client_speed( d->speed )))
   {
      memcpy(buf, d->outbuf, client_speed( d->speed ));
      memmove(d->outbuf, d->outbuf + client_speed( d->speed ), d->outtop - client_speed( d->speed ));
      d->outtop -= client_speed( d->speed );
      if (d->snoop_by)
      {
         char snoopbuf[MIL * UMAX( 1, d->speed)];

         buf[client_speed( d->speed )] = '\0';
         if (d->character && d->character->name)
         {
            if (d->original && d->original->name)
               sprintf(snoopbuf, "%s (%s)", d->character->name, d->original->name);
            else
               sprintf(snoopbuf, "%s", d->character->name);
            write_to_buffer(d->snoop_by, snoopbuf, 0);
         }
         write_to_buffer(d->snoop_by, "% ", 2);
         write_to_buffer(d->snoop_by, buf, 0);
      }
      if (!write_to_descriptor(d->descriptor, buf, client_speed( d->speed )))
      {
         d->outtop = 0;
         return FALSE;
      }
      return TRUE;
   }


   /*
    * Bust a prompt.
    */
   if (fPrompt && !mud_down && d->connected == CON_PLAYING)
   {
      CHAR_DATA *ch;

      ch = d->original ? d->original : d->character;
      if (xIS_SET(ch->act, PLR_BLANK))
         write_to_buffer(d, "\n\r", 2);


      if (xIS_SET(ch->act, PLR_PROMPT))
         display_prompt(d);
      if (xIS_SET(ch->act, PLR_TELNET_GA))
         write_to_buffer(d, (char *) go_ahead_str, 0);
   }

   /*
    * Short-circuit if nothing to write.
    */
   if (d->outtop == 0)
      return TRUE;

   /*
    * Snoop-o-rama.
    */
   if (d->snoop_by)
   {
      /* without check, 'force mortal quit' while snooped caused crash, -h */
      if (d->character && d->character->name)
      {
         /* Show original snooped names. -- Altrag */
         if (d->original && d->original->name)
            sprintf(buf, "%s (%s)", d->character->name, d->original->name);
         else
            sprintf(buf, "%s", d->character->name);
         write_to_buffer(d->snoop_by, buf, 0);
      }
      write_to_buffer(d->snoop_by, "% ", 2);
      write_to_buffer(d->snoop_by, d->outbuf, d->outtop);
   }

   /*
    * OS-dependent output.
    */
   if (!write_to_descriptor(d->descriptor, d->outbuf, d->outtop))
   {
      d->outtop = 0;
      return FALSE;
   }
   else
   {
      d->outtop = 0;
      return TRUE;
   }
}

/*
* Count number of mxp tags need converting
*    ie. < becomes &lt;
*        > becomes &gt;
*        & becomes &amp;
*/
 
int count_mxp_tags (const int bMXP, const char *txt, int length)
{
  char c;
   const char * p;
   int count;
   int bInTag = FALSE;
   int bInEntity = FALSE;
  
   for (p = txt, count = 0; 
        length > 0; 
        p++, length--)
     {
     c = *p;
 
     if (bInTag)  /* in a tag, eg. <send> */
       {
       if (!bMXP)
         count--;     /* not output if not MXP */  
       if (c == MXP_ENDc)
         bInTag = FALSE;
       } /* end of being inside a tag */
     else if (bInEntity)  /* in a tag, eg. <send> */
       {
       if (!bMXP)
         count--;     /* not output if not MXP */   
       if (c == ';')
         bInEntity = FALSE;
       } /* end of being inside a tag */
     else switch (c)
       {
 
       case MXP_BEGc:
         bInTag = TRUE;
         if (!bMXP)
           count--;     /* not output if not MXP */   
         else
           count += 4;  /* allow for ESC [1z */
         break;
 
       case MXP_ENDc:   /* shouldn't get this case */
         if (!bMXP)
           count--;     /* not output if not MXP */   
         break;
 
       case MXP_AMPc:
         bInEntity = TRUE;
         if (!bMXP)
           count--;     /* not output if not MXP */   
         break;
 
       default:
         if (bMXP)
           {
           switch (c)
             {
             case '<':       /* < becomes &lt; */
             case '>':       /* > becomes &gt; */
               count += 3;    
               break;
 
             case '&':
               count += 4;    /* & becomes &amp; */
               break;
 
             case '"':        /* " becomes &quot; */
               count += 5;    
               break;
 
             } /* end of inner switch */
           }   /* end of MXP enabled */
       } /* end of switch on character */
 
      }   /* end of counting special characters */
 
   return count;
   } /* end of count_mxp_tags */
 
void convert_mxp_tags (const int bMXP, char * dest, const char *src, int length)
{
 char c;
 const char * ps;
 char * pd;
 int bInTag = FALSE;
 int bInEntity = FALSE;
 
   for (ps = src, pd = dest; 
        length > 0; 
        ps++, length--)
     {
     c = *ps;
     if (bInTag)  /* in a tag, eg. <send> */
       {
       if (c == MXP_ENDc)
         {
         bInTag = FALSE;
         if (bMXP)
           *pd++ = '>';
         }
       else if (bMXP)
         *pd++ = c;  /* copy tag only in MXP mode */
       } /* end of being inside a tag */
     else if (bInEntity)  /* in a tag, eg. <send> */
       {
       if (bMXP)
         *pd++ = c;  /* copy tag only in MXP mode */
       if (c == ';')
         bInEntity = FALSE;
       } /* end of being inside a tag */
     else switch (c)
       {
       case MXP_BEGc:
         bInTag = TRUE;
         if (bMXP)
         {
           memcpy (pd, MXPMODE (1), 4);
           pd += 4;
           *pd++ = '<';
         }
         break;
 
       case MXP_ENDc:    /* shouldn't get this case */
         if (bMXP)
           *pd++ = '>';
         break;
 
       case MXP_AMPc:
         bInEntity = TRUE;
         if (bMXP)
           *pd++ = '&';
         break;
 
       default:
         if (bMXP)
           {
           switch (c)
             {
             case '<':
               memcpy (pd, "&lt;", 4);
               pd += 4;    
               break;
 
             case '>':
               memcpy (pd, "&gt;", 4);
               pd += 4;    
               break;
 
             case '&':
               memcpy (pd, "&amp;", 5);
               pd += 5;    
               break;
 
             case '"':
               memcpy (pd, "&quot;", 6);
               pd += 6;    
               break;
 
             default:
               *pd++ = c;
               break;  /* end of default */
 
             } /* end of inner switch */
           }
         else
           *pd++ = c;  /* not MXP - just copy character */
         break;  
 
       } /* end of switch on character */
 
     }   /* end of converting special characters */
} /* end of convert_mxp_tags */

/*
 * Append onto an output buffer.
 */
void write_to_buffer(DESCRIPTOR_DATA * d, const char *txt, int length)
{
   int origlength;
   if (!d)
   {
      bug("Write_to_buffer: NULL descriptor");
      return;
   }
   /*
    * Normally a bug... but can happen if loadup is used.
    */
   if (!d->outbuf)
      return;

   /*
    * Find length in case caller didn't.
    */
   if (length <= 0)
      length = strlen(txt);

/* Uncomment if debugging or something
    if ( length != strlen(txt) )
    {
	bug( "Write_to_buffer: length(%d) != strlen(txt)!", length );
	length = strlen(txt);
    }
*/
   origlength = length;
   /* work out how much we need to expand/contract it */
   length += count_mxp_tags (d->mxp, txt, length);
   /*
    * Initial \n\r if needed.
    */
   if (d->outtop == 0 && !d->fcommand)
   {
      d->outbuf[0] = '\n';
      d->outbuf[1] = '\r';
      d->outtop = 2;
   }

   /*
    * Expand the buffer as needed.
    */
   while (d->outtop + length >= d->outsize)
   {
      if (d->outsize > 64000)
      {
         /* empty buffer */
         d->outtop = 0;
         close_socket(d, TRUE);
         bug("Buffer overflow. Closing (%s).", d->character ? d->character->name : "???");
         return;
      }
      d->outsize *= 2;
      RECREATE(d->outbuf, char, d->outsize);
   }

   /*
    * Copy.
    */
   convert_mxp_tags (d->mxp, d->outbuf + d->outtop, txt, origlength );
   d->outtop += length;
   d->outbuf[d->outtop] = '\0';
   return;
}

 
#ifdef MCCP
#define COMPRESS_BUF_SIZE MAX_INBUF_SIZE

bool write_to_descriptor( int desc, char *txt, int length )
{
    DESCRIPTOR_DATA *d;
    int     iStart = 0;
    int     nWrite = 0;
    int     nBlock;
    int     len;

    if (length <= 0)
        length = strlen(txt);

    for (d = first_descriptor; d; d = d->next)
        if (d->descriptor == desc)
            break;

    if (!d || d->descriptor != desc)
        d = NULL;

    if (d && d->out_compress)
    {
        d->out_compress->next_in = (unsigned char *)txt;
        d->out_compress->avail_in = length;

        while (d->out_compress->avail_in)
        {
            d->out_compress->avail_out = COMPRESS_BUF_SIZE - (d->out_compress->next_out - d->out_compress_buf);

            if (d->out_compress->avail_out)
            {
                int status = deflate(d->out_compress, Z_SYNC_FLUSH);

                if (status != Z_OK)
                    return FALSE;
            }

            len = d->out_compress->next_out - d->out_compress_buf;
            if (len > 0)
            {
                for (iStart = 0; iStart < len; iStart += nWrite)
                {
                    nBlock = UMIN (len - iStart, MAX_INBUF_SIZE*4);
                    if ((nWrite = write(d->descriptor, d->out_compress_buf + iStart, nBlock)) < 0)
                    {
                        perror( "Write_to_descriptor: compressed" );
                        return FALSE;
                    }

                    if (!nWrite)
                        break;
                }

                if (!iStart)
                   break;

                if (iStart < len)
                    memmove(d->out_compress_buf, d->out_compress_buf+iStart, len - iStart);

                d->out_compress->next_out = d->out_compress_buf + len - iStart;
            }
        }
        return TRUE;
    }

    for (iStart = 0; iStart < length; iStart += nWrite)
    {
        nBlock = UMIN (length - iStart, MAX_INBUF_SIZE*4);
        if ((nWrite = write(desc, txt + iStart, nBlock)) < 0)
        {
            perror( "Write_to_descriptor" );
            return FALSE;
        }
    }

    return TRUE;
}
#else


/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor(int desc, char *txt, int length)
{
   int iStart;
   int nWrite;
   int nBlock;

   if (length <= 0)
      length = strlen(txt);

   for (iStart = 0; iStart < length; iStart += nWrite)
   {
      nBlock = UMIN(length - iStart, MAX_INBUF_SIZE*4);
      if ((nWrite = send(desc, txt + iStart, nBlock, 0)) < 0)
      {
         perror("Write_to_descriptor");
         return FALSE;
      }
   }

   return TRUE;
}
#endif

#ifdef MCCP
/*
 * Ported to SMAUG by Garil of DOTDII Mud
 * aka Jesse DeFer <dotd@dotd.com>  http://www.dotd.com
 *
 * revision 1: MCCP v1 support
 * revision 2: MCCP v2 support
 * revision 3: Correct MMCP v2 support
 * revision 4: clean up of write_to_descriptor() suggested by Noplex@CB
 *
 * See the web site below for more info.
 */

/*
 * mccp.c - support functions for mccp (the Mud Client Compression Protocol)
 *
 * see http://homepages.ihug.co.nz/~icecube/compress/ and README.Rom24-mccp
 *
 * Copyright (c) 1999, Oliver Jowett <icecube@ihug.co.nz>.
 *
 * This code may be freely distributed and used if this copyright notice is
 * retained intact.
 */

void *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
    return calloc(items, size);
}

void zlib_free(void *opaque, void *address)
{
    DISPOSE(address);
}

bool process_compressed(DESCRIPTOR_DATA *d)
{
    int iStart = 0, nBlock, nWrite, len;

    if (!d->out_compress)
        return TRUE;

    // Try to write out some data..
    len = d->out_compress->next_out - d->out_compress_buf;

    if (len > 0)
    {
        // we have some data to write
        for (iStart = 0; iStart < len; iStart += nWrite)
        {
            nBlock = UMIN (len - iStart, 4096);
            if ((nWrite = write(d->descriptor, d->out_compress_buf + iStart, nBlock)) < 0)
            {
                if (errno == EAGAIN ||
                    errno == ENOSR)
                    break;

                return FALSE;
            }

            if (!nWrite)
                break;
        }

        if (iStart)
        {
            // We wrote "iStart" bytes
            if (iStart < len)
                memmove(d->out_compress_buf, d->out_compress_buf+iStart, len - iStart);

            d->out_compress->next_out = d->out_compress_buf + len - iStart;
        }
    }

    return TRUE;
}

char enable_compress[] =
{
    IAC, SB, TELOPT_COMPRESS, WILL, SE, 0
};
char enable_compress2[] =
{
    IAC, SB, TELOPT_COMPRESS2, IAC, SE, 0
};

bool compressStart(DESCRIPTOR_DATA *d, unsigned char telopt)
{
    z_stream *s;

    if (d->out_compress)
        return TRUE;

    bug("Starting compression for descriptor %d", d->descriptor);

    CREATE(s, z_stream, 1);
    CREATE(d->out_compress_buf, unsigned char, COMPRESS_BUF_SIZE);

    s->next_in = NULL;
    s->avail_in = 0;

    s->next_out = d->out_compress_buf;
    s->avail_out = COMPRESS_BUF_SIZE;

    s->zalloc = zlib_alloc;
    s->zfree  = zlib_free;
    s->opaque = NULL;

    if (deflateInit(s, 9) != Z_OK)
    {
        DISPOSE(d->out_compress_buf);
        DISPOSE(s);
        return FALSE;
    }

    if (telopt == TELOPT_COMPRESS)
        write_to_descriptor(d->descriptor, enable_compress, 0);
    else if (telopt == TELOPT_COMPRESS2)
        write_to_descriptor(d->descriptor, enable_compress2, 0);
    else
        bug("compressStart: bad TELOPT passed");

    d->compressing = telopt;
    d->out_compress = s;

    return TRUE;
}

bool compressEnd(DESCRIPTOR_DATA *d)
{
    unsigned char dummy[1];

    if (!d->out_compress)
        return TRUE;

    bug("Stopping compression for descriptor %d", d->descriptor);

    d->out_compress->avail_in = 0;
    d->out_compress->next_in = dummy;

    if (deflate(d->out_compress, Z_FINISH) != Z_STREAM_END)
        return FALSE;

    if (!process_compressed(d)) /* try to send any residual data */
        return FALSE;

    deflateEnd(d->out_compress);
    DISPOSE(d->out_compress_buf);
    DISPOSE(d->out_compress);
    d->compressing = 0;

    return TRUE;
}

void do_compress( CHAR_DATA *ch, char *argument )
{
    if (!ch->desc) 
    {
       send_to_char("What descriptor?!\n", ch);
       return;
    }
    
    if (argument[0] == '\0')
    {
       send_to_char("Syntax:  compress [toggle/stats]\n\r", ch);
       if (ch->desc->out_compress) 
          send_to_char("&w&WMCCP compression is &w&PON&w\n\r", ch);
       else
          send_to_char("&w&WMCCP compression is &w&zOFF&w\n\r", ch);
       return;
    }
    
    if (!str_cmp(argument, "stats"))
    {
       if ( ch->desc->out_compress && ch->desc->out_compress->total_in)
          ch_printf(ch, "Total size of input compressed:  &B[&w%d&B]&D\n\r", ch->desc->out_compress->total_in);
	   if ( ch->desc->out_compress && ch->desc->out_compress->total_out)
   	      ch_printf(ch, "Total size of output compressed: &B[&w%d&B]&D\n\r", ch->desc->out_compress->total_out);
	   if ( ch->desc->out_compress && ch->desc->out_compress->total_in && ch->desc->out_compress->total_out)
          ch_printf(ch, "Current compression ratio:       &B[&w%.2f%&B]&D&D\n\r", 
          (float)ch->desc->out_compress->total_out /  (float)ch->desc->out_compress->total_in * (float)100);
       return;
    }
    if (!str_cmp(argument, "toggle"))
    {
       if (!ch->desc->out_compress) 
       {
          send_to_char("Initiating compression.\n\r", ch);
          write_to_buffer( ch->desc, compress2_on_str, 0 );
          write_to_buffer( ch->desc, compress_on_str, 0 );
       } 
       else
       {
          send_to_char("Terminating compression.\n\r", ch);
          compressEnd(ch->desc);
       }
    }
    do_compress(ch, "");
}
#else
//Putting a MCCP check would of screwed up insertskills program, just put in an empty
//compress routine instead
void do_compress( CHAR_DATA *ch, char *argument )
{
    return;
}
#endif


void skill_addition(CHAR_DATA * ch, sh_int group)
{
   sh_int sn;

   if ((group < 23 || group > 29) && (group < 31 || group > 34))
   {
      ch->pcdata->spellgroups[group] = 1;
      ch->pcdata->spellpoints[group] = 1;
   }
   else
   {
      for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
      {
         if (skill_table[sn]->group[0] == group)
         {
            if (skill_table[sn]->masterydiff[0] == 1)
            {
               ch->pcdata->learned[sn] = 1;
               ch->pcdata->ranking[sn] = 1;
            }
         }
      }
   }
}

char *get_skin_colors(int race)
{
   static char buf[MIL];
   
   if (race == RACE_HUMAN || race == RACE_ELF || race == RACE_DWARF || race == RACE_HOBBIT)
   {
      sprintf(buf, "\n\r[1]  Pale White  [2]  Fair White [3]  White       [4]  Light Tan  [5]  Moderate Tan\n\r"); 
      strcat(buf,  "[6]  Tanned      [7]  Soft Brown [8]  Light Brown [9]  Fair Brown [10] Brown    \n\r"); 
      strcat(buf,  "[11] Light Black [12] Fair Black [13] Black       [14] Dark Black [15] Pure Black\n\r");
   }
   if (race == RACE_OGRE)
   {
      sprintf(buf, "\n\r[1]  Pale Green  [2]  Fair Green  [3]  Green      [4]  Dark Green [5]  Pure Green\n\r"); 
      strcat(buf,  "[6]  Soft Brown  [7]  Light Brown [8]  Fair Brown [9]  Brown      [10] Light Black\n\r");
      strcat(buf,  "[11] Fair Black  [12] Black       [13] Dark Black [14] Pure Black\n\r");
   }
   if (race == RACE_FAIRY)
   {
      sprintf(buf, "\n\r[1]  Pale Blue  [2]  Fair Blue  [3]  Blue  [4]  Dark Blue  [5]  Pure Blue\n\r");
      strcat(buf,  "[6]  Pale Green [7]  Fair Green [8]  Green [9]  Dark Green [10]  Pure Green\n\r");
      strcat(buf,  "[11] Pale Pink  [12] Fair Pink  [13] Pink  [14] Pale Red   [15] Fair Red\n\r");
   }
   return &buf[0];
}

char *get_hair_color()
{
   static char buf[MIL];
   
   sprintf(buf, "\n\r[1]  Soft White  [2]  Fair White  [3]  White       [4]  Soft Grey   [5]  Fair Grey\n\r");
   strcat(buf,  "[6]  Grey        [7]  Dark Grey   [8]  Soft Brown  [9]  Fair Brown  [10] Brown \n\r");   
   strcat(buf,  "[11] Dark Brown  [12] Soft Black  [13] Fair Black  [14] Black       [15] Dark Black\n\r");
   strcat(buf,  "[16] Pure Black  [17] Soft Blonde [18] Fair Blonde [19] Blonde      [20] True Blonde\n\r");
   strcat(buf,  "[21] Soft Red    [22] Fair Red    [23] Red         [24] Dark Red    [25] Pure Red\n\r");
   strcat(buf,  "[26] Soft Green  [27] Fair Green  [28] Green       [29] Dark Green  [30] Pure Green\n\r");
   strcat(buf,  "[31] Soft Blue   [32] Fair Blue   [33] Blue        [34] Dark Blue   [35] Pure Blue\n\r");
   strcat(buf,  "[36] Soft Purple [37] Fair Purple [38] Purple      [28] Dark Purple [40] Pure Purple\n\r");
   return &buf[0];
}

char *get_hair_length()
{
   static char buf[MIL];
   
   sprintf(buf, "\n\r[1]  Bald          [2]  Freshly Shaved [3]  Light Stubble [4]  Crew Cut    [5]  Short\n\r");     
   strcat(buf,  "[6]  Temple Length [7]  Below Ears     [8]  Shoulders     [9]  Upper Back  [10] Middle Back\n\r");
   strcat(buf,  "[11] Lower Back    [12] Legs           [13] Ground\n\r");
   return &buf[0];
}

char *get_hair_style(CHAR_DATA *ch)
{
   static char buf[MIL];
   if (ch->pcdata->hairlength >=1 && ch->pcdata->hairlength <= 3)
   {
      sprintf(buf, "\n\r[1]  Straight Thin [2]  Straight Normal [3]  Straight Thick  [4]  Curled Thin   [5]  Curled Normal\n\r");
      strcat(buf,  "[6]  Curled Thick\n\r");    
   }
   if (ch->pcdata->hairlength >=4 && ch->pcdata->hairlength <= 6)
   {
      sprintf(buf, "\n\r[1]  Straight Thin [2]  Straight Normal [3]  Straight Thick  [4]  Curled Thin   [5]  Curled Normal\n\r");
      strcat(buf,  "[6]  Curled Thick  [7]  Permed          [8]  Spiked          [9]  Mohawk\n\r");
   } 
   if (ch->pcdata->hairlength >= 7 && ch->pcdata->hairlength <= 9)
   {
      sprintf(buf, "\n\r[1]  Straight Thin [2]  Straight Normal [3]  Straight Thick  [4]  Curled Thin   [5]  Curled Normal\n\r");
      strcat(buf,  "[6]  Curled Thick  [7]  Permed          [8]  Spiked          [9]  Mohawk        [10]  Mullet Straight\n\r"); 
      strcat(buf,  "[11] Mullet Curled [12] Mullet Permed   [13] Dreadlocks      [14] Braided       [15] Afro\n\r");
   }
   if (ch->pcdata->hairlength > 9)
   {
      sprintf(buf, "\n\r[1]  Straight Thin [2]  Straight Normal [3]  Straight Thick  [4]  Curled Thin   [5]  Curled Normal\n\r");
      strcat(buf,  "[6] Curled Thick   [7]  Permed          [10] Mullet Straight [11] Mullet Curled [12] Mullet Permed\n\r");
      strcat(buf,  "[13] Dreadlocks    [14] Braided\n\r");
   }
   return &buf[0];
}

char *get_eye_color()
{
   static char buf[MIL];
   
   sprintf(buf, "\n\r[1]  Dull Blue   [2]  Blue   [3]  Radiant Blue   [4]  Dull Green  [5]  Green  [6]  Radiant Green\n\r");
   strcat(buf,  "[7]  Dull Brown  [8]  Brown  [9]  Radiant Brown  [10]  Dull Red    [11] Red    [12] Radiant Red\n\r");
   strcat(buf,  "[13] Dull Black  [14] Black  [15] Radiant Black  [16] Dull White  [17] White  [18] Radiant White\n\r");
   strcat(buf,  "[19] Dull Yellow [20] Yellow [21] Radiant Yellow [22] Dull Purple [23] Purple [24] Radiant Purple\n\r");
   return &buf[0];
}

char *get_char_height()
{
   static char buf[MIL];
   
   sprintf(buf, "\n\r[1] Extremely Short      [2]  Moderately Short       [3]  Adequately Short\n\r");    
   strcat(buf,  "[4] Short                [5]  Below Average          [6]  Slightly below Average\n\r"); 
   strcat(buf,  "[7] Average              [8]  Slightly Above Average [9]  Above Average\n\r");    
   strcat(buf,  "[10] Tall                [11] Adequately Towering   [12] Moderately Towering\n\r"); 
   strcat(buf,  "[13] Extremely Towering\n\r");
   return &buf[0];
}

char *get_char_weight()
{
   static char buf[MIL];
   
   sprintf(buf, "\n\r[1]  Extremely Slender  [2]  Moderately Slender     [3]  Adequately Slender\n\r");    
   strcat(buf,  "[4]  Slender            [5]  Below Average          [6]  Slightly below Average\n\r"); 
   strcat(buf,  "[7]  Average            [8]  Slightly Above Average [9]  Above Average\n\r");    
   strcat(buf,  "[10] Thick              [11] Adequately Thick      [12] Moderately Thick\n\r"); 
   strcat(buf,  "[13] Extremely Thick\n\r");
   return &buf[0];
}

int get_heightweight_percent(int value)
{
   if (value == 1)
      return 80;
   if (value == 2)
      return 85;
   if (value == 3)
      return 88;
   if (value == 4)
      return 92;
   if (value == 5)
      return 95;
   if (value == 6)
      return 97;
   if (value == 7)
      return 100;
   if (value == 8)
      return 104;
   if (value == 9)
      return 108;
   if (value == 10)
      return 112;
   if (value == 11)
      return 115;
   if (value == 12)
      return 118;
   if (value == 13)
      return 122;
      
   return 100;
}

char *clean_message(char *str)
{
   static char cleanstring[1000];
   int x = 0;
   
   strcpy(cleanstring, "");
   
   for (;;)
   {
      if (*str == '\0')
      {
         cleanstring[x] = *str;
         return &cleanstring[0];
      }
      if (*str != '\r')
      {
         cleanstring[x]= *str;
         str++;
         x++;
      }
      else
         str++;
   }
   return NULL;
}  

void send_email(char *subject, char *email, char *message, CHAR_DATA *ch, DESCRIPTOR_DATA *d)
{
   static char sendstring[1000];
   FILE *fp;
   FILE *mfp;
   DESCRIPTOR_DATA *nd = NULL;
   #define MAIL_ROOT_DIR "/bin/mail"
   
   
   if (sysdata.accountemail == 0 || sysdata.accountemail == 2)
   {
      if (ch && ch->desc)
         nd = ch->desc;
      else if (d)
         nd = d;
         
      if (nd && (sysdata.accountemail == 0 || (sysdata.accountemail == 2 && nd->account->noemail == 1)))
      {
         if (ch && ch->desc)
         {
            write_to_buffer(ch->desc, message, 0);
            return;
         }
         else if (d)
         {
            write_to_buffer(d, message, 0);
            return;
         }
      }
   }
   
   strcpy(sendstring, "");
   
   message = clean_message(message); //gets rid of the /r in the message if there are any..
   fp = fopen(EMAIL_FILE, "w");
   fprintf(fp, "%s", message);
   fclose(fp);
   //sprintf(sendstring, "mail -s \"%s\"  \"%s\" < %s", subject, email, EMAIL_FILE);
   //system(&sendstring[0]);
   sprintf(sendstring, "%s -s \"%s\"  \"%s\" < %s", MAIL_ROOT_DIR, subject, email, EMAIL_FILE);
   if ((mfp = popen( sendstring, "w" )) == NULL)
   {
      if (ch)
         send_to_char("The message was not sent because the mail program could not be found.\n\r", ch);
      bug("send_email:  Could not location mail.");
      return;
   }
   pclose(mfp); 
   remove(EMAIL_FILE);
   if (ch)
      ch_printf(ch, "Your email has been sent to %s\n\r", email);
}
   
int check_email_syntax(char *arg1, char *arg2)
{
   int x;
   
   for (x=0;;x++)
   {
      if (arg1[x] == 34 || arg1[x] == 39) // a " and a '
         return FALSE;
      if (arg1[x] == '\0')
         break;
   }
   for (x=0;;x++)
   {
      if (arg2[x] == 34 || arg2[x] == 39) // a " and a '
         return FALSE;
      if (arg2[x] == '\0')
         break;
   }
   return TRUE;
}

void do_sendmail(CHAR_DATA *ch, char *argument)
{
   char arg1[100];
   char arg2[100];
   static char *passargument;
   
   if (ch->dest_buf)
   {
      argument = ch->dest_buf;
   }
   else
   {
      passargument = "";
   }
      
   passargument = argument;
   
   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  sendmail \"<subject>\" \"<recepient's email address>\"\n\r", ch);
      send_to_char("Once that is typed, you will be sent into the buffer.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   
   if (arg1[0] == '\0')
   {
      send_to_char("You need to supply a subject.\n\r", ch);
      return;
   }
   if (arg2[0] == '\0')
   {
      send_to_char("You need to supply an email address.\n\r", ch);
      return;
   }
   
   if (strlen(arg1) > 95)
   {
      send_to_char("Subject cannot be longer than 95 characters.\n\r", ch);
      return;
   }
   if (strlen(arg2) > 95)
   {
      send_to_char("recepient's email address cannot be longer than 95 characters.\n\r", ch);
      return;
   }
   
   if (!check_email_syntax(arg1, arg2))
   {
      send_to_char("You cannot supply any \" or \' in your subject or recepient.\n\r", ch);
      return;
   }
   
   switch (ch->substate)
   {
      default:
         bug("do_description: illegal substate", 0);
         return;

      case SUB_RESTRICTED:
         send_to_char("You cannot use this command from within another command.\n\r", ch);
         return;

      case SUB_NONE:
         if (ch->pcdata->sendmail)
            STRFREE(ch->pcdata->sendmail);
         ch->pcdata->sendmail = STRALLOC("");
         ch->substate = SUB_WRITING_EMAIL;
         ch->dest_buf = passargument;
         start_editing(ch, ch->pcdata->sendmail);
         return;

      case SUB_WRITING_EMAIL:
         STRFREE(ch->pcdata->sendmail);
         passargument = "";
         ch->pcdata->sendmail = copy_buffer(ch);
         send_email(&arg1[0], &arg2[0], ch->pcdata->sendmail, ch, NULL);
         bug("-------------%s is sending an email to %s-------------\n\r", ch->name, arg2);
         stop_editing(ch);
         return;
   }
}

char *generate_new_pass()
{
   static char passwd[8];
   int x;
   int num;
   
   for (x = 0; x <= 6; x++)
   {
      num = number_range(1, 3);
      if (num == 1)
         passwd[x] = number_range(48, 57); //numbers
      else if (num == 2)
         passwd[x] = number_range(65, 90); //uppercase letters
      else
         passwd[x] = number_range(97, 122); //lowercase letters
   }
   passwd[x] = '\0';
   return passwd;
}

void close_area args((AREA_DATA * pArea));

void delete_character(DESCRIPTOR_DATA *d, CHAR_DATA * ch)
{
   char buf[MSL];
   char buf2[MSL];
   char *name;

   set_char_color(AT_RED, ch);
   
   if (ch->last_name && ch->name)
   {
      remove_from_lastname_file(ch->last_name, ch->name); //remove the player from the lastname file
   }
   else
      bug("do_destroy: That player's lastname could not be removed, need to do it manually");

   name = capitalize(ch->name);
   sprintf(buf, "%s%c/%s", PLAYER_DIR, tolower(name[0]), name);
   sprintf(buf2, "%s%c/%s", BACKUP_DIR, tolower(name[0]), name);
   if (!rename(buf, buf2))
   {
      AREA_DATA *pArea;

      set_char_color(AT_RED, ch);
      sprintf(buf2, "Player %s destroyed.  Pfile saved in backup directory.\n\r", name);
      write_to_buffer(d, buf2, 0);
      sprintf(buf, "%s%s", GOD_DIR, name);
      if (!remove(buf))
         write_to_buffer(d, "Player's immortal data destroyed.\n\r", 0);
      else if (errno != ENOENT)
      {
         sprintf(buf2, "Unknown error #%d - %s (immortal data).  Report to the Administrator.\n\r", errno, strerror(errno));
         write_to_buffer(d, buf2, 0);
         sprintf(buf2, "%s destroying %s", ch->name, buf);
         perror(buf2);
      }

      sprintf(buf2, "%s.are", name);
      for (pArea = first_build; pArea; pArea = pArea->next)
      {
         if (!str_cmp(pArea->filename, buf2))
         {
            sprintf(buf, "%s%s", BUILD_DIR, buf2);
            if (IS_SET(pArea->status, AREA_LOADED))
               fold_area(pArea, buf, FALSE, 0);
            close_area(pArea);
            sprintf(buf2, "%s.bak", buf);
            set_char_color(AT_RED, ch); /* Log message changes colors */
            if (!rename(buf, buf2))
            {
               write_to_buffer(d, "Player's area data destroyed.  Area saved as backup.\n\r", 0);
            }
            else if (errno != ENOENT)
            {
               sprintf(buf2, "Unknown error #%d - %s (area data).  Report to the Administrator.\n\r", errno, strerror(errno));
               write_to_buffer(d, buf2, 0);
               sprintf(buf2, "%s destroying %s", ch->name, buf);
               perror(buf2);
            }
            break;
         }
      }
   }
   else if (errno == ENOENT)
   {
      set_char_color(AT_PLAIN, ch);
      write_to_buffer(d, "Player does not exist.\n\r", 0);
   }
   else
   {
      set_char_color(AT_WHITE, ch);
      sprintf(buf2, "Unknown error #%d - %s.  Report to the Administrator.\n\r", errno, strerror(errno));
      write_to_buffer(d, buf2, 0);
      sprintf(buf, "Account %s deleted %s", d->account->name, ch->name);
      perror(buf);
   }
   return;
}
   
void send_account_menu(DESCRIPTOR_DATA *d)
{
   char buf[220];
   
   if (d->account)
   {
      if (d->account->qplayer1[0] != '\0')
      {
         sprintf(buf, "[1] Login %-13s   ", d->account->qplayer1);
         write_to_buffer(d, buf, 0);
      }
      else
      {
         write_to_buffer(d, "[1] Empty                 ", 0);
      }
      if (d->account->qplayer2[0] != '\0')
      {
         sprintf(buf, "[2] Login %-13s   \n\r", d->account->qplayer2);
         write_to_buffer(d, buf, 0);
      }
      else
      {
         write_to_buffer(d, "[2] Empty\n\r", 0);
      }
      if (d->account->qplayer3[0] != '\0')
      {
         sprintf(buf, "[3] Login %-13s   ", d->account->qplayer3);
         write_to_buffer(d, buf, 0);
      }
      else
      {
         write_to_buffer(d, "[3] Empty                 ", 0);
      }
      if (d->account->qplayer4[0] != '\0')
      {
         sprintf(buf, "[4] Login %-13s   \n\r", d->account->qplayer4);
         write_to_buffer(d, buf, 0);
      }
      else
      {
         write_to_buffer(d, "[4] Empty\n\r", 0);
      }
   }      
   write_to_buffer(d, "\n\r[C] Create a New Player   [I] Import an old Player\n\r", 0);
   write_to_buffer(d, "[D] Delete a Player       [P] Change your password\n\r", 0);
   write_to_buffer(d, "[L] Login with a Player   [S] Get a List of your Players\n\r", 0);
   write_to_buffer(d, "[Q] Logout                [K] Delete this Account\n\r", 0);
   write_to_buffer(d, "[E] Change your email     [R] Release a Player\n\r", 0);
   write_to_buffer(d, "[B] Set Bypass Login      [M] Toggle Player Menu\n\r", 0);   
   if (sysdata.accountemail == 2)
      write_to_buffer(d, "[N] Toggle NoEmail Status \n\r", 0);     
}

//In order to make changes you can be the only one logged in.
bool check_connected_accounts(DESCRIPTOR_DATA *d)
{
   DESCRIPTOR_DATA *od;
   
   for (od = first_descriptor; od; od = od->next)
   {
      if (od != d && od->account && !str_cmp(od->account->name, d->account->name))
      {
         write_to_buffer(d, "You cannot make changes to your account while others are logged on to it.\n\r"
                            "Have everyone disconnect then try again.\n\r\n\r", 0);
         send_account_menu(d);
         return 0;
      }
   }
   if (d->account->changes >= sysdata.max_account_changes)
   {
      write_to_buffer(d, "You have made too many changes this hour, wait till the next hour to make more.\n\r\n\r", 0);
      send_account_menu(d);
      return 0;
   }
   d->account->changes++;
   for (od = first_descriptor; od; od = od->next)
   {
      if (od != d && od->account && !str_cmp(od->account->name, d->account->name))
      {
         od->account->changes++;
      }
   }
   //Don't really need this editing line, but oh well
   d->account->editing = 1;
   save_account(d, 1);
   return 1;
}

void save_char_account(CHAR_DATA *ch, int value, char *password)
{
   char strsave[MIL];
   char strcopy[MIL];
   FILE *fp;
   FILE *wfp;
   char *ln;
   int pnext = 0;
   
   sprintf(strsave, "%s%c/%s", PLAYER_DIR, tolower(ch->pcdata->filename[0]), capitalize(ch->pcdata->filename));
   if ((fp = fopen(strsave, "r")) == NULL)
   {
      bug("Save_char_account: fopen", 0);
      perror(strsave);
   }
   sprintf(strcopy, "%s%c/%s.ac", PLAYER_DIR, tolower(ch->pcdata->filename[0]), capitalize(ch->pcdata->filename));
   wfp = fopen(strcopy, "w");
   for (;;)
   {
      if (feof(fp))
         break;
      ln = fread_line(fp);
      if (!strncmp(ln, "Password", 8))
      {
         if (value == 1)
         {
            fprintf(wfp, "Password     %s~\n", password);
            pnext = 0;
            continue;
         }
         else
         {
            pnext = 0;
            continue;
         }
      }
      if (pnext == 1)
      {
         if (value == 1)
         {
            fprintf(wfp, "Password     %s~\n", password);
            pnext = 0;
         }
         else
            pnext = 0;
      }
      if (!strncmp(ln, "Title", 5))
         pnext = 1;
      fprintf(wfp, "%s", ln);
   }
   fclose(wfp);
   fclose(fp);
   remove(strsave);
   rename(strcopy, strsave);
}

void alter_all_forge_items(CHAR_DATA *ch, OBJ_DATA *sobj)
{
   OBJ_DATA *obj;
   OBJ_DATA *slabobj;
   SLAB_DATA *slab;
   char arg1[MIL];
   
   for (obj = sobj; obj; obj = obj->next_content) 
   {
      if (xIS_SET(obj->extra_flags, ITEM_FORGEABLE))
      {
         one_argument(obj->name, arg1);
         for (slab = first_slab; slab; slab = slab->next)
         {
            if (!str_cmp(slab->adj, arg1))
            {
               slabobj = create_object(get_obj_index(slab->vnum), 1);
               ore_alter(ch, obj, slabobj);
               extract_obj(slabobj);
            }
         }
      }
      if (obj->first_content)
          alter_all_forge_items(ch, obj->first_content);
   }   
}
int get_maxstat args((CHAR_DATA *ch, int stat));

void prepare_to_login(CHAR_DATA *ch, DESCRIPTOR_DATA *d)
{
   char motdbuf[MSL];
   char buf[MSL];
   QUEST_DATA *quest;
   
   sprintf(motdbuf, "\n\rWelcome to %s...\n\r", sysdata.mud_name);
   write_to_buffer(d, motdbuf, 0);
 
   add_char(ch);
   d->connected = CON_PLAYING;
   if (ch->level == 0)
   {
      OBJ_DATA *obj;

      ch->pcdata->clan_name = STRALLOC("");
      ch->pcdata->clan = NULL;
      /*
         ch->alignment  += race_table[ch->race]->alignment;
         --Taken out for now, might put something back in a bit later..Xerves 8/1/99 */
      ch->attacks = race_table[ch->race]->attacks;
      ch->defenses = race_table[ch->race]->defenses;
      ch->saving_poison_death = race_table[ch->race]->saving_poison_death;
      ch->saving_wand = race_table[ch->race]->saving_wand;
      ch->saving_para_petri = race_table[ch->race]->saving_para_petri;
      ch->saving_breath = race_table[ch->race]->saving_breath;
      ch->saving_spell_staff = race_table[ch->race]->saving_spell_staff;

      ch->pcdata->hometown = 0; //Automatic part of Rafermand City
      ch->pcdata->authwait = 2; // 1 to 2 minute Auth Wait.

      /* Moved up top so you cannot pick these two languages out of your selection
         -- Xerves 2/00

      if ( (iLang = skill_lookup( "common" )) < 0 )
         bug( "Nanny: cannot find common language." );
      else
      {
         ch->pcdata->learned[iLang] = 20;
         ch->pcdata->ranking[iLang] = 4;
      }
      for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
         if ( lang_array[iLang] == race_table[ch->race]->language )
             break;
      if ( lang_array[iLang] == LANG_UNKNOWN )
         bug( "Nanny: invalid racial language." );
      else
      {
         if ( (iLang = skill_lookup( lang_names[iLang] )) < 0 )
            bug( "Nanny: cannot find racial language." );
         else
         {
            ch->pcdata->learned[iLang] = 20;
            ch->pcdata->ranking[iLang] = 4;
         }
      }  */

      /* ch->resist           += race_table[ch->race]->resist;    drats */
      /* ch->susceptible     += race_table[ch->race]->suscept;    drats */
      if (ch->pcdata->tier < 2)
         reset_colors(ch);
         
      /*name_stamp_stats( ch ); -- Removed for Statrolled -- Xerves */

      ch->level = 1;

      /* Added by Narn.  Start new characters with autoexit and autgold
         already turned on.  Very few people don't use those. */
      xSET_BIT(ch->act, PLR_AUTOGOLD);
      xSET_BIT(ch->act, PLR_AUTOEXIT);
      //Set the pid, used to identify players in RP and other stuff, DO NOT REMOVE
      sysdata.top_pid++;
      ch->pcdata->pid = sysdata.top_pid;
      save_sysdata(sysdata);
      if (xIS_SET(ch->act, PLR_REMORT))
         xREMOVE_BIT(ch->act, PLR_REMORT);
      /* Added by Brittany, Nov 24/96.  The object is the adventurer's guide
         to the realms of despair, part of Academy.are. */
      {
         OBJ_INDEX_DATA *obj_ind;
         OBJ_DATA *sobj = NULL;
         OBJ_DATA *backpack = NULL;
         int lcnt;

         if (first_slab)
            sobj = create_object(get_obj_index(first_slab->vnum), 1);

         obj_ind = NULL;
         //Brown Cloak
         obj_ind = get_obj_index(21241);
         if (obj_ind != NULL)
         {
            obj = create_object(obj_ind, 0);
            obj_to_char(obj, ch);
            equip_char(ch, obj, WEAR_BACK);
         }
         obj_ind = NULL;
         //Lantern
         obj_ind = get_obj_index(21211);
         if (obj_ind != NULL)
         {
            obj = create_object(obj_ind, 0);
            obj_to_char(obj, ch);
            equip_char(ch, obj, WEAR_LIGHT);
         }
         obj_ind = NULL;
         //Backpack
         obj_ind = get_obj_index(21221);
         if (obj_ind != NULL)
         {
            obj = create_object(obj_ind, 0);
            obj_to_char(obj, ch);
            backpack = obj;
         }
         obj_ind = NULL;
         //beef slabs for food
         obj_ind = get_obj_index(21218);
         for (lcnt = 1; lcnt <= 10; lcnt++)
         {
            if (obj_ind != NULL)
            {
               obj = create_object(obj_ind, 0);
               if (backpack)
                  obj_to_obj(obj, backpack);
               else
                  obj_to_char(obj, ch);
            }
         }
         obj_ind = NULL;
         //Dragonskin for water
         obj_ind = get_obj_index(21212);
         if (obj_ind != NULL)
         {
            obj = create_object(obj_ind, 0);
            if (backpack)
               obj_to_obj(obj, backpack);
            else
               obj_to_char(obj, ch);
         }
         obj_ind = NULL;
         //Katana
         obj_ind = get_obj_index(21010);
         if (obj_ind != NULL)
         {
            obj = create_object(obj_ind, 0);
            obj_to_char(obj, ch);
            if (first_slab)
            {
               alter_forge_obj(ch, obj, sobj, first_slab);	
            }
         }
         obj_ind = NULL;
         //Hammer
         obj_ind = get_obj_index(21031);
         if (obj_ind != NULL)
         {
            obj = create_object(obj_ind, 0);
            obj_to_char(obj, ch);
            if (first_slab && sobj)
            {
               alter_forge_obj(ch, obj, sobj, first_slab);	
            }
         }
         obj_ind = NULL;
         //Kris
         obj_ind = get_obj_index(21019);
         if (obj_ind != NULL)
         {
            obj = create_object(obj_ind, 0);
            obj_to_char(obj, ch);
            if (first_slab && sobj)
            {
               alter_forge_obj(ch, obj, sobj, first_slab);	
            }
         }
         obj_ind = NULL;
         //Round Shield
         obj_ind = get_obj_index(21068);
         if (obj_ind != NULL)
         {
            obj = create_object(obj_ind, 0);
            obj_to_char(obj, ch);
            if (first_slab && sobj)
            {
               alter_forge_obj(ch, obj, sobj, first_slab);	
            }
         }
         obj_ind = NULL;
         //Chain Mail
         obj_ind = get_obj_index(21045);
         if (obj_ind != NULL)
         {
            obj = create_object(obj_ind, 0);
            obj_to_char(obj, ch);
            if (first_slab && sobj)
            {
               alter_forge_obj(ch, obj, sobj, first_slab);	
            }
            equip_char(ch, obj, WEAR_BODY);
         }

      }
      if (!sysdata.WAIT_FOR_AUTH)
         char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL));
      else
      {
         char_to_room(ch, get_room_index(ROOM_AUTH_START));
         ch->pcdata->auth_state = 1;
         SET_BIT(ch->pcdata->flags, PCFLAG_UNAUTHED);
         auth_update();
      }
   }
   else if (!IS_IMMORTAL(ch) && ch->pcdata->release_date > 0 && ch->pcdata->release_date > current_time)
   {
      if (ch->in_room->vnum == 6 || ch->in_room->vnum == 8 || ch->in_room->vnum == 1206)
         char_to_room(ch, ch->in_room);
      else
         char_to_room(ch, get_room_index(8));
   }
   else if (ch->in_room)
   {
      if( xIS_SET(ch->in_room->room_flags, ROOM_FORGEROOM))
      {
         char_to_room(ch, get_room_index(ROOM_VNUM_TEMPLE));
         if (ch->pcdata->mount)
         {
            char_from_room(ch->pcdata->mount);
            char_to_room(ch->pcdata->mount, ch->in_room);
         }
      }
      else
      {
         char_to_room(ch, ch->in_room);
      }
   }
   else if (IS_IMMORTAL(ch))
   {
      char_to_room(ch, get_room_index(ROOM_VNUM_CHAT));
   }
   else
   {
      char_to_room(ch, get_room_index(ROOM_VNUM_TEMPLE));
   }
   if (IN_WILDERNESS(ch) && !IS_IMMORTAL(ch))
   {
      if (ch->coord->x >= 1040 && ch->coord->x <= 1500 && ch->coord->y >= 430 && ch->coord->y <= 940)
      {
         ch->coord->x -= 1000;
      }
   }

   if (get_timer(ch, TIMER_SHOVEDRAG) > 0)
      remove_timer(ch, TIMER_SHOVEDRAG);

   if (xIS_SET(ch->act, PLR_PKRESET)) //Just toggle this flag if you EVER need to reset pk status mud wide, it works well
   {
      ch->pcdata->pkills = 0;
      ch->pcdata->pdeaths = 0;
      ch->pcdata->pranking = 0; 
      //PLEASE NOTE!!!!  Do not reset pkpower, it will update itself automatically
      xREMOVE_BIT(ch->act, PLR_PKRESET);
   }
   if (xIS_SET(ch->act, PLR_KRESET)) //Used now to fix forge equipment on players, should be set if you have played before
   {
      xREMOVE_BIT(ch->act, PLR_KRESET);
      alter_all_forge_items(ch, ch->first_carrying);
   }
   //str dex int wis con agi
   //Need to fix stats from the old system.....
   
   if (ch->perm_str > get_maxstat(ch, 1))
      ch->perm_str = get_maxstat(ch, 1);
   if (ch->perm_dex > get_maxstat(ch, 2))
      ch->perm_dex = get_maxstat(ch, 2);
   if (ch->perm_int > get_maxstat(ch, 3))
      ch->perm_int = get_maxstat(ch, 3);
   if (ch->perm_wis > get_maxstat(ch, 4))
      ch->perm_wis = get_maxstat(ch, 4);
   if (ch->perm_con > get_maxstat(ch, 5))
      ch->perm_con = get_maxstat(ch, 5);
   if (ch->perm_con > get_maxstat(ch, 6))
      ch->perm_con = get_maxstat(ch, 6);
   if (ch->perm_agi > get_maxstat(ch, 7))
      ch->perm_agi = get_maxstat(ch, 7);
      
   if (ch->perm_str < (14 + race_table[ch->race]->str_plus - 5 + race_table[ch->race]->str_range))
      ch->perm_str = 14 + race_table[ch->race]->str_plus - 5 + race_table[ch->race]->str_range;
   if (ch->perm_dex < (14 + race_table[ch->race]->dex_plus - 5 + race_table[ch->race]->dex_range))
      ch->perm_dex = 14 + race_table[ch->race]->dex_plus - 5 + race_table[ch->race]->dex_range;
   if (ch->perm_con < (14 + race_table[ch->race]->con_plus - 5 + race_table[ch->race]->con_range))
      ch->perm_con = 14 + race_table[ch->race]->con_plus - 5 + race_table[ch->race]->con_range;
   if (ch->perm_int < (14 + race_table[ch->race]->int_plus - 5 + race_table[ch->race]->int_range))
      ch->perm_int = 14 + race_table[ch->race]->int_plus - 5 + race_table[ch->race]->int_range;
   if (ch->perm_wis < (14 + race_table[ch->race]->wis_plus - 5 + race_table[ch->race]->wis_range))
      ch->perm_wis = 14 + race_table[ch->race]->wis_plus - 5 + race_table[ch->race]->wis_range;
   if (ch->perm_agi < (race_table[ch->race]->agi_start - race_table[ch->race]->agi_range))
      ch->perm_agi = race_table[ch->race]->agi_start - race_table[ch->race]->agi_range;
      
   //Reattach a quest if the player is in one.
   for (quest = first_quest; quest; quest = quest->next)
   {
      int x;
      for (x = 0; x <= 5; x++)
      {
         if (quest->player[x] == ch->pcdata->pid)
            ch->pcdata->quest = quest;
      }
   }
      
   update_interest(ch); //Updates bank interest
   if (ch->pcdata->pid == 0)
   {
      sysdata.top_pid++;
      ch->pcdata->pid = sysdata.top_pid;
      save_sysdata(sysdata);
   }
   
   /* Rantic's info channel */
   if (!IS_IMMORTAL(ch) || (IS_IMMORTAL(ch) && !xIS_SET(ch->act, PLR_WIZINVIS)))
   {
      sprintf(buf, "You feel the presence of another soul entering %s...", sysdata.mud_name);
      talk_info(AT_BLUE, buf);
   }


   act(AT_ACTION, "$n has entered the game.", ch, NULL, NULL, TO_CANSEE);
   if (ch->pcdata->pet)
   {
      act(AT_ACTION, "$n returns to $s master from the Void.", ch->pcdata->pet, NULL, ch, TO_NOTVICT);
      act(AT_ACTION, "$N returns with you to the realms.", ch, NULL, ch->pcdata->pet, TO_CHAR);
   }
   if (ch->pcdata->mount)
   {
      act(AT_ACTION, "$n returns to $s master from the Void.", ch->pcdata->mount, NULL, ch, TO_NOTVICT);
      act(AT_ACTION, "$N returns with you to the realms.", ch, NULL, ch->pcdata->mount, TO_CHAR);
   }
   do_global_boards(ch, "");
   do_look(ch, "auto");
   ch_printf(ch, "%s", MXPTAG("VERSION"));
   if (ch->level > 0)
   {
      INTRO_DATA *intro;
      if (ch->pcdata->kingdompid > 1)
      {
         int kx;
            
         if (ch->pcdata->hometown >= sysdata.max_kingdom)
         {
            ch->pcdata->hometown = 0;
            ch->pcdata->kingdompid = 0;
            ch->pcdata->town = NULL;
         }
         if (ch->pcdata->kingdompid != kingdom_table[ch->pcdata->hometown]->kpid)
         {
            for (kx = 2; kx < sysdata.max_kingdom; kx++)
            {
               if (ch->pcdata->kingdompid == kingdom_table[kx]->kpid)
               {
                  ch->pcdata->hometown = kx;
                  break;
               }
            }
            if (kx == sysdata.max_kingdom)
            {
               sprintf(motdbuf, "&R*************************************************\n\rIt appears your kingdom has been destroyed!!!!!!!!\n\r*************************************************\n\r");
               send_to_char(motdbuf, ch);
               ch->pcdata->hometown = 0;
               ch->pcdata->kingdompid = 0;
               ch->pcdata->town = NULL;
            }
         }
      } 
      if (ch->pcdata->hometown >= sysdata.max_kingdom)
      {
         ch->pcdata->hometown = 0;
         ch->pcdata->kingdompid = 0;
         ch->pcdata->town = NULL;
      }
      if (ch->pcdata->kingdompid == 0 && ch->pcdata->hometown != 0)
      {
         ch->pcdata->kingdompid = kingdom_table[ch->pcdata->hometown]->kpid;
      }
      for (intro = kingdom_table[ch->pcdata->hometown]->first_introduction; intro; intro = intro->next)
      {
         if (ch->pcdata->pid == intro->pid)
         {
            intro->value = 150000;
            intro->lastseen = time(0);
            break;
         }
      }
      if (!intro)
      {
         CREATE(intro, INTRO_DATA, 1);
         intro->value = 150000;
         intro->lastseen = time(0);
         intro->pid = ch->pcdata->pid;
         LINK(intro, kingdom_table[ch->pcdata->hometown]->first_introduction, kingdom_table[ch->pcdata->hometown]->last_introduction, next, prev);   
      }
   }
   if (ch->pcdata->hometown > 1)
   {
      TOWN_DATA *town = NULL;
          
      if (ch->pcdata->town)
         town = get_town(ch->pcdata->town->name);
            
      if (!town || town->kingdom != ch->pcdata->hometown)//no hometown
      {
         send_to_char("&R*******************************************************\n\rYour town has been destroyed, putting you in the default town if your kingdom still exists.\n\r*******************************************************\n\r", ch);
         town = get_town(kingdom_table[ch->pcdata->hometown]->dtown);
         if (town)
            ch->pcdata->town = town;
         else
            ch->pcdata->town = NULL;
      }
   } 
   tax_player(ch); /* Here we go, let's tax players to lower the gold
                      pool -- TRI */
   mail_count(ch);

   if (!ch->was_in_room && ch->in_room == get_room_index(ROOM_VNUM_TEMPLE))
      ch->was_in_room = get_room_index(ROOM_VNUM_TEMPLE);
   else if (ch->was_in_room == get_room_index(ROOM_VNUM_TEMPLE))
      ch->was_in_room = get_room_index(ROOM_VNUM_TEMPLE);
   else if (!ch->was_in_room)
      ch->was_in_room = ch->in_room;
}

void find_next_con(CHAR_DATA *ch, DESCRIPTOR_DATA *d)
{
   char buf[MSL];
   
   if (chk_watch(get_trust(ch), ch->name, d->host)) /*  --Gorog */
      SET_BIT(ch->pcdata->flags, PCFLAG_WATCH);
   else
      REMOVE_BIT(ch->pcdata->flags, PCFLAG_WATCH);
   set_char_color(AT_DGREEN, ch);
   if (d->account->skiplmenu == 0)
   {
      sprintf(buf, "\n\r\n\r\n\rWelcome to Rafermand, please choose an option from the menu.\n\r\n\r");
      if (IS_IMMORTAL(ch))
      {
         strcat(buf, "[W] Who is online\n\r[I] Go Wizinvis\n\r[G] Read Gboard notes\n\r[N] News\n\r[L] Login\n\r> ");
      }
      else
      {
         strcat(buf, "[W] Who is online\n\r[G] Read Gboard notes\n\r[N] News\n\r[L] Login\n\r> ");
      }
      write_to_buffer(d, buf, 0);
      d->connected = CON_LOGIN_MENU;
      return;
   }
   else
   {      
      if (xIS_SET(ch->act, PLR_RIP))
         send_rip_screen(ch);
      if (xIS_SET(ch->act, PLR_ANSI))
         send_to_pager("\033[2J", ch);
      else
         send_to_pager("\014", ch);
      prepare_to_login(ch, d);
      return;
   }
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !strcmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

int fread_resetchar(CHAR_DATA * ch, bool preload)
{
   char buf[MSL];
   FILE *fp;
   char *word;
   int max_colors = 0; /* Color code */
   bool fMatch;
      
   sprintf(buf, "%s%c/%s", RESET_DIR, tolower(ch->name[0]), capitalize(ch->name));
   if ((fp = fopen(buf, "r")) == NULL)
   {
      bug("Could not pull up reset char %s", ch->name);
      write_to_buffer(ch->desc, "There was an error, could not read the pfile, tell an IMM.\n\r", 0);
      return 0;
   }

   /* Setup color values in case player has none set - Samson */
   memcpy(&ch->pcdata->colors, &default_set, sizeof(default_set));
   for (;;)
   {
      word = feof(fp) ? "End" : fread_word(fp);
      if (!str_cmp(word, "#PLAYER"))
         continue;
      fMatch = FALSE;

      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;

         case 'A':
            if (!str_cmp(word, "Alias"))
            {
               ALIAS_DATA *pal;

               if (preload)
               {
                  fMatch = TRUE;
                  fread_to_eol(fp);
                  break;
               }
               CREATE(pal, ALIAS_DATA, 1);

               pal->name = fread_string_nohash(fp);
               pal->cmd = fread_string_nohash(fp);
               LINK(pal, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev);
               fMatch = TRUE;
               break;
            }
            KEY("AuthedBy", ch->pcdata->authed_by, fread_string(fp));
            break;

         case 'B':
            /* Read in board status */
            if (!str_cmp(word, "Boards"))
            {
               int i, num = fread_number(fp); /* number of boards saved */
               char *boardname;

               for (; num; num--) /* for each of the board saved */
               {
                  boardname = fread_word(fp);
                  i = board_lookup(boardname); /* find board number */

                  if (i == BOARD_NOTFOUND) /* Does board still exist ? */
                  {
                     sprintf(buf, "fread_char: %s had unknown board name: %s. Skipped.", ch->name, boardname);
                     log_string(buf);
                     fread_number(fp); /* read last_note and skip info */
                  }
                  else /* Save it */
                     ch->pcdata->last_note[i] = fread_number(fp);
               } /* for */

               fMatch = TRUE;
            } /* Boards */
            KEY("Bio", ch->pcdata->bio, fread_string(fp));
            break;

         case 'C':
            KEY("CameFrom", ch->pcdata->came_from, fread_string(fp));
            KEY("Cheight", ch->pcdata->cheight, fread_number(fp));                
            KEY("Cweight", ch->pcdata->cweight, fread_number(fp));          

/* Load color values - Samson 9-29-98 */
            {
               int x;

               if (!str_cmp(word, "Colors"))
               {
                  for (x = 0; x < max_colors; x++)
                     ch->pcdata->colors[x] = fread_number(fp);
                  fMatch = TRUE;
                  break;
               }
            }
            break;

         case 'D':
            KEY("Description", ch->description, fread_string(fp));
            break;

            /* 'E' was moved to after 'S' */
         case 'F':
	        KEY("Fame", ch->fame, fread_number(fp));
            KEY("FPrompt", ch->pcdata->fprompt, fread_string(fp));
            break;


         case 'H':
            KEY("Haircolor", ch->pcdata->haircolor, fread_number(fp));
            KEY("Hairlength", ch->pcdata->hairlength, fread_number(fp));
            KEY("Hairstyle", ch->pcdata->hairstyle, fread_number(fp));
            KEY("Height", ch->height, fread_number(fp));
            KEY("Homepage", ch->pcdata->homepage, fread_string_nohash(fp));            
            break;

         case 'I':
            KEY("ICQ", ch->pcdata->icq, fread_number(fp));            
            break;

         case 'L':         
 	        KEY("LastName", ch->last_name, fread_string(fp));        
            KEY("Level", ch->level, fread_number(fp));           
            if (!strcmp(word, "Languages"))
            {
               ch->speaks = fread_number(fp);
               ch->speaking = fread_number(fp);
               fMatch = TRUE;
            }
            break;

         case 'M':
            KEY("MaxColors", max_colors, fread_number(fp));
            break;

         case 'N':
            KEY("Name", ch->name, fread_string(fp));
            break;

         case 'P':
            KEY("Pagerlen", ch->pcdata->pagerlen, fread_number(fp));
            KEY("Pid", ch->pcdata->pid, fread_number(fp));
            KEY("Played", ch->played, fread_number(fp));
            KEY("Prompt", ch->pcdata->prompt, fread_string(fp));
            break;

            
         case 'R':
            KEY("Race", ch->race, fread_number(fp));
            KEY("Righthanded", ch->pcdata->righthanded, fread_number(fp));
            break;

         case 'S':
            KEY("Sex", ch->sex, fread_number(fp));
            KEY("Skincolor", ch->pcdata->skincolor, fread_number(fp));
            if (!str_cmp(word, "Speed"))
            {
               ch->speed = fread_number(fp);
               if (ch->desc)
                  ch->desc->speed = ch->speed;
               fMatch = TRUE;
               break;
            }
            break;
            
         case 'T':
            KEY("Talent", ch->pcdata->talent, fread_bitvector(fp));

         case 'E':
            if (!strcmp(word, "End"))
            {
               if (!ch->short_descr)
                  ch->short_descr = STRALLOC("");
               if (!ch->long_descr)
                  ch->long_descr = STRALLOC("");
               if (!ch->description)
                  ch->description = STRALLOC("");
               if (!ch->pcdata->bamfin)
                  ch->pcdata->bamfin = str_dup("");
               if (!ch->pcdata->bamfout)
                  ch->pcdata->bamfout = str_dup("");
               if (!ch->pcdata->bio)
                  ch->pcdata->bio = STRALLOC("");
               if (!ch->pcdata->came_from)
                  ch->pcdata->came_from = STRALLOC("");
               if (!ch->pcdata->rank)
                  ch->pcdata->rank = str_dup("");
               if (!ch->pcdata->bestowments)
                  ch->pcdata->bestowments = str_dup("");
               if (!ch->pcdata->title)
                  ch->pcdata->title = STRALLOC("");
               if (!ch->pcdata->pretit)
                  ch->pcdata->pretit = str_dup("");

               if (!ch->pcdata->homepage)
                  ch->pcdata->homepage = str_dup("");
               if (!ch->pcdata->email)
                  ch->pcdata->email = str_dup("");
               if (!ch->pcdata->authed_by)
                  ch->pcdata->authed_by = STRALLOC("");
               if (!ch->pcdata->prompt)
                  ch->pcdata->prompt = STRALLOC("");
               if (!ch->pcdata->fprompt)
                  ch->pcdata->fprompt = STRALLOC("");
               ch->editor = NULL;

               /* no good for newbies at all */
               if (!IS_IMMORTAL(ch) && !ch->speaking)
                  ch->speaking = LANG_COMMON;
               /* ch->speaking = race_table[ch->race]->language; */
               if (IS_IMMORTAL(ch))
               {
                  int i;

                  ch->speaks = ~0;
                  if (ch->speaking == 0)
                     ch->speaking = ~0;

                  CREATE(ch->pcdata->tell_history, char *, 26);

                  for (i = 0; i < 26; i++)
                     ch->pcdata->tell_history[i] = NULL;
               }
               if (!ch->pcdata->prompt)
                  ch->pcdata->prompt = STRALLOC("");

               /* this disallows chars from being 6', 180lbs, but easier than a flag */
               if (ch->height == 72)
                  ch->height = number_range(race_table[ch->race]->height * .9, race_table[ch->race]->height * 1.1);
               if (ch->weight == 180)
                  ch->weight = number_range(race_table[ch->race]->weight * .9, race_table[ch->race]->weight * 1.1);

               REMOVE_PLR_FLAG(ch, PLR_MAPEDIT); /* In case they saved while editing */

               return 1;
            }
            KEY("Email", ch->pcdata->email, fread_string_nohash(fp));
            KEY("Elements", ch->elementb, fread_number(fp));
            KEY("Eyecolor", ch->pcdata->eyecolor, fread_number(fp));
            break;

         case 'V':
            KEY("Version", file_ver, fread_number(fp));
            break;

         case 'W':
            KEY("Weight", ch->weight, fread_number(fp));
            break;
      }

      if (!fMatch)
      {
         sprintf(buf, "Fread_reset_char: no match: %s", word);
         bug(buf, 0);
      }
   }
   return 0;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny(DESCRIPTOR_DATA * d, char *argument)
{
/*	extern int lang_array[];
	extern char *lang_names[];*/
   char buf[MSL];
   char arg[MSL];
   char arg2[MSL];
   CHAR_DATA *ch;
   char *pwdnew;
   char *p;
   int iRace;
   int iLang;
   bool fOld, chk;
   sh_int cont;
   ACCOUNT_NAME *aname;
   char fname[1024];
   struct stat fst;
   DESCRIPTOR_DATA *dd;
   char *strtime;

   while (isspace(*argument))
      argument++;

   cont = 0;
   ch = d->character;

   switch (d->connected)
   {

      default:
         bug("Nanny: bad d->connected %d.", d->connected);
         close_socket(d, TRUE);
         return;
         
      case CON_GET_ACCOUNT:
         if (argument[0] == '\0')
         {
            close_socket(d, FALSE);
            return;
         }

         argument = capitalize(argument);

         /* Old players can keep their characters. -- Alty */
         sprintf(fname, "%s%c/%s", ACCOUNT_DIR, tolower(argument[0]), capitalize(argument));
         if (d->newstate == 1 && stat(fname, &fst) != -1)
         {
            write_to_buffer(d, "That account name is already taken, please choose another.\n\rAccount Name: ", 0);
            return;
         }         
         if (!str_cmp(argument, "Edit"))
         {
            write_to_buffer(d, "Please type your account name that you want to disable editing status on: ", 0);
            d->connected = CON_CHANGE_EDIT_STATUS;
            return;
         }

         if (!str_cmp(argument, "New"))
         {
            if (d->newstate == 0)
            {
               /* New player */
               /* Don't allow new players if DENY_NEW_PLAYERS is true */
               if (sysdata.DENY_NEW_PLAYERS == TRUE)
               {
                  sprintf(buf, "The mud is currently preparing for a reboot.\n\r");
                  write_to_buffer(d, buf, 0);
                  sprintf(buf, "New accounts are not accepted during this time.\n\r");
                  write_to_buffer(d, buf, 0);
                  sprintf(buf, "Please try again in a few minutes.\n\r");
                  write_to_buffer(d, buf, 0);
                  close_socket(d, FALSE);
               }
               if (sysdata.accounts >= sysdata.max_accounts)
               {
                  write_to_buffer(d, "The maximum allowed accounts per hour has been reached.  This\n\r", 0);
                  write_to_buffer(d, "is probably due to someone abusing the creation process and creating\n\r", 0);
                  write_to_buffer(d, "many accounts, try back in an hour.  If you want, you can try to talk\n\r", 0);
                  write_to_buffer(d, "to Xerves at xerves@rafermand.net to see about getting in.\n\rEnter an existing account: ", 0);
               }
               sprintf(buf,
                  "\n\rYou my choose an account of any name that suits you.  Accounts cannot be seen by\n\r"
                  "players, but immortals can see your account name and might change it if they find it to\n\r"
                  "be offensive in nature, so do not choose an offensive name for your account.\n\r"
                  "After you choose a valid account, you will be asked to supply a valid email address.\n\r"
                  "We will not block any email account, so you may use a hotmail account if you wish.\n\r"
                  "We will keep this information private and only use it to send you email if you request\n\r"
                  "your password or any other email you might request be sent to your account.  Once you enter\n\r"
                  "your email you will be sent a password.  Get this password and then enter it in to get into\n\r"
                  "your account.  Once you are in, you can change your password and create characters."
                  "\n\r\n\rPlease choose a name for your account: ");
               write_to_buffer(d, buf, 0);
               d->newstate++;
               d->connected = CON_GET_ACCOUNT;
               return;
            }
            else
            {
               write_to_buffer(d, "Illegal name, try another.\n\rName: ", 0);
               return;
            }
         }
         if (d->newstate == 1)
         {
            if (!check_parse_name(argument, 1))
            {
               write_to_buffer(d, "That name is invalid, try another.\n\r", 0);
               return;
            }
         }
         fOld = load_account(d, argument, TRUE);
         if (!d->account)
         {
            sprintf(log_buf, "Bad account name %s@%s.", argument, d->host);
            log_string(log_buf);
            write_to_buffer(d, "Your account is corrupt...Please notify xerves@rafermand.net\n\r", 0);
            close_socket(d, FALSE);
            return;
         }
         if (fOld && d->account->editing == 1)
         {
            write_to_buffer(d, "This accout is currently being edited, try another account: ", 0);
            {
               ACCOUNT_NAME *naname;
               ACCOUNT_NAME *nanext;
      
               for (naname = d->account->first_player; naname; naname = nanext)
               {
                  nanext = naname->next;
                  if (naname->name)
                     STRFREE(naname->name);
                  DISPOSE(naname);
               }
               if (d->account->name)
                  STRFREE(d->account->name);
               if (d->account->passwd)
                  STRFREE(d->account->passwd);
               if (d->account->email)
                  STRFREE(d->account->email);
               DISPOSE(d->account);
            }
            return;
         }

         /* telnet negotiation to see if they support MXP */
         write_to_buffer( d, (char *) will_mxp_str, 0 );   

         if (fOld)
         {
            if (d->newstate != 0)
            {
               write_to_buffer(d, "That account name is already taken.  Please choose another: ", 0);
               d->connected = CON_GET_ACCOUNT;
               return;
            }
            if (check_bans(d, NULL, BAN_SITE, 0))
            {
               write_to_buffer(d, "Your site has been banned from Rafermand.\n\r", 0);
               close_socket(d, FALSE);
               return;
            }
            if (d->account->ban == ABAN_BAN)
            {
               write_to_buffer(d, "Your account has been banned from Rafermand.\n\r", 0);
               close_socket(d, FALSE);
               return;
            }
            /* Old account */
            d->newstate = 3;
            write_to_buffer(d, "Password: ", 0);
            write_to_buffer(d, (char *) echo_off_str, 0);
            d->connected = CON_CONFIRM_ACCOUNT_PASSWORD;
            return;
         }
         else
         {
            if (d->newstate == 0)
            {
               /* No such account */
               write_to_buffer(d, "\n\rNo such account exists.\n\rPlease check your spelling, or type new to start a new account.\n\r\n\rAccount: ", 0);
               DISPOSE(d->account);
               d->account = NULL;
               d->connected = CON_GET_ACCOUNT;
               return;
            }
           
            d->account->name = STRALLOC(argument);
            if (check_bans(d, NULL, BAN_SITE, 1))
            {
               write_to_buffer(d, "Your site has been banned from Rafermand.\n\r", 0);
               close_socket(d, FALSE);
               return;
            }
            sprintf(buf, "Did I get that right, %s (Y/N)? ", argument);
            write_to_buffer(d, buf, 0);
            d->connected = CON_CONFIRM_NEW_ACCOUNT;
            return;
         }
         break;
         
      case CON_CHANGE_EDIT_STATUS:
         if (d->account)
         {
            if (strcmp(crypt(argument, d->account->name), d->account->passwd))
            {   
               write_to_buffer(d, "\n\rIncorrect password!.\n\r", 0);
               close_socket(d, FALSE);
               return;
            }
            else
            {
               d->account->editing = 0;
               save_account(d, 0);
               write_to_buffer(d, "\n\rEditing status removed, please relog back in.\n\r", 0);
               close_socket(d, FALSE);
               return;
            }              
         }
         fOld = load_account(d, argument, TRUE);
         if (!fOld)
         {
            write_to_buffer(d, "That account either does not exist or is banned.\n\r", 0);
            close_socket(d, FALSE);
            return;
         }
         else
         {
            write_to_buffer(d, "Please type the password of that account: ", 0);
            return;
         }
            
         
      case CON_CONFIRM_NEW_ACCOUNT:
         switch (*argument)
         {
            case 'y':
            case 'Y':
               if (sysdata.accountemail == 0)
               {
                  d->connected = CON_CONFIRM_EMAIL;
                  write_to_buffer(d, "Please hit enter\n\r", 0);
                  return;
               }
               if (sysdata.accountemail == 2)
               {
                  write_to_buffer(d, "Do you wish to use an email address to protect this account?  If you choose\n\r"
                                     "to use an email account, you will be sent a password to enter your new account\n\r"
                                     "and to make any serious changes to the account.  If you choose not to go the\n\r"
                                     "email route, you will be able to make changes freely.  The email is mainly used\n\r"
                                     "as a security measure to recover passwords and to safely share an account.  It is\n\r"
                                     "optional but recommended for the security reasons.\n\r\n\r"
                                     "Please type 'yes' to use email or 'no' to not use email: ", 0);
                  d->connected = CON_CHOOSE_EMAIL;
                  return;
               }
               sprintf(buf, "\n\rPlease provide a real email address.\n\rYou will need to check it to get your password"
                  "\n\rPlease enter your email for %s: %s", d->account->name, echo_off_str);
               write_to_buffer(d, buf, 0);
               d->connected = CON_GET_EMAIL;
               return;

            case 'n':
            case 'N':
               write_to_buffer(d, "Ok, then what should it be: ", 0);
               if (d->account)
               {
                  ACCOUNT_NAME *naname;
                  ACCOUNT_NAME *nanext;
      
                  for (naname = d->account->first_player; naname; naname = nanext)
                  {
                     nanext = naname->next;
                     if (naname->name)
                        STRFREE(naname->name);
                     DISPOSE(naname);
                  }
                  if (d->account->name)
                     STRFREE(d->account->name);
                  if (d->account->passwd)
                     STRFREE(d->account->passwd);
                  if (d->account->email)
                     STRFREE(d->account->email);
                  DISPOSE(d->account);
               }
               d->connected = CON_GET_ACCOUNT;
               break;

            default:
               write_to_buffer(d, "Please type Yes or No. ", 0);
               break;
         }
      
      case CON_CHOOSE_EMAIL:
         switch (*argument)
         {
            case 'y':
            case 'Y':
               //Was causing crashes, DO NOT REMOVE
               if (!d->account)
               {
                  write_to_buffer(d, "Your account is corrupt...Please notify xerves@rafermand.net\n\r", 0);
                  close_socket(d, FALSE);
               }
               sprintf(buf, "\n\rPlease provide a real email address.\n\rYou will need to check it to get your password"
                  "\n\rPlease enter your email for %s: %s", d->account->name, echo_off_str);
               write_to_buffer(d, buf, 0);
               d->connected = CON_GET_EMAIL;
               break;
            
            case 'n':
            case 'N':
               //Was causing crashes, DO NOT REMOVE
               if (!d->account)
               {
                  write_to_buffer(d, "Your account is corrupt...Please notify xerves@rafermand.net\n\r", 0);
                  close_socket(d, FALSE);
                  return;
               }
               d->account->noemail = 1;
               d->connected = CON_CONFIRM_EMAIL;
               write_to_buffer(d, "No Email was choosen.  Please hit enter\n\r", 0);
               return;
            
            default:
               write_to_buffer(d, "Please type Yes or No. ", 0);
               break;
         }
      case CON_GET_EMAIL:            
         if (strlen(argument) <= 3)
         {
            write_to_buffer(d, "Your email needs to be more than 3 characters.\n\r", 0);
            return;
         }
         if (!check_email_syntax(argument, argument))
         {
            send_to_char("You cannot supply any \" or \' in your email address.\n\r", ch);
            return;
         }
         d->account->email = STRALLOC(argument);
         sprintf(buf, "Is this email, %s correct (Y/N)? ", argument);
         write_to_buffer(d, buf, 0);
         d->connected = CON_CONFIRM_EMAIL;
         return;
                  
      case CON_CONFIRM_EMAIL:
         if (sysdata.accountemail == 0 || (sysdata.accountemail == 2 && d->account->noemail == 1))
         {
            pwdnew = generate_new_pass();
            sprintf(d->account->pbuf, "Your *NEW* Rafermand Account Password for %s", d->account->name);
            sprintf(buf, "Your Password for account %s is %s.  You can change this password in the next screen.\n\r"
                         "Please type your account password to continue:", d->account->name, pwdnew);   
            send_email(d->account->pbuf, d->account->email, buf, NULL, d);
            pwdnew = crypt(pwdnew, d->account->name);
            d->account->passwd = STRALLOC(pwdnew);
            d->account->email = STRALLOC("");
            sysdata.accounts++;
            sprintf(buf, "New Account of %s from host %s", d->account->name, d->host);
            to_channel(buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);
            strtime = ctime(&current_time);
            strtime[strlen(strtime) - 1] = '\0';
            sprintf(buf, "%-30s > %-20s %s", strtime, d->account->name, d->host);
            append_to_file(ACCOUNT_LOG, buf); //Don't want some bastards abusing accounts now...
            d->connected = CON_CONFIRM_ACCOUNT_PASSWORD;
            break;
         }
         switch (*argument)
         {            
            case 'y':
            case 'Y':
               sprintf(buf, "\n\rAn email has been sent to you with your password in it"
                  "\n\rIncluded are some rules of account useage and some help working with it"
                  "\n\r\n\rPlease enter your password from your email: %s", echo_off_str);
               write_to_buffer(d, buf, 0);
               pwdnew = generate_new_pass();
               strcpy(buf, "");
               strcpy(d->account->pbuf, "");
               sprintf(d->account->pbuf, "Your *NEW* Rafermand Account Password for %s", d->account->name);
               sprintf(buf, "Your Password for account %s is %s\n\r\n\r"
                            "Thanks for choosing to play Rafermand.  If you are new or old to Rafermand\n\r"
                            "it is important to understand how the system works.  When you first create\n\r"
                            "an account, this password is emailed to you.  You can either use this email\n\r"
                            "to keep logging into the account, or enter a new one once you reach the account\n\r"
                            "menu.  From this menu you can import characters (ones before the menu system),\n\r"
                            "create new characters, change your password and email address, and delete your\n\r"
                            "account and players in it.\n\r\n\r"
                            "This system is setup so you can play alts through your account, or let others into\n\r"
                            "your account so you can use the same players.  You should check the rules of your\n\r"
                            "mud to see if multiplaying is allowed or sharing of accounts is allowed.  In addition\n\r"
                            "only 1 connection can be made if an account is being editted.  To edit the account you\n\r"
                            "have to have access to the account's email because passwords are sent there that you have\n\r"
                            "to use to edit the account.  Lastly, whomever owns the password to the account is the\n\r"
                            "only one that can edit it because every time you go to edit an account, you are emailed\n\r"
                            "a password.\n\r\n\r"
                            "Lastly, to avoid abuse, the system is setup to allow only a handful of accounts per\n\r"
                            "hour.  If too many are created, the system will stop accepting new accounts till the\n\r"
                            "hour period is up.  Also, we will not send any email to your email address unless you\n\r"
                            "ask us to, or you have If you have any questions about your account, need to get back\n\r"
                            "into your account, or any other general questions, you may email xerves at\n\r"
                            "xerves@rafermand.net.  Thanks\n\r", d->account->name, pwdnew);
               send_email(d->account->pbuf, d->account->email, buf, NULL, d);
               sprintf(d->account->pbuf, "Loading Old Players from the Previous System");
               sprintf(buf, "If you played here at Rafermand before there was an account system, this is\n\r"
                            "some information on how to get your players back.  First, you will want to\n\r"
                            "create an account since there will be no account waiting for you.  Once you\n\r"
                            "have your account created, type I for import from the Account Options.  You\n\r"
                            "will be emailed a password that you enter, enter it then type the name of the\n\r"
                            "player you want to import.  You will either be asked for a password or be told\n\r"
                            "you cannot import a player.  If you haven't imported a player, either someone\n\r"
                            "else imported it before you or there is a problem.  Email xerves at\n\r"
                            "xerves@rafermand.net to see what happened to the player.  Thanks\n\r");
               send_email(d->account->pbuf, d->account->email, buf, NULL, d);       
               pwdnew = crypt(pwdnew, d->account->name);
               d->account->passwd = STRALLOC(pwdnew);
               sysdata.accounts++;
               sprintf(buf, "New Account of %s from host %s", d->account->name, d->host);
               to_channel(buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);
               strtime = ctime(&current_time);
               strtime[strlen(strtime) - 1] = '\0';
               sprintf(buf, "%-30s > %-20s %s", strtime, d->account->name, d->host);
               append_to_file(ACCOUNT_LOG, buf); //Don't want some bastards abusing accounts now...
               d->connected = CON_CONFIRM_ACCOUNT_PASSWORD;
               break;

            case 'n':
            case 'N':
               write_to_buffer(d, "Ok, then what is it: ", 0);
               STRFREE(d->account->email);
               d->connected = CON_GET_EMAIL;
               break;

            default:
               write_to_buffer(d, "Please type Yes or No. ", 0);
               break;
         }
         break;
         
      case CON_CONFIRM_ACCOUNT_PASSWORD:
         if (strcmp(crypt(argument, d->account->name), d->account->passwd))
         {
            if (d->newstate != 1)
            {
               write_to_buffer(d, "Wrong password.\n\r", 0);
               close_socket(d, FALSE);
               return;
            }
            write_to_buffer(d, "\n\rThat password is not valid, try again: ", 0);
            break;
         }
         sprintf(buf, "Welcome %s, here are your options for your account.\n\r\n\r", d->account->name);
         write_to_buffer(d, buf, 0);
         if (d->account->lasttimereset == 0)
         {
            d->account->lasttimereset = time(0);
            save_account(d, 0);
         }
         if (d->newstate == 1)
         {
            d->account->lasttimereset = time(0);
            d->account->changes = 0;
            d->account->qplayer1 = STRALLOC("");
            d->account->qplayer2 = STRALLOC("");
            d->account->qplayer3 = STRALLOC("");
            d->account->qplayer4 = STRALLOC("");
            save_account(d, 0);
            sprintf(buf, "New Account %s has entered the account menu", d->account->name);
            log_string_plus(buf, LOG_NORMAL, LEVEL_IMMORTAL);
         }
         else
         {
            sprintf(buf, "Account %s has been accessed from %s", d->account->name, d->host);
            log_string_plus(buf, LOG_NORMAL, LEVEL_IMMORTAL);
            to_channel(buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);
            if (time(0) - d->account->lasttimereset > 3600)
            {
               for (dd = first_descriptor; dd; dd = dd->next)
               {
                  if (dd != d && dd->account && dd->account->lasttimereset && !str_cmp(dd->account->name, d->account->name))
                  {
                     if (dd->account->lasttimereset > d->account->lasttimereset)
                        d->account->lasttimereset = dd->account->lasttimereset;
                  }
               }
               if (time(0) - d->account->lasttimereset > 3600)
               {
                  d->account->lasttimereset = time(0);
                  d->account->changes = 0;
                  save_account(d, 0);
               }
            }
         }
         send_account_menu(d);
         d->connected = CON_SHOW_ACCOUNT_MENU;
         break;
         
      case CON_SHOW_ACCOUNT_MENU:
         d->newstate = 0;
         if (argument[0] == '\0')
         {
            send_account_menu(d);
            return;
         }
         switch(*argument)
         {
            
            case 'N': case 'n':
               if (sysdata.accountemail == 2)
               {
                  if (!check_connected_accounts(d))
                     return;
                  if (sysdata.accountemail == 0 || (sysdata.accountemail == 2 && d->account->noemail == 1))
                     write_to_buffer(d, "Please type the following password to continue: ", 0);
                  else
                     write_to_buffer(d, "A password is being emailed to your email account, enter this password\n\r"
                                  "below, afterwards you can toggle your email status.\n\r"
                                  "Enter the password emailed to you: ", 0);
                  d->account->pbuffer = generate_new_pass();
                  send_email("Your Password to toggle your email status", d->account->email, d->account->pbuffer, NULL, d);
                  d->connected = CON_CHANGE_EMAILSTATUS;
                  return;   
               }
            case 'B': case 'b':
               write_to_buffer(d, "Choose a a number of 1-4 to use:\n\r", 0);
               d->connected = CON_BYPASS_LOGIN;
               return;
               
            case 'I': case 'i':
               if (!check_connected_accounts(d))
                  return;
               write_to_buffer(d, "You can only import players that still have passwords on them.\n\r"
                                  "This includes old characters before the account system or any\n\r"
                                  "players in a deleted account.\n\rChoose a Player to import: ", 0);
               d->connected = CON_IMPORT_MENU;
               return;
               
            case 'P': case 'p':
               if (!check_connected_accounts(d))
                  return;
               if (sysdata.accountemail == 0 || (sysdata.accountemail == 2 && d->account->noemail == 1))
                  write_to_buffer(d, "Please type the following password to continue: ", 0);
               else
                  write_to_buffer(d, "A password is being emailed to your email account, enter this password\n\r"
                                  "below, afterwards you can change your password to something new.\n\r"
                                  "Enter the password emailed to you: ", 0);
               d->account->pbuffer = generate_new_pass();
               send_email("Your Password to change your Password", d->account->email, d->account->pbuffer, NULL, d);
               d->connected = CON_NEWPASS_MENU;
               return;
               
            case 'S': case 's':
               write_to_buffer(d, "Your character choices are:\n\r", 0);
               for (aname = d->account->first_player; aname; aname = aname->next)
               {
                  sprintf(buf, ">   %s\n\r", aname->name);
                  write_to_buffer(d, buf, 0);
               }
               write_to_buffer(d, "\n\r", 0);
               send_account_menu(d);
               return;
               
            case 'Q': case 'q':
               write_to_buffer(d, "Goodbye!\n\r", 0);
               close_socket(d, FALSE);
               return;
               
            case 'K': case 'k':
               if (!check_connected_accounts(d))
                  return;
               if (sysdata.accountemail == 0 || (sysdata.accountemail == 2 && d->account->noemail == 1))
                  write_to_buffer(d, "Please type the following password to continue: ", 0);
               else
                  write_to_buffer(d, "A password is being emailed to your email account, enter this password\n\r"
                                  "below and once you enter it twice, this account will be deleted.\n\r"
                                  "Enter the password emailed to you: ", 0);
               d->account->pbuffer = generate_new_pass();
               send_email("Your Password to delete your account", d->account->email, d->account->pbuffer, NULL, d);
               d->connected = CON_DELETEACCOUNT_MENU;
               return;    
               
            case 'R': case 'r':
               if (!check_connected_accounts(d))
                  return;
               if (sysdata.accountemail == 0 || (sysdata.accountemail == 2 && d->account->noemail == 1))
                  write_to_buffer(d, "Please type the following password to continue: ", 0);
               else
                  write_to_buffer(d, "A password is being emailed to your email account, enter this password\n\r"
                                  "below, afterwards you can type a name of a character to release and enter\n\r"
                                  "a password so someone else can import it.  Enter the password emailed to you: ", 0); 
               d->account->pbuffer = generate_new_pass();
               send_email("Your Password to release a Player", d->account->email, d->account->pbuffer, NULL, d);
               d->connected = CON_RELEASEPLAYER_MENU;
               return;              
               
            case 'E': case 'e':
               if (!check_connected_accounts(d))
                  return;
               if (sysdata.accountemail == 0 || (sysdata.accountemail == 2 && d->account->noemail == 1))
                  write_to_buffer(d, "Please type the following password to continue: ", 0);
               else
                  write_to_buffer(d, "A password is being emailed to your email account, enter this password\n\r"
                                  "below, afterwards you can change your email to something new.\n\r"
                                  "Enter the password emailed to you: ", 0);
               d->account->pbuffer = generate_new_pass();
               send_email("Your Password to change your Email", d->account->email, d->account->pbuffer, NULL, d);
               d->connected = CON_NEWEMAIL_MENU;
               return;    
               
            case 'M': case 'm':
               if (!check_connected_accounts(d))
                  return;
               if (d->account->skiplmenu == 0)
               {
                  write_to_buffer(d, "Skipping Player Menu is now ENABLED!", 0);
                  d->account->skiplmenu = 1;
                  save_account(d, 0);
                  return;
               }
               else
               {
                  write_to_buffer(d, "Skipping Player Menu is now DISABLED!", 0);
                  d->account->skiplmenu = 0;
                  save_account(d, 0);
                  return;
               }
               
            case 'D': case 'd':
               if (!check_connected_accounts(d))
                  return;
               if (sysdata.accountemail == 0 || (sysdata.accountemail == 2 && d->account->noemail == 1))
                  write_to_buffer(d, "Please type the following password to continue: ", 0);
               else
                  write_to_buffer(d, "A password is being emailed to your email account, enter this password\n\r"
                                  "below, afterwards you can delete as many characters as you wish.\n\r"
                                  "Enter the password emailed to you: ", 0);
               d->account->pbuffer = generate_new_pass();
               send_email("Your Password to delete Players", d->account->email, d->account->pbuffer, NULL, d);
               d->connected = CON_DELETEPLAYER_MENU;   
               return;          
               
            case '1':
               if (d->account->qplayer1[0] == '\0')
               {
                  write_to_buffer(d, "Need to set Bypass Login for Number 1 before you can do this!\n\r", 0);
                  write_to_buffer(d, "\n\r", 0);
                  send_account_menu(d);
                  return;
               }
               sprintf(d->account->nbuf, "%s", d->account->qplayer1);
               d->connected = CON_GET_NAME;
               goto getname;
               return;
               
            case '2':
               if (d->account->qplayer2[0] == '\0')
               {
                  write_to_buffer(d, "Need to set Bypass Login for Number 2 before you can do this!\n\r", 0);
                  write_to_buffer(d, "\n\r", 0);
                  send_account_menu(d);
                  return;
               }
               sprintf(d->account->nbuf, "%s", d->account->qplayer2);
               d->connected = CON_GET_NAME;
               goto getname;
               return;
               
            case '3':
               if (d->account->qplayer3[0] == '\0')
               {
                  write_to_buffer(d, "Need to set Bypass Login for Number 3 before you can do this!\n\r", 0);
                  write_to_buffer(d, "\n\r", 0);
                  send_account_menu(d);
                  return;
               }
               sprintf(d->account->nbuf, "%s", d->account->qplayer3);
               d->connected = CON_GET_NAME;
               goto getname;
               return;
            
            case '4':
               if (d->account->qplayer4[0] == '\0')
               {
                  write_to_buffer(d, "Need to set Bypass Login for Number 4 before you can do this!\n\r", 0);
                  write_to_buffer(d, "\n\r", 0);
                  send_account_menu(d);
                  return;
               }
               sprintf(d->account->nbuf, "%s", d->account->qplayer4);
               d->connected = CON_GET_NAME;
               goto getname;
               return;
               
            case 'L': case 'l':
               strcpy(d->account->nbuf, "");
               write_to_buffer(d, "Type the name of the character you want to login with: ", 0);
               d->connected = CON_GET_NAME;
               return;   
               
            case 'C': case 'c':
               if (!check_connected_accounts(d))
                  return;
               /* New player */
               /* Don't allow new players if DENY_NEW_PLAYERS is true */
               if (sysdata.DENY_NEW_PLAYERS == TRUE)
               {
                  sprintf(buf, "The mud is currently preparing for a reboot.\n\r");
                  write_to_buffer(d, buf, 0);
                  sprintf(buf, "New players are not accepted during this time.\n\r");
                  write_to_buffer(d, buf, 0);
                  sprintf(buf, "Please try again in a few minutes.\n\r");
                  write_to_buffer(d, buf, 0);
                  close_socket(d, FALSE);
               }
               sprintf(buf,
                  "\n\rWhen choosing a name, please choose a unique name and one that is not a\n\r"
                  "main character in a book.  The name should look medieval like and should not\n\r"
                  "combine two ore more words.  The immortals have the right to deny your name,\n\r"
                  "and if your name is not accepted or denied in 4 to 7 minutes, the mud will\n\r"
                  "auto authorize you so you can start the game.  Once you are in, you will be\n\r"
                  "able to get basic equipment at our mob that gives out basic equipment.  We\n\r"
                  "do not have a academy, so feel free to explore the town you start in or go\n\r"
                  "out and get killed in our Wilderness.\n\r" "\n\r\n\rPlease choose a name for your character: ");
               write_to_buffer(d, buf, 0);
               d->newstate++;
               d->connected = CON_CREATE_NAME;
               return;
               
            default:
               write_to_buffer(d, "That is not a selection.  Pick from this menu:\n\r\n\r", 0);
               send_account_menu(d);
               return;
                         
         }
      case CON_BYPASS_LOGIN:
         if (argument[0] == '\0')
         {
            write_to_buffer(d, "You typed nothing, goint back into the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->account->passvalue = 0;
            d->connected = CON_SHOW_ACCOUNT_MENU;
            break;
         }
         if (atoi(argument) >= 1 && atoi(argument) <= 4)
         {
            d->account->passvalue = atoi(argument);
            write_to_buffer(d, "Now choose a player name to associate with this number: ", 0);
            break;
         }
         if (atoi(argument) <= 0 && d->account->passvalue > 0)
         {
            for (aname = d->account->first_player; aname; aname = aname->next)
            {
               if (!str_cmp(argument, aname->name))
                  break;
            }
            if (aname)
            {
               sprintf(buf, "Player %s has been associated with number %d\n\r\n\r", argument, d->account->passvalue);
               write_to_buffer(d, buf, 0);
               if (d->account->passvalue == 1)
               {
                  if (d->account->qplayer1)
                     STRFREE(d->account->qplayer1);
                  d->account->qplayer1 = STRALLOC(argument);
               }
               if (d->account->passvalue == 2)
               {
                  if (d->account->qplayer2)
                     STRFREE(d->account->qplayer2);
                  d->account->qplayer2 = STRALLOC(argument);
               }
               if (d->account->passvalue == 3)
               {
                  if (d->account->qplayer3)
                     STRFREE(d->account->qplayer3);
                  d->account->qplayer3 = STRALLOC(argument);
               }
               if (d->account->passvalue == 4)
               {
                  if (d->account->qplayer4)
                     STRFREE(d->account->qplayer4);
                  d->account->qplayer4 = STRALLOC(argument);
               }
               save_account(d, 0);
               send_account_menu(d);
               d->account->passvalue = 0;
               d->connected = CON_SHOW_ACCOUNT_MENU;
               break;
            }
         }       
      case CON_DELETEPLAYER_MENU:
         if (argument[0] == '\0')
         {
            write_to_buffer(d, "You typed nothing, going back into the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            save_account(d, 0);
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         if (d->account->passvalue == 2)
         {
            if (strcmp(argument, "delete"))
            {
               write_to_buffer(d, "You did not type delete, enter a new name or hit enter to go back: ", 0);
               d->account->passvalue = 1;
               d->account->pbuffer = NULL;
               break;
            }
            fOld = load_char_obj(d, d->account->pbuffer, TRUE);
            if (!fOld)
            {
               write_to_buffer(d, "That character could not be loaded, inform the Administrator\n\rChoose another name: ", 0);
               bug("%s could not be loaded by account %s", d->account->pbuffer, d->account->name);
               d->account->passvalue = 1;
               d->account->pbuffer = NULL;
               d->character->desc = NULL;
               free_char(d->character); /* Big Memory Leak before --Shaddai */
               d->character = NULL;
               return;
            }    
            sprintf(buf, "Account %s from %s has deleted player %s", d->account->name, d->host, d->character->name);
            log_string_plus(buf, LOG_NORMAL, sysdata.log_level);           
            delete_character(d, d->character);
            for (aname = d->account->first_player; aname; aname = aname->next)
            {
               if (!str_cmp(aname->name, d->account->pbuffer))
                  break;
            }
            if (aname)
            {
               STRFREE(aname->name);
               UNLINK(aname, d->account->first_player, d->account->last_player, next, prev);
               DISPOSE(aname);
               save_account(d, 0);
            }
            d->character->desc = NULL;
            free_char(d->character); /* Big Memory Leak before --Shaddai */
            d->character = NULL;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            save_account(d, 0);
            write_to_buffer(d, "Done, going back to the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            break;
         }
         if (d->account->passvalue == 1)
         {
            for (aname = d->account->first_player; aname; aname = aname->next)
            {
               if (!str_cmp(aname->name, argument))
                  break;
            }
            if (!aname)
            {
               write_to_buffer(d, "That player is not in your account, choose another player: ", 0);
               return;
            }
            d->account->passvalue = 2;
            sprintf(d->account->pbuf, argument);
            d->account->pbuffer = d->account->pbuf;
            sprintf(buf, "You are about to delete %s, type Delete now to delete this character: ", argument);
            write_to_buffer(d, buf, 0);
            break;
         }
         if (d->account->pbuffer && strcmp(argument, d->account->pbuffer))
         {
            write_to_buffer(d, "What you typed is not correct, going back to the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         d->account->pbuffer = NULL;
         d->account->passvalue = 1;
         write_to_buffer(d, "\n\r***WARNING*** This will delete a player, not remove it from your account.\n\r"
                            "The player will be deleted and won't be able to be accessed ever again.\n\r"
                            "Enter the name of the player you wish to delete: ", 0);
         break; 
         
      case CON_RELEASEPLAYER_MENU:
         if (argument[0] == '\0')
         {
            write_to_buffer(d, "You typed nothing, going back into the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         if (d->account->passvalue == 1)
         {
            for (aname = d->account->first_player; aname; aname = aname->next)
            {
               if (!str_cmp(aname->name, argument))
                  break;
            }
            if (!aname)
            {
               write_to_buffer(d, "You do not have access to that player.\n\r", 0);
               d->connected = CON_SHOW_ACCOUNT_MENU;
               d->account->passvalue = 0;
               d->account->pbuffer = NULL;
               return;
            }
            sprintf(d->account->nbuf, argument);
            write_to_buffer(d, "Enter a password to attach to this player: ", 0);
            sprintf(d->account->pbuf, argument);
            d->account->pbuffer = d->account->pbuf;
            d->account->passvalue = 2;
            return;
         }
         if (d->account->passvalue == 2)
         {
            if ( strlen(argument) < 5 )
	    {
	        write_to_buffer( d, "Password must be at least five characters long.\n\rPassword: ", 0 );
	        return;
    	    }
    	    
    	    fOld = load_char_obj(d, d->account->nbuf, TRUE);
         
            if (!fOld) //Well something happened, rofl
            {
               write_to_buffer(d, "The pfile does not exist to change to drop, tell an immortal.\n\r", 0);
               bug("%s could not be loaded for password change on player release", d->account->nbuf) ;
               d->character->desc = NULL;
               free_char(d->character); /* Big Memory Leak before --Shaddai */
               d->character = NULL;
               send_account_menu(d);
               save_account(d, 0);
               d->connected = CON_SHOW_ACCOUNT_MENU;
               d->account->passvalue = 0;
               d->account->pbuffer = NULL;
               break;
            }

	    pwdnew = crypt( argument, d->character->name );
	    for ( p = pwdnew; *p != '\0'; p++ )
    	    {
	       if ( *p == '~' )
	       {
		  write_to_buffer( d, "New password not acceptable, try again.\n\rPassword: ", 0 );
		  return;
	       }
            }
            d->character->desc = NULL;
            free_char(d->character); /* Big Memory Leak before --Shaddai */
            d->character = NULL;
            sprintf(d->account->pbuf, argument);
            d->account->pbuffer = d->account->pbuf;
            d->account->passvalue = 3;
            write_to_buffer(d, "Please type it again: ", 0);
            write_to_buffer(d, (char *) echo_off_str, 0);
            return;
         }
         if (d->account->passvalue == 3)
         {
            ACCOUNT_NAME *anext;
            write_to_buffer(d, (char *) echo_on_str, 0);
            if (strcmp(d->account->pbuffer, argument))
            {
               write_to_buffer(d, "That is not the correct password, enter a new password: ", 0);
               write_to_buffer(d, (char *) echo_off_str, 0);
               d->account->passvalue = 2;
               break;
            }   
            fOld = load_char_obj(d, d->account->nbuf, TRUE);
         
            if (!fOld) //Well something happened, rofl
            {
               write_to_buffer(d, "The pfile does not exist to change to drop, tell an immortal.\n\r", 0);
               bug("%s could not be loaded for password change on player release", d->account->nbuf) ;
               d->character->desc = NULL;
               free_char(d->character); /* Big Memory Leak before --Shaddai */
               d->character = NULL;
               send_account_menu(d);
               save_account(d, 0);
               d->connected = CON_SHOW_ACCOUNT_MENU;
               d->account->passvalue = 0;
               d->account->pbuffer = NULL;
               break;
            }
            DISPOSE(d->character->pcdata->pwd);
            d->character->pcdata->pwd = str_dup(crypt(argument, d->character->name));
            save_char_account(d->character, 1, d->character->pcdata->pwd);
            d->character->desc = NULL;
            free_char(d->character); /* Big Memory Leak before --Shaddai */
            d->character = NULL;
            for (aname = d->account->first_player; aname; aname = anext)
            {
               anext = aname->next;
               if (!str_cmp(aname->name, d->account->nbuf))
               {
                  STRFREE(aname->name);
                  UNLINK(aname, d->account->first_player, d->account->last_player, next, prev);
                  DISPOSE(aname);
               }
            }
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            save_account(d, 0);
            write_to_buffer(d, "Done, going back to the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            return;
         }
         if (strcmp(argument, d->account->pbuffer))
         {
            write_to_buffer(d, "What you typed is not correct, going back to the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         d->account->passvalue = 1;
         write_to_buffer(d, "Type the name of the player you want to release: ", 0);
         break; 
         
      case CON_DELETEACCOUNT_MENU:
         if (argument[0] == '\0')
         {
            write_to_buffer(d, "You typed nothing, going back into the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         if (d->account->passvalue == 1)
         {
            if (strcmp(argument, d->account->pbuffer))
            {
               write_to_buffer(d, "That is not the correct password, going back to the menu\n\r", 0);
               send_account_menu(d);
               save_account(d, 0);
               d->connected = CON_SHOW_ACCOUNT_MENU;
               d->account->passvalue = 0;
               d->account->pbuffer = NULL;
               break;
            }
            write_to_buffer(d, "Your account is being deleted, all characters in it will use the password for your account.\n\r", 0);
            sprintf(buf, "Account %s from %s has deleted account %s", d->account->name, d->host, d->account->name);
            log_string_plus(buf, LOG_NORMAL, sysdata.log_level);      
            for (aname = d->account->first_player; aname; aname = aname->next)
            {
               fOld = load_char_obj(d, aname->name, TRUE);
         
               if (!fOld) //Well something happened, rofl
               {
                  write_to_buffer(d, "The player exists but would not load for change, notify Xerves.\n\r", 0);
                  bug("%s could not be loaded for password change on account deletion", aname->name);
                  d->character->desc = NULL;
                  free_char(d->character); /* Big Memory Leak before --Shaddai */
                  d->character = NULL;
                  continue;
               }
               if (d->character->pcdata->pwd)
                  DISPOSE(d->character->pcdata->pwd);
               d->character->pcdata->pwd = str_dup(d->account->passwd);
               save_char_account(d->character, 1, d->character->pcdata->pwd);
               d->character->desc = NULL;
               free_char(d->character); /* Big Memory Leak before --Shaddai */
               d->character = NULL;
            }
            strcpy(fname, "");
            sprintf(fname, "%s%c/%s", ACCOUNT_DIR, tolower(d->account->name[0]), capitalize(d->account->name));
            write_to_buffer(d, "Goodbye!\n\r", 0);
            close_socket(d, FALSE);
            remove(fname);
            return;
         }
         if (strcmp(argument, d->account->pbuffer))
         {
            write_to_buffer(d, "What you typed is not correct, going back to the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         d->account->passvalue = 1;
         write_to_buffer(d, "****WARNING**** You are about to delete your account\n\rType the password one more time to delete: ", 0);
         break; 
         
      case CON_CHANGE_EMAILSTATUS:
         if (argument[0] == '\0')
         {
            write_to_buffer(d, "You typed nothing, going back into the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         if (d->account->passvalue == 2)
         {
            if (strcmp(argument, d->account->pbuffer))
            {
               write_to_buffer(d, "That is not the correct email address, enter a new email address: ", 0);
               d->account->passvalue = 1;
               break;
            }   
            if (d->account->email)
               STRFREE(d->account->email);
            d->account->email = STRALLOC(argument);
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            d->account->noemail = 0;
            save_account(d, 0);
            sprintf(buf, "Account %s from %s has changed email to %s", d->account->name, d->host, d->account->email);
            log_string_plus(buf, LOG_NORMAL, sysdata.log_level);   
            write_to_buffer(d, "Done, going back to the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            break;
         }
         if (d->account->passvalue == 1)
         {
            sprintf(d->account->pbuf, argument);
            d->account->pbuffer = d->account->pbuf;
            d->account->passvalue = 2;
            write_to_buffer(d, "Please type it again: ", 0);
            break;
         }
         if (strcmp(argument, d->account->pbuffer))
         {
            write_to_buffer(d, "What you typed is not correct, going back to the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         if (d->account->noemail == 0)
         {
            d->account->noemail = 1;
            write_to_buffer(d, "Disabled email support for your account.  Returning to the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         d->account->pbuffer = NULL;  
         d->account->passvalue = 1;
         write_to_buffer(d, "Please type your new email address: ", 0);
         break; 
 
      case CON_NEWEMAIL_MENU:
         if (argument[0] == '\0')
         {
            write_to_buffer(d, "You typed nothing, going back into the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         if (d->account->passvalue == 2)
         {
            if (strcmp(argument, d->account->pbuffer))
            {
               write_to_buffer(d, "That is not the correct email address, enter a new email address: ", 0);
               d->account->passvalue = 1;
               break;
            }   
            STRFREE(d->account->email);
            d->account->email = STRALLOC(argument);
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            save_account(d, 0);
            sprintf(buf, "Account %s from %s has changed email to %s", d->account->name, d->host, d->account->email);
            log_string_plus(buf, LOG_NORMAL, sysdata.log_level);   
            write_to_buffer(d, "Done, going back to the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            break;
         }
         if (d->account->passvalue == 1)
         {
            sprintf(d->account->pbuf, argument);
            d->account->pbuffer = d->account->pbuf;
            d->account->passvalue = 2;
            write_to_buffer(d, "Please type it again: ", 0);
            break;
         }
         if (strcmp(argument, d->account->pbuffer))
         {
            write_to_buffer(d, "What you typed is not correct, going back to the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         d->account->pbuffer = NULL;
         d->account->passvalue = 1;
         write_to_buffer(d, "Please type your new email address: ", 0);
         break; 
         

      case CON_NEWPASS_MENU:
         if (argument[0] == '\0')
         {
            write_to_buffer(d, "\n\rYou typed nothing, going back into the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         if (d->account->passvalue == 2)
         {
            write_to_buffer(d, (char *) echo_on_str, 0);
            if (strcmp(d->account->pbuffer, argument))
            {
               write_to_buffer(d, "T\n\rhat is not the correct password, enter a new password: ", 0);
               write_to_buffer(d, (char *) echo_off_str, 0);
               d->account->passvalue = 1;
               break;
            }
            STRFREE(d->account->passwd);
            d->account->passwd = STRALLOC(crypt(argument, d->account->name));
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            save_account(d, 0);
            write_to_buffer(d, "\n\rDone, going back to the Account Menu\n\r\n\r", 0);
            sprintf(buf, "Account %s from %s has changed passwords", d->account->name, d->host);
            log_string_plus(buf, LOG_NORMAL, sysdata.log_level);   
            send_account_menu(d);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            break;
         }
         if (d->account->passvalue == 1)
         {
            write_to_buffer(d, (char *) echo_on_str, 0);
            if ( strlen(argument) < 5 )
	    {
	        write_to_buffer( d, "\n\rPassword must be at least five characters long.\n\rPassword: ", 0 );
	        return;
    	    }

	    pwdnew = crypt( argument, d->account->name );
	    for ( p = pwdnew; *p != '\0'; p++ )
    	    {
	       if ( *p == '~' )
	       {
		  write_to_buffer( d, "\n\rNew password not acceptable, try again.\n\rPassword: ", 0 );
		  return;
	       }
            }
            sprintf(d->account->pbuf, argument);
            d->account->pbuffer = d->account->pbuf;
            d->account->passvalue = 2;
            write_to_buffer(d, "\n\rPlease type it again: ", 0);
            write_to_buffer(d, (char *) echo_off_str, 0);
            break;
         }
         if (strcmp(argument, d->account->pbuffer))
         {
            write_to_buffer(d, "\n\rWhat you typed is not correct, going back to the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            d->account->passvalue = 0;
            d->account->pbuffer = NULL;
            break;
         }
         d->account->pbuffer = NULL;
         d->account->passvalue = 1;
         write_to_buffer(d, "\n\rPlease type your new account Password: ", 0);
         write_to_buffer(d, (char *) echo_off_str, 0);
         break;
         
      case CON_IMPORT_MENU:
         if (argument[0] == '\0')
         {
            write_to_buffer(d, "You need to type a name to import, going back into the Account Menu\n\r\n\r", 0);
            send_account_menu(d);
            save_account(d, 0);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            break;
         }
         if (d->character) //Entered a password, well hopefully
         {
            if (strcmp(crypt(argument, d->character->name), d->character->pcdata->pwd))
            {
               write_to_buffer(d, "Invalid password, going back to Account Menu.\n\r", 0);
               d->character->desc = NULL;
               free_char(d->character); /* Big Memory Leak before --Shaddai */
               d->character = NULL;
               send_account_menu(d);
               d->connected = CON_SHOW_ACCOUNT_MENU;
               break;
            }
            sprintf(buf, "Account %s from %s has imported character %s", d->account->name, d->host, d->character->name);
            log_string_plus(buf, LOG_NORMAL, sysdata.log_level);   
            write_to_buffer(d, "The player has been added to your account, you can login as him/her now.\n\rGoing back to the Account Menu Now\n\r\n\r", 0);
            DISPOSE(d->character->pcdata->pwd);
            CREATE(aname, ACCOUNT_NAME, 1);
            aname->name = STRALLOC(capitalize(d->character->name));
            LINK(aname, d->account->first_player, d->account->last_player, next, prev);
            save_account(d, 0);
            save_char_account(d->character, 0, NULL);
            d->character->desc = NULL;
            free_char(d->character); /* Big Memory Leak before --Shaddai */
            d->character = NULL;
            send_account_menu(d);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            break;
         }            
               
         sprintf(fname, "%s%c/%s", PLAYER_DIR, tolower(argument[0]), capitalize(argument));
         if (stat(fname, &fst) == -1)
         {
            write_to_buffer(d, "That name does not exist, try another.\n\rName: ", 0);
            return;
         }
         fOld = load_char_obj(d, argument, TRUE);
         
         if (!fOld) //Well something happened, rofl
         {
            write_to_buffer(d, "The player exists but would not load, notify Xerves.\n\r", 0);
            bug("%s could not be loaded for password change on player import", argument);
            d->character->desc = NULL;
            free_char(d->character); /* Big Memory Leak before --Shaddai */
            d->character = NULL;
            return;
         }
         else //Lets check the password now...
         {
            if (!d->character->pcdata->pwd)
            {
               write_to_buffer(d, "This character is already imported.  Going back to the menu.\n\r\n\r", 0);
               d->character->desc = NULL;
               free_char(d->character); /* Big Memory Leak before --Shaddai */
               d->character = NULL;
               send_account_menu(d);
               save_account(d, 0);
               d->connected = CON_SHOW_ACCOUNT_MENU;
               break;
            }
               
            write_to_buffer(d, "Please type the password of this pfile to import it.\n\rPassword: \n\r", 0);
            return;
         }  
               
      case CON_GET_NAME:
         getname: ;  //From the 1-4 in menu....Thought this would be the easiest way
         if (d->account->nbuf[0] != '\0')
         {
            sprintf(argument, d->account->nbuf);
            strcpy(d->account->nbuf, "");
         }
         if (argument[0] == '\0')
         {
            write_to_buffer(d, "You need to enter a name of a character, returning to the menu.\n\r\n\r", 0);
            send_account_menu(d);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            return;
         }

         argument = capitalize(argument);
         
         if (!str_cmp(argument, "New"))
         {
            write_to_buffer(d, "You create characters back at the menu, hit enter one time to get back.\n\rName: ", 0);
            return;
         }

         if (check_playing(d, argument, FALSE) == BERR) //In the middle of something, don't kick that person off
         {
            write_to_buffer(d, "Name: ", 0);
            return;
         }
         
         for (aname = d->account->first_player; aname; aname = aname->next)
         {
            if (!str_cmp(argument, aname->name))
               break;
         }
         if (!aname)
         {
            write_to_buffer(d, "That is not a name in your account, choose another.\n\rName: ", 0);
            return;
         }
         fOld = load_char_obj(d, argument, TRUE);
         if (!fOld)
         {
            sprintf(fname, "%s%c/%s", RESET_DIR, tolower(argument[0]), capitalize(argument));
            if (stat(fname, &fst) != -1)
            {
               //Need to set some information since we have a quasi pfile here...
               fOld = load_char_obj(d, argument, TRUE);
               if (!d->character)
               {
                  sprintf(log_buf, "Bad player file %s@%s.", argument, d->host);
                  log_string(log_buf);
                  write_to_buffer(d, "Your playerfile is corrupt...Please notify xerves@rafermand.net\n\r", 0);
                  close_socket(d, FALSE);
                  return;
               }
               ch = d->character;

               /* telnet negotiation to see if they support MXP */
               write_to_buffer( d, (char *) will_mxp_str, 0 );   
               chk = check_reconnect(d, argument, FALSE);
               if (chk == BERR)
                  return;

               if (chk)
               {
                  fOld = TRUE;
               }
               else
               {
                  if (wizlock && !IS_IMMORTAL(ch))
                  {
                     write_to_buffer(d, "The game is wizlocked.  Only immortals can connect now.\n\r", 0);
                     write_to_buffer(d, "Either contact Xerves on ICQ at 602180, or try again later.\n\r", 0);
                     close_socket(d, FALSE);
                     return;
                  }
               }
               write_to_buffer(d, "Do you wish to use this character from your other games?  Choose yes or no: ", 0);
               d->connected = CON_USE_RESET_CHAR;
               return;
            }
            write_to_buffer(d, "That is not a valid selection, please type another name: ", 0);
            return;
         }
         if (!d->character)
         {
            sprintf(log_buf, "Bad player file %s@%s.", argument, d->host);
            log_string(log_buf);
            write_to_buffer(d, "Your playerfile is corrupt...Please notify xerves@rafermand.net\n\r", 0);
            close_socket(d, FALSE);
            return;
         }
         ch = d->character;
        /* if (check_bans(ch, BAN_SITE)) -- Is in the account check instead
         {
            write_to_buffer(d, "Your site has been banned from Rafermand.\n\r", 0);
            close_socket(d, FALSE);
            return;
         } */

         //Classes not needed in Rafermand, but will leave it in here for others

         if (fOld)
         {
            if (check_bans(d, ch, BAN_CLASS, 0))
            {
               write_to_buffer(d, "Your class has been banned from Rafermand.\n\r", 0);
               close_socket(d, FALSE);
               return;
            }
            if (check_bans(d, ch, BAN_RACE, 0))
            {
               write_to_buffer(d, "Your race has been banned from Rafermand.\n\r", 0);
               close_socket(d, FALSE);
               return;
            }
         }

         if (xIS_SET(ch->act, PLR_DENY))
         {
            sprintf(log_buf, "Denying access to %s@%s.", argument, d->host);
            log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
            write_to_buffer(d, "You are denied access.\n\r", 0);
            close_socket(d, FALSE);
            return;
         }
         /*
          *  Make sure the immortal host is from the correct place.
          *  Shaddai
          */

         if (IS_IMMORTAL(ch) && sysdata.check_imm_host && !check_immortal_domain(ch, d->host))
         {
            sprintf(log_buf, "%s's char being hacked from %s.", argument, d->host);
            log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
            write_to_buffer(d, "This hacking attempt has been logged.\n\r", 0);
            close_socket(d, FALSE);
            return;
         }

         /* telnet negotiation to see if they support MXP */
         write_to_buffer( d, (char *) will_mxp_str, 0 );   
         
         chk = check_reconnect(d, argument, FALSE);
         if (chk == BERR)
            return;

         if (chk)
         {
            fOld = TRUE;
         }
         else
         {
            if (wizlock && !IS_IMMORTAL(ch))
            {
               write_to_buffer(d, "The game is wizlocked.  Only immortals can connect now.\n\r", 0);
               write_to_buffer(d, "Either contact Xerves on ICQ at 602180, or try again later.\n\r", 0);
               close_socket(d, FALSE);
               return;
            }
         }
         //Crap below is from the password section, since we don't ask for one
         write_to_buffer(d, (char *) echo_on_str, 0);

         if (check_playing(d, ch->pcdata->filename, TRUE))
            return;

         chk = check_reconnect(d, ch->pcdata->filename, TRUE);
         if (chk == BERR)
         {
            if (d->character && d->character->desc)
               d->character->desc = NULL;
            close_socket(d, FALSE);
            return;
         }
         if (chk == TRUE)
            return;

         sprintf(buf, ch->pcdata->filename);
         d->character->desc = NULL;
         free_char(d->character);
         d->character = NULL;
         fOld = load_char_obj(d, buf, FALSE);
         ch = d->character;
         if (ch->position == POS_FIGHTING
            || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE || ch->position == POS_BERSERK)
            ch->position = POS_STANDING;

         sprintf(log_buf, "%s@%s(%s) has connected.", ch->pcdata->filename, d->host, d->user);
         if (ch->level < LEVEL_IMM)
         {
            /*to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->level ); */
            log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
         }
         else
            log_string_plus(log_buf, LOG_COMM, ch->level);
            
         {
           struct tm *tme;
           time_t now;
           char day[50];
           now = time(0);
           tme = localtime(&now);
           strftime(day, 50, "%a %b %d %H:%M:%S %Y", tme);
           sprintf(log_buf, "%-20s     %-24s    %s", ch->pcdata->filename, day, d->host);
           write_last_file(log_buf); 	
        }
        
	    if(!xIS_SET(ch->act, PLR_HASLASTNAME))
	    {
	   	   sprintf(buf, "\n\rIt has been detected that you do not have a last name!!!\n\rPlease choose a last name: \n\r");
		   write_to_buffer(d, buf, 0);
	 	   d->connected = CON_GET_LAST_NAME;
		   break;
	    }
	    if (!xIS_SET(ch->act, PLR_RPSETUP))
	    {
	       ch->managen = 0;
	       ch->hpgen = 0;
	       write_to_buffer(d, "Your hpgen and managen has been reset, remove your equipment and reequip it to fix it\n\r", 0);
	       xSET_BIT(ch->act, PLR_RPSETUP);
	    }
        find_next_con(ch, d);
        break;
         
      case CON_USE_RESET_CHAR:
         switch (*argument)
         {
            case 'y':
            case 'Y':
               write_to_buffer(d, (char *) echo_on_str, 0);
               ch = d->character;
               //Need to set some information since we have a quasi pfile here...
               xSET_BIT(ch->act, PLR_ANSI);
               xSET_BIT(ch->act, PLR_HASLASTNAME);
                        
               name_stamp_stats(ch);  
               if (!(fread_resetchar(d->character, FALSE)))
               {
                  d->character->desc = NULL;
                  free_char(d->character); /* Big Memory Leak before --Shaddai */
                  d->character = NULL;
                  d->connected = CON_GET_NAME;
                  return;   
               }
               ch->level = 0;
               ch->alignment = 0;  

               //Set languages
               if ((iLang = skill_lookup("common")) < 0)
                  bug("Nanny: cannot find common language.");
               else
               {
                  ch->pcdata->learned[iLang] = MAX_SKPOINTS;
                  ch->pcdata->ranking[iLang] = MAX_RANKING;
               }
               for (iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++)
                  if (lang_array[iLang] == race_table[ch->race]->language)
                     break;
               if (lang_array[iLang] == LANG_UNKNOWN)
                  bug("Nanny: invalid racial language.");
               else
               {
                  if ((iLang = skill_lookup(lang_names[iLang])) < 0)
                     bug("Nanny: cannot find racial language.");
                  else
                  {
                     ch->pcdata->learned[iLang] = MAX_SKPOINTS;
                     ch->pcdata->ranking[iLang] = MAX_RANKING;
                  }
               }
         
         
               ch->apply_res_fire[0] = base_resis_values(ch, 1);
               ch->apply_res_water[0] = base_resis_values(ch, 2);
               ch->apply_res_air[0] = base_resis_values(ch, 3);
               ch->apply_res_earth[0] = base_resis_values(ch, 4);
               ch->apply_res_energy[0] = base_resis_values(ch, 5);
               ch->apply_res_holy[0] = base_resis_values(ch, 6);
               ch->apply_res_unholy[0] = base_resis_values(ch, 7);
               ch->apply_res_nonmagic[0] = base_resis_values(ch, 8);
               ch->apply_res_magic[0] = base_resis_values(ch, 9);
               ch->apply_res_poison[0] = base_resis_values(ch, 10);
               ch->apply_res_paralysis[0] = base_resis_values(ch, 11);             
               prepare_to_login(ch, d);
               return;

            case 'n':
            case 'N':
               write_to_buffer(d, "Ok, then choose another name: ", 0);
               d->character->desc = NULL;
               free_char(d->character); /* Big Memory Leak before --Shaddai */
               d->character = NULL;
               d->connected = CON_GET_NAME;
               return;

            default:
               write_to_buffer(d, "Please type Yes or No: ", 0);
               break;
         }
         break;
         
         
      case CON_CREATE_NAME:
         if (argument[0] == '\0')
         {
            write_to_buffer(d, "You need to enter a name of a character, returning to the menu.\n\r\n\r", 0);
            send_account_menu(d);
            d->connected = CON_SHOW_ACCOUNT_MENU;
            return;
         }

         argument = capitalize(argument);

         /* Old players can keep their characters. -- Alty */
         if (!check_parse_name(argument, (d->newstate != 0)))
         {
            write_to_buffer(d, "Illegal name, try another.\n\rName: ", 0);
            return;
         }
         
         sprintf(fname, "%s%c/%s", LNAME_DIR, tolower(argument[0]), capitalize(argument));
         if (stat(fname, &fst) != -1)
         {
            write_to_buffer(d, "That name is already taken for a lastname.  Please choose another.\n\rName: ", 0);
            return;
         }
         
         sprintf(fname, "%s%c/%s", RESET_DIR, tolower(argument[0]), capitalize(argument));
         if (stat(fname, &fst) != -1)
         {
            write_to_buffer(d, "That name is in the reset queue, you cannot use it.\n\rName: ", 0);
            return;
         }

         if (check_playing(d, argument, FALSE) == BERR)
         {
            write_to_buffer(d, "Name: ", 0);
            return;
         }

         fOld = load_char_obj(d, argument, TRUE);
         if (!d->character)
         {
            sprintf(log_buf, "Bad player file %s@%s.", argument, d->host);
            log_string(log_buf);
            write_to_buffer(d, "Your playerfile is corrupt...Please notify xerves@rafermand.net\n\r", 0);
            close_socket(d, FALSE);
            return;
         }
         ch = d->character;

         /* telnet negotiation to see if they support MXP */
         write_to_buffer( d, (char *) will_mxp_str, 0 );   
         chk = check_reconnect(d, argument, FALSE);
         if (chk == BERR)
            return;

         if (chk)
         {
            fOld = TRUE;
         }
         else
         {
            if (wizlock && !IS_IMMORTAL(ch))
            {
               write_to_buffer(d, "The game is wizlocked.  Only immortals can connect now.\n\r", 0);
               write_to_buffer(d, "Either contact Xerves on ICQ at 602180, or try again later.\n\r", 0);
               close_socket(d, FALSE);
               return;
            }
         }

         if (fOld)
         {
            write_to_buffer(d, "That name is already taken.  Please choose another: ", 0);
            d->character->desc = NULL;
            free_char(d->character); /* Big Memory Leak before --Shaddai */
            d->character = NULL;
            return;
         }
         else
         {
            sprintf(buf, "Did I get that right, %s (Y/N)? ", argument);
            write_to_buffer(d, buf, 0);
            d->connected = CON_CONFIRM_NEW_NAME;
            return;
         }
         break;

/* Con state for self delete code, installed by Samson 1-18-98
       Code courtesy of Waldemar Thiel (Swiv) */

      case CON_CONFIRM_NEW_NAME:
         switch (*argument)
         {
            case 'y':
            case 'Y':
               write_to_buffer(d, (char *) echo_on_str, 0);
               write_to_buffer(d, "\n\rPlease choose a suitable last name for your character: ", 0);
               
               d->connected = CON_GET_LAST_NAME;
               break;

            case 'n':
            case 'N':
               write_to_buffer(d, "Ok, what IS it, then? ", 0);
               /* clear descriptor pointer to get rid of bug message in log */
               d->character->desc = NULL;
               free_char(d->character);
               d->character = NULL;
               d->connected = CON_CREATE_NAME;
               break;

            default:
               write_to_buffer(d, "Please type Yes or No. ", 0);
               break;
         }
         break;
         
      case CON_GET_LAST_NAME:
	if(argument[0] == '\0')
	{
           write_to_buffer(d, "Your character needs a last name!!!\n\r", 0);
	   return;
	}
	if (!check_parse_name(argument, TRUE))
        {
            write_to_buffer(d, "Illegal name, try another.\n\rName: ", 0);
            return;
        }
	sprintf(fname, "%s%c/%s", PLAYER_DIR, tolower(argument[0]), capitalize(argument));
        if (stat(fname, &fst) != -1)
        {
           send_to_char("That name is already taken for a player name.  Please choose another.\n\r", ch);
           return;
        }
        sprintf(fname, "%s%c/%s", LNAME_DIR, tolower(argument[0]), capitalize(argument));
        if (stat(fname, &fst) != -1)
        {
           send_to_char("That name is already taken for a last name.  Please choose another.\n\r", ch);
           return;
        }
        if (!str_cmp(argument, ch->name))
        {
           send_to_char("You cannot have your lastname the same as your first.\n\r", ch);
           return;
        }
	if(ch->last_name)
	{
	   remove_from_lastname_file(ch->last_name, ch->name);
           STRFREE(ch->last_name);
        }
	ch->last_name = STRALLOC(capitalize(argument));
	xSET_BIT(ch->act, PLR_HASLASTNAME);
	
	sprintf(buf, "\n\rDid I get that right, %s (Y/N)?", ch->last_name);
	write_to_buffer(d, buf, 0);
	d->connected = CON_CONFIRM_LAST_NAME;
        return;
                
      case CON_CONFIRM_LAST_NAME:
	switch (*argument)
	{
		case 'y':
		case 'Y':
			if(ch->level > 0)
			{
				find_next_con(ch, d);
				break;
			}
			else
			{
				write_to_buffer(d, "\n\rWhat is your sex (M/F/N)? ", 0);
				d->connected = CON_GET_NEW_SEX;
				break;
			}
		case 'n':
		case 'N':
			write_to_buffer(d, "\n\rOk, what IS it, then?", 0);
			d->character->last_name = NULL;
			d->connected = CON_GET_LAST_NAME;
			break;
		default:
			write_to_buffer(d, "\n\rPlease type Yes or No. ", 0);
			break;
	}
	break;     

      case CON_GET_NEW_SEX:
         switch (argument[0])
         {
            case 'm':
            case 'M':
               ch->sex = SEX_MALE;
               break;
            case 'f':
            case 'F':
               ch->sex = SEX_FEMALE;
               break;
            case 'n':
            case 'N':
               ch->sex = SEX_NEUTRAL;
               break;
            default:
               write_to_buffer(d, "That's not a sex.\n\rWhat IS your sex? ", 0);
               return;
         }

         write_to_buffer(d, "\n\rYou may choose from the following races, or type help [race] to learn more:\n\r[", 0);
         buf[0] = '\0';
         for (iRace = 0; iRace < MAX_RACE; iRace++)
         {
            if (race_table[iRace]->race_name && race_table[iRace]->race_name[0] != '\0'
               && str_cmp(race_table[iRace]->race_name, "unused"))
            {
               if (!race_table[iRace]->remort_race || (race_table[iRace]->remort_race && (ch->pcdata->tier > 1)))
               {
                  if (iRace > 0)
                  {
                     if (strlen(buf) + strlen(race_table[iRace]->race_name) > 77)
                     {
                        strcat(buf, "\n\r");
                        write_to_buffer(d, buf, 0);
                        buf[0] = '\0';
                     }
                     else
                        strcat(buf, " ");
                  }
                  strcat(buf, race_table[iRace]->race_name);
               }
            }
         }
         strcat(buf, "]\n\r: ");
         write_to_buffer(d, buf, 0);
         d->connected = CON_GET_NEW_RACE;
         break;

      case CON_GET_NEW_RACE:
         argument = one_argument(argument, arg);
         if (!str_cmp(arg, "help"))
         {
            for (iRace = 0; iRace < MAX_RACE; iRace++)
            {
               if (!race_table[iRace]->remort_race || (race_table[iRace]->remort_race && (ch->pcdata->tier > 1)))
               {
                  if (toupper(argument[0]) == toupper(race_table[iRace]->race_name[0]) && !str_prefix(argument, race_table[iRace]->race_name))
                  {
                     do_help(ch, argument);
                     write_to_buffer(d, "\n\rPlease choose a race (to refresh the list hit enter): ", 0);
                     return;
                  }
               }
            }
            write_to_buffer(d, "No help on that topic.  Please choose a race (to refresh the list hit enter): ", 0);
            return;
         }


         for (iRace = 0; iRace < MAX_RACE; iRace++)
         {
            if (!race_table[iRace]->remort_race || (race_table[iRace]->remort_race && (ch->pcdata->tier > 1)))
            {
               if (toupper(arg[0]) == toupper(race_table[iRace]->race_name[0]) && !str_prefix(arg, race_table[iRace]->race_name))
               {
                  ch->race = iRace;
                  break;
               }
            }
         }

         if (iRace == MAX_RACE
            || !race_table[iRace]->race_name || race_table[iRace]->race_name[0] == '\0'
            || !str_cmp(race_table[iRace]->race_name, "unused") || (race_table[iRace]->remort_race && (ch->pcdata->tier < 2)))
         {
             write_to_buffer(d, "\n\rThat's not a race.  You may choose from the following races, or type help [race] to learn more:\n\r[", 0);
             buf[0] = '\0';
             for (iRace = 0; iRace < MAX_RACE; iRace++)
             {
                if (race_table[iRace]->race_name && race_table[iRace]->race_name[0] != '\0'
                   && str_cmp(race_table[iRace]->race_name, "unused"))
                {
                   if (!race_table[iRace]->remort_race || (race_table[iRace]->remort_race && (ch->pcdata->tier > 1)))
                   {
                      if (iRace > 0)
                      {
                         if (strlen(buf) + strlen(race_table[iRace]->race_name) > 77)
                         {
                            strcat(buf, "\n\r");
                            write_to_buffer(d, buf, 0);
                            buf[0] = '\0';
                         }
                         else
                            strcat(buf, " ");
                      }
                      strcat(buf, race_table[iRace]->race_name);
                   }
                }
             }
             strcat(buf, "]\n\r: ");
             write_to_buffer(d, buf, 0); 
             return;
         }
         if (check_bans(d, ch, BAN_RACE, 0))
         {
            write_to_buffer(d, "That race is currently banned and not in use, sorry.\n\rWhat is your race? ", 0);
            return;
         }
         //Set languages
         if ((iLang = skill_lookup("common")) < 0)
            bug("Nanny: cannot find common language.");
         else
         {
            ch->pcdata->learned[iLang] = MAX_SKPOINTS;
            ch->pcdata->ranking[iLang] = MAX_RANKING;
         }
         for (iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++)
            if (lang_array[iLang] == race_table[ch->race]->language)
               break;
         if (lang_array[iLang] == LANG_UNKNOWN)
            bug("Nanny: invalid racial language.");
         else
         {
            if ((iLang = skill_lookup(lang_names[iLang])) < 0)
               bug("Nanny: cannot find racial language.");
            else
            {
               ch->pcdata->learned[iLang] = MAX_SKPOINTS;
               ch->pcdata->ranking[iLang] = MAX_RANKING;
            }
         }
         //Set stats 
         name_stamp_stats(ch);  
         ch->alignment = 0;  
         sprintf(buf, "\n\rYour Stats are:\n\rhp mana str dex con wis int lck\n\r%-2d %-2d   %-2d  %-2d  %-2d  %-2d  %-2d  %-2d\n\r", ch->hit, ch->mana, ch->perm_str, ch->perm_dex, ch->perm_con, ch->perm_wis, ch->perm_int, ch->perm_lck);
         write_to_buffer(d, buf, 0);
         sprintf(buf, "\n\rPlease choose the color of your character's skin: %s", get_skin_colors(ch->race));
         write_to_buffer(d, buf, 0);
         d->connected = CON_SKIN;
         break;
         
      case CON_SKIN:
         if (atoi(argument) < 1 || atoi(argument) > 15)
         {
            sprintf(buf, "\n\rThat choice is invalid, your choices are: %s", get_skin_colors(ch->race));
            write_to_buffer(d, buf, 0);
            return;
         }
         if (atoi(argument) == 15 && ch->race == RACE_OGRE)
         {
            sprintf(buf, "\n\rThat choice is invalid, your choices are: %s", get_skin_colors(ch->race));
            write_to_buffer(d, buf, 0);
            return;
         }
         ch->pcdata->skincolor = atoi(argument);
         sprintf(buf, "\n\rChoose the color of your hair: %s", get_hair_color());
         write_to_buffer(d, buf, 0);
         d->connected = CON_HAIRCOLOR;
         break;
         
      case CON_HAIRCOLOR:
         if (atoi(argument) < 1 || atoi(argument) > 40)
         {
            sprintf(buf, "\n\rThat choice is invalid, your choices are: %s", get_hair_color(ch));
            write_to_buffer(d, buf, 0);
            return;
         }
         ch->pcdata->haircolor = atoi(argument);
         sprintf(buf, "\n\rPlease choose the length of your hair: %s", get_hair_length());
         write_to_buffer(d, buf, 0);
         d->connected = CON_HAIRLENGTH;
         break;

      case CON_HAIRLENGTH:
         if (atoi(argument) < 1 || atoi(argument) > 13)
         {
            sprintf(buf, "\n\rThat choice is invalid, your choices are: %s", get_hair_length(ch));
            write_to_buffer(d, buf, 0);
            return;
         } 
         ch->pcdata->hairlength = atoi(argument);
         sprintf(buf, "\n\rPlease choose the style of your hair: %s", get_hair_style(ch));
         write_to_buffer(d, buf, 0);
         d->connected = CON_HAIRSTYLE;
         break;
      
      case CON_HAIRSTYLE:
         if (atoi(argument) < 1 || atoi(argument) > 15)
         {
            sprintf(buf, "\n\rThat choice is invalid, your choices are: %s", get_hair_style(ch));
            write_to_buffer(d, buf, 0);
            return;
         } 
         if (atoi(argument) == 7 && ch->pcdata->hairlength < 4)
         {   
            sprintf(buf, "\n\rThat choice is invalid, your choices are: %s", get_hair_style(ch));
            write_to_buffer(d, buf, 0);
            return;
         } 
         if (atoi(argument) >= 8 && atoi(argument) <= 9 && (ch->pcdata->hairlength < 4 || ch->pcdata->hairlength > 9))    
         {   
            sprintf(buf, "\n\rThat choice is invalid, your choices are: %s", get_hair_style(ch));
            write_to_buffer(d, buf, 0);
            return;
         } 
         if ((atoi(argument) < 9 || atoi(argument)) > 14 && ch->pcdata->hairlength < 7) 
         {   
            sprintf(buf, "\n\rThat choice is invalid, your choices are: %s", get_hair_style(ch));
            write_to_buffer(d, buf, 0);
            return;
         } 
         if (atoi(argument) == 15 && (ch->pcdata->hairlength < 7 || ch->pcdata->hairlength > 9))
         if ((atoi(argument) < 9 || atoi(argument) > 14) && ch->pcdata->hairlength < 7) 
         {   
            sprintf(buf, "\n\rThat choice is invalid, your choices are: %s", get_hair_style(ch));
            write_to_buffer(d, buf, 0);
            return;
         } 
         ch->pcdata->hairstyle = atoi(argument); 
         sprintf(buf, "\n\rPlease choose the eye color of your character: %s", get_eye_color());
         write_to_buffer(d, buf, 0);
         d->connected = CON_EYECOLOR;
         return;
         
      case CON_EYECOLOR:
         if (atoi(argument) < 1 || atoi(argument) > 24)
         {
            sprintf(buf, "\n\rThat choice is invalid, your choices are: %s", get_eye_color());
            write_to_buffer(d, buf, 0);
            return;
         } 
         ch->pcdata->eyecolor = atoi(argument);
         sprintf(buf, "\n\rPlease choose the height of your character: %s", get_char_height());
         write_to_buffer(d, buf, 0);
         d->connected = CON_HEIGHT;
         break;
         
      case CON_HEIGHT:
         if (atoi(argument) < 1 || atoi(argument) > 13)
         {
            sprintf(buf, "\n\rThat choice is invalid, your choices are: %s", get_char_height());
            write_to_buffer(d, buf, 0);
            return;
         } 
         ch->pcdata->cheight = atoi(argument);           
         sprintf(buf, "\n\rPlease choose the body frame of your character: %s", get_char_weight());
         write_to_buffer(d, buf, 0);
         d->connected = CON_WEIGHT;
         break;
         
      case CON_WEIGHT:
         if (atoi(argument) < 1 || atoi(argument) > 13)
         {
            sprintf(buf, "\n\rThat choice is invalid, your choices are: %s", get_char_weight());
            write_to_buffer(d, buf, 0);
            return;
         } 
         ch->pcdata->cweight = atoi(argument);
         if (ch->level > 0)
         {
            if(!xIS_SET(ch->act, PLR_HASLASTNAME))
	    {
		sprintf(buf, "\n\rIt has been detected that you do not have a last name!!!\n\rPlease choose a last name: \n\r");
		write_to_buffer(d, buf, 0);
	 	d->connected = CON_GET_LAST_NAME; 
   		break;
	    }
            find_next_con(ch, d);
            break;
         }
         ch->height = number_range(race_table[ch->race]->height * .9, race_table[ch->race]->height * 1.1);
         ch->weight = number_range(race_table[ch->race]->weight * .9, race_table[ch->race]->weight * 1.1);
         ch->height = ch->height * get_heightweight_percent(ch->pcdata->cheight) / 100;
         ch->weight = ch->weight * get_heightweight_percent(ch->pcdata->cweight) / 100;
         write_to_buffer(d, "\n\rPlease choose the hand orientation of your character (Right/Left)? ", 0);
         d->connected = CON_HAND;
         break;        
         
      case CON_HAND:
         switch (argument[0])
         {
            case 'l':
            case 'L':
               ch->pcdata->righthanded = 0;
               break;
            case 'r':
            case 'R':
               ch->pcdata->righthanded = 1;
               break;
            default:
               write_to_buffer(d, "Invalid selection, please choose Right or Left.", 0);
               return;
         }
         write_to_buffer(d, "\n\rBelow is where you can choose the element of your character.  This means a few\n\r"
                            "things, so you might want to type (help magic and help magic2) for more info\n\r\n\r\n\r"
                            "Please choose the element of your character:\n\r"
         "Fire Water  Air  Earth  Energy  Divine  Unholy.", 0);
         d->connected = CON_ELEMENT;
         break;
         
      case CON_ELEMENT:
         argument = one_argument(argument, arg);
         
         if (!str_cmp(arg, "help"))
         {
            if (!str_cmp(argument, "magic") || !str_cmp(argument, "magic2"))
            {
               do_help(ch, argument);
               write_to_buffer(d, "\n\rPlease choose your element: Fire   Water   Air   Earth   Energy   Divine  Unholy", 0);
               return;
            }
            else
            {
               write_to_buffer(d, "\n\rYou can only see the following two helpfiles: magic magic2", 0);
               return;
            }
         }
         if (!str_cmp(arg, "Fire"))
            SET_BIT(ch->elementb, ELEMENT_FIRE);
         else if (!str_cmp(arg, "Water"))
            SET_BIT(ch->elementb, ELEMENT_WATER);
         else if (!str_cmp(arg, "Air"))
            SET_BIT(ch->elementb, ELEMENT_AIR);
         else if (!str_cmp(arg, "Earth"))
            SET_BIT(ch->elementb, ELEMENT_EARTH);
         else if (!str_cmp(arg, "Energy"))
            SET_BIT(ch->elementb, ELEMENT_ENERGY);
         else if (!str_cmp(arg, "Divine"))
            SET_BIT(ch->elementb, ELEMENT_DIVINE);
         else if (!str_cmp(arg, "Unholy"))
            SET_BIT(ch->elementb, ELEMENT_UNHOLY);
         else
         {
            write_to_buffer(d, "\n\rInvalid selection: Fire  Water  Air  Earth  Energy  Divine  Unholy", 0);
            return;
         }
         
         ch->apply_res_fire[0] = base_resis_values(ch, 1);
         ch->apply_res_water[0] = base_resis_values(ch, 2);
         ch->apply_res_air[0] = base_resis_values(ch, 3);
         ch->apply_res_earth[0] = base_resis_values(ch, 4);
         ch->apply_res_energy[0] = base_resis_values(ch, 5);
         ch->apply_res_holy[0] = base_resis_values(ch, 6);
         ch->apply_res_unholy[0] = base_resis_values(ch, 7);
         ch->apply_res_nonmagic[0] = base_resis_values(ch, 8);
         ch->apply_res_magic[0] = base_resis_values(ch, 9);
         ch->apply_res_poison[0] = base_resis_values(ch, 10);
         ch->apply_res_paralysis[0] = base_resis_values(ch, 11);
         write_to_buffer(d, "\n\rWould you please type where you heard about us.  Such examples would be:\n\r", 0);
         write_to_buffer(d, "Mudconnector, Mailing List, Friend (please supply name).  We use this information to\n\r", 0);
         write_to_buffer(d, "keep track of who came from where and who referred who.  So please help us by typing\n\r", 0);
         write_to_buffer(d, "where you came from.  Thanks.\n\r\n\rWhere was it that you came from: ", 0);
         d->connected = CON_SOURCE;
         break;
         
      case CON_SOURCE:
         if (argument[0] == '\0')
         {
            write_to_buffer(d, "\n\rYou need to type something in, ex: mudconnector, <player's name>, email, etc\n\r", 0);
            return;
         }
         if (ch->pcdata->came_from)
            STRFREE(ch->pcdata->came_from);
         ch->pcdata->came_from = STRALLOC(argument);
         
         write_to_buffer(d, "\n\rWould you like ANSI, or no graphic/color support, (A/N)? ", 0);
	 d->connected = CON_GET_WANT_RIPANSI;
	 break;

      case CON_GET_WANT_RIPANSI:
         switch (argument[0])
         {
            case 'a':
            case 'A':
               xSET_BIT(ch->act, PLR_ANSI);
               break;
            case 'n':
            case 'N':
               break;
            default:
               write_to_buffer(d, "Invalid selection.\n\rANSI or NONE? ", 0);
               return;
         }
         sprintf(log_buf, "%s@%s new %s.", ch->name, d->host, race_table[ch->race]->race_name);
         log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
         to_channel(log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);
         sprintf(buf, "Account %s from %s has created player %s", d->account->name, d->host, d->character->name);
         log_string_plus(buf, LOG_NORMAL, sysdata.log_level);   
         write_to_buffer(d, "Press [ENTER] ", 0);
         ch->level = 0;
         /* Display_prompt interprets blank as default */
         ch->pcdata->prompt = STRALLOC("");
         ch->position = POS_STANDING;
         if (xIS_SET(ch->act, PLR_RIP))
            send_rip_screen(ch);
         if (xIS_SET(ch->act, PLR_ANSI))
            send_to_pager("\033[2J", ch);
         else
            send_to_pager("\014", ch);
         prepare_to_login(ch, d);
         return;
         break;

      case CON_LOGIN_MENU:
         argument = one_argument(argument, arg);
         argument = one_argument(argument, arg2);
         switch (arg[0])
         {
            case 'w':
            case 'W':
               write_to_buffer(d, "\n\r\n\r", 0);
               do_who(ch, "1");
               break;

            case 'i':
            case 'I':
               if (!IS_IMMORTAL(ch))
                  break;
               if (arg2[0] == '\0')
               {
                  write_to_buffer(d, "You need to supply a level with your option.\n\rYou can also type I 0 to remove invis\n\r", 0);
                  break;
               }
               if (atoi(arg2) == 0)
               {
                  write_to_buffer(d, "You can only turn off wizinvis in the game.\n\r", 0);
                  break;
               }
               if (atoi(arg2) < 1 || atoi(arg2) > MAX_LEVEL)
               {
                  write_to_buffer(d, "Your supplied level needs to be between 1 and the max level.\n\r", 0);
                  break;
               }
               if (atoi(arg2) > get_trust(ch))
               {
                  write_to_buffer(d, "Your supplied level is higher than your trust level.\n\r", 0);
                  break;
               }
               ch->pcdata->wizinvis = atoi(arg2);
               if (!xIS_SET(ch->act, PLR_WIZINVIS))
                  xSET_BIT(ch->act, PLR_WIZINVIS);
               sprintf(buf, "Invis is at level %d now.\n\r", atoi(arg2));
               write_to_buffer(d, buf, 0);
               break;

               write_to_buffer(d, buf, 0);
               break;

            case 'g':
            case 'G':
               if (arg2[0] == '\0')
               {
                  do_global_boards(ch, "");
                  set_char_color(AT_DGREEN, ch);
                  write_to_buffer(d,
                     "\n\r\n\rYou may use G board <number> to change boards\n\rOr G read [number] to read a note\n\rOr G list to get a list of notes.\n\rYou may go back to the menu by hitting enter or typing another choice\n\r> ",
                     0);
                  return;
               }
               if (!str_cmp(arg2, "board"))
               {
                  if (argument[0] == '\0')
                  {
                     set_char_color(AT_DGREEN, ch);
                     write_to_buffer(d, "You need to choose one of the boards to change it.\n\r> ", 0);
                     return;
                  }
                  if (is_number(argument))
                  {
                     write_to_buffer(d, "\n\r", 0);
                     do_global_boards(ch, argument);
                     set_char_color(AT_DGREEN, ch);
                     write_to_buffer(d, "\n\r> ", 0);
                     return;
                  }
                  else
                  {
                     set_char_color(AT_DGREEN, ch);
                     write_to_buffer(d, "Needs to be a number to change boards.\n\r> ", 0);
                     return;
                  }
               }
               if (!str_cmp(arg2, "read"))
               {
                  if (argument[0] == '\0')
                  {
                     set_char_color(AT_DGREEN, ch);
                     write_to_buffer(d, "You need to select a note in thie list to read.\n\r> ", 0);
                     return;
                  }
                  if (is_number(argument))
                  {
                     sprintf(buf, "read %s", argument);
                     write_to_buffer(d, "\n\r", 0);
                     do_global_note(ch, buf);
                     set_char_color(AT_DGREEN, ch);
                     write_to_buffer(d, "\n\r> ", 0);
                     return;
                  }
                  else
                  {
                     set_char_color(AT_DGREEN, ch);
                     write_to_buffer(d, "Needs to be a number.\n\r> ", 0);
                     return;
                  }
               }
               if (!str_cmp(arg2, "list"))
               {
                  write_to_buffer(d, "\n\r", 0);
                  do_global_note(ch, "list");
                  set_char_color(AT_DGREEN, ch);
                  write_to_buffer(d, "\n\r> ", 0);
                  return;
               }
               if (str_cmp(arg2, "list") && str_cmp(arg2, "read") && str_cmp(arg2, "board"))
               {
                  set_char_color(AT_DGREEN, ch);
                  write_to_buffer(d,
                     "\n\r\n\rYou may use G board <number> to change boards\n\rOr G read [number] to read a note\n\rOr G list to get a list of notes.\n\rYou may go back to the menu by hitting enter or typing another choice\n\r>",
                     0);
                  return;
               }
               break;

            case 'n':
            case 'N':
               write_to_buffer(d, "\n\r", 0);
               set_char_color(AT_DGREEN, ch);
               do_help(ch, "news");
               set_char_color(AT_DGREEN, ch);
               break;

            case 'l':
            case 'L':
               cont = 1;
               break;

            default:
               set_char_color(AT_DGREEN, ch);
               sprintf(buf, "\n\r\n\rThat is not an option, please choose an option from the menu.\n\r\n\r");
               if (IS_IMMORTAL(ch))
               {
                  strcat(buf, "[W] Who is online\n\r[I] Go Wizinvis\n\r[G] Gboard notes\n\r[N] News\n\r[L] Login\n\r> ");
               }
               else
               {
                  strcat(buf, "[W] Who is online\n\r[G] Gboard notes\n\r[N] News\n\r[L] Login\n\r> ");
               }
               write_to_buffer(d, buf, 0);
               return;
         }

         if (cont == 1)
         {
            if (xIS_SET(ch->act, PLR_RIP))
               send_rip_screen(ch);
            if (xIS_SET(ch->act, PLR_ANSI))
               send_to_pager("\033[2J", ch);
            else
               send_to_pager("\014", ch);
            prepare_to_login(ch, d);
            break;
         }
         else
         {
            set_char_color(AT_DGREEN, ch);
            sprintf(buf, "\n\r\n\r\n\rPlease choose an option from the menu.\n\r\n\r");
            if (IS_IMMORTAL(ch))
            {
               strcat(buf, "[W] Who is online\n\r[I] Go Wizinvis\n\r[G] Gboard notes\n\r[N] News\n\r[L] Login\n\r> ");
            }
            else
            {
               strcat(buf, "[W] Who is online\n\r[G] Gboard notes\n\r[N] News\n\r[L] Login\n\r> ");
            }
            write_to_buffer(d, buf, 0);
            return;
         }

      case CON_NOTE_TO:
         handle_con_note_to(d, argument);
         break;

      case CON_NOTE_SUBJECT:
         handle_con_note_subject(d, argument);
         break; /* subject */

      case CON_NOTE_EXPIRE:
         handle_con_note_expire(d, argument);
         break;
/*
    case CON_NOTE_TEXT:
        handle_con_note_text (d, argument);
        break;
  */
      case CON_NOTE_FINISH:
         handle_con_note_finish(d, argument);
         break;

   }

   return;
}

bool is_reserved_name(char *name)
{
   RESERVE_DATA *res;

   for (res = first_reserved; res; res = res->next)
      if ((*res->name == '*' && !str_infix(res->name + 1, name)) || !str_cmp(res->name, name))
         return TRUE;
   return FALSE;
}


/*
 * Parse a name for acceptability.
 */
bool check_parse_name(char *name, bool newchar)
{
   /*
    * Names checking should really only be done on new characters, otherwise
    * we could end up with people who can't access their characters.  Would
    * have also provided for that new area havoc mentioned below, while still
    * disallowing current area mobnames.  I personally think that if we can
    * have more than one mob with the same keyword, then may as well have
    * players too though, so I don't mind that removal.  -- Alty
    */

   if (is_reserved_name(name) && newchar)
      return FALSE;

   /*
    * Outdated stuff -- Alty
    */
/*     if ( is_name( name, "all auto immortal self someone god supreme demigod dog guard cityguard cat cornholio spock hicaine hithoric death ass fuck shit piss crap quit" ) )
       return FALSE;*/

   /*
    * Length restrictions.
    */
   if (strlen(name) < 3)
      return FALSE;

   if (strlen(name) > 12)
      return FALSE;

   /*
    * Alphanumerics only.
    * Lock out IllIll twits.
    */
   {
      char *pc;
      bool fIll;

      fIll = TRUE;
      for (pc = name; *pc != '\0'; pc++)
      {
         if (!isalpha(*pc))
            return FALSE;
         if (LOWER(*pc) != 'i' && LOWER(*pc) != 'l')
            fIll = FALSE;
      }

      if (fIll)
         return FALSE;
   }

   /*
    * Code that followed here used to prevent players from naming
    * themselves after mobs... this caused much havoc when new areas
    * would go in...
    */

   return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect(DESCRIPTOR_DATA * d, char *name, bool fConn)
{
   CHAR_DATA *ch;
   char motdbuf[MIL];

   for (ch = first_char; ch; ch = ch->next)
   {
      if (!IS_NPC(ch) && (!fConn || !ch->desc) && ch->pcdata->filename && !str_cmp(name, ch->pcdata->filename))
      {
         if (fConn && ch->switched)
         {
            write_to_buffer(d, "Already playing.\n\rName: ", 0);
            d->connected = CON_GET_NAME;
            if (d->character)
            {
               /* clear descriptor pointer to get rid of bug message in log */
               d->character->desc = NULL;
               free_char(d->character);
               d->character = NULL;
            }
            return BERR;
         }
         if (fConn == FALSE)
         {
            /*Well this is useless
            DISPOSE(d->character->pcdata->pwd);
            d->character->pcdata->pwd = str_dup(ch->pcdata->pwd); */
            ;
         }
         else
         {
            /* clear descriptor pointer to get rid of bug message in log */
            d->character->desc = NULL;
            free_char(d->character);
            d->character = ch;
            ch->desc = d;
            ch->timer = 0;
            send_to_char("Reconnecting.\n\r", ch);
            do_look(ch, "auto");
            act(AT_ACTION, "$n has reconnected.", ch, NULL, NULL, TO_CANSEE);
            sprintf(log_buf, "%s@%s(%s) reconnected.", ch->pcdata->filename, d->host, d->user);
            log_string_plus(log_buf, LOG_COMM, UMAX(sysdata.log_level, ch->level));
            d->connected = CON_PLAYING;
            if (ch->level > 0)
            {
               if (ch->pcdata->kingdompid > 1)
               {
                  int kx;
               
                  if (ch->pcdata->hometown >= sysdata.max_kingdom)
                  {
                     ch->pcdata->hometown = 0;
                     ch->pcdata->kingdompid = 0;
                     ch->pcdata->town = NULL;
                  }
                  if (ch->pcdata->kingdompid != kingdom_table[ch->pcdata->hometown]->kpid)
                  {
                     for (kx = 2; kx < sysdata.max_kingdom; kx++)
                     {
                        if (ch->pcdata->kingdompid == kingdom_table[kx]->kpid)
                        {
                           ch->pcdata->hometown = kx;
                           break;
                        }
                     }
                     if (kx == sysdata.max_kingdom)
                     {
                        sprintf(motdbuf, "&R*************************************************\n\rIt appears your kingdom has been destroyed!!!!!!!!\n\r*************************************************\n\r");
                        send_to_char(motdbuf, ch);
                        ch->pcdata->hometown = 0;
                        ch->pcdata->kingdompid = 0;
                        ch->pcdata->town = NULL;
                     }
                 }
               } 
               if (ch->pcdata->hometown >= sysdata.max_kingdom)
               {
                  ch->pcdata->hometown = 0;
                  ch->pcdata->kingdompid = 0;
                  ch->pcdata->town = NULL;
               }
               if (ch->pcdata->kingdompid == 0 && ch->pcdata->hometown != 0)
               {
                  ch->pcdata->kingdompid = kingdom_table[ch->pcdata->hometown]->kpid;
               }
            }
            if (ch->pcdata->hometown > 1)
            {
               TOWN_DATA *town = NULL;
               
               if (ch->pcdata->town)
                  town = get_town(ch->pcdata->town->name);
            
               if (!town || town->kingdom != ch->pcdata->hometown)//no hometown
               {
                  send_to_char("&R*******************************************************\n\rYour town has been destroyed, putting you in the default town if your kingdom still exists.\n\r*******************************************************\n\r", ch);
                  town = get_town(kingdom_table[ch->pcdata->hometown]->dtown);
                  if (town)
                     ch->pcdata->town = town;
                  else
                     ch->pcdata->town = NULL;
               }
            } 
            /* Inform the character of a note in progress and the possbility of continuation! */
            if (ch->pcdata->in_progress)
            {
               send_to_char("You got disconnected while writing, will have to write it over.\n\r", ch);
               free_global_note(ch->pcdata->in_progress);
               ch->pcdata->in_progress = NULL;
               ch->editor = NULL;
               ch->dest_buf = NULL;
               ch->spare_ptr = NULL;
               ch->substate = SUB_NONE;
            }
         }
         return TRUE;
      }
   }

   return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing(DESCRIPTOR_DATA * d, char *name, bool kick)
{
   CHAR_DATA *ch;

   DESCRIPTOR_DATA *dold;
   char motdbuf[MIL];
   int cstate;

   for (dold = first_descriptor; dold; dold = dold->next)
   {
      if (dold != d
         && (dold->character || dold->original)
         && !str_cmp(name, dold->original ? dold->original->pcdata->filename : dold->character->pcdata->filename))
      {
         cstate = dold->connected;
         ch = dold->original ? dold->original : dold->character;
         if (!ch->name || (cstate != CON_PLAYING && cstate != CON_EDITING && cstate != CON_ROLL_STATS))
         {
            write_to_buffer(d, "Already connected - try again.\n\r", 0);
            sprintf(log_buf, "%s already connected.", ch->pcdata->filename);
            log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
            return BERR;
         }
         if (!kick)
            return TRUE;
         write_to_buffer(d, "Already playing... Kicking off old connection.\n\r", 0);
         write_to_buffer(dold, "Kicking off old connection... bye!\n\r", 0);
         close_socket(dold, FALSE);
         /* clear descriptor pointer to get rid of bug message in log */
         d->character->desc = NULL;
         free_char(d->character);
         d->character = ch;
         ch->desc = d;
         ch->timer = 0;
         if (ch->switched)
            do_return(ch->switched, "");
         ch->switched = NULL;
         send_to_char("Reconnecting.\n\r", ch);
         do_look(ch, "auto");
         if (ch->level > 0)
         {
            if (ch->pcdata->kingdompid > 1)
            {
               int kx;
               
               if (ch->pcdata->hometown >= sysdata.max_kingdom)
               {
                  ch->pcdata->hometown = 0;
                  ch->pcdata->kingdompid = 0;
                  ch->pcdata->town = NULL;
               }
               if (ch->pcdata->kingdompid != kingdom_table[ch->pcdata->hometown]->kpid)
               {
                  for (kx = 2; kx < sysdata.max_kingdom; kx++)
                  {
                     if (ch->pcdata->kingdompid == kingdom_table[kx]->kpid)
                     {
                        ch->pcdata->hometown = kx;
                        break;
                     }
                  }
                  if (kx == sysdata.max_kingdom)
                  {
                     sprintf(motdbuf, "&R*************************************************\n\rIt appears your kingdom has been destroyed!!!!!!!!\n\r*************************************************\n\r");
                     send_to_char(motdbuf, ch);
                     ch->pcdata->hometown = 0;
                     ch->pcdata->kingdompid = 0;
                     ch->pcdata->town = NULL;
                  }
               }
            } 
            if (ch->pcdata->hometown >= sysdata.max_kingdom)
            {
               ch->pcdata->hometown = 0;
               ch->pcdata->kingdompid = 0;
               ch->pcdata->town = NULL;
            }
            if (ch->pcdata->kingdompid == 0 && ch->pcdata->hometown != 0)
            {
               ch->pcdata->kingdompid = kingdom_table[ch->pcdata->hometown]->kpid;
            }
         }
         
         if (ch->pcdata->hometown > 1)
         {
            TOWN_DATA *town = NULL;
            
            if (ch->pcdata->town)
               town = get_town(ch->pcdata->town->name);
            
            if (!town || town->kingdom != ch->pcdata->hometown)//no hometown
            {
               send_to_char("&R*******************************************************\n\rYour town has been destroyed, putting you in the default town if your kingdom still exists.\n\r*******************************************************\n\r", ch);
               town = get_town(kingdom_table[ch->pcdata->hometown]->dtown);
               if (town)
                  ch->pcdata->town = town;
               else
                  ch->pcdata->town = NULL;
            }
         } 
         act(AT_ACTION, "$n has reconnected, kicking off old link.", ch, NULL, NULL, TO_CANSEE);
         sprintf(log_buf, "%s@%s reconnected, kicking off old link.", ch->pcdata->filename, d->host);
         log_string_plus(log_buf, LOG_COMM, UMAX(sysdata.log_level, ch->level));
         d->connected = cstate;
         return TRUE;
      }
   }

   return FALSE;
}



void stop_idling(CHAR_DATA * ch)
{
   ROOM_INDEX_DATA *was_in_room;

   if (!ch || !ch->desc || ch->desc->connected != CON_PLAYING || !ch->was_in_room || ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
      return;

   ch->timer = 0;
   was_in_room = ch->was_in_room;
   char_from_room(ch);
   char_to_room(ch, was_in_room);
   /*
      ch->was_in_room = NULL;
    */
   act(AT_ACTION, "$n has returned from the void.", ch, NULL, NULL, TO_ROOM);
   return;
}

/*
 * Same as above, but converts &color codes to ANSI sequences..
 */
void send_to_char_color(const char *txt, CHAR_DATA * ch)
{
   DESCRIPTOR_DATA *d;
   char *colstr;
   const char *prevstr = txt;
   char colbuf[20];
   int ln;

   if (!ch)
   {
      bug("Send_to_char_color: NULL *ch");
      return;
   }
   if (!txt || !ch->desc)
      return;
   d = ch->desc;
   
   /* Clear out old color stuff */
/*  make_color_sequence(NULL, NULL, NULL);*/
   while ((colstr = strpbrk(prevstr, "&^")) != NULL)
   {
      if (colstr > prevstr)
         write_to_buffer(d, prevstr, (colstr - prevstr));
      ln = make_color_sequence(colstr, colbuf, d);
      if (ln < 0)
      {
         prevstr = colstr + 1;
         break;
      }
      else if (ln > 0)
         write_to_buffer(d, colbuf, ln);
      prevstr = colstr + 2;
   }
   if (*prevstr)
      write_to_buffer(d, prevstr, 0);
   return;
}

void write_to_pager(DESCRIPTOR_DATA * d, const char *txt, int length)
{
   int pageroffset; /* Pager fix by thoric */
   int origlength;   /* for MXP */

   if (length <= 0)
      length = strlen(txt);
   if (length == 0)
      return;
      
   origlength = length;
 
   /* work out how much we need to expand/contract it */
   length += count_mxp_tags (d->mxp, txt, length);

   if (!d->pagebuf)
   {
      d->pagesize = MSL;
      CREATE(d->pagebuf, char, d->pagesize);
   }
   if (!d->pagepoint)
   {
      d->pagepoint = d->pagebuf;
      d->pagetop = 0;
      d->pagecmd = '\0';
   }
   if (d->pagetop == 0 && !d->fcommand)
   {
      d->pagebuf[0] = '\n';
      d->pagebuf[1] = '\r';
      d->pagetop = 2;
   }
   pageroffset = d->pagepoint - d->pagebuf; /* pager fix (goofup fixed 08/21/97) */
   while (d->pagetop + length >= d->pagesize)
   {
      if (d->pagesize > 32000)
      {
         bug("Pager overflow.  Ignoring.\n\r");
         d->pagetop = 0;
         d->pagepoint = NULL;
         DISPOSE(d->pagebuf);
         d->pagesize = MSL;
         return;
      }
      d->pagesize *= 2;
      RECREATE(d->pagebuf, char, d->pagesize);
   }
   d->pagepoint = d->pagebuf + pageroffset; /* pager fix (goofup fixed 08/21/97) */
   convert_mxp_tags (d->mxp, d->pagebuf+d->pagetop, txt, origlength );
   d->pagetop += length;
   d->pagebuf[d->pagetop] = '\0';
   return;
}

void send_to_pager_color(const char *txt, CHAR_DATA * ch)
{
   DESCRIPTOR_DATA *d;
   char *colstr;
   const char *prevstr = txt;
   char colbuf[20];
   int ln;

   if (!ch)
   {
      bug("Send_to_pager_color: NULL *ch");
      return;
   }
   if (!txt || !ch->desc)
      return;
   d = ch->desc;
   ch = d->original ? d->original : d->character;
   if (IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON))
   {
      send_to_char_color(txt, d->character);
      return;
   }
   /* Clear out old color stuff */
/*  make_color_sequence(NULL, NULL, NULL);*/
   while ((colstr = strpbrk(prevstr, "&^")) != NULL)
   {
      if (colstr > prevstr)
         write_to_pager(d, prevstr, (colstr - prevstr));
      ln = make_color_sequence(colstr, colbuf, d);
      if (ln < 0)
      {
         prevstr = colstr + 1;
         break;
      }
      else if (ln > 0)
         write_to_pager(d, colbuf, ln);
      prevstr = colstr + 2;
   }
   if (*prevstr)
      write_to_pager(d, prevstr, 0);
   return;
}

/*
void set_char_color( sh_int AType, CHAR_DATA *ch )
{
    char buf[16];
    CHAR_DATA *och;
    
    if ( !ch || !ch->desc )
      return;
    
    och = (ch->desc->original ? ch->desc->original : ch);
    if ( !IS_NPC(och) && xIS_SET(och->act, PLR_ANSI) )
    {
	if ( AType == 7 )
	  strcpy( buf, "\033[m" );
	else
	  sprintf(buf, "\033[0;%d;%s%dm", (AType & 8) == 8,
	        (AType > 15 ? "5;" : ""), (AType & 7)+30);
	write_to_buffer( ch->desc, buf, strlen(buf) );
      ch->desc->prevcolor = AType;
    }
    return;
}

void set_pager_color( sh_int AType, CHAR_DATA *ch )
{
    char buf[16];
    CHAR_DATA *och;
    
    if ( !ch || !ch->desc )
      return;
    
    och = (ch->desc->original ? ch->desc->original : ch);
    if ( !IS_NPC(och) && xIS_SET(och->act, PLR_ANSI) )
    {
	if ( AType == 7 )
	  strcpy( buf, "\033[m" );
	else
	  sprintf(buf, "\033[0;%d;%s%dm", (AType & 8) == 8,
	        (AType > 15 ? "5;" : ""), (AType & 7)+30);
	send_to_pager( buf, ch );
	ch->desc->pagecolor = AType;
    }
    return;
}
*/

/* source: EOD, by John Booth <???> */
void ch_printf(CHAR_DATA * ch, char *fmt, ...)
{
   char buf[MSL * 2]; /* better safe than sorry */
   va_list args;

   va_start(args, fmt);
   vsprintf(buf, fmt, args);
   va_end(args);

   send_to_char(buf, ch);
}

void pager_printf(CHAR_DATA * ch, char *fmt, ...)
{
   char buf[MSL * 2];
   va_list args;

   va_start(args, fmt);
   vsprintf(buf, fmt, args);
   va_end(args);

   send_to_pager(buf, ch);
}


/*
 * Function to strip off the "a" or "an" or "the" or "some" from an object's
 * short description for the purpose of using it in a sentence sent to
 * the owner of the object.  (Ie: an object with the short description
 * "a long dark blade" would return "long dark blade" for use in a sentence
 * like "Your long dark blade".  The object name ipsn't always appropriate
 * since it contains keywords that may not look proper.		-Thoric
 */
char *myobj(OBJ_DATA * obj)
{
   if (!str_prefix("a ", obj->short_descr))
      return obj->short_descr + 2;
   if (!str_prefix("an ", obj->short_descr))
      return obj->short_descr + 3;
   if (!str_prefix("the ", obj->short_descr))
      return obj->short_descr + 4;
   if (!str_prefix("some ", obj->short_descr))
      return obj->short_descr + 5;
   return obj->short_descr;
}

void log_printf(char *fmt, ...)
{
   char buf[MSL * 2];
   va_list args;

   va_start(args, fmt);
   vsprintf(buf, fmt, args);
   va_end(args);

   log_string(buf);
}

char *obj_short(OBJ_DATA * obj)
{
   static char buf[MSL];

   if (obj->count > 1)
   {
      sprintf(buf, "%s (%d)", obj->short_descr, obj->count);
      return buf;
   }
   return obj->short_descr;
}

/*
 * The primary output interface for formatted output.
 */
/* Major overhaul. -- Alty */

void ch_printf_color(CHAR_DATA * ch, char *fmt, ...)
{
   char buf[MSL * 2];
   va_list args;

   va_start(args, fmt);
   vsprintf(buf, fmt, args);
   va_end(args);

   send_to_char_color(buf, ch);
}

void pager_printf_color(CHAR_DATA * ch, char *fmt, ...)
{
   char buf[MSL * 2];
   va_list args;

   va_start(args, fmt);
   vsprintf(buf, fmt, args);
   va_end(args);

   send_to_pager_color(buf, ch);
}

#define MORPHNAME(ch)   ((ch->morph&&ch->morph->morph)? \
                         ch->morph->morph->short_desc: \
                         IS_NPC(ch) ? ch->short_descr : ch->name)
#define NAME(ch)        (IS_NPC(ch) ? ch->short_descr : ch->name)

char *act_string(const char *format, CHAR_DATA * to, CHAR_DATA * ch, const void *arg1, const void *arg2, int flags)
{
   static char *const he_she[] = { "it", "he", "she" };
   static char *const him_her[] = { "it", "him", "her" };
   static char *const his_her[] = { "its", "his", "her" };
   static char buf[MSL];
   char fname[MIL];
   char temp[MSL];
   char *point = buf;
   const char *str = format;
   const char *i;
   CHAR_DATA *vch = (CHAR_DATA *) arg2;
   OBJ_DATA *obj1 = (OBJ_DATA *) arg1;
   OBJ_DATA *obj2 = (OBJ_DATA *) arg2;

   while (*str != '\0')
   {
      if (*str != '$')
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      if (!arg2 && *str >= 'A' && *str <= 'Z')
      {
         bug("Act: missing arg2 for code %c:", *str);
         bug(format);
         i = " <@@@> ";
      }
      else
      {
         switch (*str)
         {
            default:
               bug("Act: bad code %c.", *str);
               i = " <@@@> ";
               break;
            case 't':
               i = (char *) arg1;
               break;
            case 'T':
               i = (char *) arg2;
               break;
            case 'n':
               if (ch->morph == NULL)
               {
                  if (to && ch == to)
                     i = "You";
                  else
                     i = (to ? PERS_MAP(ch, to) : NAME(ch));
               }
               else if (!IS_SET(flags, STRING_IMM))
                  i = (to ? MORPHPERS(ch, to) : MORPHNAME(ch));
               else
               {
                  sprintf(temp, "(MORPH) %s", (to ? PERS_MAP(ch, to) : NAME(ch)));
                  i = temp;
               }
               break;
            case 'N':
               if (vch->morph == NULL)
               {
                  if (to && to == vch)
                     i = "You";
                  else
                     i = (to ? PERS_MAP(vch, to) : NAME(vch));
               }
               else if (!IS_SET(flags, STRING_IMM))
                  i = (to ? MORPHPERS(vch, to) : MORPHNAME(vch));
               else
               {
                  sprintf(temp, "(MORPH) %s", (to ? PERS_MAP(vch, to) : NAME(vch)));
                  i = temp;
               }
               break;

            case 'e':
               if (ch->sex > 2 || ch->sex < 0)
               {
                  bug("act_string: player %s has sex set at %d!", ch->name, ch->sex);
                  i = "it";
               }
               else if (to && to == ch)
                  i = "you";
               else if (get_wear_hidden_cloak(ch) && ch != to)
                  i = "it";
               else
                  i = he_she[URANGE(0, ch->sex, 2)];
               break;
            case 'E':
               if (vch->sex > 2 || vch->sex < 0)
               {
                  bug("act_string: player %s has sex set at %d!", vch->name, vch->sex);
                  i = "it";
               }
               else if (to && to == vch)
                  i = "you";
               else if (get_wear_hidden_cloak(vch) && vch != to)
                  i = "it";
               else
                  i = he_she[URANGE(0, vch->sex, 2)];
               break;
            case 'm':
               if (ch->sex > 2 || ch->sex < 0)
               {
                  bug("act_string: player %s has sex set at %d!", ch->name, ch->sex);
                  i = "it";
               }
               else if (to && to == ch)
                  i = "you";
               else if (get_wear_hidden_cloak(ch) && ch != to)
                  i = "it";
               else
                  i = him_her[URANGE(0, ch->sex, 2)];
               break;
            case 'M':
               if (vch->sex > 2 || vch->sex < 0)
               {
                  bug("act_string: player %s has sex set at %d!", vch->name, vch->sex);
                  i = "it";
               }
               else if (to && to == vch)
                  i = "you";
               else if (get_wear_hidden_cloak(vch) && vch != to)
                  i = "it";
               else
                  i = him_her[URANGE(0, vch->sex, 2)];
               break;
            case 's':
               if (ch->sex > 2 || ch->sex < 0)
               {
                  bug("act_string: player %s has sex set at %d!", ch->name, ch->sex);
                  i = "its";
               }
               else if (to && ch == to)
                  i = "your";
               else if (get_wear_hidden_cloak(ch) && ch != to)
                  i = "its";
               else
                  i = his_her[URANGE(0, ch->sex, 2)];
               break;
            case 'S':
               if (vch->sex > 2 || vch->sex < 0)
               {
                  bug("act_string: player %s has sex set at %d!", vch->name, vch->sex);
                  i = "its";
               }
               else if (to && vch == to)
                  i = "your";
               else if (get_wear_hidden_cloak(vch) && vch != to)
                  i = "it";
               else
                  i = his_her[URANGE(0, vch->sex, 2)];
               break;
            case 'q':
               i = (to == ch) ? "" : "s";
               break;
            case 'Q':
               i = (to == ch) ? "your" : his_her[URANGE(0, ch->sex, 2)];
               if (get_wear_hidden_cloak(ch) && ch != to)
                  i = "it";
               break;
            case 'p':
               i = (!to || can_see_obj(to, obj1) ? obj_short(obj1) : "something");
               break;
            case 'P':
               i = (!to || can_see_obj(to, obj2) ? obj_short(obj2) : "something");
               break;
            case 'd':
               if (!arg2 || ((char *) arg2)[0] == '\0')
                  i = "door";
               else
               {
                  one_argument((char *) arg2, fname);
                  i = fname;
               }
               break;
         }
      }
      ++str;
      while ((*point = *i) != '\0')
         ++point, ++i;

      /*  #0  0x80c6c62 in act_string (
         format=0x81db42e "$n has reconnected, kicking off old link.", to=0x0, 
         ch=0x94fcc20, arg1=0x0, arg2=0x0, flags=2) at comm.c:2901 */
   }
   strcpy(point, "\n\r");
   buf[0] = UPPER(buf[0]);
   return buf;
}

#undef NAME

void act(sh_int AType, const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type)
{
   char *txt;
   CHAR_DATA *to;
   CHAR_DATA *vch = (CHAR_DATA *) arg2;

   /*
    * Discard null and zero-length messages.
    */
   if (!format || format[0] == '\0')
      return;

   if (!ch)
   {
      bug("Act: null ch. (%s)", format);
      return;
   }

   if (type == TO_MUD || type == TO_ALLMUD)
      to = first_char;
   if (!ch->in_room)
      to = NULL;
   else if (type == TO_CHAR)
      to = ch;
   else
      to = ch->in_room->first_person;
   

   /*
    * ACT_SECRETIVE handling
    */
   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_SECRETIVE) && type != TO_CHAR && type != TO_MUD && type != TO_ALLMUD)
      return;

   if (type == TO_VICT)
   {
      if (!vch)
      {
         bug("Act: null vch with TO_VICT.");
         bug("%s (%s)", ch->name, format);
         return;
      }
      if (!vch->in_room)
      {
         bug("Act: vch in NULL room!");
         bug("%s -> %s (%s)", ch->name, vch->name, format);
         return;
      }
      to = vch;
/*	to = vch->in_room->first_person;*/
   }

   if (MOBtrigger && type != TO_CHAR && type != TO_VICT && to)
   {
      OBJ_DATA *to_obj;

      txt = act_string(format, NULL, ch, arg1, arg2, STRING_IMM);
      if (HAS_PROG(to->in_room, ACT_PROG))
         rprog_act_trigger(txt, to->in_room, ch, (OBJ_DATA *) arg1, (void *) arg2);
      for (to_obj = to->in_room->first_content; to_obj; to_obj = to_obj->next_content)
         if (HAS_PROG(to_obj->pIndexData, ACT_PROG))
            oprog_act_trigger(txt, to_obj, ch, (OBJ_DATA *) arg1, (void *) arg2);
   }
   for (; to; to = (type == TO_ALLMUD || type == TO_MUD) ? to->next : (type == TO_CHAR || type == TO_VICT) ? NULL : to->next_in_room)
   {
      if ((!to->desc && (IS_NPC(to) && !HAS_PROG(to->pIndexData, ACT_PROG))) || !IS_AWAKE(to))
         continue;

      if (type == TO_CHAR)
      {
         if (to != ch)
            continue;
         if (IS_ONMAP_FLAG(to))
         {
            if (to->map != ch->map
               || to->coord->x != ch->coord->x
               || to->coord->y != ch->coord->y)
               continue;
         }
         if (IS_ONMAP_FLAG(ch))
         {
            if (to->map != ch->map
               || to->coord->x != ch->coord->x
               || to->coord->y != ch->coord->y)
               continue;
         }
      }
      if (type == TO_VICT && (to != vch || to == ch))
         continue;
      if (type == TO_ROOM)
      {
         if (to == ch)
            continue;
         if (IS_ONMAP_FLAG(to))
         {
            if (to->map != ch->map
               || to->coord->x != ch->coord->x
               || to->coord->y != ch->coord->y)
               continue;
         }
         if (IS_ONMAP_FLAG(ch))
         {
            if (to->map != ch->map
               || to->coord->x != ch->coord->x
               || to->coord->y != ch->coord->y)
               continue;
         }
      }
      if (type == TO_MUD && (to == ch || to == vch))
         continue;
      if (type == TO_ALLMUD && ((!IS_NPC(to)) && (get_trust(to) < ch->pcdata->board->read_level)))
         continue;
      if (type == TO_ALLMUD && (to == ch))
         continue;
      if (type == TO_NOTVICT)
      {
         if (to == ch || to == vch)
            continue;

         if (IS_ONMAP_FLAG(to))
         {
            if (to->map != ch->map
               || to->coord->x != ch->coord->x
               || to->coord->y != ch->coord->y)
               continue;
         }
         if (IS_ONMAP_FLAG(ch))
         {
            if (to->map != ch->map
               || to->coord->x != ch->coord->x
               || to->coord->y != ch->coord->y)
               continue;
         }
      }
      if (type == TO_CANSEE)
      {
         if (to == ch)
            continue;

         if (!IS_NPC(ch) && IS_IMMORTAL(ch) && xIS_SET(ch->act, PLR_WIZINVIS))
         {
            if (to->level < ch->pcdata->wizinvis)
               continue;
         }

         if (IS_ONMAP_FLAG(to))
         {
            if (to->map != ch->map
               || to->coord->x != ch->coord->x
               || to->coord->y != ch->coord->y)
               continue;
         }
         if (IS_ONMAP_FLAG(ch))
         {
            if (to->map != ch->map
               || to->coord->x != ch->coord->x
               || to->coord->y != ch->coord->y)
               continue;
         }
      }

      if (IS_IMMORTAL(to))
         txt = act_string(format, to, ch, arg1, arg2, STRING_IMM);
      else
         txt = act_string(format, to, ch, arg1, arg2, STRING_NONE);

      if (to->desc)
      {
         set_char_color(AType, to);
         /* write_to_buffer( to->desc, txt, strlen(txt) ); */
         send_to_char_color(txt, to);
      }
      if (MOBtrigger)
      {
         /* Note: use original string, not string with ANSI. -- Alty */
         mprog_act_trigger(txt, to, ch, (OBJ_DATA *) arg1, (void *) arg2);
      }
   }
   /* Anyone feel like telling me the point of looping through the whole
      room when we're only sending to one char anyways..? -- Alty */
   for (; to; to = (type == TO_MUD) ? to->next : (type == TO_CHAR || type == TO_VICT) ? NULL : to->next_in_room)
   {
      if ((!to->desc && (IS_NPC(to) && !HAS_PROG(to->pIndexData, ACT_PROG))) || !IS_AWAKE(to))
         continue;

      if (type == TO_CHAR && to != ch)
         continue;
      if (type == TO_VICT && (to != vch || to == ch))
         continue;
      if (type == TO_ROOM && to == ch)
         continue;
      if (type == TO_MUD && (to == ch || to == vch))
         continue;
      if (type == TO_NOTVICT && (to == ch || to == vch))
         continue;
      if (type == TO_CANSEE && (to == ch ||
            (!IS_IMMORTAL(to) && !IS_NPC(ch) && (xIS_SET(ch->act, PLR_WIZINVIS) && (get_trust(to) < (ch->pcdata ? ch->pcdata->wizinvis : 0))))))
         continue;

      if (IS_IMMORTAL(to))
         txt = act_string(format, to, ch, arg1, arg2, STRING_IMM);
      else
         txt = act_string(format, to, ch, arg1, arg2, STRING_NONE);

      if (to->desc)
      {
         set_char_color(AType, to);
         /* write_to_buffer( to->desc, txt, strlen(txt) ); */
         send_to_char_color(txt, to);
      }
      if (MOBtrigger)
      {
         /* Note: use original string, not string with ANSI. -- Alty */
         mprog_act_trigger(txt, to, ch, (OBJ_DATA *) arg1, (void *) arg2);
      }
   }
   MOBtrigger = TRUE;
   return;
}

#define MORPHNAME(ch)   ((ch->morph&&ch->morph->morph)? \
                         ch->morph->morph->short_desc: \
                         IS_NPC(ch) ? ch->short_descr : ch->name)
#define NAME(ch)        (IS_NPC(ch) ? ch->short_descr : ch->name)

/* No \n\r for the act, will be used for color string additions hopefully */
char *act_stringns(const char *format, CHAR_DATA * to, CHAR_DATA * ch, const void *arg1, const void *arg2, int flags)
{
   static char *const he_she[] = { "it", "he", "she" };
   static char *const him_her[] = { "it", "him", "her" };
   static char *const his_her[] = { "its", "his", "her" };
   static char buf[MSL];
   char fname[MIL];
   char temp[MSL];
   char *point = buf;
   const char *str = format;
   const char *i;
   CHAR_DATA *vch = (CHAR_DATA *) arg2;
   OBJ_DATA *obj1 = (OBJ_DATA *) arg1;
   OBJ_DATA *obj2 = (OBJ_DATA *) arg2;

   while (*str != '\0')
   {
      if (*str != '$')
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      if (!arg2 && *str >= 'A' && *str <= 'Z')
      {
         bug("Act: missing arg2 for code %c:", *str);
         bug(format);
         i = " <@@@> ";
      }
      else
      {
         switch (*str)
         {
            default:
               bug("Act: bad code %c.", *str);
               i = " <@@@> ";
               break;
            case 't':
               i = (char *) arg1;
               break;
            case 'T':
               i = (char *) arg2;
               break;
            case 'n':
               if (ch->morph == NULL)
               {
                  if (to && ch == to)
                     i = "You";
                  else
                     i = (to ? PERS_MAP(ch, to) : NAME(ch));
               }
               else if (!IS_SET(flags, STRING_IMM))
                  i = (to ? MORPHPERS(ch, to) : MORPHNAME(ch));
               else
               {
                  sprintf(temp, "(MORPH) %s", (to ? PERS_MAP(ch, to) : NAME(ch)));
                  i = temp;
               }
               break;
            case 'N':
               if (vch->morph == NULL)
               {
                  if (to && to == vch)
                     i = "You";
                  else
                     i = (to ? PERS_MAP(vch, to) : NAME(vch));
               }
               else if (!IS_SET(flags, STRING_IMM))
                  i = (to ? MORPHPERS(vch, to) : MORPHNAME(vch));
               else
               {
                  sprintf(temp, "(MORPH) %s", (to ? PERS_MAP(vch, to) : NAME(vch)));
                  i = temp;
               }
               break;

            case 'e':
               if (ch->sex > 2 || ch->sex < 0)
               {
                  bug("act_string: player %s has sex set at %d!", ch->name, ch->sex);
                  i = "it";
               }
               else if (to && to == ch)
                  i = "you";
               else if (get_wear_hidden_cloak(ch) && ch != to)
                  i = "it";
               else
                  i = he_she[URANGE(0, ch->sex, 2)];
               break;
            case 'E':
               if (vch->sex > 2 || vch->sex < 0)
               {
                  bug("act_string: player %s has sex set at %d!", vch->name, vch->sex);
                  i = "it";
               }
               else if (to && to == vch)
                  i = "you";
               else if (get_wear_hidden_cloak(vch) && vch != to)
                  i = "it";
               else
                  i = he_she[URANGE(0, vch->sex, 2)];
               break;
            case 'm':
               if (ch->sex > 2 || ch->sex < 0)
               {
                  bug("act_string: player %s has sex set at %d!", ch->name, ch->sex);
                  i = "it";
               }
               else if (to && to == ch)
                  i = "you";
               else if (get_wear_hidden_cloak(ch) && ch != to)
                  i = "it";
               else
                  i = him_her[URANGE(0, ch->sex, 2)];
               break;
            case 'M':
               if (vch->sex > 2 || vch->sex < 0)
               {
                  bug("act_string: player %s has sex set at %d!", vch->name, vch->sex);
                  i = "it";
               }
               else if (to && to == vch)
                  i = "you";
               else if (get_wear_hidden_cloak(vch) && vch != to)
                  i = "it";
               else
                  i = him_her[URANGE(0, vch->sex, 2)];
               break;
            case 's':
               if (ch->sex > 2 || ch->sex < 0)
               {
                  bug("act_string: player %s has sex set at %d!", ch->name, ch->sex);
                  i = "its";
               }
               else if (to && ch == to)
                  i = "your";
               else if (get_wear_hidden_cloak(ch) && ch != to)
                  i = "its";
               else
                  i = his_her[URANGE(0, ch->sex, 2)];
               break;
            case 'S':
               if (vch->sex > 2 || vch->sex < 0)
               {
                  bug("act_string: player %s has sex set at %d!", vch->name, vch->sex);
                  i = "its";
               }
               else if (to && vch == to)
                  i = "your";
               else if (get_wear_hidden_cloak(vch) && vch != to)
                  i = "it";
               else
                  i = his_her[URANGE(0, vch->sex, 2)];
               break;
            case 'q':
               i = (to == ch) ? "" : "s";
               break;
            case 'Q':
               i = (to == ch) ? "your" : his_her[URANGE(0, ch->sex, 2)];
               if (get_wear_hidden_cloak(ch) && ch != to)
                  i = "it";
               break;
            case 'p':
               i = (!to || can_see_obj(to, obj1) ? obj_short(obj1) : "something");
               break;
            case 'P':
               i = (!to || can_see_obj(to, obj2) ? obj_short(obj2) : "something");
               break;
            case 'd':
               if (!arg2 || ((char *) arg2)[0] == '\0')
                  i = "door";
               else
               {
                  one_argument((char *) arg2, fname);
                  i = fname;
               }
               break;
         }
      }
      ++str;
      while ((*point = *i) != '\0')
         ++point, ++i;

      /*  #0  0x80c6c62 in act_string (
         format=0x81db42e "$n has reconnected, kicking off old link.", to=0x0, 
         ch=0x94fcc20, arg1=0x0, arg2=0x0, flags=2) at comm.c:2901 */
   }
   buf[0] = UPPER(buf[0]);
   return buf;
}

#undef NAME

/* No ending /n/r -- Xerves */
void actns(sh_int AType, const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type)
{
   char *txt;
   CHAR_DATA *to;
   CHAR_DATA *vch = (CHAR_DATA *) arg2;

   /*
    * Discard null and zero-length messages.
    */
   if (!format || format[0] == '\0')
      return;

   if (!ch)
   {
      bug("Act: null ch. (%s)", format);
      return;
   }

   if (type == TO_MUD || type == TO_ALLMUD)
      to = first_char;
   if (!ch->in_room)
      to = NULL;
   else if (type == TO_CHAR)
      to = ch;
   else
      to = ch->in_room->first_person;
   

   /*
    * ACT_SECRETIVE handling
    */
   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_SECRETIVE) && type != TO_CHAR && type != TO_MUD && type != TO_ALLMUD)
      return;

   if (type == TO_VICT)
   {
      if (!vch)
      {
         bug("Act: null vch with TO_VICT.");
         bug("%s (%s)", ch->name, format);
         return;
      }
      if (!vch->in_room)
      {
         bug("Act: vch in NULL room!");
         bug("%s -> %s (%s)", ch->name, vch->name, format);
         return;
      }
      to = vch;
/*	to = vch->in_room->first_person;*/
   }

   if (MOBtrigger && type != TO_CHAR && type != TO_VICT && to)
   {
      OBJ_DATA *to_obj;

      txt = act_stringns(format, NULL, ch, arg1, arg2, STRING_IMM);
      if (HAS_PROG(to->in_room, ACT_PROG))
         rprog_act_trigger(txt, to->in_room, ch, (OBJ_DATA *) arg1, (void *) arg2);
      for (to_obj = to->in_room->first_content; to_obj; to_obj = to_obj->next_content)
         if (HAS_PROG(to_obj->pIndexData, ACT_PROG))
            oprog_act_trigger(txt, to_obj, ch, (OBJ_DATA *) arg1, (void *) arg2);
   }

   /* Anyone feel like telling me the point of looping through the whole
      room when we're only sending to one char anyways..? -- Alty */
   for (; to; to = (type == TO_MUD || type == TO_ALLMUD) ? to->next : (type == TO_CHAR || type == TO_VICT) ? NULL : to->next_in_room)
   {
      if ((!to->desc && (IS_NPC(to) && !HAS_PROG(to->pIndexData, ACT_PROG))) || !IS_AWAKE(to))
         continue;

      if (type == TO_CHAR && to != ch)
         continue;
      if (type == TO_VICT && (to != vch || to == ch))
         continue;
      if (type == TO_ROOM && to == ch)
         continue;
      if (type == TO_MUD && (to == ch || to == vch))
         continue;
      if (type == TO_NOTVICT && (to == ch || to == vch))
         continue;
      if (type == TO_CANSEE && (to == ch ||
            (!IS_IMMORTAL(to) && !IS_NPC(ch) && (xIS_SET(ch->act, PLR_WIZINVIS) && (get_trust(to) < (ch->pcdata ? ch->pcdata->wizinvis : 0))))))
         continue;

      if (IS_IMMORTAL(to))
         txt = act_stringns(format, to, ch, arg1, arg2, STRING_IMM);
      else
         txt = act_stringns(format, to, ch, arg1, arg2, STRING_NONE);

      if (to->desc)
      {
         set_char_color(AType, to);
         /* write_to_buffer( to->desc, txt, strlen(txt) ); */
         send_to_char_color(txt, to);
      }
      if (MOBtrigger)
      {
         /* Note: use original string, not string with ANSI. -- Alty */
         mprog_act_trigger(txt, to, ch, (OBJ_DATA *) arg1, (void *) arg2);
      }
   }
   MOBtrigger = TRUE;
   return;
}

void do_name(CHAR_DATA * ch, char *argument)
{
   char fname[1024];
   struct stat fst;
   CHAR_DATA *tmp;
   char buf[100];
   char name[20];

   if (!NOT_AUTHED(ch) || ch->pcdata->auth_state != 2)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument[0] = UPPER(argument[0]);

   if (!check_parse_name(argument, TRUE))
   {
      send_to_char("Illegal name, try another.\n\r", ch);
      return;
   }

   if (!str_cmp(ch->name, argument))
   {
      send_to_char("That's already your name!\n\r", ch);
      return;
   }

   for (tmp = first_char; tmp; tmp = tmp->next)
   {
      if (!str_cmp(argument, tmp->name))
         break;
   }

   if (tmp)
   {
      send_to_char("That name is already taken.  Please choose another.\n\r", ch);
      return;
   }

   sprintf(fname, "%s%c/%s", PLAYER_DIR, tolower(argument[0]), capitalize(argument));
   if (stat(fname, &fst) != -1)
   {
      send_to_char("That name is already taken.  Please choose another.\n\r", ch);
      return;
   }
   sprintf(name, "%s", ch->name);
   STRFREE(ch->name);
   ch->name = STRALLOC(argument);
   STRFREE(ch->pcdata->filename);
   ch->pcdata->filename = STRALLOC(argument);
   send_to_char("Your name has been changed.  Please apply again.\n\r", ch);
   ch->pcdata->auth_state = 1;
   sprintf(buf, "%s changed to %s, awaiting authorization.", name, ch->name);
   to_channel(buf, CHANNEL_AUTH, "Auth", LEVEL_IMMORTAL);
   return;
}

void do_lastname(CHAR_DATA * ch, char *argument)
{
   char fname[1024];
   struct stat fst;
   CHAR_DATA *tmp;
   char lastname[20];
   char buf[200];

   if (!IS_NPC(ch) && IS_IMMORTAL(ch) && get_trust(ch) >= LEVEL_STAFF)
   {
      char arg[MIL];
      CHAR_DATA *victim;
      
      if (argument[0] == '\0')
      {
         send_to_char("lastname <target's first name> <new lastname>\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg);
      argument = capitalize(argument);
      if ((victim = get_char_world(ch, arg)) == NULL)  
      {
         send_to_char("That Player does not exist.\n\r", ch);
         return;
      }
      if (IS_NPC(victim))
      {
         send_to_char("Not on mobs you fool.\n\r", ch);
         return;
      }
      if (!check_parse_name(argument, TRUE))
      {
         send_to_char("Illegal name, try another.\n\r", ch);
         return;
      }

      if (!str_cmp(victim->last_name, argument))
      {
         send_to_char("That's already your target's lastname!\n\r", ch);
         return;
      }
   
      if (!str_cmp(ch->name, argument))
      {
         send_to_char("Your target's lastname cannot be the same as your name!\n\r", ch);
         return;
      }

      for (tmp = first_char; tmp; tmp = tmp->next)
      {
         if (!str_cmp(argument, tmp->name))
            break;
      }

      if (tmp)
      {
         send_to_char("That name is already taken.  Please choose another.\n\r", ch);
         return;
      }

      sprintf(fname, "%s%c/%s", PLAYER_DIR, tolower(argument[0]), capitalize(argument));
      if (stat(fname, &fst) != -1)
      {
         send_to_char("That name is already taken.  Please choose another.\n\r", ch);
         return;
      }
   
      sprintf(fname, "%s%c/%s", LNAME_DIR, tolower(argument[0]), capitalize(argument));
      if (stat(fname, &fst) != -1)
      {
         send_to_char("That name is already taken.  Please choose another.\n\r", ch);
         return;
      }
      ch_printf(ch, "You have changed %s's lastname of %s to %s\n\r", victim->name, victim->last_name, argument);
      ch_printf(victim, "%s has changed your lastname of %s to %s\n\r", ch->name, victim->last_name, argument);
      bug("%s has changed %s's lastname of %s to %s", ch->name, victim->name, victim->last_name, argument);
      sprintf(lastname, argument);
      remove_from_lastname_file(victim->last_name, victim->name);
      STRFREE(victim->last_name);
      victim->last_name = STRALLOC(lastname);
      write_lastname_file(victim->last_name, victim->name);
      return;
   }    
   if (!NOT_AUTHED(ch) || ch->pcdata->auth_state != 4)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = capitalize(argument);

   if (!check_parse_name(argument, TRUE))
   {
      send_to_char("Illegal name, try another.\n\r", ch);
      return;
   }

   if (!str_cmp(ch->last_name, argument))
   {
      send_to_char("That's already your lastname!\n\r", ch);
      return;
   }
   
   if (!str_cmp(ch->name, argument))
   {
      send_to_char("Your's lastname cannot be the same as your name!\n\r", ch);
      return;
   }

   for (tmp = first_char; tmp; tmp = tmp->next)
   {
      if (!str_cmp(argument, tmp->name))
         break;
   }

   if (tmp)
   {
      send_to_char("That name is already taken.  Please choose another.\n\r", ch);
      return;
   }

   sprintf(fname, "%s%c/%s", PLAYER_DIR, tolower(argument[0]), capitalize(argument));
   if (stat(fname, &fst) != -1)
   {
      send_to_char("That name is already taken.  Please choose another.\n\r", ch);
      return;
   }
   
   sprintf(fname, "%s%c/%s", LNAME_DIR, tolower(argument[0]), capitalize(argument));
   if (stat(fname, &fst) != -1)
   {
      send_to_char("That name is already taken.  Please choose another.\n\r", ch);
      return;
   }
   sprintf(lastname, "%s", ch->last_name);
   STRFREE(ch->last_name);
   ch->last_name = STRALLOC(argument);
   send_to_char("Your lastname has been changed.  Please apply again.\n\r", ch);
   ch->pcdata->auth_state = 1;
   sprintf(buf, "%s:  Changed lastname from %s to %s, awaiting authorization.", ch->name, lastname, ch->last_name);
   to_channel(buf, CHANNEL_AUTH, "Auth", LEVEL_IMMORTAL);
   return;
}

char *default_fprompt(CHAR_DATA * ch)
{
   static char buf[60];

   strcpy(buf, "&w<&Y%hhp ");
   if (IS_VAMPIRE(ch))
      strcat(buf, "&R%bbp");
   else
      strcat(buf, "&C%mm");
   strcat(buf, " &G%vmv %c &w&W<%z> &w&c%frnd&w> ");
   if (IS_NPC(ch) || IS_IMMORTAL(ch))
      strcat(buf, "%i%R");
   return buf;
}

char *default_prompt(CHAR_DATA * ch)
{
   static char buf[60];

   strcpy(buf, "&w<&Y%h/%Hhp ");
   if (IS_VAMPIRE(ch))
      strcat(buf, "&R%bbp");
   else
      strcat(buf, "&C%m/%Mm");
   strcat(buf, " &G%v/%Vmv &RPK(%p)&w> ");
   if (IS_NPC(ch) || IS_IMMORTAL(ch))
      strcat(buf, "%i%R");
   return buf;
}

int getcolor(char clr)
{
   static const char colors[16] = "xrgObpcwzRGYBPCW";
   int r;

   for (r = 0; r < 16; r++)
      if (clr == colors[r])
         return r;
   return -1;
}

/*
A
B b
C c
D d
e
F f
 g
H h
I i
L
M m
N n
p
R r
S
T
U u
W w
V v
z */

void display_prompt(DESCRIPTOR_DATA * d)
{
   CHAR_DATA *ch = d->character;
   CHAR_DATA *och = (d->original ? d->original : d->character);
   CHAR_DATA *victim;
   bool ansi = (!IS_NPC(och) && xIS_SET(och->act, PLR_ANSI));
   const char *prompt;
   const char *helpstart = "<Type HELP START>";
   char buf[MSL];
   char extra[MSL];
   char *pbuf = buf;
   int stat, percent;
   int roomstat, p, temp, inside;
   WBLOCK_DATA *wblock;

   inside = p = temp = 0;

   if (!ch)
   {
      bug("display_prompt: NULL ch");
      return;
   }

   if (!IS_NPC(ch) && !IS_SET(ch->pcdata->flags, PCFLAG_HELPSTART))
      prompt = helpstart;
   else if (!IS_NPC(ch) && ch->substate != SUB_NONE && ch->pcdata->subprompt && ch->pcdata->subprompt[0] != '\0')
      prompt = ch->pcdata->subprompt;
   else if (IS_NPC(ch) || (!ch->fighting && (!ch->pcdata->prompt || !*ch->pcdata->prompt)))
      prompt = default_prompt(ch);
   else if (ch->fighting)
   {
      if (!ch->pcdata->fprompt || !*ch->pcdata->fprompt)
         prompt = default_fprompt(ch);
      else
         prompt = ch->pcdata->fprompt;
   }
   else
      prompt = ch->pcdata->prompt;
      
   /* reset MXP to default operation */
   if (d->mxp)
   {
      strcpy (pbuf, ESC "[3z");
      pbuf += 4;
   }

   if (ansi)
   {
      strcpy(pbuf, "\033[m");
      d->prevcolor = 0x07;
      pbuf += 3;
   }
   /* Clear out old color stuff */
/*  make_color_sequence(NULL, NULL, NULL);*/
   for (; *prompt; prompt++)
   {
      /*
       * '&' = foreground color/intensity bit
       * '^' = background color/blink bit
       * '%' = prompt commands
       * Note: foreground changes will revert background to 0 (black)
       */
      if (*prompt != '&' && *prompt != '^' && *prompt != '%')
      {
         *(pbuf++) = *prompt;
         continue;
      }
      ++prompt;
      if (!*prompt)
         break;
      if (*prompt == *(prompt - 1))
      {
         *(pbuf++) = *prompt;
         continue;
      }
      switch (*(prompt - 1))
      {
         default:
            bug("Display_prompt: bad command char '%c'.", *(prompt - 1));
            break;
         case '&':
         case '^':
            stat = make_color_sequence(&prompt[-1], pbuf, d);
            if (stat < 0)
               --prompt;
            else if (stat > 0)
               pbuf += stat;
            break;
         case '%':
            *pbuf = '\0';
            stat = 0x80000000;
            switch (*prompt)
            {
               case '%':
                  *pbuf++ = '%';
                  *pbuf = '\0';
                  break;

               case '\\':
                  sprintf(pbuf, "\n\r");
                  break;

               case 'p':
                  roomstat = check_room_pk(ch);
                  if (roomstat == 1)
                  {
                     sprintf(pbuf, "S");
                     break;
                  }
                  if (roomstat == 4)
                  {
                     sprintf(pbuf, "F");
                     break;
                  }
                  if (roomstat == 3)
                  {
                     sprintf(pbuf, "A");
                     break;
                  }
                  if (roomstat == 2)
                  {
                     sprintf(pbuf, "N");
                     break;
                  }
                  break;

               case 'A':
                  sprintf(pbuf, "%s%s%s%s%s", IS_AFFECTED(ch, AFF_INVISIBLE) ? "I" : "",
                     IS_AFFECTED(ch, AFF_HIDE) ? "H" : "", IS_AFFECTED(ch, AFF_SNEAK) ? "S" : "",
                     get_wear_hidden_cloak(ch) ? "C" : "", IS_AFFECTED(ch, AFF_STALK) ? "ST" : "");
                  break;
               case 'C': /* Tank */
                  if (!ch->fighting || (victim = ch->fighting->who) == NULL)
                     strcpy(pbuf, "(          )");
                  else if (!victim->fighting || (victim = victim->fighting->who) == NULL)
                     strcpy(pbuf, "(          )");
                  else
                  {
                     if (victim->max_hit > 0)
                        percent = (100 * victim->hit) / victim->max_hit;
                     else
                        percent = -1;
                     if (percent >= 100)
                        strcpy(pbuf, "(&r**&R**&Y**&C**&P**)");
                     else if (percent >= 90)
                        strcpy(pbuf, "(&r**&R**&Y**&C**&P* )");
                     else if (percent >= 80)
                        strcpy(pbuf, "(&r**&R**&Y**&C**&P  )");
                     else if (percent >= 70)
                        strcpy(pbuf, "(&r**&R**&Y**&C*   )");
                     else if (percent >= 60)
                        strcpy(pbuf, "(&r**&R**&Y**&C    )");
                     else if (percent >= 50)
                        strcpy(pbuf, "(&r**&R**&Y*     )");
                     else if (percent >= 40)
                        strcpy(pbuf, "(&r**&R**&Y      )");
                     else if (percent >= 30)
                        strcpy(pbuf, "(&r**&R*       )");
                     else if (percent >= 20)
                        strcpy(pbuf, "(&r**&R        )");
                     else if (percent >= 10)
                        strcpy(pbuf, "(&r*         )");
                     else
                        strcpy(pbuf, "(          )");
                  }
                  break;
               case 'c':
                  if (!ch->fighting || (victim = ch->fighting->who) == NULL)
                     strcpy(pbuf, "(          )");
                  else
                  {
                     if (victim->max_hit > 0)
                        percent = (100 * victim->hit) / victim->max_hit;
                     else
                        percent = -1;
                     if (percent >= 100)
                        strcpy(pbuf, "(&r**&R**&Y**&C**&P**)");
                     else if (percent >= 90)
                        strcpy(pbuf, "(&r**&R**&Y**&C**&P* )");
                     else if (percent >= 80)
                        strcpy(pbuf, "(&r**&R**&Y**&C**&P  )");
                     else if (percent >= 70)
                        strcpy(pbuf, "(&r**&R**&Y**&C*   )");
                     else if (percent >= 60)
                        strcpy(pbuf, "(&r**&R**&Y**&C    )");
                     else if (percent >= 50)
                        strcpy(pbuf, "(&r**&R**&Y*     )");
                     else if (percent >= 40)
                        strcpy(pbuf, "(&r**&R**&Y      )");
                     else if (percent >= 30)
                        strcpy(pbuf, "(&r**&R*       )");
                     else if (percent >= 20)
                        strcpy(pbuf, "(&r**&R        )");
                     else if (percent >= 10)
                        strcpy(pbuf, "(&r*         )");
                     else
                        strcpy(pbuf, "(          )");
                  }
                  break;
               case 'h':
                  stat = ch->hit;
                  break;
               case 'H':
                  stat = ch->max_hit;
                  break;
               case 'm':
                  if (IS_VAMPIRE(ch))
                     stat = 0;
                  else
                     stat = ch->mana;
                  break;
               case 'M':
                  if (IS_VAMPIRE(ch))
                     stat = 0;
                  else
                     stat = ch->max_mana;
                  break;
               case 'N': /* Tank */
                  if (!ch->fighting || (victim = ch->fighting->who) == NULL)
                     strcpy(pbuf, "N/A");
                  else if (!victim->fighting || (victim = victim->fighting->who) == NULL)
                     strcpy(pbuf, "N/A");
                  else
                  {
                     if (ch == victim)
                        strcpy(pbuf, "You");
                     else if (IS_NPC(victim))
                        strcpy(pbuf, victim->short_descr);
                     else
                     {
                        sprintf(extra, "%s", PERS_MAP(victim, ch));
                        strcpy(pbuf, extra);
                     }
                     pbuf[0] = UPPER(pbuf[0]);
                  }
                  break;
               case 'n':
                  if (!ch->fighting || (victim = ch->fighting->who) == NULL)
                     strcpy(pbuf, "N/A");
                  else
                  {
                     if (ch == victim)
                        strcpy(pbuf, "You");
                     else if (IS_NPC(victim))
                        strcpy(pbuf, victim->short_descr);
                     else
                     {
                        sprintf(extra, "%s", PERS_MAP(victim, ch));
                        strcpy(pbuf, extra);
                     }
                     pbuf[0] = UPPER(pbuf[0]);
                  }
                  break;
               case 't':
                 sprintf(extra, "%d", get_timer(ch, TIMER_RECENTFIGHT)/2);
                 strcpy(pbuf, extra);
                 break;
               case 'T':
                  if (gethour() < 5)
                     strcpy(pbuf, "night");
                  else if (gethour() < 6)
                     strcpy(pbuf, "dawn");
                  else if (gethour() < 19)
                     strcpy(pbuf, "day");
                  else if (gethour() < 21)
                     strcpy(pbuf, "dusk");
                  else
                     strcpy(pbuf, "night");
                  break;

               case 'w':
                  inside = 0;
                  if (!IN_WILDERNESS(ch))
                  {
                     if (ch->in_room->area->map < 0 || ch->in_room->area->x < 1 || ch->in_room->area->y < 1)
                        strcpy(pbuf, "N");
                     else
                     {
                        p = weather_sector[ch->in_room->area->map][ch->in_room->area->x][ch->in_room->area->y] % 10;
                        temp = generate_temperature(ch, ch->in_room->area->x, ch->in_room->area->y, ch->in_room->area->map);
                        if (!IS_OUTSIDE(ch) || NO_WEATHER_SECT(ch->in_room->sector_type))
                        {
                           if (!IS_OUTSIDE(ch) && !NO_WEATHER_SECT(ch->in_room->sector_type)) //Indoors/Tunnel
                           {
                              if (p < 6)
                                 strcpy(pbuf, "N");

                              inside = 1;
                           }
                           else
                              strcpy(pbuf, "N");
                        }
                     }
                  }
                  else
                  {
                     p = weather_sector[ch->map][ch->coord->x][ch->coord->y] % 10;
                     temp = generate_temperature(ch, -1, -1, -1);
                  }
                  if (p > 0)
                  {
                     if (temp >= 20 && temp <= 35) //Snow
                     {
                        if (inside == 0 || p >= 7)
                           strcpy(pbuf, "S");
                        else
                           strcpy(pbuf, "N");
                     }
                     if (temp < 20) //Ice
                     {
                        if (inside == 0 || p >= 7)
                           strcpy(pbuf, "I");
                        else
                           strcpy(pbuf, "N");
                     }

                     if (temp > 35) //Rain
                     {
                        if (inside == 0 || p >= 7)
                           strcpy(pbuf, "R");
                        else
                           strcpy(pbuf, "N");
                     }
                  }
                  else
                     strcpy(pbuf, "N");
                  break;

               case 'W':
                  inside = 0;
                  if (!IN_WILDERNESS(ch))
                  {
                     if (ch->in_room->area->map < 0 || ch->in_room->area->x < 1 || ch->in_room->area->y < 1)
                        strcpy(pbuf, "N");
                     else
                     {
                        p = weather_sector[ch->in_room->area->map][ch->in_room->area->x][ch->in_room->area->y] % 10;
                        temp = generate_temperature(ch, ch->in_room->area->x, ch->in_room->area->y, ch->in_room->area->map);
                        if (!IS_OUTSIDE(ch) || NO_WEATHER_SECT(ch->in_room->sector_type))
                        {
                           if (!IS_OUTSIDE(ch) && !NO_WEATHER_SECT(ch->in_room->sector_type)) //Indoors/Tunnel
                           {
                              if (p < 6)
                                 strcpy(pbuf, "N");

                              inside = 1;
                           }
                           else
                              strcpy(pbuf, "N");
                        }
                     }
                  }
                  else
                  {
                     p = weather_sector[ch->map][ch->coord->x][ch->coord->y] % 10;
                     temp = generate_temperature(ch, -1, -1, -1);
                  }
                  if (p > 0)
                  {
                     if (p < 7 && inside == 1)
                        strcpy(pbuf, "N");
                     else
                     {
                        if (p <= 3)
                           strcpy(pbuf, "L");
                        else if (p <= 6)
                           strcpy(pbuf, "M");
                        else if (p < 9)
                           strcpy(pbuf, "H");
                        else if (p == 9)
                           strcpy(pbuf, "D");
                     }
                  }
                  else
                     strcpy(pbuf, "N");
                  break;

               case 'b':
                  if (IS_VAMPIRE(ch))
                     stat = ch->pcdata->condition[COND_BLOODTHIRST];
                  else
                     stat = 0;
                  break;
               case 'B':
                  if (IS_VAMPIRE(ch))
                     stat = ch->level + 10;
                  else
                     stat = 0;
                  break;
               case 'u':
                  if (IS_IMMORTAL(och))
                     stat = num_descriptors;
                  break;
               case 'U':
                  if (IS_IMMORTAL(och))
                     stat = sysdata.maxplayers;
                  break;
               case 'l':
                  if (!IN_WILDERNESS(ch))
                  {
                     stat = -1;
                     break;
                  }
                  for (wblock = first_wblock; wblock; wblock = wblock->next)
                  {
                     if (ch->coord->x <= wblock->endx && ch->coord->x >= wblock->stx 
                     &&  ch->coord->y <= wblock->endy && ch->coord->y >= wblock->sty && ch->map == wblock->map && !IS_NPC(ch))
                     {
                        stat = wblock->lvl;
                        break;
                     }
                  }
                  break;
               case 'L':
                  if (ch->pcdata->mount && IS_NPC(ch->pcdata->mount) && IN_SAME_ROOM(ch, ch->pcdata->mount) 
                      && xIS_SET(ch->pcdata->mount->act, ACT_MOUNTSAVE))
                     stat = ch->pcdata->mount->move;
                  else
                     stat = -1;
                  break;
               case 'v':
                  stat = ch->move;
                  break;
               case 'V':
                  stat = ch->max_move;
                  break;
               case 'g':
                  stat = ch->gold;
                  break;
               case 'f':
                  stat = ch->fight_timer;
                  break;
               case 'd':
                  stat = get_ch_carry_weight(ch);
                  break;
               case 'D':
                  stat = can_carry_w(ch);
                  break;
               case 'r':
                  if (IS_IMMORTAL(och))
                     stat = ch->in_room->vnum;
                  break;
               case 'F':
                  if (IS_IMMORTAL(och))
                     sprintf(pbuf, "%s", ext_flag_string(&ch->in_room->room_flags, r_flags));
                  break;
               case 'R':
                  if (xIS_SET(och->act, PLR_ROOMVNUM))
                     sprintf(pbuf, "<#%d> ", ch->in_room->vnum);
                  break;
               case 'S':
                  if (ch->style == STYLE_BERSERK)
                     strcpy(pbuf, "B");
                  else if (ch->style == STYLE_AGGRESSIVE)
                     strcpy(pbuf, "A");
                  else if (ch->style == STYLE_DEFENSIVE)
                     strcpy(pbuf, "D");
                  else if (ch->style == STYLE_EVASIVE)
                     strcpy(pbuf, "E");
                  else if (ch->style == STYLE_WIZARDRY)
                     strcpy(pbuf, "W");
                  else if (ch->style == STYLE_DIVINE)
                     strcpy(pbuf, "DI");
                  else
                     strcpy(pbuf, "S");
                  break;
               case 'e':
                  stat = ch->pcdata->spoints;
                  break;
               case 'E':
                  if (xIS_SET(ch->act, PLR_PARRY))
                     strcpy(pbuf, "P");
                  if (!xIS_SET(ch->act, PLR_DODGE))
                     strcpy(pbuf, "D");
                  if (!xIS_SET(ch->act, PLR_TUMBLE))
                     strcpy(pbuf, "T");
                  if (!xIS_SET(ch->act, PLR_COUNTER))
                     strcpy(pbuf, "C");
                  break;
               case 'z':
                  sprintf(pbuf, MXPFTAG("Command 'Attack \"%s\" body' desc='Click to attack the body of your target'", "BD", "/Command") " "
                                MXPFTAG("Command 'Attack \"%s\" rarm' desc='Click to attack the right arm of your target'", "RA", "/Command") " "
                                MXPFTAG("Command 'Attack \"%s\" larm' desc='Click to attack the left arm of your target'", "LA", "/Command") " "
                                MXPFTAG("Command 'Attack \"%s\" rleg' desc='Click to attack the right leg of your target'", "RL", "/Command") " "
                                MXPFTAG("Command 'Attack \"%s\" lleg' desc='Click to attack the left leg of your target'", "LL", "/Command") " "
                                MXPFTAG("Command 'Attack \"%s\" neck' desc='Click to attack the neck of your target'", "NK", "/Command") " "
                                MXPFTAG("Command 'Attack \"%s\" head' desc='Click to attack the head of your target'", "HD", "/Command"),
                                (ch->fighting && ch->fighting->who) ? ch->fighting->who->name : "", (ch->fighting && ch->fighting->who) ? ch->fighting->who->name : "",
                                (ch->fighting && ch->fighting->who) ? ch->fighting->who->name : "", (ch->fighting && ch->fighting->who) ? ch->fighting->who->name : "",
                                (ch->fighting && ch->fighting->who) ? ch->fighting->who->name : "", (ch->fighting && ch->fighting->who) ? ch->fighting->who->name : "",
                                (ch->fighting && ch->fighting->who) ? ch->fighting->who->name : "");
                  break;
               case 'i':
                  if ((!IS_NPC(ch) && xIS_SET(ch->act, PLR_WIZINVIS)) || (IS_NPC(ch) && xIS_SET(ch->act, ACT_MOBINVIS)))
                     sprintf(pbuf, "(Invis %d) ", (IS_NPC(ch) ? 1 : ch->pcdata->wizinvis));
                  else if (IS_AFFECTED(ch, AFF_INVISIBLE))
                     sprintf(pbuf, "(Invis) ");
                  break;
               case 'I':
                  stat = (IS_NPC(ch) ? (xIS_SET(ch->act, ACT_MOBINVIS) ? 1 : 0)
                     : (xIS_SET(ch->act, PLR_WIZINVIS) ? ch->pcdata->wizinvis : 0));
                  break;
            }
            if (stat != 0x80000000)
               sprintf(pbuf, "%d", stat);
            pbuf += strlen(pbuf);
            break;
      }
   }
   *pbuf = '\0';
   if (ch && ch->pcdata && d->connected == CON_PLAYING)
      send_to_char(buf, ch);
   else
      write_to_buffer(d, buf, (pbuf - buf));
      
   if (ch && ch->pcdata && d->connected == CON_PLAYING && ch->pcdata->gprompt != 2)
   {
      CHAR_DATA *gch;
      int cnt = 0;
     
      if (!ch->fighting && ch->pcdata->gprompt == 0)
         return;
         
      for (gch = first_char; gch; gch = gch->next)
      {
         if (is_same_group(gch, ch) && ch != gch)
         {
            if (cnt % 4 == 0)
               send_to_char("\n\r", ch);
            ch_printf(ch, "&w&z%12.12s &w&R%3d &w&B%3d&w   ", capitalize(PERS_MAP(gch, ch)), gch->hit*100/gch->max_hit,
               gch->mana*100/gch->max_mana);
            cnt++;
         }
      }
   }               
}
int make_color_sequence(const char *col, char *buf, DESCRIPTOR_DATA * d)
{
   int ln;
   const char *ctype = col;
   unsigned char cl;
   CHAR_DATA *och;
   bool ansi;

   och = (d->original ? d->original : d->character);
   ansi = (!IS_NPC(och) && xIS_SET(och->act, PLR_ANSI));
   col++;
   if (!*col)
      ln = -1;
   else if (*ctype != '&' && *ctype != '^')
   {
      bug("Make_color_sequence: command '%c' not '&' or '^'.", *ctype);
      ln = -1;
   }
   else if (*col == *ctype)
   {
      buf[0] = *col;
      buf[1] = '\0';
      ln = 1;
   }
   else if (*col == 'e')
   {
      write_to_buffer(d, MXPTAG("color #FFB6C1"), 0);
      ln = 0;
   }
   else if (*col == 'f')
   {
      write_to_buffer(d, MXPTAG("color #f9b107"), 0);
      ln = 0;
   }
   else if (*col == 'E')
   {
      write_to_buffer(d, MXPTAG("/color"), 0);
      ln = 0;
   }
   else if (!ansi)
      ln = 0;
   else
   {
      cl = d->prevcolor;
      switch (*ctype)
      {
         default:
            bug("Make_color_sequence: bad command char '%c'.", *ctype);
            ln = -1;
            break;
         case '&':
            if (*col == '-')
            {
               buf[0] = '~';
               buf[1] = '\0';
               ln = 1;
               break;
            }
         case '^':
            {
               int newcol;

               if ((newcol = getcolor(*col)) < 0)
               {
                  ln = 0;
                  break;
               }
               else if (*ctype == '&')
                  cl = (cl & 0xF0) | newcol;
               else
                  cl = (cl & 0x0F) | (newcol << 4);
            }
            if (cl == d->prevcolor)
            {
               ln = 0;
               break;
            }
            strcpy(buf, "\033[");
            if ((cl & 0x88) != (d->prevcolor & 0x88))
            {
               if (cl == 0x07)
               {
                  strcpy(buf, "\033[0;37");
               }
               else
               {
                  if ((cl & 0x08))
                     strcat(buf, "1;");
                  else
                     strcat(buf, "0;");
                  if ((cl & 0x80))
                     strcat(buf, "5;");
               }
               d->prevcolor = 0x07 | (cl & 0x88);
               ln = strlen(buf);
            }
            else
               ln = 2;
            /*
               if ( (cl & 0x88) != (d->prevcolor & 0x88) )
               {
               strcat(buf, "m\033[");
               if ( (cl & 0x08) )
               strcat(buf, "1;");
               if ( (cl & 0x80) )
               strcat(buf, "5;");
               d->prevcolor = 0x07 | (cl & 0x88);
               ln = strlen(buf);
               }
               else
               ln = 2; */
            if ((cl & 0x07) != (d->prevcolor & 0x07))
            {
               sprintf(buf + ln, "3%d;", cl & 0x07);
               ln += 3;
            }
            if ((cl & 0x70) != (d->prevcolor & 0x70))
            {
               sprintf(buf + ln, "4%d;", (cl & 0x70) >> 4);
               ln += 3;
            }
            if (buf[ln - 1] == ';')
               buf[ln - 1] = 'm';
            else
            {
               buf[ln++] = 'm';
               buf[ln] = '\0';
            }
            d->prevcolor = cl;
      }
   }
   if (ln <= 0)
      *buf = '\0';
   return ln;
}

void set_pager_input(DESCRIPTOR_DATA * d, char *argument)
{
   while (isspace(*argument))
      argument++;
   d->pagecmd = *argument;
   return;
}

bool pager_output(DESCRIPTOR_DATA * d)
{
   register char *last;
   CHAR_DATA *ch;
   int pclines;
   register int lines;
   bool ret;

   if (!d || !d->pagepoint || d->pagecmd == -1)
      return TRUE;
   ch = d->original ? d->original : d->character;
   pclines = UMAX(ch->pcdata->pagerlen, 5) - 1;
   switch (LOWER(d->pagecmd))
   {
      default:
         lines = 0;
         break;
      case 'b':
         lines = -1 - (pclines * 2);
         break;
      case 'r':
         lines = -1 - pclines;
         break;
      case 'q':
         d->pagetop = 0;
         d->pagepoint = NULL;
         flush_buffer(d, TRUE);
         DISPOSE(d->pagebuf);
         d->pagesize = MSL;
         return TRUE;
   }
   while (lines < 0 && d->pagepoint >= d->pagebuf)
      if (*(--d->pagepoint) == '\n')
         ++lines;
   if (*d->pagepoint == '\n' && *(++d->pagepoint) == '\r')
      ++d->pagepoint;
   if (d->pagepoint < d->pagebuf)
      d->pagepoint = d->pagebuf;
   for (lines = 0, last = d->pagepoint; lines < pclines; ++last)
      if (!*last)
         break;
      else if (*last == '\n')
         ++lines;
   if (*last == '\r')
      ++last;
   if (last != d->pagepoint)
   {
      if (!write_to_descriptor(d->descriptor, d->pagepoint, (last - d->pagepoint)))
         return FALSE;
      d->pagepoint = last;
   }
   while (isspace(*last))
      ++last;
   if (!*last)
   {
      d->pagetop = 0;
      d->pagepoint = NULL;
      flush_buffer(d, TRUE);
      DISPOSE(d->pagebuf);
      d->pagesize = MSL;
      return TRUE;
   }
   d->pagecmd = -1;
   if (xIS_SET(ch->act, PLR_ANSI))
      if (write_to_descriptor(d->descriptor, "\033[1;36m", 7) == FALSE)
         return FALSE;
   if ((ret = write_to_descriptor(d->descriptor, "(C)ontinue, (R)efresh, (B)ack, (Q)uit: [C] ", 0)) == FALSE)
   {
      return FALSE;
   }
   if (xIS_SET(ch->act, PLR_ANSI))
   { 
      char buf[32];
      
      if (d->pagecolor == 7)
         strcpy(buf, "\033[m");
      else
         sprintf(buf, "\033[0;%d;%s%dm", (d->pagecolor & 8) == 8, (d->pagecolor > 15 ? "5;" : ""), (d->pagecolor & 7) + 30);
      ret = write_to_descriptor(d->descriptor, buf, 0);
   }
   return ret;
}


#ifdef WIN32

void shutdown_mud(char *reason);

void bailout(void)
{
   echo_to_all(AT_IMMORT, "MUD shutting down by system operator NOW!!", ECHOTAR_ALL);
   shutdown_mud("MUD shutdown by system operator");
   log_string("MUD shutdown by system operator");
   Sleep(5000); /* give "echo_to_all" time to display */
   mud_down = TRUE; /* This will cause game_loop to exit */
   service_shut_down = TRUE; /* This will cause characters to be saved */
   fflush(stderr);
   return;
}

#endif
void write_corpses args((CHAR_DATA * ch, char *name, OBJ_DATA * objrem));

void save_pc_corpses(void)
{
   OBJ_DATA *obj;
   
   for (obj = first_object; obj; obj = obj->next)
   {
      if (obj->item_type != ITEM_CORPSE_PC)
            continue; 
      write_corpses(NULL, obj->short_descr + 14, NULL);
   }
   return;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !strcmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}
	
int write_reset_char(CHAR_DATA *ch, char *filename, FILE *ofp, char *fname)
{
   char newpfile[MSL];
   FILE *fp;
   int i;
   ALIAS_DATA *pal;
   
   sprintf(newpfile, "%s%c/%s2", RESET_DIR, tolower(filename[0]), filename); 	
   
   if ((fp = fopen(newpfile, "w")) == NULL)
      return 0;

   bug("[PFILE DELETE] %s has been deleted", ch->name);
   fprintf(fp, "#PLAYER\n");

   fprintf(fp, "Version      %d\n", SAVEVERSION);
   fprintf(fp, "Name         %s~\n", ch->name);
   fprintf(fp, "LastName     %s~\n", ch->last_name);
   fprintf(fp, "Pid	     %d\n", ch->pcdata->pid);
   if (ch->description[0] != '\0')
      fprintf(fp, "Description  %s~\n", ch->description);
   fprintf(fp, "Sex          %d\n", ch->sex);
   fprintf(fp, "Race         %d\n", ch->race);
   fprintf(fp, "Languages    %d %d\n", ch->speaks, ch->speaking);
   fprintf(fp, "Level        %d\n", ch->level);
   fprintf(fp, "Played       %d\n", ch->played + (int) (current_time - ch->pcdata->logon));
   fprintf(fp, "Righthanded  %d\n", ch->pcdata->righthanded);
   fprintf(fp, "Height          %d\n", ch->height);
   fprintf(fp, "Weight          %d\n", ch->weight);
   fprintf(fp, "Elements     %d\n", ch->elementb);
   if (ch->speed > 0)
   fprintf(fp, "Speed        %d\n", ch->speed);
   fprintf(fp, "Skincolor    %d\n", ch->pcdata->skincolor);
   fprintf(fp, "Haircolor    %d\n", ch->pcdata->haircolor);
   fprintf(fp, "Hairlength   %d\n", ch->pcdata->hairlength);
   fprintf(fp, "Hairstyle    %d\n", ch->pcdata->hairstyle);
   fprintf(fp, "Eyecolor     %d\n", ch->pcdata->eyecolor);
   fprintf(fp, "Cheight      %d\n", ch->pcdata->cheight);
   fprintf(fp, "Cweight      %d\n", ch->pcdata->cweight);
   fprintf(fp, "Talent       %s\n", print_bitvector(&ch->pcdata->talent));


   if (ch->pcdata->homepage && ch->pcdata->homepage[0] != '\0')
      fprintf(fp, "Homepage     %s~\n", ch->pcdata->homepage);
   if (ch->pcdata->email && ch->pcdata->email[0] != '\0') /* Samson 4-19-98 */
      fprintf(fp, "Email	     %s~\n", ch->pcdata->email);
   if (ch->pcdata->icq > 0) /* Samson 1-4-99 */
      fprintf(fp, "ICQ          %d\n", ch->pcdata->icq);
   if (ch->pcdata->bio && ch->pcdata->bio[0] != '\0')
      fprintf(fp, "Bio          %s~\n", ch->pcdata->bio);
   if (ch->pcdata->authed_by && ch->pcdata->authed_by[0] != '\0')
      fprintf(fp, "AuthedBy     %s~\n", ch->pcdata->authed_by);
   if (ch->pcdata->came_from && ch->pcdata->came_from[0] != '\0')
      fprintf(fp, "CameFrom     %s~\n", ch->pcdata->came_from);
   if (ch->pcdata->prompt && *ch->pcdata->prompt)
      fprintf(fp, "Prompt       %s~\n", ch->pcdata->prompt);
   if (ch->pcdata->fprompt && *ch->pcdata->fprompt)
      fprintf(fp, "FPrompt	     %s~\n", ch->pcdata->fprompt);
   if (ch->pcdata->pagerlen != 24)
      fprintf(fp, "Pagerlen     %d\n", ch->pcdata->pagerlen);
   fprintf(fp, "Boards       %d ", MAX_BOARD);
   for (i = 0; i < MAX_BOARD; i++)
      fprintf(fp, "%s %ld ", boards[i].short_name, ch->pcdata->last_note[i]);
   fprintf(fp, "\n");

   for (pal = ch->pcdata->first_alias; pal; pal = pal->next) 
   {
      if (!pal->name || !pal->cmd || !*pal->name || !*pal->cmd)
         continue;
      fprintf(fp, "Alias           %s~ %s~\n", pal->name, pal->cmd);
   }
   fprintf(fp, "Fame	     %d\n", ch->fame);
   /* Save color values - Samson 9-29-98 */
   {
      int x;

      fprintf(fp, "MaxColors    %d\n", MAX_COLORS);
      fprintf(fp, "Colors       ");
      for (x = 0; x < MAX_COLORS; x++)
         fprintf(fp, "%d ", ch->pcdata->colors[x]);
      fprintf(fp, "\n");
   }

   fprintf(fp, "End\n\n");
   fclose(fp);
   fclose(ofp);
   remove(fname);
   rename(newpfile, fname);
   remove(newpfile);
   return 1;
}

void fread_char args((CHAR_DATA * ch, FILE * fp, bool preload));
int prep_reset_data(char *fname, char *filename)
{
   FILE *fp; 
   char buf[MSL];
   CHAR_DATA *ch;
   
   CREATE(ch, CHAR_DATA, 1);
   CREATE(ch->pcdata, PC_DATA, 1);
   clear_char(ch);
   if ((fp = fopen(fname, "r")) != NULL)
   {
      char letter;
      char *word;

      letter = fread_letter(fp);
      if (letter == '*')
      {
         fread_to_eol(fp);
         return 0;
      }

      if (letter != '#')
      {
         bug("prep_reset_data: # not found.", 0);
         bug(filename, 0);
         return 0;
      }

      word = fread_word(fp);
      if (!strcmp(word, "PLAYER"))
      {
         fread_char(ch, fp, FALSE);
         if (!write_reset_char(ch, filename, fp, fname))
         {
            sprintf(buf, "Could not write the reset prep file for %s", filename);
            perror(buf);
            return 0;
         }
         return 1;
      }
      else
      {
         bug("Failed to find the PLAYER tag in %s", filename);
         fclose(fp);
         return 0;
      }
   }
   else
   {
      bug("Could now find directory for %s", filename);
      perror(buf);
   }
   return 0;
}
void fread_pfile2(FILE * fp, char *fname, char *filename)
{
   char *word;
   int level = 0;
   bool fMatch;

   for (;;)
   {
      word = feof(fp) ? "End" : fread_word(fp);
      fMatch = FALSE;

      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;

         case 'E':
            if (!strcmp(word, "End"))
               goto timecheck;
            break;

         case 'L':
            KEY("Level", level, fread_number(fp));
            break;
      }

      if (!fMatch)
         fread_to_eol(fp);
   }

 timecheck:

   if (level < LEVEL_IMMORTAL)
   {
      char buf[MSL];
      sprintf(buf, "%s%c/%s", RESET_DIR, tolower(filename[0]), filename); 
      if (!rename(fname, buf))
      {
         remove(fname);
         if (!prep_reset_data(buf, filename))
         {
            sprintf(buf, "Could not prep %s for a reset", filename);
            perror(buf);
         }
      }
      else
      {
         sprintf(buf, "Could not copy %s", filename);
         perror(buf);
      }
   }
}
void read_pfile2(char *dirname, char *filename)
{
   FILE *fp;
   char fname[MSL];
   struct stat fst;

   sprintf(fname, "%s/%s", dirname, filename);

   if (stat(fname, &fst) != -1)
   {

      if ((fp = fopen(fname, "r")) != NULL)
      {
         for (;;)
         {
            char letter;
            char *word;

            letter = fread_letter(fp);

            if (letter != '#')
               continue;

            word = fread_word(fp);

            if (!str_cmp(word, "End"))
               break;

            if (!str_cmp(word, "PLAYER"))
               fread_pfile2(fp, fname, filename);
            else if (!str_cmp(word, "END")) /* Done  */
               break;
         }
         FCLOSE(fp);
      }
   }
   return;
}

void gamereset_pfiles(void)
{
   DIR *dp;
   struct dirent *dentry;
   char dir_name[100];

   int alpha_loop;

   for (alpha_loop = 0; alpha_loop <= 25; alpha_loop++)
   {
      sprintf(dir_name, "%s%c", PLAYER_DIR, 'a' + alpha_loop);
      dp = opendir(dir_name);
      dentry = readdir(dp);
      while (dentry)
      {
         if (dentry->d_name[0] != '.')
         {
            read_pfile2(dir_name, dentry->d_name);
         }
         dentry = readdir(dp);
      }
      closedir(dp);
   }

   return;
}

void copy_backup_contents(char *sourcefile, char *destfile)
{
   FILE *fsource;
   FILE *fdestination;
   int ch;
   if ((fdestination = fopen(destfile, "w")) == NULL)
   {
      bug("Cannot open: %s for writing", destfile);
      return;
   }
   if ((fsource = fopen(sourcefile, "r")) != NULL)
   {
      for (;;)
      {
         ch = fgetc( fsource );
         if (!feof(fsource))
         {
             fputc( ch, fdestination);
         }
         else
             break;
      }
   }
   fclose(fdestination); 
   fclose(fsource);   
}  

void copy_backup_files(void)
{
   copy_backup_contents("backup/Raferover.are", "Raferover.are");
   copy_backup_contents(SYSTEM_DIR "backup/market.dat", SYSTEM_DIR "market.dat");
   copy_backup_contents(SYSTEM_DIR "backup/piggyback.dat", SYSTEM_DIR "piggyback.dat");
   copy_backup_contents(KINGDOM_DIR "backup/buykingdom.dat", KINGDOM_DIR "buykingdom.dat");
   copy_backup_contents(KINGDOM_DIR "backup/extract.dat", KINGDOM_DIR "extract.dat");
   copy_backup_contents(KINGDOM_DIR "backup/Niemria.depo", KINGDOM_DIR "Niemria.depo");
   copy_backup_contents(KINGDOM_DIR "backup/Rafermand.depo", KINGDOM_DIR "Rafermand.depo");
   copy_backup_contents(KINGDOM_DIR "backup/trade.lst", KINGDOM_DIR "trade.lst");
   copy_backup_contents(KINGDOM_DIR "backup/conquer.lst", KINGDOM_DIR "conquer.lst");
   copy_backup_contents(KINGDOM_DIR "backup/milist.dat", KINGDOM_DIR "milist.dat");
   copy_backup_contents(KINGDOM_DIR "backup/Niemria.kingdom", KINGDOM_DIR "Niemria.kingdom");
   copy_backup_contents(KINGDOM_DIR "backup/Rafermand.kingdom", KINGDOM_DIR "Rafermand.kingdom");
   copy_backup_contents(KINGDOM_DIR "backup/training.dat", KINGDOM_DIR "training.dat");
   copy_backup_contents(MAP_DIR "backup/solan.map", MAP_DIR "solan.map");
   copy_backup_contents(MAP_DIR "backup/solan.res", MAP_DIR "solan.res");
   copy_backup_contents(MAP_DIR "backup/wblock.dat", MAP_DIR "wblock.dat");
   copy_backup_contents(CASTE_DIR "backup/kchest.dat", CASTE_DIR "kchest.dat");
   return;
}

void fread_pfile3(FILE * fp, char *fname, char *filename, int *sworth, int *eworth, int *plevel, int *pranking, int *twinkpoints)
{
   char *word;
   int level = 0;
   bool fMatch;
   char *trash = NULL;

   for (;;)
   {
      word = feof(fp) ? "End" : fread_word(fp);
      fMatch = FALSE;

      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;

         case 'B':
            KEY("Bio", trash, fread_string(fp));
            break;
            
         case 'E':
            KEY("EWorth", *eworth, fread_number(fp));
            if (!strcmp(word, "End"))
               goto timecheck;
            break;
            
         case 'L':
            KEY("Level", level, fread_number(fp));

         case 'P':
            KEY("PLevel", *plevel, fread_number(fp));
            KEY("PowerRanking", *pranking, fread_number(fp));
            break;
            
         case 'S':
            KEY("Sworth", *sworth, fread_number(fp));
            break;
            
         case 'T':
            KEY("TwinkPoints", *twinkpoints, fread_number(fp));
            break;
      }

      if (!fMatch)
         fread_to_eol(fp);
   }

 timecheck:

   if (level >= LEVEL_IMMORTAL)
   {
      *sworth = 0;
   }
   return;
}
void read_pfile3(char *dirname, char *filename, int *sworth, int *eworth, int *plevel, int *pranking, int *twinkpoints)
{
   FILE *fp;
   char fname[MSL];
   struct stat fst;

   sprintf(fname, "%s/%s", dirname, filename);

   if (stat(fname, &fst) != -1)
   {

      if ((fp = fopen(fname, "r")) != NULL)
      {
         for (;;)
         {
            char letter;
            char *word;

            letter = fread_letter(fp);

            if (letter != '#')
               continue;

            word = fread_word(fp);

            if (!str_cmp(word, "End"))
               break;

            if (!str_cmp(word, "PLAYER"))
               fread_pfile3(fp, fname, filename, sworth, eworth, plevel, pranking, twinkpoints);
            else if (!str_cmp(word, "END")) /* Done  */
               break;
         }
         FCLOSE(fp);
      }
   }
   return;
}

void read_pfiles_for_stats(char *filename)
{
   DIR *dp;
   FILE *fp;
   struct dirent *dentry;
   char dir_name[100];
   int sworth = 0;
   int eworth = 0;
   int plevel = 0;
   int pranking = 0;
   int twinkpoints = 0;
   int x;
   int y;
   int currvalue;
   int prevvalue;
   char currname[100];
   char prevname[100];
   char sworthchart[11][100];
   char eworthchart[11][100];
   char plevelchart[11][100];
   char prankingchart[11][100];
   char twinkpointschart[11][100];
   int sworthvalue[11];
   int eworthvalue[11];
   int plevelvalue[11];
   int prankingvalue[11];
   int twinkpointsvalue[11];
   int alpha_loop;
   
   for (x = 0; x <= 9; x++)
   {
      strcpy(sworthchart[x], "");
      strcpy(eworthchart[x], "");
      strcpy(plevelchart[x], "");
      strcpy(prankingchart[x], "");
      strcpy(twinkpointschart[x], "");
      sworthvalue[x]=0;
      eworthvalue[x]=0;
      plevelvalue[x]=0;
      prankingvalue[x]=0;
      twinkpointsvalue[x]=0;
   }
   

   for (alpha_loop = 0; alpha_loop <= 25; alpha_loop++)
   {
      sprintf(dir_name, "%s%c", PLAYER_DIR, 'a' + alpha_loop);
      dp = opendir(dir_name);
      dentry = readdir(dp);
      while (dentry)
      {
         if (dentry->d_name[0] != '.')
         {
            read_pfile3(dir_name, dentry->d_name, &sworth, &eworth, &plevel, &pranking, &twinkpoints);
         }
         if (sworth > 0)
         {
            if (sworthvalue[0] == 0)
            {
               sworthvalue[0] = sworth;
               sprintf(sworthchart[0], dentry->d_name);
            }
            else
            {
               for (x=0; x <= 9; x++)
               {
                  if (sworthvalue[x] == 0)
                  {
                     sworthvalue[x] = sworth;
                     sprintf(sworthchart[x], dentry->d_name);
                     break;
                  }
                  if (sworthvalue[x] < sworth)
                  {
                     prevvalue = sworthvalue[x];
                        sprintf(prevname, sworthchart[x]);                        
                        sworthvalue[x] = sworth;
                        sprintf(sworthchart[x], dentry->d_name);
                        for (y = x+1; y <= 9; y++)
                        {
                           currvalue = sworthvalue[y];
                           sprintf(currname, sworthchart[y]); 
                           sworthvalue[y] = prevvalue;
                           sprintf(sworthchart[y], prevname);
                           if (currvalue == 0)
                              break;
                           prevvalue = currvalue;
                           sprintf(prevname, currname);
                        }
                     break;
                  }
               }
            }
            if (eworth > 0)
            {
               if (eworthvalue[0] == 0)
               {
                  eworthvalue[0] = eworth;
                  sprintf(eworthchart[0], dentry->d_name);
               }
               else
               {
                  for (x=0; x <= 9; x++)
                  {
                     if (eworthvalue[x] == 0)
                     {
                        eworthvalue[x] = eworth;
                        sprintf(eworthchart[x], dentry->d_name);
                        break;
                     }
                     if (eworthvalue[x] < eworth)
                     {
                        prevvalue = eworthvalue[x];
                        sprintf(prevname, eworthchart[x]);                        
                        eworthvalue[x] = eworth;
                        sprintf(eworthchart[x], dentry->d_name);
                        for (y = x+1; y <= 9; y++)
                        {
                           currvalue = eworthvalue[y];
                           sprintf(currname, eworthchart[y]); 
                           eworthvalue[y] = prevvalue;
                           sprintf(eworthchart[y], prevname);
                           if (currvalue == 0)
                              break;
                           prevvalue = currvalue;
                           sprintf(prevname, currname);
                        }
                        break;
                     }
                  }
               }
            }
            if (plevel > 0)
            {
               if (plevelvalue[0] == 0)
               {
                  plevelvalue[0] = plevel;
                  sprintf(plevelchart[0], dentry->d_name);
               }
               else
               {
                  for (x=0; x <= 9; x++)
                  {
                     if (plevelvalue[x] == 0)
                     {
                        plevelvalue[x] = plevel;
                        sprintf(plevelchart[x], dentry->d_name);
                        break;
                     }
                     if (plevelvalue[x] < plevel)
                     {
                        prevvalue = plevelvalue[x];
                        sprintf(prevname, plevelchart[x]);                        
                        plevelvalue[x] = plevel;
                        sprintf(plevelchart[x], dentry->d_name);
                        for (y = x+1; y <= 9; y++)
                        {
                           currvalue = plevelvalue[y];
                           sprintf(currname, plevelchart[y]); 
                           plevelvalue[y] = prevvalue;
                           sprintf(plevelchart[y], prevname);
                           if (currvalue == 0)
                              break;
                           prevvalue = currvalue;
                           sprintf(prevname, currname);
                        }
                        break;
                     }
                  }
               }
            }
            if (pranking > 0)
            {
               if (prankingvalue[0] == 0)
               {
                  prankingvalue[0] = pranking;
                  sprintf(prankingchart[0], dentry->d_name);
               }
               else
               {
                  for (x=0; x <= 9; x++)
                  {
                     if (prankingvalue[x] == 0)
                     {
                        prankingvalue[x] = pranking;
                        sprintf(prankingchart[x], dentry->d_name);
                        break;
                     }
                     if (prankingvalue[x] < pranking)
                     {
                        prevvalue = prankingvalue[x];
                        sprintf(prevname, prankingchart[x]);                        
                        prankingvalue[x] = pranking;
                        sprintf(prankingchart[x], dentry->d_name);
                        for (y = x+1; y <= 9; y++)
                        {
                           currvalue = prankingvalue[y];
                           sprintf(currname, prankingchart[y]); 
                           prankingvalue[y] = prevvalue;
                           sprintf(prankingchart[y], prevname);
                           if (currvalue == 0)
                              break;
                           prevvalue = currvalue;
                           sprintf(prevname, currname);
                        }
                        break;
                     }
                  }
               }
            }
            if (twinkpoints > 0)
            {
               if (twinkpointsvalue[0] == 0)
               {
                  twinkpointsvalue[0] = twinkpoints;
                  sprintf(twinkpointschart[0], dentry->d_name);
               }
               else
               {
                  for (x=0; x <= 9; x++)
                  {
                     if (twinkpointsvalue[x] == 0)
                     {
                        twinkpointsvalue[x] = twinkpoints;
                        sprintf(twinkpointschart[x], dentry->d_name);
                        break;
                     }
                     if (twinkpointsvalue[x] < twinkpoints)
                     {
                        prevvalue = twinkpointsvalue[x];
                        sprintf(prevname, twinkpointschart[x]);                        
                        twinkpointsvalue[x] = twinkpoints;
                        sprintf(twinkpointschart[x], dentry->d_name);
                        for (y = x+1; y <= 9; y++)
                        {
                           currvalue = twinkpointsvalue[y];
                           sprintf(currname, twinkpointschart[y]); 
                           twinkpointsvalue[y] = prevvalue;
                           sprintf(twinkpointschart[y], prevname);
                           if (currvalue == 0)
                              break;
                           prevvalue = currvalue;
                           sprintf(prevname, currname);
                        }
                        break;
                     }
                  }
               }
            }
         }
         sworth = 0;
         eworth = 0;
         plevel = 0;
         pranking = 0;
         twinkpoints = 0;
         dentry = readdir(dp);
      }
      closedir(dp);
   }
   
   if (!filename)
   {
      if ((fp = fopen(TOCSTAT_FILE, "w")) == NULL)
      {
         bug("Cannot open: %s for writing", TOCSTAT_FILE);
         return;
      }
   }
   else
   {
      if ((fp = fopen(filename, "w")) == NULL)
      {
         bug("Cannot open: %s for writing", filename);
         return;
      }
   }
   fprintf(fp, "<html>\n");
   fprintf(fp, "<head>\n");
   fprintf(fp, "<title>Top 10 Lists</title>\n");
   fprintf(fp, "</head>\n");
   fprintf(fp, "<body><pre>\n");
   fprintf(fp, "                      Top Ten Lists\n\n");
   fprintf(fp, "Stat Worth Value        Player Name\n----------------------------------------------------------\n");
   for (x = 0; x <= 9; x++)
   {
      if (sworthvalue[x] > 0)
         fprintf(fp, "%-7d                 %s\n", sworthvalue[x], sworthchart[x]);
   }
   fprintf(fp, "\nEquipment Worth Value   Player Name\n----------------------------------------------------------\n");
   for (x = 0; x <= 9; x++)
   {
      if (eworthvalue[x] > 0)
         fprintf(fp, "%-7d                 %s\n", eworthvalue[x], eworthchart[x]);
   }
   fprintf(fp, "\nPower Level Value       Player Name\n----------------------------------------------------------\n");
   for (x = 0; x <= 9; x++)
   {
      if (plevelvalue[x] > 0)
         fprintf(fp, "%-3d                     %s\n", plevelvalue[x], plevelchart[x]);
   }
   fprintf(fp, "\nPower Ranking Value     Player Name\n----------------------------------------------------------\n");
   for (x = 0; x <= 9; x++)
   {
      if (prankingvalue[x] > 0)
         fprintf(fp, "%-4d                    %s\n", prankingvalue[x], prankingchart[x]);
   }
   fprintf(fp, "\nTwink Points Value      Player Name\n----------------------------------------------------------\n");
   for (x = 0; x <= 9; x++)
   {
      if (twinkpointsvalue[x] > 0)
         fprintf(fp, "%-3d                     %s\n", twinkpointsvalue[x], twinkpointschart[x]);
   }
   fprintf(fp, "\nLast update: %s", ctime(&current_time));
   fprintf(fp, "</html>\n");
   fclose(fp);
   return;
}
   
void do_gamereset(CHAR_DATA *ch, char *argument)
{
   DESCRIPTOR_DATA *d;
   int x;
   BUYKBIN_DATA *kbin;
   BTRAINER_DATA *btrain;
   BUYKBIN_DATA *kbin_next;
   BTRAINER_DATA *btrain_next;
   CHAR_DATA *mob;
   CHAR_DATA *mob_prev;
   
   if (!sysdata.resetgame)
      return;
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  gamereset now\n\r", ch);
      send_to_char("Syntax:  CAUTION:  This will reset the game, read the helpfile.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "test"))
   {
      read_pfiles_for_stats(NULL);
      send_to_char("Saved.\n\r", ch);
      return;
   }
      
   if (str_cmp(argument, "now"))
   {
      send_to_char("You need to follow the command with the word now.\n\r", ch);
      return;
   }
   bug("[GAME RESET] Started by %s", ch->name);
   for (d = first_descriptor; d; d = d_next)
   {
      if (d->character && d->character->level >= LEVEL_IMMORTAL)
         continue;
      write_to_descriptor(d->descriptor, "\n\rThe game is in the process of being reset, try again in a few seconds.\n\r", 0);
      close_socket(d, FALSE); /* throw'em out */
   }
   gamereset_pfiles(); //Resets the pfiles
   //Clean up kingdoms
   for (x = sysdata.max_kingdom-1; x > 1; x--)
   {
      remove_all_towns(x);
      remove_kingdom(x);
   }
   //Clean up kingdom trainers
   for (btrain = first_boughttrainer; btrain; btrain = btrain_next)
   {
      btrain_next = btrain->next;
      UNLINK(btrain, first_boughttrainer, last_boughttrainer, next, prev);
      DISPOSE(btrain);
   }
   //Clean up Bins
   for (kbin = first_buykbin; kbin; kbin = kbin_next)
   {
      kbin_next = kbin->next;
      STRFREE(kbin->name);
      UNLINK(kbin, first_buykbin, last_buykbin, next, prev);
      DISPOSE(kbin);
   }
   save_buykingdom_data();
   //Clean up extraction mobs
   for (mob = last_char; mob; mob = mob_prev)   
   { 
      mob_prev = mob->prev;
      if (IS_NPC(mob) && IS_ACT_FLAG(mob, ACT_EXTRACTMOB) && mob->in_room)
      {  
         extract_char(mob, TRUE);   
      }
   }
   save_extraction_data();
   copy_backup_files();
   read_pfiles_for_stats(NULL);
   do_copyover(NULL, "nosave");
   return;
}
      


void do_copyover(CHAR_DATA * ch, char *argument)
{
   FILE *fp;
   FILE *pfp;
   int x;
   DESCRIPTOR_DATA *d, *d_next;
   char buf[100], buf2[100], buf3[100], buf4[100], buf5[100];
   
   if (argument[0] == '\0')
   {
      if (ch)
      {
         send_to_char("Syntax:  copyover now\n\r", ch);
         send_to_char("Syntax:  copyover nosave\n\r", ch);
         send_to_char("Syntax:  copyover noquest\n\r", ch);
      }
      return;
   }
   if (str_cmp(argument, "now") && str_cmp(argument, "nosave") && str_cmp(argument, "noquest"))
   {
      do_copyover(ch, "");
      return;
   }
   
   fp = fopen(COPYOVER_FILE, "w");
   pfp = fopen(PIGGYBACK_FILE, "w");

   if (!fp)
   {
      if (ch)
         send_to_char("Copyover file not writeable, aborted.\n\r", ch);
      log_printf("Could not write to copyover file: %s", COPYOVER_FILE);
      perror("do_copyover:fopen");
      return;
   }
   if (!pfp)
   {
      if (ch)
         send_to_char("Piggyback file not writeable, aborted.\n\r", ch);
      log_printf("Could not write to piggyback file: %s", PIGGYBACK_FILE);
      perror("do_copyover:fopen");
      return;
   }
      
   if (!str_cmp(argument, "now"))
   {
      for (x = 0; x < sysdata.max_kingdom; x++)
      {
        write_kingdom_file(x);
      }
      write_kingdom_list();
      weather_update();
      save_resources("solan", 0);
      save_sysdata(sysdata);
      save_map("solan", 0);
      save_bin_data();
      remove(MILIST_FILE);
      save_wblock_data();
      save_mlist_data();
      save_extraction_data();
      write_channelhistory_file();
      save_pc_corpses();
      save_quest_data();
   }
   if (!str_cmp(argument, "noquest"))
   {
      for (x = 0; x < sysdata.max_kingdom; x++)
      {
        write_kingdom_file(x);
      }
      write_kingdom_list();
      weather_update();
      save_resources("solan", 0);
      save_sysdata(sysdata);
      save_map("solan", 0);
      save_bin_data();
      remove(MILIST_FILE);
      save_wblock_data();
      save_mlist_data();
      save_extraction_data();
      write_channelhistory_file();
      save_pc_corpses();
   }
   sprintf(buf, "\n\r *** COPYOVER initated  - please don't become too impatient, it is unbecoming!\n\r");
   /* For each playing descriptor, save its state */
   for (d = first_descriptor; d; d = d_next)
   {
      CHAR_DATA *och = CH(d);

      d_next = d->next; /* We delete from the list , so need to save this */
      if (!d->character || d->connected != CON_PLAYING) /* drop those logging on */
      {
         write_to_descriptor(d->descriptor, "\n\rSorry, we are in the process of a copyover.  Come back in a few seconds.\n\r", 0);
         close_socket(d, FALSE); /* throw'em out */
      }
      else
      {
         fprintf(fp, "%d %s %s %s\n", d->descriptor, d->account->name, och->name, d->host);
         if (d->character->riding && !IS_NPC(d->character->riding))
            fprintf(pfp, "%d %d\n", d->character->pcdata->pid, d->character->riding->pcdata->pid);
         if (d->character->rider)
            fprintf(pfp, "%d %d\n", d->character->rider->pcdata->pid, d->character->pcdata->pid);
         if (d->mxp)
            xSET_BIT(och->act, PLR_MXP); //so I know who to turn mxp back on for
         if (d->character->riding && IS_NPC(d->character->riding))
            do_dismount(d->character, "");
         save_char_obj(och);
         #ifdef MCCP
            write_to_descriptor( d->descriptor, buf, 0 );
            compressEnd( d );
         #else
		    write_to_descriptor( d->descriptor, buf, 0 );
         #endif
      }
   }
   fprintf(fp, "-1\n");
   fclose(fp);
   fprintf(pfp, "-1\n");
   fclose(pfp);

   /* Close reserve and other always-open files and release other resources */
   fclose(fpReserve);
   fclose(fpLOG);
   
   /* exec - descriptors are inherited */
    snprintf( buf,  100, "%d", port );
    snprintf( buf2, 100, "%d", control );
    snprintf( buf3, 100, "%d", control2 );
    snprintf( buf4, 100, "%d", conclient );
    snprintf( buf5, 100, "%d", conjava );

   execl("../src/fear", "fear", buf, "copyover", buf2, buf3, buf4, buf5, (char *) NULL);

   /* Failed - sucessful exec will not return */

   perror("do_copyover: execl");
   if (ch)
      send_to_char("Reboot FAILED!\n\r", ch);

   /* Here you might want to reopen fpReserve */
   /* Since I'm a neophyte type guy, I'll assume this is a good idea and cut and past from main()  */

   if ((fpReserve = fopen(NULL_FILE, "r")) == NULL)
   {
      perror(NULL_FILE);
      exit(1);
   }
   if ((fpLOG = fopen(NULL_FILE, "r")) == NULL)
   {
      perror(NULL_FILE);
      exit(1);
   }

}

/* Recover from a copyover - load players */
void copyover_recover()
{
   DESCRIPTOR_DATA *d;
   DESCRIPTOR_DATA *pd;
   FILE *fp;
   FILE *pfp;
   char name[100];
   char account[100];
   QUEST_DATA *quest;

#ifdef DNS_SLAVE
   char host[60];
#else
   char host[MSL];
#endif
   int desc;
   bool fOld;

   log_string("Warmboot recovery initiated");
   fp = fopen(COPYOVER_FILE, "r");

   if (!fp) /* there are some descriptors open which will hang forever then ? */
   {
      perror("copyover_recover:fopen");
      log_string("Warmboot file not found. Exitting.\n\r");
      exit(1);
   }

   unlink(COPYOVER_FILE); /* In case something crashes
                             - doesn't prevent reading */
   for (;;)
   {
      fscanf(fp, "%d %s %s %s\n", &desc, account, name, host);
      if (desc == -1)
         break;

      /* Write something, and check if it goes error-free */
      if (!write_to_descriptor(desc, "\n\rRestoring from copyover...\n\r", 0))
      {
         close(desc); /* nope */
         continue;
      }

      CREATE(d, DESCRIPTOR_DATA, 1);
      init_descriptor(d, desc); /* set up various stuff */

#ifdef DNS_SLAVE
      strcpy(d->host, host);
#else
      d->host = STRALLOC(host);
#endif
#ifdef MCCP
      write_to_buffer(d, eor_on_str, 0);
      write_to_buffer(d, compress2_on_str, 0);
      write_to_buffer(d, compress_on_str, 0);
#endif

      LINK(d, first_descriptor, last_descriptor, next, prev);
      d->connected = CON_COPYOVER_RECOVER; /* negative so close_socket will cut them off */

      /* Now, find the pfile */

      fOld = load_char_obj(d, name, FALSE);

      if (!fOld) /* Player file not found?! */
      {
         write_to_descriptor(desc, "\n\rSomehow, your character was lost in the warmboot.  \n\rPlease contact an Immortal.\n\r", 0);
         close_socket(d, FALSE);
      }
      
      fOld = load_account(d, account, FALSE);
      
      if (!fOld) /* Account file not found?! */
      {
         write_to_descriptor(desc, "\n\rSomehow, your account was lost in the warmboot.  \n\rPlease contact an Immortal.\n\r", 0);
         close_socket(d, FALSE);
      }
      else /* ok! */
      {
         write_to_descriptor(desc, "\n\rWarmboot recovery complete.\n\r", 0);

         /* This isn't necassary, but _why_ do we want to dump someone in limbo? */
         if (!d->character->in_room)
            d->character->in_room = get_room_index(ROOM_VNUM_TEMPLE);
         /* Insert in the char_list */
         LINK(d->character, first_char, last_char, next, prev);

         /*
          * So that it doesn't reset as default of 0 when 20 playersare on
          * -- Callidyrr
          */
         if (xIS_SET(d->character->act, PLR_MXP)) //restore mxp if they had it on before...
         {
            turn_on_mxp(d);
            xREMOVE_BIT(d->character->act, PLR_MXP);
         }
         num_descriptors++;
         sysdata.maxplayers++;

         for (quest = first_quest; quest; quest = quest->next)
         {
            int x;
            for (x = 0; x <= 5; x++)
            {
               if (quest->player[x] == d->character->pcdata->pid)
                  d->character->pcdata->quest = quest;
            }
         }
         char_to_room(d->character, d->character->in_room);
      }
   }
   for (d = first_descriptor; d; d = d->next)
   {
      int riding;
      int rider;
      
      if (d->character)
      {
         pfp = fopen(PIGGYBACK_FILE, "r");
         if (pfp)
         {
            for (;;)
            {
               fscanf(pfp, "%d %d\n", &riding, &rider);   
               if (riding == -1)
                  break;
               if (riding == d->character->pcdata->pid)
               {
                  for (pd = first_descriptor; pd; pd = pd->next)
                  {
                     if (pd->character && pd->character->pcdata->pid == rider)
                     {
                        d->character->riding = pd->character;
                        pd->character->rider = d->character;
                        d->character->position = POS_RIDING;
                     }
                  }             
               }
               if (rider == d->character->pcdata->pid)
               {
                  for (pd = first_descriptor; pd; pd = pd->next)
                  {
                     if (pd->character && pd->character->pcdata->pid == riding)
                     {
                        d->character->rider = pd->character;
                        pd->character->riding = d->character;
                        pd->character->position = POS_RIDING;
                     }
                  }             
               }
            }
            fclose(pfp);
            pfp = NULL;
         }
         do_look(d->character, "auto noprog");
         ch_printf(d->character, "%s", MXPTAG("VERSION"));
         act(AT_ACTION, "$n materializes!", d->character, NULL, NULL, TO_ROOM);
         d->connected = CON_PLAYING;
      }
   }         
   fclose(fp);
   fp = NULL;
}
