/* 
**  mod_layout.h -- Common defines.
**  $Revision: 1.14 $
*/

#define CORE_PRIVATE
#include <sys/stat.h>
#include <unistd.h>
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "http_log.h"
#include "buff.h"
#include "fnmatch.h"
#include "http_core.h"
#include <sys/mman.h>


#define BUFFER_LENGTH 1024

#define UNSET (-1)
#define OFF (0)
#define ON (1)
#define isOff(a) ((a == ON) ? 0 : 1 )
#define isOn(a) ((a == ON) ? 1 : 0 )

/* layout states */
#define LAYOUT (0)
#define HTTP (1)
#define HEADER (2)
#define ORIGIN (3)
#define FOOTER (3)

#define WATCHPOINT printf("WATCHPOINT %s %d\n", __FILE__, __LINE__);
#define AWATCHPOINT ap_rprintf(r, "WATCHPOINT %s %d\n", __FILE__, __LINE__);
#define LAYOUT_EXPORT(type)    type

#define LAYOUT_CACHE "/tmp"
#define LAYOUT_BEGINTAG "<body*>"
#define LAYOUT_ENDTAG "</body>"
#define LAYOUT_TIMEFORMAT "%A, %d-%b-%Y %H:%M:%S %Z"

typedef struct {
	int proxy;
	int glob;
	int bypasserror;
	array_header *headertype;
	array_header *header;
	int footer_enabled;
	array_header *footertype;
	array_header *footer;
	int display_middle;
	table *override;
	table *override_uri;
	int header_enabled;
	int async_post;
	const char *async_cache;
	int http_header_enabled;
	int comment;
	/* These next three variables change with each request */
	int request_header;
	int request_http_header;
	int request_footer;
	const char *request_type;
	char *http_header;
	const char *time_format;
	table *types;
	table *uris_ignore;
	table *uris_ignore_header;
	table *uris_ignore_http_header;
	table *uris_ignore_footer;
	int append_header;
	int append_footer;
	int error_occuring;
	int cache_needed;
	int merge;
	const char *begin_tag;
	const char *end_tag;
} layout_conf;

module MODULE_VAR_EXPORT layout_module;

LAYOUT_EXPORT(int) string_search(request_rec *r, char *string, const char *delim, int init_pos, int flag);
LAYOUT_EXPORT(int) write_file(request_rec *r, char *filename);
LAYOUT_EXPORT(int) get_fd_in(request_rec *r, char *filename);
LAYOUT_EXPORT(int) get_fd_out(request_rec *r, char *filename);
LAYOUT_EXPORT(int) read_content(request_rec *r, char *filename, long length);
LAYOUT_EXPORT(int) check_table(const char *a);
LAYOUT_EXPORT(int) table_find(const table * t, const char *key);
LAYOUT_EXPORT(int) call_main(request_rec *r, int assbackwards);
LAYOUT_EXPORT(int) layout_headers(request_rec *r, layout_conf *cfg);
LAYOUT_EXPORT(int) call_error(request_rec *r, int status, layout_conf *cfg);
LAYOUT_EXPORT(int) call_container(request_rec *r, char *uri, layout_conf *cfg, int assbackwards);
LAYOUT_EXPORT(void) print_layout_headers(request_rec *r, layout_conf *cfg);
LAYOUT_EXPORT(char *)add_file(cmd_parms * cmd, layout_conf *cfg, char *file);
