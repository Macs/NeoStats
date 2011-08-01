#include "socks.h"
#include "neostats.h"
#include "config.h"

/* max sockets we can open */
#define MAXSOCKS 64

/* prototypes */
int InspIRCd(void);
int InitConfig(void);
void Error(int status);
void send_error(char *s);
void ReadConfig(void);
void WriteOpers(char* text, ...);
