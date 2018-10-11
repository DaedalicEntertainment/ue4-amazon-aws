#include "DaedalicAmazonAWSPlugin.h"
#include "DaeAmazonAWS.h"

#include "Kismet/KismetTextLibrary.h"

#include "DaeAmazonAWSHelper.h"
#include "DaeSHA256.h"

void UDaeAmazonAWS::AuthorizeRequest(TSharedRef<IHttpRequest> Request, const FString& Region, const FString& Service,
                                     const FString& AccessKey, const FString& SecretKey)
{
    // Parse request URL.
    FString URL = Request->GetURL();
    int32 QueryStringIndex = URL.Find(TEXT("?"));

    // Add missing headers.
    FString Host = Request->GetHeader(TEXT("Host"));
    FString ContentType = Request->GetHeader(TEXT("Content-Type"));
    FString AmazonDate = Request->GetHeader(TEXT("X-Amz-Date"));

    if (Host.IsEmpty())
    {
        Host = URL.Mid(URL.Find(TEXT("://")));
        Host = Host.Left(Host.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 3)).Mid(3);

        Request->SetHeader(TEXT("Host"), Host);
    }

    if (ContentType.IsEmpty())
    {
        ContentType = TEXT("application/x-www-form-urlencoded; charset=utf-8");
        Request->SetHeader(TEXT("Content-Type"), ContentType);
    }

    if (AmazonDate.IsEmpty())
    {
        FDateTime Now = FDateTime::UtcNow();

        FString YearString = UKismetTextLibrary::Conv_IntToText(Now.GetYear(), false, 4, 4).ToString();
        FString MonthString = UKismetTextLibrary::Conv_IntToText(Now.GetMonth(), false, 2, 2).ToString();
        FString DayString = UKismetTextLibrary::Conv_IntToText(Now.GetDay(), false, 2, 2).ToString();

        FString DateString = FString::Printf(TEXT("%s%s%s"), *YearString, *MonthString, *DayString);
        UE_LOG(LogDaeAmazonAWS, Verbose, TEXT("DateString = %s"), *DateString);

        FString HourString = UKismetTextLibrary::Conv_IntToText(Now.GetHour(), false, 2, 2).ToString();
        FString MinuteString = UKismetTextLibrary::Conv_IntToText(Now.GetMinute(), false, 2, 2).ToString();
        FString SecondString = UKismetTextLibrary::Conv_IntToText(Now.GetSecond(), false, 2, 2).ToString();

        AmazonDate = FString::Printf(TEXT("%sT%s%s%sZ"), *DateString, *HourString, *MinuteString, *SecondString);
        UE_LOG(LogDaeAmazonAWS, Verbose, TEXT("AmazonDate = %s"), *AmazonDate);

        Request->SetHeader(TEXT("X-Amz-Date"), AmazonDate);
    }

    // Task 1: Create a Canonical Request for Signature Version 4
    // https://docs.aws.amazon.com/general/latest/gr/sigv4-create-canonical-request.html
    // Build canonical headers.
    TMap<FString, FString> RequestHeaders = UDaeAmazonAWSHelper::GetRequestHeaders(Request);

    FString CanonicalHeaders;

    for (auto& RequestHeader : RequestHeaders)
    {
        CanonicalHeaders += FString::Printf(TEXT("%s:%s\n"), *RequestHeader.Key.ToLower(), *RequestHeader.Value);
    }

    UE_LOG(LogDaeAmazonAWS, Verbose, TEXT("CanonicalHeaders:\n---\n%s\n---"), *CanonicalHeaders);

    // Build signed headers.
    FString SignedHeaders;

    for (auto& RequestHeader : RequestHeaders)
    {
        SignedHeaders += FString::Printf(TEXT("%s;"), *RequestHeader.Key.ToLower());
    }

    SignedHeaders.RemoveFromEnd(";");

    UE_LOG(LogDaeAmazonAWS, Verbose, TEXT("SignedHeaders = %s"), *SignedHeaders);

    // Hash payload.
    TArray<uint8> Content = Request->GetContent();
    TArray<uint8> HashedData = FDaeSHA256::Hash(Content);

    FString HashedPayload = BytesToHex(HashedData.GetData(), HashedData.Num()).ToLower();

    UE_LOG(LogDaeAmazonAWS, Verbose, TEXT("HashedPayload = %s"), *HashedPayload);

    // Build canonical request.
    FString HTTPRequestMethod = Request->GetVerb();

    FString CanonicalURI = URL.Mid(URL.Find(TEXT("://")) + 3 + Host.Len());
    CanonicalURI = CanonicalURI.Left(CanonicalURI.Find(TEXT("?")));

    UE_LOG(LogDaeAmazonAWS, Verbose, TEXT("CanonicalURI = %s"), *CanonicalURI);

    FString CanonicalQueryString = URL.Mid(QueryStringIndex + 1);

    UE_LOG(LogDaeAmazonAWS, Verbose, TEXT("CanonicalQueryString = %s"), *CanonicalQueryString);

    FString CanonicalRequest =
        FString::Printf(TEXT("%s\n%s\n%s\n%s\n%s\n%s"), *HTTPRequestMethod, *CanonicalURI, *CanonicalQueryString,
                        *CanonicalHeaders, *SignedHeaders, *HashedPayload);

    UE_LOG(LogDaeAmazonAWS, Verbose, TEXT("CanonicalRequest:\n---\n%s\n---"), *CanonicalRequest);

    // Hash canonical request.
    TArray<uint8> ByteArray = UDaeAmazonAWSHelper::StringToBytes(CanonicalRequest);
    HashedData = FDaeSHA256::Hash(ByteArray);

    FString HashedCanonicalRequest = BytesToHex(HashedData.GetData(), HashedData.Num()).ToLower();

    UE_LOG(LogDaeAmazonAWS, Verbose, TEXT("HashedCanonicalRequest = %s"), *HashedCanonicalRequest);

    // Task 2: Create a String to Sign for Signature Version 4
    // https://docs.aws.amazon.com/general/latest/gr/sigv4-create-string-to-sign.html
    FString Algorithm = TEXT("AWS4-HMAC-SHA256");
    FString Signing = TEXT("aws4_request");
    FString DateString = AmazonDate.Left(8);
    FString CredentialScope = FString::Printf(TEXT("%s/%s/%s/%s"), *DateString, *Region, *Service, *Signing);

    FString StringToSign =
        FString::Printf(TEXT("%s\n%s\n%s\n%s"), *Algorithm, *AmazonDate, *CredentialScope, *HashedCanonicalRequest);

    UE_LOG(LogDaeAmazonAWS, Verbose, TEXT("StringToSign:\n---\n%s\n---"), *StringToSign);

    // Task 3: Calculate the Signature for AWS Signature Version 4
    // https://docs.aws.amazon.com/general/latest/gr/sigv4-calculate-signature.html
    TArray<uint8> DateBytes = UDaeAmazonAWSHelper::StringToBytes(DateString);
    TArray<uint8> SecretBytes = UDaeAmazonAWSHelper::StringToBytes(TEXT("AWS4") + SecretKey);
    TArray<uint8> RegionBytes = UDaeAmazonAWSHelper::StringToBytes(Region);
    TArray<uint8> ServiceBytes = UDaeAmazonAWSHelper::StringToBytes(Service);
    TArray<uint8> SigningBytes = UDaeAmazonAWSHelper::StringToBytes(Signing);

    TArray<uint8> kDate = FDaeSHA256::HMAC(DateBytes, SecretBytes);
    TArray<uint8> kRegion = FDaeSHA256::HMAC(RegionBytes, kDate);
    TArray<uint8> kService = FDaeSHA256::HMAC(ServiceBytes, kRegion);
    TArray<uint8> kSigning = FDaeSHA256::HMAC(SigningBytes, kService);

    FString SigningKey = BytesToHex(kSigning.GetData(), kSigning.Num()).ToLower();

    TArray<uint8> SignatureBytes = FDaeSHA256::HMAC(UDaeAmazonAWSHelper::StringToBytes(StringToSign), kSigning);
    FString Signature = BytesToHex(SignatureBytes.GetData(), SignatureBytes.Num()).ToLower();

    // Task 4: Add the Signature to the HTTP Request
    // https://docs.aws.amazon.com/general/latest/gr/sigv4-add-signature-to-request.html
    FString Authorization =
        FString::Printf(TEXT("%s Credential=%s/%s/%s/%s/%s, SignedHeaders=%s, Signature=%s"), *Algorithm, *AccessKey,
                        *DateString, *Region, *Service, *Signing, *SignedHeaders, *Signature);
    UE_LOG(LogDaeAmazonAWS, Verbose, TEXT("Authorization = %s"), *Authorization);

    Request->SetHeader(TEXT("Authorization"), Authorization);
}
