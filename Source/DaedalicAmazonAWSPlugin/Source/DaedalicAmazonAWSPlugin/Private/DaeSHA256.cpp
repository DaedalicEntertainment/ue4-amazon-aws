#include "DaedalicAmazonAWSPlugin.h"
#include "DaeSHA256.h"

TArray<uint8> FDaeSHA256::Hash(const TArray<uint8>& Bytes)
{
    FDaeSHA256 Sha256 = FDaeSHA256();
    Sha256.Initialize();
    Sha256.Update(Bytes, 0, Bytes.Num());
    return Sha256.Digest();
}

TArray<uint8> FDaeSHA256::HMAC(const TArray<uint8>& Message, const TArray<uint8>& Key)
{
    int32 Length = Message.Num();
    int32 BlockSize = 64;

    TArray<uint8> KeyBytes = Key;
    if (KeyBytes.Num() > BlockSize)
    {
        KeyBytes = Hash(KeyBytes); // key = hash(key)
    }
    if (KeyBytes.Num() != BlockSize)
    {
        KeyBytes.SetNumZeroed(BlockSize); // key = key + 0x00 (till KeyBytes has BlockSize length)
    }

    TArray<uint8> ArrayO = KeyBytes;
    TArray<uint8> ArrayI = KeyBytes;
    for (int32 Index = 0; Index < KeyBytes.Num(); Index++)
    {
        ArrayO[Index] ^= 0x5c; // o_key_pad = 0x5c ^ key
        ArrayI[Index] ^= 0x36; // i_key_pad = 0x36 ^ key
    }

    if (Length > 0) // (i_key_pad + message)
    {
        ArrayI.Append(Message.GetData(), Length);
    }
    ArrayI = Hash(ArrayI);                         // hash(i_key_pad + message)
    ArrayO.Append(ArrayI.GetData(), ArrayI.Num()); // (o_key_pad + hash(i_key_pad + message))
    return Hash(ArrayO);                           // hash(o_key_pad + hash(i_key_pad + message))
}

const int32 FDaeSHA256::K[64] = {
    ((int32)0x428a2f98), ((int32)0x71374491), ((int32)0xb5c0fbcf), ((int32)0xe9b5dba5), ((int32)0x3956c25b),
    ((int32)0x59f111f1), ((int32)0x923f82a4), ((int32)0xab1c5ed5), ((int32)0xd807aa98), ((int32)0x12835b01),
    ((int32)0x243185be), ((int32)0x550c7dc3), ((int32)0x72be5d74), ((int32)0x80deb1fe), ((int32)0x9bdc06a7),
    ((int32)0xc19bf174), ((int32)0xe49b69c1), ((int32)0xefbe4786), ((int32)0x0fc19dc6), ((int32)0x240ca1cc),
    ((int32)0x2de92c6f), ((int32)0x4a7484aa), ((int32)0x5cb0a9dc), ((int32)0x76f988da), ((int32)0x983e5152),
    ((int32)0xa831c66d), ((int32)0xb00327c8), ((int32)0xbf597fc7), ((int32)0xc6e00bf3), ((int32)0xd5a79147),
    ((int32)0x06ca6351), ((int32)0x14292967), ((int32)0x27b70a85), ((int32)0x2e1b2138), ((int32)0x4d2c6dfc),
    ((int32)0x53380d13), ((int32)0x650a7354), ((int32)0x766a0abb), ((int32)0x81c2c92e), ((int32)0x92722c85),
    ((int32)0xa2bfe8a1), ((int32)0xa81a664b), ((int32)0xc24b8b70), ((int32)0xc76c51a3), ((int32)0xd192e819),
    ((int32)0xd6990624), ((int32)0xf40e3585), ((int32)0x106aa070), ((int32)0x19a4c116), ((int32)0x1e376c08),
    ((int32)0x2748774c), ((int32)0x34b0bcb5), ((int32)0x391c0cb3), ((int32)0x4ed8aa4a), ((int32)0x5b9cca4f),
    ((int32)0x682e6ff3), ((int32)0x748f82ee), ((int32)0x78a5636f), ((int32)0x84c87814), ((int32)0x8cc70208),
    ((int32)0x90befffa), ((int32)0xa4506ceb), ((int32)0xbef9a3f7), ((int32)0xc67178f2)};

void FDaeSHA256::Initialize()
{
    W.SetNum(64);
    Buffer.SetNum(64);
}

void FDaeSHA256::Update(const TArray<uint8>& B, const int32 Offset, const int32 Len)
{
    int32 N = Count % BLOCK_SIZE;
    Count += Len;
    int32 PartLen = BLOCK_SIZE - N;
    int32 I = 0;

    if (Len >= PartLen)
    {
        for (int32 Index = 0; Index < PartLen; Index++)
        {
            Buffer[N + Index] = B[Offset + Index];
        }
        Sha(Buffer, 0);
        for (I = PartLen; ((I + BLOCK_SIZE) - 1) < Len; I += BLOCK_SIZE)
        {
            Sha(B, Offset + I);
        }
        N = 0;
    }

    if (I < Len)
    {
        for (int32 Index = 0; Index < Len - I; Index++)
        {
            Buffer[N + Index] = B[Offset + I + Index];
        }
    }
}

TArray<uint8> FDaeSHA256::Digest()
{
    TArray<uint8> Tail = PadBuffer();
    Update(Tail, 0, Tail.Num());
    return GetResult();
}

TArray<uint8> FDaeSHA256::PadBuffer()
{
    int32 N = Count % BLOCK_SIZE;
    int32 Padding = (N < 56) ? (56 - N) : (120 - N);
    TArray<uint8> Result;
    Result.SetNum(Padding + 8);
    Result[0] = (uint8)0x80;
    int64 Bits = Count << 3;
    Result[Padding + 0] = (uint8)(Bits >> 56);
    Result[Padding + 1] = (uint8)(Bits >> 48);
    Result[Padding + 2] = (uint8)(Bits >> 40);
    Result[Padding + 3] = (uint8)(Bits >> 32);
    Result[Padding + 4] = (uint8)(Bits >> 24);
    Result[Padding + 5] = (uint8)(Bits >> 16);
    Result[Padding + 6] = (uint8)(Bits >> 8);
    Result[Padding + 7] = (uint8)Bits;
    Padding += 8;
    return Result;
}

TArray<uint8> FDaeSHA256::GetResult()
{
    TArray<uint8> Result;
    Result.SetNum(32);

    Result[0] = (uint8)(H0 >> 24);
    Result[1] = (uint8)(H0 >> 16);
    Result[2] = (uint8)(H0 >> 8);
    Result[3] = (uint8)(H0);

    Result[4] = (uint8)(H1 >> 24);
    Result[5] = (uint8)(H1 >> 16);
    Result[6] = (uint8)(H1 >> 8);
    Result[7] = (uint8)(H1);

    Result[8] = (uint8)(H2 >> 24);
    Result[9] = (uint8)(H2 >> 16);
    Result[10] = (uint8)(H2 >> 8);
    Result[11] = (uint8)(H2);

    Result[12] = (uint8)(H3 >> 24);
    Result[13] = (uint8)(H3 >> 16);
    Result[14] = (uint8)(H3 >> 8);
    Result[15] = (uint8)(H3);

    Result[16] = (uint8)(H4 >> 24);
    Result[17] = (uint8)(H4 >> 16);
    Result[18] = (uint8)(H4 >> 8);
    Result[19] = (uint8)(H4);

    Result[20] = (uint8)(H5 >> 24);
    Result[21] = (uint8)(H5 >> 16);
    Result[22] = (uint8)(H5 >> 8);
    Result[23] = (uint8)(H5);

    Result[24] = (uint8)(H6 >> 24);
    Result[25] = (uint8)(H6 >> 16);
    Result[26] = (uint8)(H6 >> 8);
    Result[27] = (uint8)(H6);

    Result[28] = (uint8)(H7 >> 24);
    Result[29] = (uint8)(H7 >> 16);
    Result[30] = (uint8)(H7 >> 8);
    Result[31] = (uint8)(H7);

    return Result;
}

void FDaeSHA256::Sha(const TArray<uint8>& In, int32 Offset)
{
    int32 A = H0;
    int32 B = H1;
    int32 C = H2;
    int32 D = H3;
    int32 E = H4;
    int32 F = H5;
    int32 G = H6;
    int32 H = H7;
    int32 T;
    int32 T2;
    int32 R;
    for (R = 0; R < 16; R++)
    {
        W[R] = ((In[Offset] << 24) | ((In[Offset + 1] & 0xFF) << 16) | ((In[Offset + 2] & 0xFF) << 8) |
                (In[Offset + 3] & 0xFF));
        Offset += 4;
    }
    for (R = 16; R < 64; R++)
    {
        T = W[R - 2];
        T2 = W[R - 15];
        W[R] = (((S(T, 17) | (T << 15)) ^ (S(T, 19) | (T << 13)) ^ S(T, 10)) + W[R - 7] +
                ((S(T2, 7) | (T2 << 25)) ^ (S(T2, 18) | (T2 << 14)) ^ S(T2, 3)) + W[R - 16]);
    }
    for (R = 0; R < 64; R++)
    {
        T = (H + ((S(E, 6) | (E << 26)) ^ (S(E, 11) | (E << 21)) ^ (S(E, 25) | (E << 7))) + ((E & F) ^ (~E & G)) +
             K[R] + W[R]);
        T2 =
            (((S(A, 2) | (A << 30)) ^ (S(A, 13) | (A << 19)) ^ (S(A, 22) | (A << 10))) + ((A & B) ^ (A & C) ^ (B & C)));
        H = G;
        G = F;
        F = E;
        E = D + T;
        D = C;
        C = B;
        B = A;
        A = T + T2;
    }
    H0 += A;
    H1 += B;
    H2 += C;
    H3 += D;
    H4 += E;
    H5 += F;
    H6 += G;
    H7 += H;
}

FORCEINLINE int32 FDaeSHA256::S(const int32 InA, const int32 InB)
{
    const uint32 A = InA;
    const uint32 B = InB;
    return (A >> B);
}
