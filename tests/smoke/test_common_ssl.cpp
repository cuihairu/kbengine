#include <gtest/gtest.h>

#include <openssl/prov_ssl.h>

#include "common/memorystream.h"
#include "common/ssl.h"

TEST(CommonSslBootstrapTest, InitializesAndFinalizesOpenSsl) {
  EXPECT_TRUE(KBEngine::KB_SSL::initialize());
  KBEngine::KB_SSL::finalise();
}

TEST(CommonSslBootstrapTest, DetectsTls12HandshakePrefix) {
  KBEngine::MemoryStream stream;
  const unsigned char handshake[] = {
    0x16, 0x03, 0x03, 0x00, 0x2f,
    0x01, 0x00, 0x00, 0x2b,
    0x03, 0x03
  };

  stream.append(handshake, sizeof(handshake));
  stream.data_resize(47);
  stream.wpos(47);

  EXPECT_EQ(KBEngine::KB_SSL::isSSLProtocal(&stream), TLS1_2_VERSION);
}

TEST(CommonSslBootstrapTest, RejectsNonSslPayload) {
  KBEngine::MemoryStream stream;
  const unsigned char payload[] = {'G', 'E', 'T', ' ', '/', ' '};

  stream.append(payload, sizeof(payload));

  EXPECT_EQ(KBEngine::KB_SSL::isSSLProtocal(&stream), -1);
}
