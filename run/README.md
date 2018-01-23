
# Run files

Server binaries should be executed in this folder for development.

For production, copy `config.info` and customize it. You should not copy the other files.


## config.info

Default configuration file.

TODO


## Development SSL files

All the keys and certificates in this folder are generated and for development purpose only.

You should never use any of these files in production.


### localhost.full.pem

Used by front server, configuration keys: `front.ssl.certificate_chain` and `front.ssl.private_key`

This file correspond to the fullchain used for external communications (web, websockets...).

In production, you should not generate and sign it yourself, you should use the file(s) provided by your SSL provider instead.

For example, if your SSL provider is Letsencrypt, you should use this file: `/etc/letsencrypt/live/<mywebsite.com>/fullchain.pem`

Note that for development purpose this file has been signed with `dev-ca.crt`.


### dev-ca.crt

Self signed Certificate Authority for the cluster. Configuration key: `cluster.ssl.certificate_authority_file`

For your own cluster, you should generate your own CA. It will be used to verify all internal connections (for example, between a front server and a game instance server).

Generation commands used for this file:

```
# Private key
openssl genrsa -passout pass:IAN-ca -out dev-ca.key 2048
# Self signed CA
openssl req -new -x509 -key dev-ca.key -out dev-ca.crt
```

The private key `dev-ca.key` will only be used for certificate generation, and should not be deployed to your servers.


### internal.full.pem

Used by all servers for internal communications, configuration keys: `cluster.ssl.certificate_chain` and `cluster.ssl.private_key`

Actual chain used for internal communications, must be signed using the cluster's CA.

Generation commands used for this file:

```
# Private key
openssl genrsa -passout pass:IAN-internal -out internal.key 2048
# Sign request
openssl req -new -key internal.key -out internal.csr
# Sign with CA
openssl x509 -req -in internal.csr -CA dev-ca.crt -CAkey dev-ca.key -CAcreateserial -out internal.crt
# Create bundle
cat internal.key internal.crt dev-ca.crt > internal.full.pem
```
