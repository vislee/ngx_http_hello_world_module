
/*
 * Copyright (C) liwq
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_str_t                hello_by;
} ngx_http_hello_world_loc_conf_t;


static void *ngx_http_hello_world_create_conf(ngx_conf_t *cf);
static char *ngx_http_hello_world_merge_conf(ngx_conf_t *cf, void *parent, void *child);
static char *ngx_http_hello_world(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_hello_world_handler(ngx_http_request_t *r);

static ngx_command_t  ngx_http_hello_world_commands[] = {

    { ngx_string("hello_world"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_http_hello_world,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("hello_by"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_hello_world_loc_conf_t, hello_by),
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_hello_world_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_hello_world_create_conf,      /* create location configuration */
    ngx_http_hello_world_merge_conf        /* merge location configuration */
};


ngx_module_t  ngx_http_hello_world_module = {
    NGX_MODULE_V1,
    &ngx_http_hello_world_module_ctx,        /* module context */
    ngx_http_hello_world_commands,           /* module directives */
    NGX_HTTP_MODULE,                         /* module type */
    NULL,                                    /* init master */
    NULL,                                    /* init module */
    NULL,                                    /* init process */
    NULL,                                    /* init thread */
    NULL,                                    /* exit thread */
    NULL,                                    /* exit process */
    NULL,                                    /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_http_hello_world_create_conf(ngx_conf_t *cf)
{
    ngx_http_hello_world_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hello_world_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /* conf->hello_by = { 0, NULL }; */

    return conf;
}


static char *
ngx_http_hello_world_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_hello_world_loc_conf_t *prev = parent;
    ngx_http_hello_world_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->hello_by,
                             prev->hello_by, "by liwq!");

    return NGX_CONF_OK;
}


static char *
ngx_http_hello_world(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t    *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    if (clcf == NULL) {
        return NGX_CONF_ERROR;
    }

    clcf->handler = ngx_http_hello_world_handler;

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_hello_world_handler(ngx_http_request_t *r)
{
    ngx_int_t                           rc;
    ngx_buf_t                          *b;
    ngx_chain_t                         out;
    ngx_http_hello_world_loc_conf_t    *hlcf;

    ngx_str_t    type = ngx_string("text/plain");
    ngx_str_t    resp = ngx_string("hello world");


    if (!(r->method & NGX_HTTP_GET)) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    hlcf = ngx_http_get_module_loc_conf(r, ngx_http_hello_world_module);
    if (hlcf == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = resp.len + hlcf->hello_by.len + 1;
    r->headers_out.content_type = type;

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    b = ngx_create_temp_buf(r->pool, resp.len + hlcf->hello_by.len + 1);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    b->last = ngx_cpymem(b->last, resp.data, resp.len);
    *b->last++ = ' ';
    b->last = ngx_cpymem(b->last, hlcf->hello_by.data, hlcf->hello_by.len);
    b->last_buf = 1;

    out.buf = b;
    out.next = NULL;

    return ngx_http_output_filter(r, &out);
}
