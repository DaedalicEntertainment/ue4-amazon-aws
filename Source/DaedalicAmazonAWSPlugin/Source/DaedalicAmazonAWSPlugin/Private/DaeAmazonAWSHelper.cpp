#include "DaedalicAmazonAWSPlugin.h"
#include "DaeAmazonAWSHelper.h"

TArray<uint8> UDaeAmazonAWSHelper::StringToBytes(FString S)
{
    TArray<uint8> ByteArray;
    FTCHARToUTF8 TCHARToUTF8 = FTCHARToUTF8(S.GetCharArray().GetData());
    ByteArray.Append((uint8*)TCHARToUTF8.Get(), TCHARToUTF8.Length());
    return ByteArray;
}

TMap<FString, FString> UDaeAmazonAWSHelper::GetRequestHeaders(TSharedRef<IHttpRequest> Request)
{
    TArray<FString> HeaderList = Request->GetAllHeaders();

    HeaderList.Sort([=](const FString& Lhs, const FString& Rhs) {
        return Lhs.Left(Lhs.Find(TEXT(":"))).ToLower() < Rhs.Left(Rhs.Find(TEXT(":"))).ToLower();
    });

    TMap<FString, FString> HeaderMap;

    for (FString& RequestHeader : HeaderList)
    {
        FString HeaderKey;
        FString HeaderValue;
        RequestHeader.Split(TEXT(":"), &HeaderKey, &HeaderValue);
        HeaderMap.Add(HeaderKey, HeaderValue.Mid(1));
    }

    return HeaderMap;
}
