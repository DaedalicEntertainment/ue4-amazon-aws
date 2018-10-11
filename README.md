# ue4-amazon-aws
Facilitates accessing web services hosted on Amazon AWS using Unreal Engine 4.

# Quickfacts

* Current Version: 1.0
* Engine Version: Unreal Engine 4.18.3

# Adding The Plugin

1. Clone the repository.
1. Close the Unreal Editor.
1. Copy the DaedalicAmazonAWSPlugin folder to Plugins folder next to your .uproject file.
1. Right-click your .uproject file and select Re-generate Visual Studio project files.
1. Build the resulting solution in Visual Studio.
1. Start the Unreal Editor.
1. Enable the plugin in Edit > Plugins > Daedalic.

# Using The Plugin

First, add `DaedalicAmazonAWSPlugin` to the `PublicDependencyModuleNames` of the `Build.cs` file of your project.

Now, after creating any HTTP request as usual, you can pass it to `UDaeAmazonAWS::AuthorizeRequest` in order to have the plugin generate the matching Authorization header as outlined in https://docs.aws.amazon.com/general/latest/gr/sigv4_signing.html.

    // Create request.
    FHttpModule* Http = &FHttpModule::Get();

    TSharedRef<IHttpRequest> Request = Http->CreateRequest();

    Request->SetVerb(TEXT("GET"));
    Request->SetURL(TEXT("https://iam.amazonaws.com/?Action=ListUsers&Version=2010-05-08"));

    FString Region = TEXT("us-east-1");
    FString Service = TEXT("iam");

    FString AccessKey = TEXT("AKIDEXAMPLE");
    FString SecretKey = TEXT("wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY");

    // Add authorization header.
    UDaeAmazonAWS::AuthorizeRequest(Request, Region, Service, AccessKey, SecretKey);

    // Send request.
    Request->ProcessRequest();

The method `UDaeAmazonAWS::AuthorizeRequest` assumes that Verb, URL, Headers and Content have already been set, because they are part of the signature.

* If the `Host` header isn't already set, it will be parsed from the request URL.
* If the `Content-Type` header isn't already set, it will be set to `application/x-www-form-urlencoded; charset=utf-8`.
* If the `X-Amz-Date header` isn't already set, the current one will be added.
