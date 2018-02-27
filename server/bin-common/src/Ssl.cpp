/*
 * This file is part of the IAN project - https://github.com/Meoo/IAN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bin-common/Ssl.hpp>

#include <bin-common/config/Config.hpp>

namespace ssl = boost::asio::ssl;


namespace
{

const char default_cert[] = "cert.pem";
// Mozilla modern (as of 12/11/2017) https://wiki.mozilla.org/Security/Server_Side_TLS
const char default_cipher_list[] =
    "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-"
    "RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-"
    "AES256-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256";

} // namespace


void init_ssl_context(spdlog::logger * logger, const ConfigGroup & config,
                      ssl::context & ssl_context)
{
  // Config
  std::string ca         = config.get_string_value("certificate_authority_file");
  std::string certChain  = config.get_string_value("certificate_chain", ::default_cert);
  std::string privKey    = config.get_string_value("private_key", ::default_cert);
  std::string dh         = config.get_string_value("dh");
  std::string password   = config.get_string_value("private_key_password");
  std::string cipherList = config.get_string_value("cipher_list", ::default_cipher_list);

  // SSL setup
  try
  {
    ssl_context.set_options(ssl::context::default_workarounds | ssl::context::no_sslv2 |
                            ssl::context::no_sslv3 | ssl::context::no_tlsv1 |
                            ssl::context::no_tlsv1_1 | ssl::context::single_dh_use);

    ssl_context.set_password_callback(
        std::bind([password] { return password; })); // Use bind to ignore args
    ssl_context.use_certificate_chain_file(certChain);
    ssl_context.use_private_key_file(privKey, ssl::context::pem);

    boost::system::error_code ec;

    if (!dh.empty())
    {
      ssl_context.use_tmp_dh_file(dh, ec);
      if (ec)
        IAN_ERROR(logger, "Failed to set DH file: {}", ec.message());
    }

    if (!ca.empty())
    {
      ssl_context.load_verify_file(ca, ec);
      if (ec)
        IAN_ERROR(logger, "Failed to set certificate authority: {}", ec.message());
    }

    if (SSL_CTX_set_dh_auto(ssl_context.native_handle(), 1) != 1)
    {
      IAN_WARN(logger, "Failed to initialize SSL dh auto");
    }

    if (SSL_CTX_set_ecdh_auto(ssl_context.native_handle(), 1) != 1)
    {
      IAN_WARN(logger, "Failed to initialize SSL ecdh auto");
    }

    if (SSL_CTX_set_cipher_list(ssl_context.native_handle(), cipherList.c_str()) != 1)
    {
      IAN_WARN(logger, "Failed to initialize SSL cipher list");
    }
  }
  catch (std::exception & ex)
  {
    // TODO
    IAN_ERROR(logger, "Failed to initialize SSL context: {}", ex.what());
  }
}
