// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class UE_Assimp : ModuleRules
{
	public UE_Assimp(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"UE_AssimpLibrary",
				
				// ... add other public dependencies that you statically link with here ...
			}
		);


        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore","ProceduralMeshComponent","Projects","MeshDescription", "MeshConversion","StaticMeshDescription"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        string pluginDLLPath = Path.Combine(ModuleDirectory, "../../",
            "Binaries", "Win64", "assimp.dll");
        string binariesPath = CopyToProjectBinaries(pluginDLLPath, Target);
        System.Console.WriteLine("Using DLL: " + binariesPath);
        RuntimeDependencies.Add(binariesPath);
    }

    public string GetUProjectPath()
    {
        return Path.Combine(ModuleDirectory, "../../../..");
    }

    private string CopyToProjectBinaries(string Filepath, ReadOnlyTargetRules Target)
    {
        string BinariesDir = Path.Combine(GetUProjectPath(), "Binaries", Target.Platform.ToString());
        string Filename = Path.GetFileName(Filepath);

        //convert relative path 
        string FullBinariesDir = Path.GetFullPath(BinariesDir);

        if (!Directory.Exists(FullBinariesDir))
        {
            Directory.CreateDirectory(FullBinariesDir);
        }

        string FullExistingPath = Path.Combine(FullBinariesDir, Filename);
        bool ValidFile = false;

        //File exists, check if they're the same
        if (File.Exists(FullExistingPath))
        {
            ValidFile = true;
        }

        //No valid existing file found, copy new dll
        if (!ValidFile)
        {
            File.Copy(Filepath, Path.Combine(FullBinariesDir, Filename), true);
        }
        return FullExistingPath;
    }
}
