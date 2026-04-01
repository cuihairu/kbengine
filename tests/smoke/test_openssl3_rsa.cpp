#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <chrono>

#include "common/common.h"
#include "common/rsa.h"
#include "common/kbekey.h"
#include <openssl/opensslv.h>

// Platform-specific getpid wrapper
#ifdef _WIN32
	#include <process.h>
	#ifndef getpid
		#define getpid() _getpid()
	#endif
#else
	#include <unistd.h>
#endif

// Define KBEngine global variables for testing
namespace KBEngine
{
	COMPONENT_TYPE g_componentType = UNKNOWN_COMPONENT_TYPE;
	COMPONENT_ID g_componentID = 0;
	GAME_TIME g_kbetime = 0;
}

/**
 * OpenSSL 3.x Compatibility Test Suite
 * Tests RSA functionality with both OpenSSL 1.x and 3.x APIs
 */
class OpenSSLRSACompatTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		// Create temporary directory for test keys
		temp_dir_ = std::filesystem::temp_directory_path();
		test_id_ = std::to_string(getpid()) + "_openssl3_compat_";

		public_key_ = temp_dir_ / (test_id_ + "public.pem");
		private_key_ = temp_dir_ / (test_id_ + "private.pem");

		// Clean up any existing test files
		std::filesystem::remove(public_key_);
		std::filesystem::remove(private_key_);
	}

	void TearDown() override
	{
		// Clean up test files
		std::filesystem::remove(public_key_);
		std::filesystem::remove(private_key_);
	}

	std::filesystem::path temp_dir_;
	std::filesystem::path public_key_;
	std::filesystem::path private_key_;
	std::string test_id_;
};

/**
 * Test RSA key generation with OpenSSL 3.x
 */
TEST_F(OpenSSLRSACompatTest, GeneratesKeysWithOpenSSL3)
{
	KBEngine::KBE_RSA rsa;
	ASSERT_TRUE(rsa.generateKey(public_key_.string(), private_key_.string(), 2048, 65537))
		<< "RSA key generation should succeed with OpenSSL 3.x";

	ASSERT_TRUE(rsa.isGood())
		<< "Generated RSA keys should be valid";

	// Verify key files exist
	EXPECT_TRUE(std::filesystem::exists(public_key_));
	EXPECT_TRUE(std::filesystem::exists(private_key_));
}

/**
 * Test RSA encryption/decryption round-trip
 */
TEST_F(OpenSSLRSACompatTest, EncryptDecryptRoundTrip)
{
	// Generate keys
	KBEngine::KBE_RSA rsa;
	ASSERT_TRUE(rsa.generateKey(public_key_.string(), private_key_.string(), 2048, 65537));

	// Test data
	const std::string plaintext = "OpenSSL 3.x compatibility test message!";

	std::cout << "Testing encryption with plaintext length: " << plaintext.length() << "\n";

	std::string encrypted;
	int enc_result = rsa.encrypt(plaintext, encrypted);

	std::cout << "Encryption result: " << enc_result << ", encrypted length: " << encrypted.length() << "\n";

	ASSERT_GT(enc_result, 0) << "Encryption should succeed";

	std::string decrypted;
	int dec_result = rsa.decrypt(encrypted, decrypted);

	std::cout << "Decryption result: " << dec_result << ", decrypted length: " << decrypted.length() << "\n";

	ASSERT_GT(dec_result, 0) << "Decryption should succeed";

	EXPECT_EQ(decrypted, plaintext)
		<< "Decrypted text should match original plaintext";
}

/**
 * Test RSA encryption/decryption with binary data
 */
TEST_F(OpenSSLRSACompatTest, HandlesBinaryData)
{
	KBEngine::KBE_RSA rsa;
	ASSERT_TRUE(rsa.generateKey(public_key_.string(), private_key_.string(), 2048, 65537));

	// Binary test data with various byte values
	// For 2048-bit RSA with OAEP padding, max plaintext is ~214 bytes
	std::string binary_data;
	for (int i = 0; i < 200; ++i)
	{
		binary_data += static_cast<char>(i);
	}

	std::string encrypted;
	ASSERT_GT(rsa.encrypt(binary_data, encrypted), 0)
		<< "Binary data encryption should succeed";

	std::string decrypted;
	ASSERT_GT(rsa.decrypt(encrypted, decrypted), 0)
		<< "Binary data decryption should succeed";

	EXPECT_EQ(decrypted, binary_data)
		<< "Decrypted binary data should match original";
}

/**
 * Test RSA key loading from disk
 */
TEST_F(OpenSSLRSACompatTest, LoadsKeysFromDisk)
{
	// Generate keys first
	{
		KBEngine::KBE_RSA generator;
		ASSERT_TRUE(generator.generateKey(public_key_.string(), private_key_.string(), 2048, 65537));
	}

	// Load keys in new RSA instance
	KBEngine::KBE_RSA loaded_rsa(public_key_.string(), private_key_.string());
	ASSERT_TRUE(loaded_rsa.isGood())
		<< "Loaded RSA keys should be valid";

	// Test encryption/decryption with loaded keys
	const std::string test_msg = "Keys loaded from disk test";
	std::string encrypted = loaded_rsa.encrypt(test_msg);
	ASSERT_FALSE(encrypted.empty());

	std::string decrypted = loaded_rsa.decrypt(encrypted);
	EXPECT_EQ(decrypted, test_msg);
}

/**
 * Test different key sizes
 */
TEST_F(OpenSSLRSACompatTest, SupportsDifferentKeySizes)
{
	std::vector<int> key_sizes = {1024, 2048};

	for (int key_size : key_sizes)
	{
		auto test_public = temp_dir_ / (test_id_ + "key_" + std::to_string(key_size) + "_pub.pem");
		auto test_private = temp_dir_ / (test_id_ + "key_" + std::to_string(key_size) + "_priv.pem");

		KBEngine::KBE_RSA rsa;
		ASSERT_TRUE(rsa.generateKey(test_public.string(), test_private.string(), key_size, 65537))
			<< "Should generate " << key_size << "-bit RSA key";

		// Quick functional test
		const std::string msg = "Test for " + std::to_string(key_size) + " bit key";
		std::string encrypted = rsa.encrypt(msg);
		std::string decrypted = rsa.decrypt(encrypted);
		EXPECT_EQ(decrypted, msg);

		std::filesystem::remove(test_public);
		std::filesystem::remove(test_private);
	}
}

/**
 * Performance test for OpenSSL 3.x RSA operations
 */
TEST_F(OpenSSLRSACompatTest, PerformanceBenchmark)
{
	KBEngine::KBE_RSA rsa;
	ASSERT_TRUE(rsa.generateKey(public_key_.string(), private_key_.string(), 2048, 65537));

	const std::string test_data = "Performance test data for OpenSSL 3.x RSA operations";
	const int iterations = 5;

	// Warm-up
	for (int i = 0; i < 2; ++i)
	{
		std::string enc = rsa.encrypt(test_data);
		rsa.decrypt(enc);
	}

	// Benchmark encryption
	auto encrypt_start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < iterations; ++i)
	{
		std::string enc = rsa.encrypt(test_data);
	}
	auto encrypt_end = std::chrono::high_resolution_clock::now();

	auto encrypt_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
		encrypt_end - encrypt_start).count();

	// Benchmark decryption
	std::string encrypted = rsa.encrypt(test_data);
	auto decrypt_start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < iterations; ++i)
	{
		rsa.decrypt(encrypted);
	}
	auto decrypt_end = std::chrono::high_resolution_clock::now();

	auto decrypt_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
		decrypt_end - decrypt_start).count();

	// Log performance (should be reasonably fast)
	EXPECT_LT(encrypt_duration, 5000) << "Encryption should complete in reasonable time";
	EXPECT_LT(decrypt_duration, 5000) << "Decryption should complete in reasonable time";

	std::cout << "OpenSSL 3.x RSA Performance (2048-bit, " << iterations << " iterations):\n";
	std::cout << "  Encryption: " << encrypt_duration << " ms\n";
	std::cout << "  Decryption: " << decrypt_duration << " ms\n";
}

/**
 * Edge case tests
 */
TEST_F(OpenSSLRSACompatTest, HandlesLargeData)
{
	KBEngine::KBE_RSA rsa;
	ASSERT_TRUE(rsa.generateKey(public_key_.string(), private_key_.string(), 2048, 65537));

	// Create data that's close to the maximum RSA can handle
	// For 2048-bit RSA with OAEP padding, max plaintext is ~214 bytes
	std::string large_data(200, 'X');  // 200 bytes of 'X'

	std::string encrypted;
	ASSERT_GT(rsa.encrypt(large_data, encrypted), 0)
		<< "Should encrypt data close to maximum size";

	std::string decrypted;
	ASSERT_GT(rsa.decrypt(encrypted, decrypted), 0)
		<< "Should decrypt data close to maximum size";

	EXPECT_EQ(decrypted, large_data);
}

/**
 * OpenSSL version compatibility test
 */
TEST_F(OpenSSLRSACompatTest, DetectsOpenSSLVersion)
{
	// This test verifies that our code works with the detected OpenSSL version
	std::cout << "OpenSSL Version: " << OPENSSL_VERSION_TEXT << "\n";
	std::cout << "OpenSSL Version Number: 0x" << std::hex << OPENSSL_VERSION_NUMBER << std::dec << "\n";

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	std::cout << "Using OpenSSL 3.x API\n";
#elif OPENSSL_VERSION_NUMBER >= 0x10100000L
	std::cout << "Using OpenSSL 1.1.x API\n";
#else
	std::cout << "Using OpenSSL 1.0.x API\n";
#endif

	// Regardless of version, basic RSA operations should work
	KBEngine::KBE_RSA rsa;
	ASSERT_TRUE(rsa.generateKey(public_key_.string(), private_key_.string(), 2048, 65537))
		<< "RSA key generation should work with any supported OpenSSL version";

	const std::string test_msg = "Version compatibility test";
	std::string encrypted = rsa.encrypt(test_msg);
	std::string decrypted = rsa.decrypt(encrypted);

	EXPECT_EQ(decrypted, test_msg)
		<< "RSA operations should work across OpenSSL versions";
}

/**
 * KBEKey OpenSSL 3.x compatibility test
 */
class OpenSSLKBEKeyCompatTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		temp_dir_ = std::filesystem::temp_directory_path();
		test_id_ = std::to_string(getpid()) + "_kbekey_openssl3_";

		public_key_ = temp_dir_ / (test_id_ + "public.pem");
		private_key_ = temp_dir_ / (test_id_ + "private.pem");

		std::filesystem::remove(public_key_);
		std::filesystem::remove(private_key_);

		// Set up component type for testing
		original_component_type_ = KBEngine::g_componentType;
		KBEngine::g_componentType = KBEngine::BASEAPP_TYPE;
	}

	void TearDown() override
	{
		KBEngine::g_componentType = original_component_type_;

		std::filesystem::remove(public_key_);
		std::filesystem::remove(private_key_);
	}

	std::filesystem::path temp_dir_;
	std::filesystem::path public_key_;
	std::filesystem::path private_key_;
	std::string test_id_;
	KBEngine::COMPONENT_TYPE original_component_type_;
};

TEST_F(OpenSSLKBEKeyCompatTest, ServerModeGeneratesKeys)
{
	KBEngine::KBEKey key(public_key_.string(), private_key_.string());
	ASSERT_TRUE(key.isGood())
		<< "KBEKey should generate and load keys in server mode";

	const std::string plaintext = "KBEKey OpenSSL 3.x server test";
	const std::string encrypted = key.encrypt(plaintext);
	ASSERT_FALSE(encrypted.empty());

	EXPECT_EQ(key.decrypt(encrypted), plaintext);
}

TEST_F(OpenSSLKBEKeyCompatTest, ClientModeLoadsPublicKey)
{
	// Generate keys first
	{
		KBEngine::KBE_RSA generator;
		ASSERT_TRUE(generator.generateKey(public_key_.string(), private_key_.string(), 2048, 65537));
	}

	// Client mode - only public key
	KBEngine::g_componentType = KBEngine::CLIENT_TYPE;
	KBEngine::KBEKey key(public_key_.string(), "");
	ASSERT_TRUE(key.isGood())
		<< "KBEKey in client mode should only need public key";

	// Client can only encrypt, not decrypt
	const std::string plaintext = "Client encryption test";
	const std::string encrypted = key.encrypt(plaintext);
	ASSERT_FALSE(encrypted.empty())
		<< "Client should be able to encrypt with public key";
}
