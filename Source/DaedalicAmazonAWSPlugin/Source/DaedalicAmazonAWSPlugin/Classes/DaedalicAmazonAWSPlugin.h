#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IDaedalicAmazonAWSPlugin.h"

class FDaedalicAmazonAWSPlugin : public IDaedalicAmazonAWSPlugin
{

public:
    FDaedalicAmazonAWSPlugin()
    {
    }
    virtual ~FDaedalicAmazonAWSPlugin()
    {
    }

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

DECLARE_LOG_CATEGORY_EXTERN(LogDaeAmazonAWS, Log, All);