#pragma once

#include "DaedalicAmazonAWSPlugin.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "DaeAmazonAWS.generated.h"

/** Provides methods for accessing web services hosted on Amazon AWS. */
UCLASS()
class DAEDALICAMAZONAWSPLUGIN_API UDaeAmazonAWS : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Authorized the specified request, adding the Signature Version 4 Authorization header.
     * Assumes that Verb, URL, Headers and Content have already been set, because they are part of the signature.
     *
     * If the Host header isn't already set, it will be parsed from the request URL.
     * If the Content-Type header isn't already set, it will be set to application/x-www-form-urlencoded; charset=utf-8.
     * If the X-Amz-Date header isn't already set, the current one will be added.
     */
    static void AuthorizeRequest(TSharedRef<IHttpRequest> Request, const FString& Region, const FString& Service,
                                 const FString& AccessKey, const FString& SecretKey);
};
