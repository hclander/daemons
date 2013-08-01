
#include <syslog.h>

#define INITLOG(tag) openlog(tag, LOG_PID, LOG_DAEMON)

#define INITLOGEX(tag, facility) openlog(tag LOG_PID, facility)

#define LOG(level, msg) syslog(level, format)
#define LOG_F(level, format, ...) syslog(level, format, __VA_ARGS__)

#define LOG_D(msg) syslog(LOG_DEBUG, msg)
#define LOG_I(msg) syslog(LOG_INFO, msg)
#define LOG_N(msg) syslog(LOG_NOTICE, msg)
#define LOG_W(msg) syslog(LOG_WARNING, msg)
#define LOG_E(msg) syslog(LOG_ERR, msg)
#define LOG_C(msg) syslog(LOG_CRIT, msg)
#define LOG_A(msg) syslog(LOG_ALERT, msg)

#define LOG_F_D(format, ...) syslog(LOG_DEBUG, format, __VA_ARGS__)
#define LOG_F_I(format, ...) syslog(LOG_INFO, format, __VA_ARGS__)
#define LOG_F_N(format, ...) syslog(LOG_NOTICE, format, __VA_ARGS__)
#define LOG_F_W(format, ...) syslog(LOG_WARNING, format, __VA_ARGS__)
#define LOG_F_E(format, ...) syslog(LOG_ERR, format, __VA_ARGS__)
#define LOG_F_C(format, ...) syslog(LOG_CRIT, format, __VA_ARGS__)
#define LOG_F_A(format, ...) syslog(LOG_ALERT, format, __VA_ARGS__)


#define LOG_LEVEL(level) setlogmask(LOG_UPTO(level))

#define CLOSELOG()  closelog()
