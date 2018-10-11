#include "DaedalicAmazonAWSPlugin.h"

IMPLEMENT_MODULE(FDaedalicAmazonAWSPlugin, DaedalicAmazonAWSPlugin)

DEFINE_LOG_CATEGORY(LogDaeAmazonAWS);

void FDaedalicAmazonAWSPlugin::StartupModule()
{
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of
    // course.)
}

void FDaedalicAmazonAWSPlugin::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
}
