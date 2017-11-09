// Some copyright should be here...

using UnrealBuildTool;
using System.IO;

public class KinectPlugin : ModuleRules
{

    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }

	public KinectPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
		
		PublicIncludePaths.AddRange(
			new string[] {
				"KinectPlugin/Public"
				
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"KinectPlugin/Private",
				
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
			    "Engine",
                "InputCore",
                "RenderCore",
                "RHI",
                "Slate",
			    "SlateCore",
			    "UMG"
				
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate", "SlateCore"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				
				// ... add any modules that your module loads dynamically here ...
			}
			);

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "amd64" : "x86";
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "v1.8","lib", PlatformString, "Kinect10.lib"));

            PublicIncludePaths.AddRange(
                new string[] {
					Path.Combine(ThirdPartyPath, "v1.8/inc")
				});
        }
	}
}
