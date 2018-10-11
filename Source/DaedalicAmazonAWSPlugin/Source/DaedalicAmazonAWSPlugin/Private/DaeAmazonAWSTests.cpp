#include "DaedalicAmazonAWSPlugin.h"

#include "HttpModule.h"
#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"

#include "DaeAmazonAWS.h"
#include "DaeAmazonAWSHelper.h"
#include "DaeSHA256.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDaeAmazonAWSTestSignRequest, "Daedalic.AmazonAWS.HashCanonicalRequest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDaeAmazonAWSTestSignRequest::RunTest(const FString& Parameters)
{
    // Test signing for the official Amazon example provided at
    // https://docs.aws.amazon.com/general/latest/gr/sigv4_signing.html.

    // ARRANGE.
    FHttpModule* Http = &FHttpModule::Get();

    TSharedRef<IHttpRequest> Request = Http->CreateRequest();

    Request->SetVerb(TEXT("GET"));
    Request->SetURL(TEXT("https://iam.amazonaws.com/?Action=ListUsers&Version=2010-05-08"));

    Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded; charset=utf-8"));
    Request->SetHeader(TEXT("X-Amz-Date"), TEXT("20150830T123600Z"));

    FString Region = TEXT("us-east-1");
    FString Service = TEXT("iam");

    FString AccessKey = TEXT("AKIDEXAMPLE");
    FString SecretKey = TEXT("wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY");

    // ACT.
    UDaeAmazonAWS::AuthorizeRequest(Request, Region, Service, AccessKey, SecretKey);

    // ASSERT.
    FString Signature = Request->GetHeader(TEXT("Authorization"));
    TestEqual(TEXT("Signature"), Signature,
              TEXT("AWS4-HMAC-SHA256 Credential=AKIDEXAMPLE/20150830/us-east-1/iam/aws4_request, "
                   "SignedHeaders=content-type;host;x-amz-date, "
                   "Signature=5d672d79c15b13162d9279b0855cfba6789a8edb4c82c400e06b5924a6f2b5d7"));

    return true;
}

#endif
