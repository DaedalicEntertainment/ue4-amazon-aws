#pragma once

#include "DaedalicAmazonAWSPlugin.h"
#include "DaeAmazonAWSHelper.generated.h"

/** Provides utility methods for accessing web services hosted on Amazon AWS. */
UCLASS()
class DAEDALICAMAZONAWSPLUGIN_API UDaeAmazonAWSHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Converts the specified string to a byte array. */
    UFUNCTION(BlueprintPure, Category = "Daedalic")
    static TArray<uint8> StringToBytes(FString S);

    /** Gets the headers of the passed request as key-value map. */
    static TMap<FString, FString> GetRequestHeaders(TSharedRef<IHttpRequest> Request);
};
