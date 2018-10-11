#pragma once

#include "DaedalicAmazonAWSPlugin.h"
#include "DaeSHA256.generated.h"

/**
 * Allows hashing and generating the hash message authentication code (HMAC) using the SHA256 algorithm.
 * Loosely based on the Extended Standard Library by Low Entry:
 * https://code.lowentry.com/Applications/Plugins/UE4/ExtendedStandardLibrary
 */
USTRUCT()
struct DAEDALICAMAZONAWSPLUGIN_API FDaeSHA256
{
    GENERATED_USTRUCT_BODY()

public:
    /** Hashes the specified byte array using SHA256. */
    static TArray<uint8> Hash(const TArray<uint8>& Bytes);

    /** Calculates the HMAC SHA256 of the specified message using the passed key. */
    static TArray<uint8> HMAC(const TArray<uint8>& Message, const TArray<uint8>& Key);

private:
    const static int32 K[64];
    const static int32 BLOCK_SIZE = 64;

    TArray<int32> W;

    int32 H0 = 0x6a09e667;
    int32 H1 = 0xbb67ae85;
    int32 H2 = 0x3c6ef372;
    int32 H3 = 0xa54ff53a;
    int32 H4 = 0x510e527f;
    int32 H5 = 0x9b05688c;
    int32 H6 = 0x1f83d9ab;
    int32 H7 = 0x5be0cd19;

    int32 Count = 0;
    TArray<uint8> Buffer;

    void Initialize();
    void Update(const TArray<uint8>& B, const int32 Offset, const int32 Len);
    TArray<uint8> Digest();
    TArray<uint8> PadBuffer();
    TArray<uint8> GetResult();

    void Sha(const TArray<uint8>& In, int32 Offset);

    FORCEINLINE static int32 S(const int32 InA, const int32 InB);
};
