/* 
**  mod_layout.c -- Apache layout module
**  $Revision: 1.17 $
*/

#include "mod_layout.h"

LAYOUT_EXPORT(int) string_search(request_rec *r, char *string, const char *delim, int init_pos,  int flag) {
	char *temp = NULL;
	char *sub_temp = NULL;
	char *lower = NULL;
	char *substring = NULL;
	char *token = NULL;
	int position = 0;
	int end = 0;
	int length;
	int delim_size;
	int complete_position = 0;

	if(delim == NULL || string == NULL) 
		return -1;

	length = strlen(string);
	delim_size = strlen(delim);

	temp = string + init_pos;
	complete_position = init_pos;

	while((position = ap_ind(temp, delim[0])) != -1) {
		sub_temp = temp + position;
		if((end = ap_ind(sub_temp, delim[delim_size - 1])) != -1) { 
			substring = ap_pstrndup(r->pool, sub_temp , end + 1);
			lower = ap_pstrdup(r->pool, substring);
			ap_str_tolower(lower);
			if(!ap_fnmatch(delim, lower, FNM_CASE_BLIND)) {
				if(flag) {
					complete_position += position;
				} else {
					complete_position += position + end + 1;
				}
				return complete_position;
			}
		} else {
			return -1;
		}
		temp += end + 1;
		complete_position += end + 1;
	}

	return -1;
}


LAYOUT_EXPORT(int) write_file(request_rec *r, char *filename) {
	FILE *file_ptr;
	char buf[HUGE_STRING_LEN];

	if (!(file_ptr = ap_pfopen(r->pool, filename, "r"))) {
		ap_log_rerror(APLOG_MARK, APLOG_ERR, r,
			"Could not open layout file: %s", filename);
		return 1;
	}
	while (fgets(buf, HUGE_STRING_LEN, file_ptr)) {
		ap_rputs(buf, r);
	}
	ap_pfclose(r->pool, file_ptr);

	return 0;
}

LAYOUT_EXPORT(int) get_fd_in(request_rec *r, char *filename) {
	int status = OK;
	int temp_fd = 0;

	if ((temp_fd = open(filename,O_RDONLY,S_IRWXU)) < 0) {
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r,
		"mod_layout couldn't open a file descriptor for : %s", filename);

		return HTTP_INTERNAL_SERVER_ERROR;
	}

	r->connection->client->fd_in = temp_fd;

	return status;
}

LAYOUT_EXPORT(int) get_fd_out(request_rec *r, char *filename) {
	int status = OK;
	int temp_fd = 0;

	if ((temp_fd = open(filename,O_RDWR|O_CREAT|O_TRUNC,S_IRWXU)) == -1) {
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r,
		"mod_layout couldn't create a file descriptor at HTTP : %s", filename);

		return HTTP_INTERNAL_SERVER_ERROR;
	}

	r->connection->client->fd = temp_fd;

	return status;
}

/*
	 Pretty simple method. Sucks up the POST into a file specified
	 by filename. 
*/
LAYOUT_EXPORT(int) read_content(request_rec *r, char *filename, long length) {
	int rc;
	char argsbuffer[HUGE_STRING_LEN];
	int rsize, rpos=0;
	long len_read = 0;
	FILE *file;

	if (!(file = ap_pfopen(r->pool, filename, "w"))) {
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r,
				"mod_layout couldn't create a file for async : %s", filename);
	}

	if ((rc = ap_setup_client_block(r, REQUEST_CHUNKED_ERROR)) != OK){
		return rc;
	}

	if (ap_should_client_block(r)) {
		ap_hard_timeout("client_read", r);
		while((len_read = ap_get_client_block(r, argsbuffer, HUGE_STRING_LEN)) > 0 ){
			ap_reset_timeout(r);
			if ((rpos + (int)len_read) > length) {
				rsize = length -rpos;
			} else {
				rsize = (int)len_read;
			}
			(void)fwrite(argsbuffer, (size_t)rsize, 1, file);
			rpos += rsize;
		}

		ap_kill_timeout(r);
	}
	ap_pfclose(r->pool, file);

	return rc;
}

LAYOUT_EXPORT(int) check_table(const char *a) {
	if (a == NULL) 
		return 0;
	if('1' == a[0])
		return 1;

	return 0;
}

/* This method is borrowed from alloc.c in the main apache 
	 distribution. */

LAYOUT_EXPORT(int) table_find(const table * t, const char *key) {
	array_header *hdrs_arr = ap_table_elts(t);
	table_entry *elts = (table_entry *) hdrs_arr->elts;
	int i;

	if (key == NULL)
		return 0;

	for (i = 0; i < hdrs_arr->nelts; ++i) {
		if (!ap_fnmatch(elts[i].key, key, FNM_PATHNAME | FNM_CASE_BLIND))
			if(check_table(elts[i].val))
				return 1;
	}


	return 0;
}

LAYOUT_EXPORT(int) call_main(request_rec *r, int assbackwards) {
	int status = OK;
	request_rec *subr;
	subr = (request_rec *) ap_sub_req_method_uri((char *) r->method, r->uri, r);

	subr->assbackwards = assbackwards;
	subr->args = ap_pstrdup(subr->pool, r->args);
	status = ap_run_sub_req(subr);
	ap_destroy_sub_req(subr);

	return status;
}

LAYOUT_EXPORT(int) layout_headers(request_rec *r, layout_conf *cfg) {
	int x;
	int status = LAYOUT;
	int *current_headertype = (int *) cfg->headertype->elts;

	if(cfg->request_http_header)
		status =  HTTP;


	/* First, lets see if override is an issue */
	if (table_find(cfg->override, cfg->request_type) || table_find(cfg->override_uri, r->uri)){
		if(isOn(cfg->merge)) {
			status = ORIGIN;
		} else if (isOn(cfg->request_header)){
			if(current_headertype != 0) {
				status = HEADER;
			} else { 
				status = LAYOUT;
			}
		} else {
			status = ORIGIN;
		}
	}

	return status;
}

LAYOUT_EXPORT(int) call_error(request_rec *r, int status, layout_conf *cfg) {
	int tmp_status = OK;
	request_rec *subr;
	int idx = ap_index_of_response(status);
	int length = 0;
	char *response = NULL;
	core_dir_config *conf = (core_dir_config *)ap_get_module_config(r->per_dir_config, &core_module);

	if(conf->response_code_strings != NULL)
		response = conf->response_code_strings[idx];
	

	if(isOn(cfg->merge) && isOn(cfg->bypasserror)) {
		return status;
	}

	if(response) {
		if(isOn(cfg->merge)) {
			length = strlen(response);
			if (length > 7){
/* If this should happen without merge turned on
   things are going to be pretty messy.
*/
				if(!strncmp(response, "http://", 7)) {
					return status;
				}
			}
		}
		if(response[0] == '/') {
			subr = (request_rec *) ap_sub_req_method_uri("GET", response, r);
			ap_table_setn(r->headers_in, "Content-Length", "0");
			subr->status = status;
			subr->assbackwards = 1;
			subr->args = ap_pstrdup(subr->pool, r->args);
			status = ap_run_sub_req(subr);
			ap_destroy_sub_req(subr);
		} else {
			/* Ok, we are going to act like Apache here and leave on the stupid
				 end quote.
			*/
			ap_rputs(response + 1, r);
		}
	} else {
		tmp_status = r->status;
		r->status_line = ap_psprintf(r->pool, "%d", status);
		r->status = status;
		r->assbackwards = 1;
		ap_send_error_response(r, 0);
		r->status = tmp_status;
		r->assbackwards = 0;
	}

	return OK;
}

LAYOUT_EXPORT(int) call_container(request_rec *r, char *uri, layout_conf *cfg, int assbackwards) {
	int status = OK;
	request_rec *subr;

	if(isOn(cfg->async_post)) {
		subr = (request_rec *) ap_sub_req_method_uri((char *) r->method, uri, r);
	} else {
		subr = (request_rec *) ap_sub_req_lookup_uri(uri, r);
		ap_table_setn(subr->headers_in, "Content-Length", "0");
	}
	subr->assbackwards = assbackwards;

	if(!cfg->async_post) {
		ap_table_setn(subr->headers_in, "Content-Length", "0");
	}
	ap_table_setn(subr->subprocess_env, "LAYOUT_SCRIPT_NAME", r->uri);
	ap_table_setn(subr->subprocess_env, "LAYOUT_PATH_INFO", r->path_info);
	ap_table_setn(subr->subprocess_env, "LAYOUT_QUERY_STRING", r->args);
	ap_table_setn(subr->subprocess_env, "LAYOUT_FILENAME", r->filename);
	ap_table_setn(subr->subprocess_env, "LAYOUT_LAST_MODIFIED",
			ap_ht_time(r->pool, r->finfo.st_mtime, cfg->time_format, 0));

	status = ap_run_sub_req(subr);
	ap_destroy_sub_req(subr);

	return status;
}

LAYOUT_EXPORT(void) print_layout_headers(request_rec *r, layout_conf *cfg) {
	r->content_type = "text/html"; 
	ap_update_mtime(r, r->finfo.st_mtime);
	ap_set_last_modified(r);
/*	ap_set_etag(r); */
/* Until I come up with a more intelligent way of doing this,
	 everything is getting this header.
*/
	if(cfg->cache_needed == ON) {
		ap_table_setn(r->headers_out, "Cache-Control", "no-cache");
	}
	ap_send_http_header(r); 
}


LAYOUT_EXPORT(char *)add_file(cmd_parms * cmd, layout_conf *cfg, char *file) {
	FILE *file_ptr;
	char buf[HUGE_STRING_LEN];
	char *content = NULL;

	if (!(file_ptr = ap_pfopen(cmd->temp_pool, file, "r"))) {
		ap_log_error(APLOG_MARK, APLOG_ERR, cmd->server,
			"Could not open layout file: %s", file);
		return NULL;
	}
	while (fgets(buf, HUGE_STRING_LEN, file_ptr)) {
		if(content) {
			content = ap_pstrcat(cmd->temp_pool, content, buf, NULL);
		} else {
			content = ap_pstrcat(cmd->temp_pool, buf, NULL);
		}
	}
	ap_pfclose(cmd->temp_pool, file_ptr);

	return (char *)ap_pstrdup(cmd->pool, content);
}

