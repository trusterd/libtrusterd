#include <stdio.h>
#include <assert.h>

#ifdef __APPLE__
#include <unistd.h>
#include <libproc.h>
#endif

/* begin trusterd header */
#include <netdb.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <nghttp2/nghttp2.h>
#include "mrb_http2_gzip.h"


/* end trasterd header */

#include "mruby.h"
#include "mruby/proc.h"
#include "mruby/compile.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/object.h"
/* begin trasuterd code */
#define TRACER
enum {
  IO_NONE,
  WANT_READ,
  WANT_WRITE
};

struct mrb_http2_conn_t {
  SSL *ssl;
  nghttp2_session *session;
  int want_io;
  mrb_state *mrb;
  mrb_value response;
  mrb_value cb_block_hash;
};

struct mrb_http2_request_t {
  char *host;
  uint16_t port;
  char *path;
  char *hostport;
  int32_t stream_id;
  nghttp2_gzip *inflater;
};

char *strcopy(const char *s, size_t len)
{
  char *dst;
  dst = malloc(len+1);
  memcpy(dst, s, len);
  dst[len] = '\0';
  return dst;
}

static ssize_t recv_callback(nghttp2_session *session, uint8_t *buf,
    size_t length, int flags, void *user_data)
{
  struct mrb_http2_conn_t *conn;
  ssize_t rv;
  conn = (struct mrb_http2_conn_t*)user_data;
  conn->want_io = IO_NONE;
  ERR_clear_error();
  rv = SSL_read(conn->ssl, buf, length);
  if(rv < 0) {
    int err = SSL_get_error(conn->ssl, rv);
    if(err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
      conn->want_io = (err == SSL_ERROR_WANT_READ ?
                             WANT_READ : WANT_WRITE);
      rv = NGHTTP2_ERR_WOULDBLOCK;
    } else {
      rv = NGHTTP2_ERR_CALLBACK_FAILURE;
    }
  } else if(rv == 0) {
    rv = NGHTTP2_ERR_EOF;
  }
  if (!mrb_nil_p(conn->cb_block_hash)) {
    mrb_value cb_block = mrb_hash_get(conn->mrb, conn->cb_block_hash,
        mrb_str_new_cstr(conn->mrb, "recv_callback"));
    if (!mrb_nil_p(cb_block)) {
      mrb_yield_argv(conn->mrb, cb_block, 0, NULL);
    }
  }
  return rv;
}

static int before_frame_send_callback(nghttp2_session *session,
    const nghttp2_frame *frame, void *user_data)
{
  struct mrb_http2_conn_t *conn = (struct mrb_http2_conn_t*)user_data;

  if (!mrb_nil_p(conn->cb_block_hash)) {
    mrb_value cb_block = mrb_hash_get(conn->mrb, conn->cb_block_hash,
        mrb_str_new_cstr(conn->mrb, "before_frame_send_callback"));
    if (!mrb_nil_p(cb_block)) {
      mrb_yield_argv(conn->mrb, cb_block, 0, NULL);
    }
  }
  return 0;
}

static int on_frame_send_callback(nghttp2_session *session,
    const nghttp2_frame *frame, void *user_data)
{
  struct mrb_http2_conn_t *conn;
  mrb_value req_headers;
  size_t i;
  conn = (struct mrb_http2_conn_t*)user_data;
  switch(frame->hd.type) {
  case NGHTTP2_HEADERS:
    if(nghttp2_session_get_stream_user_data(session, frame->hd.stream_id)) {
      const nghttp2_nv *nva = frame->headers.nva;
      req_headers = mrb_hash_new(conn->mrb);
      for(i = 0; i < frame->headers.nvlen; ++i) {
        mrb_hash_set(conn->mrb, req_headers,
            mrb_str_new(conn->mrb, (char *)nva[i].name, nva[i].namelen),
            mrb_str_new(conn->mrb, (char *)nva[i].value, nva[i].valuelen));
      }
      mrb_hash_set(conn->mrb, conn->response,
          mrb_symbol_value(mrb_intern_cstr(conn->mrb, "request_headers")),
          req_headers);
    }
    break;
  case NGHTTP2_RST_STREAM:
    mrb_hash_set(conn->mrb, conn->response, mrb_symbol_value(mrb_intern_cstr
          (conn->mrb, "frame_send_header_rst_stream")), mrb_true_value());
    break;
  case NGHTTP2_GOAWAY:
    mrb_hash_set(conn->mrb, conn->response,
        mrb_symbol_value(mrb_intern_cstr(conn->mrb, "frame_send_header_goway")),
        mrb_true_value());
    break;
  }
  if (!mrb_nil_p(conn->cb_block_hash)) {
    mrb_value cb_block = mrb_hash_get(conn->mrb, conn->cb_block_hash,
        mrb_str_new_cstr(conn->mrb, "on_frame_send_callback"));
    if (!mrb_nil_p(cb_block)) {
      mrb_yield_argv(conn->mrb, cb_block, 0, NULL);
    }
  }
  return 0;
}

static int on_frame_recv_callback(nghttp2_session *session,
    const nghttp2_frame *frame, void *user_data)
{
  struct mrb_http2_conn_t *conn;
  conn = (struct mrb_http2_conn_t*)user_data;

  switch(frame->hd.type) {
  case NGHTTP2_HEADERS:
    if(frame->headers.cat != NGHTTP2_HCAT_RESPONSE &&
       frame->headers.cat != NGHTTP2_HCAT_PUSH_RESPONSE) {
      break;
    }
    TRACER;
    break;
  case NGHTTP2_RST_STREAM:
    TRACER;
    mrb_hash_set(conn->mrb, conn->response,
        mrb_symbol_value(mrb_intern_cstr
          (conn->mrb, "frame_recv_header_rst_stream")), mrb_true_value());
    break;
  case NGHTTP2_GOAWAY:
    TRACER;
    mrb_hash_set(conn->mrb, conn->response,
        mrb_symbol_value(mrb_intern_cstr(conn->mrb, "frame_recv_header_goway")),
        mrb_true_value());
    break;
  }
  TRACER;
  if (!mrb_nil_p(conn->cb_block_hash)) {
    mrb_value cb_block = mrb_hash_get(conn->mrb, conn->cb_block_hash,
        mrb_str_new_lit(conn->mrb, "on_frame_recv_callback"));
    if (!mrb_nil_p(cb_block)) {
      mrb_yield_argv(conn->mrb, cb_block, 0, NULL);
    }
  }
  return 0;
}

static int on_header_callback(nghttp2_session *session,
    const nghttp2_frame *frame, const uint8_t *name, size_t namelen,
    const uint8_t *value, size_t valuelen, uint8_t flags, void *user_data)

{
  struct mrb_http2_conn_t *conn = (struct mrb_http2_conn_t*)user_data;
  mrb_value response_headers;
  //size_t i;
  mrb_state *mrb = conn->mrb;

  switch(frame->hd.type) {
  case NGHTTP2_HEADERS:
    if(frame->headers.cat != NGHTTP2_HCAT_RESPONSE &&
       frame->headers.cat != NGHTTP2_HCAT_PUSH_RESPONSE) {
      break;
    }
    TRACER;
    if(nghttp2_session_get_stream_user_data(session, frame->hd.stream_id)) {
      mrb_value v = mrb_hash_get(mrb, conn->response,
          mrb_symbol_value(mrb_intern_cstr(conn->mrb, "response_headers")));
      if (mrb_nil_p(v)) {
        response_headers = mrb_hash_new(mrb);
      } else {
        response_headers =  v;
      }
      //const nghttp2_nv *nva = frame->headers.nva;
      mrb_hash_set(mrb, response_headers, mrb_str_new(mrb, (char *)name,
            namelen), mrb_str_new(mrb, (char *)value, valuelen));
      mrb_hash_set(mrb, conn->response,
          mrb_symbol_value(mrb_intern_cstr(conn->mrb, "response_headers")),
          response_headers);
    }
    break;
  case NGHTTP2_GOAWAY:
    mrb_hash_set(conn->mrb, conn->response,
        mrb_symbol_value(mrb_intern_cstr(conn->mrb, "on_header_goway")),
        mrb_true_value());
    break;
  }
  if (!mrb_nil_p(conn->cb_block_hash)) {
    mrb_value cb_block = mrb_hash_get(conn->mrb, conn->cb_block_hash,
        mrb_str_new_cstr(conn->mrb, "on_header_callback"));
    if (!mrb_nil_p(cb_block)) {
      mrb_yield_argv(conn->mrb, cb_block, 0, NULL);
    }
  }
  return 0;
}

static int on_stream_close_callback(nghttp2_session *session,
    int32_t stream_id, nghttp2_error_code error_code, void *user_data)
{
  struct mrb_http2_conn_t *conn;
  struct mrb_http2_request_t *req;
  mrb_state *mrb;

  conn = (struct mrb_http2_conn_t*)user_data;
  mrb = conn->mrb;
  req = nghttp2_session_get_stream_user_data(session, stream_id);
  if(req) {
    int rv;
    rv = nghttp2_session_terminate_session(session, NGHTTP2_NO_ERROR);
    if(rv != 0) {
      mrb_raisef(mrb, E_RUNTIME_ERROR, "nghttp2_session_terminate_session: %S",
          mrb_fixnum_value(rv));
    }
  }
  if (!mrb_nil_p(conn->cb_block_hash)) {
    mrb_value cb_block = mrb_hash_get(conn->mrb, conn->cb_block_hash,
        mrb_str_new_cstr(conn->mrb, "on_stream_close_callback"));
    if (!mrb_nil_p(cb_block)) {
      mrb_yield_argv(conn->mrb, cb_block, 0, NULL);
    }
  }
  return 0;
}

#define MAX_OUTLEN 4096

static int on_data_chunk_recv_callback(nghttp2_session *session, uint8_t flags,
    int32_t stream_id, const uint8_t *data, size_t len, void *user_data)
{
  struct mrb_http2_conn_t *conn;
  struct mrb_http2_request_t *req;
  char *body;
  conn = (struct mrb_http2_conn_t*)user_data;
  req = nghttp2_session_get_stream_user_data(session, stream_id);
  if(req) {
    mrb_value body_len;
    mrb_value body_data;

    mrb_hash_set(conn->mrb, conn->response,
        mrb_symbol_value(mrb_intern_cstr(conn->mrb, "recieve_bytes")),
        mrb_float_value(conn->mrb, (float)len));
    body = NULL;
    if(req->inflater) {
      while(len > 0) {
        uint8_t out[MAX_OUTLEN];
        size_t outlen = MAX_OUTLEN;
        size_t tlen = len;
        int rv;
        char *merge_body;
        rv = nghttp2_gzip_inflate(req->inflater, out, &outlen, data, &tlen);
        if(rv == -1) {
          nghttp2_submit_rst_stream(session, NGHTTP2_FLAG_NONE, stream_id,
              NGHTTP2_INTERNAL_ERROR);
          break;
        }
        merge_body = strcopy((const char *)out, outlen);
        if (body == NULL) {
          body = merge_body;
        }
        else {
          strcat(body, merge_body);
        }
        data += tlen;
        len -= tlen;
      }
    } else {
      body = strcopy((char *)data, len);
    }
    body_data = mrb_hash_get(conn->mrb, conn->response,
        mrb_symbol_value(mrb_intern_cstr(conn->mrb, "body")));
    if (!mrb_nil_p(body_data)) {
      mrb_str_concat(conn->mrb, body_data,
          mrb_str_new_cstr(conn->mrb, (char *)body));
    }
    else {
      body_data = mrb_str_new_cstr(conn->mrb, (char *)body);
    }
    body_len = mrb_fixnum_value(strlen(body));
    mrb_hash_set(conn->mrb, conn->response,
        mrb_symbol_value(mrb_intern_cstr(conn->mrb, "body")), body_data);
    mrb_hash_set(conn->mrb, conn->response,
        mrb_symbol_value(mrb_intern_cstr(conn->mrb, "body_length")), body_len);
  }
  if (!mrb_nil_p(conn->cb_block_hash)) {
    mrb_value cb_block = mrb_hash_get(conn->mrb, conn->cb_block_hash,
        mrb_str_new_cstr(conn->mrb, "on_data_chunk_recv_callback"));
    if (!mrb_nil_p(cb_block)) {
      mrb_yield_argv(conn->mrb, cb_block, 0, NULL);
    }
  }
  return 0;
}

static ssize_t send_callback(nghttp2_session *session,
    const uint8_t *data, size_t length, int flags, void *user_data)
{
  struct mrb_http2_conn_t *conn;
  ssize_t rv;
  conn = (struct mrb_http2_conn_t*)user_data;
  conn->want_io = IO_NONE;
  ERR_clear_error();
  rv = SSL_write(conn->ssl, data, length);
  if(rv < 0) {
    int err = SSL_get_error(conn->ssl, rv);
    if(err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
      conn->want_io = (err == SSL_ERROR_WANT_READ ?
                             WANT_READ : WANT_WRITE);
      rv = NGHTTP2_ERR_WOULDBLOCK;
    } else {
      rv = NGHTTP2_ERR_CALLBACK_FAILURE;
    }
  }
  if (!mrb_nil_p(conn->cb_block_hash)) {
    mrb_value cb_block = mrb_hash_get(conn->mrb, conn->cb_block_hash,
        mrb_str_new_cstr(conn->mrb, "send_callback"));
    if (!mrb_nil_p(cb_block)) {
      mrb_yield_argv(conn->mrb, cb_block, 0, NULL);
    }
  }
  return rv;
}

static void mrb_http2_setup_nghttp2_callbacks(mrb_state *mrb,
    nghttp2_session_callbacks *callbacks)
{
  nghttp2_session_callbacks_set_send_callback(callbacks, send_callback);
  nghttp2_session_callbacks_set_recv_callback(callbacks, recv_callback);
  nghttp2_session_callbacks_set_before_frame_send_callback(callbacks,
      before_frame_send_callback);
  nghttp2_session_callbacks_set_on_frame_send_callback(callbacks,
      on_frame_send_callback);
  nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks,
      on_frame_recv_callback);
  nghttp2_session_callbacks_set_on_stream_close_callback(callbacks,
      on_stream_close_callback);
  nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks,
      on_data_chunk_recv_callback);
  nghttp2_session_callbacks_set_on_header_callback(callbacks,
      on_header_callback);
}

struct mrb_http2_uri_t {
  const char *host;
  size_t hostlen;
  uint16_t port;
  const char *path;
  size_t pathlen;
  const char *hostport;
  size_t hostportlen;
};



static int select_next_proto_cb(SSL* ssl, unsigned char **out,
    unsigned char *outlen, const unsigned char *in, unsigned int inlen,
    void *arg)
{
  int rv;
  rv = nghttp2_select_next_protocol(out, outlen, in, inlen);
  if(rv <= 0) {
    fprintf(stderr, "FATAL: %s\n", "Server did not advertise HTTP/2 protocol");
    exit(EXIT_FAILURE);
  }
  return SSL_TLSEXT_ERR_OK;
}

static void mrb_http2_init_ssl_ctx(mrb_state *mrb, SSL_CTX *ssl_ctx)
{
  SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL|SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3
      | SSL_OP_NO_COMPRESSION | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);
  SSL_CTX_set_mode(ssl_ctx, SSL_MODE_AUTO_RETRY);
  SSL_CTX_set_mode(ssl_ctx, SSL_MODE_RELEASE_BUFFERS);
  SSL_CTX_set_next_proto_select_cb(ssl_ctx, select_next_proto_cb, NULL);
}

static void mrb_http2_request_init(mrb_state *mrb,
    struct mrb_http2_request_t *req, const struct mrb_http2_uri_t *uri)
{
  req->host = strcopy(uri->host, uri->hostlen);
  req->port = uri->port;
  req->path = strcopy(uri->path, uri->pathlen);
  req->hostport = strcopy(uri->hostport, uri->hostportlen);
  req->stream_id = -1;
  req->inflater = NULL;
}

static void mrb_http2_ssl_handshake(mrb_state *mrb, SSL *ssl, int fd)
{
  int rv;
  if(SSL_set_fd(ssl, fd) == 0) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "SSL_set_fd: %S",
        mrb_str_new_cstr(mrb, ERR_error_string(ERR_get_error(), NULL)));
  }
  ERR_clear_error();
  rv = SSL_connect(ssl);
  if(rv <= 0) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "SSL_connect: %S",
        mrb_str_new_cstr(mrb, ERR_error_string(ERR_get_error(), NULL)));
  }
}

static int mrb_http2_connect_to(mrb_state *mrb, const char *host,
    uint16_t port)
{
  struct addrinfo hints;
  int fd = -1;
  int rv;
  char service[NI_MAXSERV];
  struct addrinfo *res, *rp;
  snprintf(service, sizeof(service), "%u", port);
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  rv = getaddrinfo(host, service, &hints, &res);
  if(rv != 0) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "getaddrinfo: %S",
        mrb_str_new_cstr(mrb, gai_strerror(rv)));
  }
  for(rp = res; rp; rp = rp->ai_next) {
    fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(fd == -1) {
      continue;
    }
    while((rv = connect(fd, rp->ai_addr, rp->ai_addrlen)) == -1 &&
          errno == EINTR);
    if(rv == 0) {
      break;
    }
    close(fd);
    fd = -1;
  }
  freeaddrinfo(res);
  return fd;
}

static void mrb_http2_make_non_block(mrb_state *mrb, int fd)
{
  int flags, rv;
  while((flags = fcntl(fd, F_GETFL, 0)) == -1 && errno == EINTR);
  if(flags == -1) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "fcntl: %S",
        mrb_str_new_cstr(mrb, strerror(errno)));
  }
  while((rv = fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1 && errno == EINTR);
  if(rv == -1) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "fcntl: %S",
        mrb_str_new_cstr(mrb, strerror(errno)));
  }
}

static void mrb_http2_set_tcp_nodelay(mrb_state *mrb, int fd)
{
  int val = 1;
  int rv;
  rv = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, (socklen_t)sizeof(val));
  if(rv == -1) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "setsockopt: %S",
        mrb_str_new_cstr(mrb, strerror(errno)));
  }
}

static void mrb_http2_request_free(mrb_state *mrb,
    struct mrb_http2_request_t *req)
{
  free(req->host);
  free(req->path);
  free(req->hostport);
  nghttp2_gzip_inflate_del(req->inflater);
}

static mrb_value mrb_http2_fetch_uri(mrb_state *mrb,
    const struct mrb_http2_uri_t *uri)
{
  nghttp2_session_callbacks *callbacks;
  int fd;
  SSL_CTX *ssl_ctx;
  SSL *ssl;
  struct mrb_http2_request_t req;
  struct mrb_http2_conn_t conn;
  int rv;
  nfds_t npollfds = 1;
  struct pollfd pollfds[1];
  mrb_http2_request_init(mrb, &req, uri);


  fd = mrb_http2_connect_to(mrb, req.host, req.port);
  if(fd == -1) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "Could not open file descriptor");
  }
  ssl_ctx = SSL_CTX_new(SSLv23_client_method());
  if(ssl_ctx == NULL) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "SSL_CTX_new: %S",
        mrb_str_new_cstr(mrb, ERR_error_string(ERR_get_error(), NULL)));
  }
  mrb_http2_init_ssl_ctx(mrb, ssl_ctx);
  ssl = SSL_new(ssl_ctx);
  if(ssl == NULL) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "SSL_new: %S",
        mrb_str_new_cstr(mrb, ERR_error_string(ERR_get_error(), NULL)));
  }
  mrb_http2_ssl_handshake(mrb, ssl, fd);

  conn.ssl = ssl;
  conn.want_io = IO_NONE;

  SSL_write(ssl, NGHTTP2_CLIENT_CONNECTION_PREFACE,
      NGHTTP2_CLIENT_CONNECTION_PREFACE_LEN);

  mrb_http2_make_non_block(mrb, fd);
  mrb_http2_set_tcp_nodelay(mrb, fd);
  conn.mrb = mrb;
  conn.response = mrb_hash_new(mrb);
  conn.cb_block_hash = mrb_nil_value();

  rv = nghttp2_session_callbacks_new(&callbacks);
  if(rv != 0) {
      mrb_raisef(mrb, E_RUNTIME_ERROR, "nghttp2_session_client_new: %S",
          mrb_fixnum_value(rv));
  }

  mrb_http2_setup_nghttp2_callbacks(mrb, callbacks);
  rv = nghttp2_session_client_new(&conn.session, callbacks, &conn);
  nghttp2_session_callbacks_del(callbacks);
  if(rv != 0) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "nghttp2_session_client_new: %S",
        mrb_fixnum_value(rv));
  }

  nghttp2_submit_settings(conn.session, NGHTTP2_FLAG_NONE, NULL, 0);

  mrb_http2_submit_request(mrb, &conn, &req);

  pollfds[0].fd = fd;
  mrb_http2_ctl_poll(mrb, pollfds, &conn);

  while(nghttp2_session_want_read(conn.session)
      || nghttp2_session_want_write(conn.session)) {
    int nfds = poll(pollfds, npollfds, -1);
    if(nfds == -1) {
      mrb_raisef(mrb, E_RUNTIME_ERROR, "poll: %S", mrb_str_new_cstr(mrb,
            strerror(errno)));
    }
    if(pollfds[0].revents & (POLLIN | POLLOUT)) {
      mrb_http2_exec_io(mrb, &conn);
    }
    if((pollfds[0].revents & POLLHUP) || (pollfds[0].revents & POLLERR)) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "connection error");
    }
    mrb_http2_ctl_poll(mrb, pollfds, &conn);
  }

  nghttp2_session_del(conn.session);
  SSL_set_shutdown(ssl, SSL_RECEIVED_SHUTDOWN);
  ERR_clear_error();
  SSL_shutdown(ssl);
  SSL_free(ssl);
  SSL_CTX_free(ssl_ctx);
  shutdown(fd, SHUT_WR);
  close(fd);
  mrb_http2_request_free(mrb, &req);

  return conn.response;
}

static mrb_value mrb_http2_client_get2(mrb_state *mrb, mrb_value self)
{
  char *uri, *agentname;
  struct mrb_http2_uri_t uri_data;
  struct sigaction act;
  int rv;

  mrb_get_args(mrb, "zz", &uri,&agentname);
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &act, 0);

  SSL_load_error_strings();
  SSL_library_init();

  rv = parse_uri(&uri_data, uri);
  if(rv != 0) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "parse_uri failed");
  }
  return mrb_http2_fetch_uri(mrb, &uri_data);
}
/**/

typedef int (*FUNCPTR)(char *script);
FUNCPTR gcb;

void setCallback(FUNCPTR cb)
{
	gcb = cb;
}

FUNCPTR getCallback()
{
	return gcb;
}

#ifdef __APPLE__
mrb_value get_procpathname(mrb_state* mrb, mrb_value self)
{
	int ret;
	pid_t pid;
	char pathbuf[PROC_PIDPATHINFO_MAXSIZE];

	pid = getpid();
	ret = proc_pidpath(pid, pathbuf, sizeof(pathbuf));

	return mrb_str_new(mrb,pathbuf,strlen(pathbuf));
}
#endif

mrb_value exec(mrb_state* mrb, mrb_value self)
{
	char *script;

	printf("class:Call, method:py_exec\n");
	// 第一引数を引数にコールバック関数を実行する。
	mrb_get_args(mrb, "z", &script);

	(*getCallback())(script);
	return self;
}

void mrbAddMyCallBack(mrb_state* mrb, FUNCPTR cb)
{
	setCallback(cb);

	struct RClass *hoge_module;
	hoge_module = mrb_define_module(mrb, "MyCall");
	// クラスを定義する

	// クラスメソッドを定義する
	mrb_define_class_method(mrb, hoge_module, "my_exec", exec, ARGS_REQ(1));
	#ifdef __APPLE__
	mrb_define_class_method(mrb, hoge_module, "procpathname", get_procpathname, ARGS_NONE());
	#endif
}

void mrbAddSetUserAgent(mrb_state *mrb) {

	struct RClass *tr_module, *tr_client_cls;
	// get HTTP2 module
	tr_module = mrb_module_get(mrb,"HTTP2");
	tr_client_cls = mrb_class_get_under(mrb,tr_module,"Client");
	mrb_define_class_method(mrb, tr_client_cls, "http2_get2", mrb_http2_client_get2, MRB_ARGS_REQ(2));
}

int boot(char *name, FUNCPTR cb)
{
	assert(name != NULL);
	mrb_state* mrb = mrb_open();

	// のようにmrubyから呼び出し元の言語でコードを渡すことで、
	// 呼び出し元でコードを評価してもらう。
	mrbAddMyCallBack(mrb, cb);
	mrbAddSetUserAgent(mrb);
	mrb_load_string(mrb, name);
	mrb_close(mrb);
	return printf("%s", name);
}
