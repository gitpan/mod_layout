/* 
**  mod_layout.c -- Apache layout module
**  $Revision: 1.76 $
*/

#include "mod_layout_directives.h"
#include "mod_layout.h"


static void *create_dir_mconfig(pool *p, char *dir)
{
/* So why -1, 0, and 1?  You see, C lacks an arithmatic if. We need to
	know three states at any point. We need to know if something is unset, off or
	on. Hence we use these values. Apache already understands Off as 0 and 1 as
	on. 
*/
	layout_conf *cfg;
	cfg = ap_pcalloc(p, sizeof(layout_conf));
	cfg->merge = UNSET;
	cfg->bypasserror = UNSET;
	cfg->proxy = UNSET;
	cfg->comment = UNSET;
	cfg->append_header = UNSET;
	cfg->append_footer = UNSET;
	cfg->cache_needed = UNSET;
	cfg->glob = UNSET;
	cfg->display_middle = UNSET;
	cfg->override = ap_make_table(p, 0);
	cfg->override_uri = ap_make_table(p, 0);
	cfg->header_enabled = UNSET;
	cfg->footer_enabled = UNSET;
	cfg->http_header_enabled = UNSET;
	cfg->http_header = NULL;
	cfg->header = ap_make_array (p, 3, sizeof (char *));
	cfg->headertype = ap_make_array (p, 3, sizeof (char *));
	cfg->footer = ap_make_array (p, 3, sizeof (char *));
	cfg->footertype = ap_make_array (p, 3, sizeof (int));
	cfg->async_cache = LAYOUT_CACHE;
	cfg->begin_tag = LAYOUT_BEGINTAG;
	cfg->end_tag = LAYOUT_ENDTAG;
	cfg->request_header = OFF;
	cfg->request_http_header = OFF;
	cfg->request_footer = OFF;
	cfg->request_type = NULL;
	cfg->async_post = OFF;
	cfg->time_format = LAYOUT_TIMEFORMAT;
	cfg->types = ap_make_table(p, 8);
	cfg->uris_ignore = ap_make_table(p, 8);
	cfg->uris_ignore_header = ap_make_table(p, 8);
	cfg->uris_ignore_http_header = ap_make_table(p, 8);
	cfg->uris_ignore_footer = ap_make_table(p, 8);

	ap_table_setn(cfg->types, INCLUDES_MAGIC_TYPE, "1");
	ap_table_setn(cfg->types, INCLUDES_MAGIC_TYPE3, "1");
	ap_table_setn(cfg->types, "server-parsed", "1");
	ap_table_setn(cfg->types, "text/html", "1");
	ap_table_setn(cfg->types, "text/plain", "1");
	ap_table_setn(cfg->types, "perl-script", "1");
	ap_table_setn(cfg->types, "cgi-script", "1");
	ap_table_setn(cfg->types, "application/x-httpd-cgi", "1");
	cfg->error_occuring = OFF;

	return (void *) cfg;
}

static void *merge_dir_mconfig(pool *p, void *origin, void *new) {
	layout_conf *cfg = ap_pcalloc(p, sizeof(layout_conf));
	layout_conf *cfg_origin = (layout_conf *)origin;
	layout_conf *cfg_new = (layout_conf *)new;
	cfg->merge = UNSET;
	cfg->bypasserror = UNSET;
	cfg->proxy = UNSET;
	cfg->comment = UNSET;
	cfg->cache_needed = UNSET;
	cfg->glob = UNSET;
	cfg->header = ap_make_array(p, 6, sizeof (char *));
	cfg->headertype = ap_make_array(p, 6, sizeof (char *));
	cfg->footer = ap_make_array(p, 6, sizeof (char *));;
	cfg->footertype = ap_make_array(p, 6, sizeof (int));;
	cfg->display_middle = UNSET;
	cfg->override = ap_make_table(p, 0);
	cfg->override_uri = ap_make_table(p, 0);
	cfg->header_enabled = UNSET;
	cfg->footer_enabled = UNSET;
	cfg->http_header_enabled = UNSET;
	cfg->http_header = NULL;
	cfg->time_format = LAYOUT_TIMEFORMAT;
	cfg->request_header = OFF;
	cfg->request_footer = OFF;
	cfg->request_http_header = OFF;
	cfg->request_type = NULL;
	cfg->async_post = OFF;
	cfg->async_cache = LAYOUT_CACHE;
	cfg->error_occuring = OFF;
	cfg->begin_tag = LAYOUT_BEGINTAG;
	cfg->end_tag = LAYOUT_ENDTAG;
	cfg->append_header = UNSET;
	cfg->append_footer = UNSET;
	
	if(strcmp(cfg_new->async_cache, LAYOUT_CACHE)){
		cfg->async_cache = ap_pstrdup(p, cfg_new->async_cache);
	} else if (strcmp(cfg_origin->async_cache, LAYOUT_CACHE)){
		cfg->async_cache = ap_pstrdup(p, cfg_origin->async_cache);
	}

	if(strcmp(cfg_new->time_format, LAYOUT_TIMEFORMAT)){
		cfg->time_format = ap_pstrdup(p, cfg_new->time_format);
	} else if (strcmp(cfg_origin->time_format, LAYOUT_TIMEFORMAT)){
		cfg->time_format = ap_pstrdup(p, cfg_origin->time_format);
	}

	if(strcmp(cfg_new->begin_tag, LAYOUT_BEGINTAG)){
		cfg->begin_tag = ap_pstrdup(p, cfg_new->begin_tag);
	} else if (strcmp(cfg_origin->begin_tag, LAYOUT_BEGINTAG)){
		cfg->begin_tag = ap_pstrdup(p, cfg_origin->begin_tag);
	}

	if(strcmp(cfg_new->end_tag, LAYOUT_ENDTAG)){
		cfg->end_tag = ap_pstrdup(p, cfg_new->end_tag);
	} else if (strcmp(cfg_origin->end_tag, LAYOUT_ENDTAG)){
		cfg->end_tag = ap_pstrdup(p, cfg_origin->end_tag);
	}
	
	cfg->cache_needed = (cfg_new->cache_needed == UNSET) ? cfg_origin->cache_needed : cfg_new->cache_needed;
	cfg->bypasserror = (cfg_new->bypasserror == UNSET) ? cfg_origin->bypasserror : cfg_new->bypasserror;
	cfg->proxy = (cfg_new->proxy == UNSET) ? cfg_origin->proxy : cfg_new->proxy;
	cfg->merge = (cfg_new->merge == UNSET) ? cfg_origin->merge : cfg_new->merge;
	cfg->comment = (cfg_new->comment == UNSET) ? cfg_origin->comment : cfg_new->comment;
	cfg->async_post = (cfg_new->async_post == UNSET) ? cfg_origin->async_post : cfg_new->async_post;
	cfg->display_middle = (cfg_new->display_middle == UNSET) ? cfg_origin->display_middle : cfg_new->display_middle;
	cfg->glob = (cfg_new->glob == UNSET) ? cfg_origin->glob : cfg_new->glob;

	cfg->append_header = (cfg_new->append_header == UNSET) ? cfg_origin->append_header : cfg_new->append_header;
	cfg->append_footer = (cfg_new->append_footer == UNSET) ? cfg_origin->append_footer : cfg_new->append_footer;

	if(isOn(cfg->append_header))  {
		if(isOn(cfg_new->header_enabled) || isOn(cfg_origin->header_enabled)) {
			cfg->header_enabled = ON;
			cfg->header = ap_append_arrays(p, cfg_origin->header, cfg_new->header);
			cfg->headertype = ap_append_arrays(p, cfg_origin->headertype, cfg_new->headertype);
		}
	} else {
		if(cfg_new->header_enabled == UNSET){
			cfg->headertype = cfg_origin->headertype;
			cfg->header = cfg_origin->header;
			cfg->header_enabled = cfg_origin->header_enabled;
		} else if (isOn(cfg_new->header_enabled)){
			cfg->headertype = cfg_new->headertype;
			cfg->header = cfg_new->header;
			cfg->header_enabled = cfg_new->header_enabled;
		} else {
			cfg->header_enabled = OFF;
		}
	}

	if(isOn(cfg->append_footer))  {
		if(isOn(cfg_new->footer_enabled) || isOn(cfg_origin->footer_enabled)) {
			cfg->footer_enabled = ON;
			cfg->footer = ap_append_arrays(p, cfg_origin->footer, cfg_new->footer);
			cfg->footertype = ap_append_arrays(p, cfg_origin->footertype, cfg_new->footertype);
		}
	} else {
		if(cfg_new->footer_enabled == UNSET){
			cfg->footertype = cfg_origin->footertype;
			cfg->footer = cfg_origin->footer;
			cfg->footer_enabled = cfg_origin->footer_enabled;
		} else if (isOn(cfg_new->footer_enabled)){
			cfg->footertype = cfg_new->footertype;
			cfg->footer = cfg_new->footer;
			cfg->footer_enabled = cfg_new->footer_enabled;
		} else {
			cfg->footer_enabled = OFF;
		}
	}

	if(cfg_new->http_header_enabled == UNSET){
		cfg->http_header = ap_pstrdup(p, cfg_origin->http_header);
		cfg->http_header_enabled = cfg_origin->http_header_enabled;
	} else if (isOn(cfg_new->http_header_enabled)){
		cfg->http_header = ap_pstrdup(p, cfg_new->http_header);
		cfg->http_header_enabled = cfg_new->http_header_enabled;
	} else {
		cfg->http_header_enabled = OFF;
	}

	cfg->types = ap_overlay_tables(p, cfg_new->types, cfg_origin->types);
	cfg->uris_ignore = ap_overlay_tables(p, cfg_new->uris_ignore, cfg_origin->uris_ignore);
	cfg->uris_ignore_header = ap_overlay_tables(p, cfg_new->uris_ignore_header, cfg_origin->uris_ignore_header);
	cfg->uris_ignore_http_header = ap_overlay_tables(p, cfg_new->uris_ignore_http_header, cfg_origin->uris_ignore_http_header);
	cfg->uris_ignore_footer = ap_overlay_tables(p, cfg_new->uris_ignore_footer, cfg_origin->uris_ignore_footer);
	if(cfg->merge) {
		cfg->display_middle = ON;
	}

	cfg->override = ap_overlay_tables(p, cfg_new->override, cfg_origin->override);
	cfg->override_uri = ap_overlay_tables(p, cfg_new->override_uri, cfg_origin->override_uri);

	return (void *) cfg;
}

static int layout_handler(request_rec * r) {
	int status;
	int status_tmp;
	int pid = 0;
	int assbackwards;
	int x = 0; /* used for increments */
	int stack_position = 0;
	layout_conf *cfg;
	const char *content_length = NULL;
	char *filename_post = NULL;
	char *filename_header = NULL;
	char *filename_body = NULL;
	char *filename_footer = NULL;
	struct stat sbuf;
	int length = 0;
	char **current_header;
	int *current_headertype;
	char **current_footer;
	int *current_footertype;
	int socket;
	int body_fd;
	int headers_out;
	char *body;

	if (r->main) {
		return DECLINED;
	}
	/* Since I keep finding that people want me to help debug, yet 
		have their server only listing Apache. This of course came
		from the nice folks at PHP over a night of beers at a conference.
	*/
	ap_table_setn(r->headers_out, "ModLayout", "2.8");

	cfg = ap_get_module_config(r->per_dir_config, &layout_module);
	current_header = (char **) cfg->header->elts;
	current_headertype = (int *) cfg->headertype->elts;
	current_footer = (char **) cfg->footer->elts;
	current_footertype = (int *) cfg->footertype->elts;
	content_length = ap_table_get(r->headers_in, "Content-Length");
/* We only need to get the pid for a couple of conditions */
	if((isOn(cfg->async_post) && content_length)  || isOn(cfg->merge)) 
		pid = getpid();
	if(isOn(cfg->merge)) 
		socket = r->connection->client->fd;

	if(cfg->async_post > OFF && content_length) {
		filename_post = ap_psprintf(r->pool, "%s/.mod_layout.post.%d",cfg->async_cache, pid);
//		ap_rflush(r);
		/* Now lets suck up some content */
		length = (content_length ? atoi(content_length) : 0);
		if(status = read_content(r, filename_post, length) != OK) {
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r,
				"mod_layout got an error while doing async post : %d", status);
		}
		/* Let us be paranoid about content length */
		if(stat(filename_post, &sbuf)) {
			/* This would be very bad */
			status = HTTP_INTERNAL_SERVER_ERROR;
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, r, 
					"fstat blew chunks in mod_filter for file: %s", filename_post);

			return status;
		}
		content_length = ap_psprintf(r->pool, "%d", sbuf.st_size);
		if((status = get_fd_in(r, filename_post)) != OK) {
			return status;
		}
	}
	headers_out = layout_headers(r, cfg);

/********************************************************************/
	/* Now, if someone has a CGI that is printing out HTTP header
		 stuff for their entire site enter it here 
	*/
	if (cfg->request_http_header){
		if(cfg->async_post > OFF && pid) {
			r->remaining = length;
			r->read_length = 0;
			r->read_chunked = 0;
			lseek(r->connection->client->fd_in, 0, SEEK_SET);
		}
		if ((status = call_container(r, cfg->http_header, cfg, 0)) != OK){
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, r, "The following error occured while processing the Layout HTTP Header : %d", status); 
		}
	} 
	
	if ((headers_out == LAYOUT) && isOff(cfg->merge)){
		print_layout_headers(r, cfg);
	}
	ap_rflush(r);

/********************************************************************/
	/* Now we do headers if they exist */
	if (cfg->request_header) {
		if(isOn(cfg->merge)) {
			/* This turns chunking off while we do our thing */
			ap_bsetflag(r->connection->client, B_CHUNK, 0);
			filename_header = ap_psprintf(r->pool, "%s/.mod_layout.header.%d",cfg->async_cache, pid);
			if((status = get_fd_out(r, filename_header)) != OK) {
				return status;
			}
		}


		for(x = 0; x < cfg->header->nelts; x++) {
			if(isOn(cfg->comment))  // Sksfl sdlkjfslkkdjf
				ap_rputs("\n<!-- Beginning of a mod_layout header -->\n", r);

			/* Now lets see about headers */
			if (current_headertype[x] > 0) {
				ap_rputs(current_header[x], r);
			} else {
				if(cfg->async_post > OFF && pid) {
					r->remaining = length;
					r->read_length = 0;
					r->read_chunked = 0;
					lseek(r->connection->client->fd_in, 0, SEEK_SET);
				}

				if(x == 0 && (headers_out == HEADER)) {
					assbackwards = 0;
				} else {
					assbackwards = 1;
				}

				if ((status = call_container(r, current_header[x], cfg, assbackwards)) != OK){
					ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, r, "The following error occured while processing the LayoutHeader : %d", status); 
				}
			}
			if (isOn(cfg->comment)) 
				ap_rputs("\n<!-- End of a mod_layout header -->\n", r);
		}
	}
	ap_rflush(r);

/********************************************************************/
	/* Now we handle the orignal request.  */
	if(headers_out == ORIGIN) {
		assbackwards = 0;
	}else {
		assbackwards = 1;
	}

	if (cfg->display_middle != OFF) {
		if(cfg->merge == ON) {
			filename_body = ap_psprintf(r->pool, "%s/.mod_layout.body.%d",cfg->async_cache, pid);
			close(r->connection->client->fd);
			if((status = get_fd_out(r, filename_body)) != OK) {
				return status;
			}
		}
		if(isOn(cfg->async_post) && pid) {
			r->remaining = length;
			r->read_length = 0;
			r->read_chunked = 0;
			lseek(r->connection->client->fd_in, 0, SEEK_SET);
		}
		ap_table_setn(r->headers_in, "Content-Length", content_length);
		if ( (status = call_main(r, assbackwards)) != OK) {
				/* This will only return if a merge is going on and
				   the error doc turns out to be a URL.
				   This also happens when users use redirects.
				*/
				if((status = call_error(r, status, cfg)) != OK){
					if(isOn(cfg->merge)) {
						close(r->connection->client->fd);
						r->connection->client->fd = socket;
						if(filename_header) 
							unlink(filename_header);
						if(filename_body) 
							unlink(filename_body);
					}
					if(headers_out != ORIGIN)
						r->assbackwards = 0;
					return status;
				}
		}
	}
	ap_rflush(r);

/********************************************************************/
/* Now lets do the footer */
	if (cfg->request_footer) {
		if(isOn(cfg->merge)) {
			filename_footer = ap_psprintf(r->pool, "%s/.mod_layout.footer.%d",cfg->async_cache, pid);
			close(r->connection->client->fd);
			if((status = get_fd_out(r, filename_footer)) != OK) {
				return status;
			}
		}
		for(x = 0; x < cfg->footer->nelts; x++) {
			if (isOn(cfg->comment)) 
				ap_rputs("\n<!-- Beginning of a mod_layout footer -->\n", r);
			if(isOn(cfg->async_post) && pid) {
				/* This needs to be made to be safer. AKA it should
					 not just blow up like this if something goes wrong.
				*/
				if((status = get_fd_in(r, filename_post)) != OK) {
					return status;
				}
				r->remaining = length;
				r->read_length = 0;
				r->read_chunked = 0;
				lseek(r->connection->client->fd_in, 0, SEEK_SET);
			}
			if (current_footertype[x] > 0){ 
				ap_rputs(current_footer[x], r);
			}
			else {
				if(isOn(cfg->async_post) && pid) {
					r->remaining = length;
					r->read_length = 0;
					r->read_chunked = 0;
					lseek(r->connection->client->fd_in, 0, SEEK_SET);
				}
				if ((status = call_container(r, current_footer[x], cfg, 1)) != OK) { 
					ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, r, "The following error occured while processing the LayoutFooter : %d", status);
				}
			}
			if (isOn(cfg->comment)) 
				ap_rputs("\n<!-- End of a mod_layout footer -->\n", r);
		}
	}
	ap_rflush(r);

/********************************************************************/
/* Welcome to the problem, lets patch this all back up */
	if(isOn(cfg->merge)) {
		if (headers_out == LAYOUT){
			print_layout_headers(r, cfg);
		}
		length = 0;
		/* This turns chunking back on */
		if(r->chunked)
			ap_bsetflag(r->connection->client, B_CHUNK, 1);
		if ((body_fd = open(filename_body,O_RDONLY,S_IRWXU)) < 0) {
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r,
			"mod_layout couldn't open a file descriptor during merger for : %s", filename_body);

			return HTTP_INTERNAL_SERVER_ERROR;
		}

		(void)fstat(body_fd, &sbuf);
		body = (char *)mmap(NULL, sbuf.st_size, PROT_READ, MAP_SHARED, body_fd, 0);
		close(r->connection->client->fd);
		r->connection->client->fd = socket;
		x = 0;

		if(filename_header)  {
			if((length = string_search(r, body, cfg->begin_tag, 0, 0)) != -1) {
				for(x = 0; x < length; x++) {
					ap_rputc(body[x], r);
				}
				write_file(r, filename_header);
				x = length;
			} else {
				write_file(r, filename_header);
				x = 0;
			}
		}

		if(filename_footer)  {
			if((length = string_search(r, body, cfg->end_tag, x, 1)) != -1) {
				for(; x < length; x++) {
					ap_rputc(body[x], r);
				}
				write_file(r, filename_footer);
				for(; x < sbuf.st_size; x++) {
					ap_rputc(body[x], r);
				}
			} else {
				for(; x < sbuf.st_size; x++) {
					ap_rputc(body[x], r);
				}
				write_file(r, filename_footer);
			}
		} else {
				for(; x < sbuf.st_size; x++) {
					ap_rputc(body[x], r);
				}
		}

		/* Lets clean up after ourself */
		munmap(body, sbuf.st_size);
		close(body_fd);
		if(filename_header) 
			unlink(filename_header);
		if(filename_body) 
			unlink(filename_body);
		if(filename_footer) 
			unlink(filename_footer);
	}

/********************************************************************/
/* Clean up anything left over from a async post */
	if(filename_post) 
		unlink(filename_post);

	return OK;
}

static int layout_fixup(request_rec *r) {
	layout_conf *cfg = ap_get_module_config(r->per_dir_config, &layout_module);
	request_rec *subr;
	char *type = NULL;
	int length; 

	if (isOff(cfg->footer_enabled) && isOff(cfg->header_enabled) && isOff(cfg->http_header_enabled)) {
		return DECLINED;
	}
	if (r->main) {
		return DECLINED;
	} 
	/* If this is a HEAD only, we really don't need to involve ourselves. */
	if (r->header_only) {
		return DECLINED;
	}


	/* So why switch to doing this? Somewhere since 1.3.6 something
		 has changed about the way that CGI's are done. Not sure what
		 it is, but this is now needed */
	/* First, we check to see if this is SSI, mod_perl or cgi */
	if(r->handler) {
		type = ap_pstrdup(r->pool, r->handler);
	} else {
		type = ap_pstrdup(r->pool, r->content_type);
	}

	if (isOn(cfg->proxy) && r->proxyreq) {
		length = strlen(r->uri);
		/* 
		Damn! Ok, here is the problem. If the request is for something
		which is and index how do we determine its mime type? Currently
		we just assume that it is a NULL and wrap it. This is far
		from perfect and is still pretty much a shot in the dark.
		More research needed.
		*/
		if(r->uri[length - 1] == '/' ) {
			type = "text/html";
		} else {
			subr = (request_rec *) ap_sub_req_lookup_file(r->uri, r);
			type = ap_pstrdup(r->pool, subr->content_type);
		}
	}

	if (isOn(cfg->glob)){
		if (!table_find(cfg->types, type))
			return DECLINED;
	} else {
		if (!check_table(ap_table_get(cfg->types, type))) 
			return DECLINED;
	}

	/* Now lets look at the ignore logic */
	/* This is where we finally decide what is going to happen */

	if (table_find(cfg->uris_ignore, r->uri))
		return DECLINED;

	if(isOn(cfg->header_enabled)) {
		if (!table_find(cfg->uris_ignore_header, r->uri))
			cfg->request_header = ON;
	}

	if(isOn(cfg->http_header_enabled)) {
		if (!table_find(cfg->uris_ignore_http_header, r->uri))
			cfg->request_http_header = ON;
	}

	if(isOn(cfg->footer_enabled)) {
		if (!table_find(cfg->uris_ignore_footer, r->uri))
			cfg->request_footer = ON;
	}

	if (isOff(cfg->request_header)  && isOff(cfg->request_footer) && isOff(cfg->request_http_header)) {
		return DECLINED;
	}
/* Now, lets fix it if some goof has called a URL that 
   should have had a / in it */
	if(ap_is_directory(r->filename)) {
		if(r->uri[0] == '\0' || r->uri[strlen(r->uri) - 1] != '/') {
/* Now at this point we know things are not going to
   go over well, so lets just let it all die in some module
   designed to take care of this sort of thing */
			return DECLINED;
		}
	}
	r->handler = "layout";
	cfg->request_type = type;

	return DECLINED;
}

/* Dispatch list of content handlers */
static const handler_rec layout_handlers[] = {
	{"layout", layout_handler},
	{NULL}
};

static const char *add_http_header(cmd_parms * cmd, void *mconfig, char *uri) {
	layout_conf *cfg = (layout_conf *) mconfig;

	cfg->http_header = ap_pstrdup(cmd->pool, uri);
	cfg->http_header_enabled = ON;

	return NULL;
}

static const char *add_header(cmd_parms * cmd, void *mconfig, char *param) {
	layout_conf *cfg = (layout_conf *) mconfig;
	struct stat sbuf;
	char *temp;

	if(ap_ind(param, ' ') != -1) {
		*(char **) ap_push_array (cfg->header) = ap_pstrdup (cmd->pool, param);
		*(int *) ap_push_array (cfg->headertype) = 1;
		cfg->header_enabled = ON;
	} else if(stat(param, &sbuf) == 0){
		if(!(temp = add_file(cmd, cfg, param))) {
			return NULL;
		};
		*(char **) ap_push_array (cfg->header) = ap_pstrdup (cmd->pool, temp);
		*(int *) ap_push_array (cfg->headertype) = 1;
		cfg->header_enabled = ON;
	} else {
		*(char **) ap_push_array (cfg->header) = ap_pstrdup (cmd->pool, param);
		*(int *) ap_push_array (cfg->headertype) = 0;
		cfg->cache_needed = ON;
		cfg->header_enabled = ON;
	}

	return NULL;
}

static const char *add_footer(cmd_parms * cmd, void *mconfig, char *param) {
	layout_conf *cfg = (layout_conf *) mconfig;
	struct stat sbuf;
	const char *temp;

	if(ap_ind(param, ' ') != -1) {
		*(char **) ap_push_array (cfg->footer) = ap_pstrdup (cmd->pool, param);
		*(int *) ap_push_array (cfg->footertype) = 1;
		cfg->footer_enabled = ON;
	} else if(stat(param, &sbuf) == 0){
		if(!(temp = add_file(cmd, cfg, param))) {
			return NULL;
		};
		*(char **) ap_push_array (cfg->footer) = ap_pstrdup (cmd->pool, temp);
		*(int *) ap_push_array (cfg->footertype) = 1;
		cfg->footer_enabled = ON;
	} else {
		*(char **) ap_push_array (cfg->footer) = ap_pstrdup (cmd->pool, param);
		*(int *) ap_push_array (cfg->footertype) = 0;
		cfg->cache_needed = ON;
		cfg->footer_enabled = ON;
	}

	return NULL;
}


static const char *ignore_uri(cmd_parms * cmd, void *mconfig, char *uri) {
	layout_conf *cfg = (layout_conf *) mconfig;
	ap_table_setn(cfg->uris_ignore, uri, "1");

	return NULL;
}

static const char *ignore_header_uri(cmd_parms * cmd, void *mconfig, char *uri) {
	layout_conf *cfg = (layout_conf *) mconfig;
	ap_table_setn(cfg->uris_ignore_header, uri, "1");

	return NULL;
}

static const char *ignore_http_header_uri(cmd_parms * cmd, void *mconfig, char *uri) {
	layout_conf *cfg = (layout_conf *) mconfig;
	ap_table_setn(cfg->uris_ignore_http_header, uri, "1");

	return NULL;
}

static const char *ignore_footer_uri(cmd_parms * cmd, void *mconfig, char *uri) {
	layout_conf *cfg = (layout_conf *) mconfig;
	ap_table_setn(cfg->uris_ignore_footer, uri, "1");

	return NULL;
}

static const char *add_type(cmd_parms * cmd, void *mconfig, char *type) {
	layout_conf *cfg = (layout_conf *) mconfig;
	ap_table_setn(cfg->types, type, "1");

	return NULL;
}

static const char *override_add(cmd_parms * cmd, void *mconfig, char *type) {
	layout_conf *cfg = (layout_conf *) mconfig;

	ap_table_setn(cfg->override, type, "1");

	return NULL;
}

static const char *override_uri_add(cmd_parms * cmd, void *mconfig, char *type) {
	layout_conf *cfg = (layout_conf *) mconfig;

	ap_table_setn(cfg->override_uri, type, "1");

	return NULL;
}

static const char *display_middle_add(cmd_parms * cmd, void *mconfig, int flag) {
	layout_conf *cfg = (layout_conf *) mconfig;

	if(cfg->merge == ON) 
		cfg->display_middle = ON;
	else
		cfg->display_middle = flag;

	return NULL;
}

static const char *merge_add(cmd_parms * cmd, void *mconfig, int flag) {
	layout_conf *cfg = (layout_conf *) mconfig;

	cfg->merge = flag;
	cfg->display_middle = ON;

	return NULL;
}

static const char *footer_off(cmd_parms * cmd, void *mconfig) 
{
	layout_conf *cfg = (layout_conf *) mconfig;
	cfg->footer_enabled = OFF;

	return NULL;
}

static const char *header_off(cmd_parms * cmd, void *mconfig) {
	layout_conf *cfg = (layout_conf *) mconfig;
	cfg->header_enabled = OFF;

	return NULL;
}

static const char *http_header_off(cmd_parms * cmd, void *mconfig) {
	layout_conf *cfg = (layout_conf *) mconfig;
	cfg->header = NULL;
	cfg->http_header_enabled = OFF;

	return NULL;
}

static const char *remove_default_types(cmd_parms * cmd, void *mconfig, int flag) {
	layout_conf *cfg = (layout_conf *) mconfig;
	if (flag)
		return NULL;

	ap_table_setn(cfg->types, INCLUDES_MAGIC_TYPE, "0");
	ap_table_setn(cfg->types, INCLUDES_MAGIC_TYPE3, "0");
	ap_table_setn(cfg->types, "server-parsed", "0");
	ap_table_setn(cfg->types, "text/html", "0");
	ap_table_setn(cfg->types, "text/plain", "0");
	ap_table_setn(cfg->types, "perl-script", "0");
	ap_table_setn(cfg->types, "cgi-script", "0");
	ap_table_setn(cfg->types, "application/x-httpd-cgi", "0");

	return NULL;
}

static const command_rec layout_cmds[] = {
	{"LayoutHeader", add_header, NULL, OR_ALL, TAKE1, LayoutHeader},
	{"LayoutHeaderAppend", ap_set_flag_slot, (void *) XtOffsetOf(layout_conf, append_header), OR_ALL, FLAG, LayoutHeaderAppend},
	{"LayoutFooterAppend", ap_set_flag_slot, (void *) XtOffsetOf(layout_conf, append_footer), OR_ALL, FLAG, LayoutFooterAppend},
	{"LayoutFooter", add_footer, NULL, OR_ALL, TAKE1, LayoutFooter},
	{"LayoutHandler", add_type, NULL, OR_ALL, TAKE1, LayoutHandler},
	{"LayoutIgnoreURI", ignore_uri, NULL, OR_ALL, TAKE1, LayoutIgnoreURI},
	{"LayoutIgnoreHeaderURI", ignore_header_uri, NULL, OR_ALL, TAKE1,
	 LayoutIgnoreHeaderURI},
	{"LayoutIgnoreHTTPHeaderURI", ignore_http_header_uri, NULL, OR_ALL, TAKE1,
	 LayoutIgnoreHTTPHeaderURI},
	{"LayoutIgnoreFooterURI", ignore_footer_uri, NULL, OR_ALL, TAKE1,
	 LayoutIgnoreFooterURI},
	{"LayoutHandlerGlob", ap_set_flag_slot, (void *) XtOffsetOf(layout_conf, glob), 
		OR_ALL, FLAG, LayoutHandlerGlob},
	{"LayoutComment", ap_set_flag_slot, (void *) XtOffsetOf(layout_conf, comment),
	 OR_ALL, FLAG, LayoutComment},
	{"LayoutProxy", ap_set_flag_slot, (void *) XtOffsetOf(layout_conf, proxy),
	 OR_ALL, FLAG, LayoutProxy},
	{"LayoutDisplayOriginal", display_middle_add, NULL, OR_ALL, FLAG, LayoutDisplayOriginal},
	{"LayoutDefaultHandlers", remove_default_types, NULL, OR_ALL, FLAG, 
		LayoutDefaultHandlers},
	{"LayoutTimeFormat", ap_set_string_slot, (void *) XtOffsetOf(layout_conf, time_format),
	 OR_ALL, TAKE1, LayoutTimeFormat},
	{"LayoutHTTPHeader", add_http_header, NULL, OR_ALL, TAKE1, LayoutHTTPHeader},
	{"LayoutHTTPOverrideHandler", override_add, NULL, OR_ALL, TAKE1, LayoutHTTPOverrideHandler},
	{"LayoutHTTPOverrideURI", override_uri_add, NULL, OR_ALL, TAKE1, LayoutHTTPOverrideURI},
	{"LayoutFooterOff", footer_off, NULL , OR_ALL, NO_ARGS, LayoutFooterOff},
	{"LayoutHeaderOff", header_off, NULL, OR_ALL, NO_ARGS, LayoutHeaderOff},
	{"LayoutHTTPHeaderOff", http_header_off, NULL, OR_ALL, NO_ARGS, LayoutHTTPHeaderOff},
	{"LayoutPostAsync", ap_set_flag_slot, (void *) XtOffsetOf(layout_conf, async_post), OR_ALL, FLAG, LayoutPostAsync},
	{"LayoutCache", ap_set_string_slot, (void *) XtOffsetOf(layout_conf, async_cache), OR_ALL, TAKE1, LayoutCache},
	{"LayoutMerge", merge_add, NULL, OR_ALL, FLAG, LayoutMerge},
	{"LayoutMergeBeginTag", ap_set_string_slot, (void *) XtOffsetOf(layout_conf, begin_tag), OR_ALL, TAKE1, LayoutMergeBeginTag},
	{"LayoutMergeEndTag", ap_set_string_slot, (void *) XtOffsetOf(layout_conf, end_tag), OR_ALL, TAKE1, LayoutMergeEndTag},
	{"LayoutMergeErrorIgnore", ap_set_flag_slot, (void *) XtOffsetOf(layout_conf, bypasserror), OR_ALL, TAKE1, LayoutMergeErrorIgnore},
	{NULL},
};

static void layout_init(server_rec * s, pool * p) {
	/* Tell apache we're here */
	ap_add_version_component("mod_layout/2.8");
}

/* Dispatch list for API hooks */
module MODULE_VAR_EXPORT layout_module = {
	STANDARD_MODULE_STUFF,
	layout_init,							/* module initializer                  */
	create_dir_mconfig,				/* create per-dir    config structures */
	merge_dir_mconfig,  			/* merge  per-dir    config structures */
	NULL,											/* create per-server config structures */
	NULL,											/* merge  per-server config structures */
	layout_cmds,							/* table of config file commands       */
	layout_handlers,					/* [#8] MIME-typed-dispatched handlers */
	NULL,											/* [#1] URI to filename translation    */
	NULL,											/* [#4] validate user id from request  */
	NULL,											/* [#5] check if the user is ok _here_ */
	NULL,											/* [#3] check access by host address   */
	NULL,											/* [#6] determine MIME type            */
	layout_fixup,							/* [#7] pre-run fixups                 */
	NULL,											/* [#9] log a transaction              */
	NULL,       		  				/* [#2] header parser                  */
	NULL,											/* child_init                          */
	NULL,											/* child_exit                          */
	NULL											/* [#0] post read-request              */
};
