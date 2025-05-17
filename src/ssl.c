#include <gnutls/gnutls.h>
#include "prototypes.h"
#include "config.h"

static gnutls_certificate_credentials_t x509_cred = 0;
static gnutls_priority_t priority_cache = 0;

gnutls_session_t ssl_new(int s)
{
  int err;
  gnutls_session_t ses = 0;
  const char *errfunc = 0;

  gnutls_global_set_log_level(10);
  if (!x509_cred)
  {
    // one_time initialization
    if ((err = gnutls_certificate_allocate_credentials(&x509_cred)) < 0)
      errfunc = "gnutls_certificate_allocate_credentials";
    else if ((err = gnutls_certificate_set_x509_key_file(x509_cred, CERTFILE, KEYFILE,
                                                   GNUTLS_X509_FMT_PEM)) < 0)
      errfunc = "gnutls_certificate_set_x509_key_file";
    else if ((err = gnutls_priority_init(&priority_cache, NULL, NULL)) < 0)
      errfunc = "gnutls_priority_init";
    else if ((err = gnutls_certificate_set_known_dh_params(x509_cred, GNUTLS_SEC_PARAM_MEDIUM)) < 0)
      errfunc = "gnutls_certificate_set_known_dh_params";
  }

  if (errfunc);
  else if ((err = gnutls_init(&ses, GNUTLS_SERVER | GNUTLS_NONBLOCK)) < 0)
    errfunc = "gnutls_init";
  else if ((err = gnutls_priority_set(ses, priority_cache)) < 0)
    errfunc = "gnutls_priority_set";
  else if ((err = gnutls_credentials_set(ses, GNUTLS_CRD_CERTIFICATE, x509_cred)) < 0)
    errfunc = "gnutls_credentials_set";

  if (errfunc)
  {
    logit(LOG_SYS, "%s failed: %s", errfunc, gnutls_strerror(err));
    if (ses)
      gnutls_deinit(ses);
    return NULL;
  }

  gnutls_certificate_server_set_request(ses, GNUTLS_CERT_IGNORE);
  gnutls_handshake_set_timeout(ses, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
  gnutls_transport_set_int(ses, s);

  return ses;
}

int ssl_negotiate(gnutls_session_t ses)
{
  int err = gnutls_handshake(ses);

  if (!err)
    return 0;
  if (err == GNUTLS_E_AGAIN || err == GNUTLS_E_INTERRUPTED)
    return 1;
  logit(LOG_COMM, "gnutls_handshake failed: %s", gnutls_strerror(err));
  return 2;
}

void ssl_close(gnutls_session_t ses)
{
  gnutls_bye(ses, GNUTLS_SHUT_RDWR);
  gnutls_deinit(ses);
}
