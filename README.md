Controls for Demo:
    WASD to move, mouse to look around
    Press/hold R to add more objects to the scene
    Press/hold E to remove objects from the scene
    Press L to dynamically remove the debug button from the window

This Visual Studio project assumes that you have a directory called "ThirdPartyLibraries" at the same level as the directory containing the project. In ThirdPartyLibraries, you should have installed GLM, Qt (version 6.10.0), STB, and Vulkan (version 1.4.309.0). So, to use the source code, your directory tree should look like below.

```
└───ThirdPartyLibraries  
|   ├───glm  
|   |   └─── ...  
|   ├───Qt  
|   |   └─── ...  
|   ├───std-master  
|   |   └─── ...  
|   └───VulkanSDK  
|       └─── ...  
|  
|  
└───VoltEngine  
    └─── ...
```

Alternatively, you can download the release zip file and run the program without building. 

Please let me know if you run into any issues.
