namespace UnrealBuildTool.Rules
{
	public class DaedalicAmazonAWSPlugin : ModuleRules
	{
		public DaedalicAmazonAWSPlugin(ReadOnlyTargetRules Target) : base(Target)
		{
			PrivateIncludePaths.AddRange(
				new string[] {
                    "DaedalicAmazonAWSPlugin/Private",
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
                    "CoreUObject",
                    "Engine",
                    "Http"
                }
				);
		}
	}
}
