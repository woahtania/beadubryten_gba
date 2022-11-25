# pusheenwars

## Useful Links
- [Tonc Docs](http://www.coranac.com/tonc/text/toc.htm)
- [Tonc Code Samples](http://www.coranac.com/files/tonc-code.zip)
- [Maxmod Docs](https://maxmod.devkitpro.org/ref/tut/gbaprog.html)
- [GBATek - Technical Info](https://akkit.org/info/gbatek.htm)
- [Sound System Info](http://www.belogic.com/gba/index.php)

**Convert bmp files to .c and .h format**

`grit cope.bmp -gB4 -ftc`

# Visual Studio Code Setup

In `c_cpp_properties.json`, add include paths as follows:

```json
"includePath": [
    "${workspaceFolder}",
    "${workspaceFolder}/include",
    "${workspaceFolder}/build",
    "/opt/devkitpro/libgba/include",
    "/opt/devkitpro/libtonc/include",
    "/opt/devkitpro/devkitARM/arm-none-eabi/include",
    "/opt/devkitpro/devkitARM/lib/gcc/arm-none-eabi/12.1.0/include"
]
```

and add the compiler path:

```json
    "compilerPath": "/opt/devkitpro/devkitARM/bin/arm-none-eabi-gcc",
```

`launch.json`:

```json
{
    "version": "2.0.0",
    "configurations": [
        {
            "name": "(gdb) Launch",
            "type": "cppdbg",
            "request": "launch",
            "preLaunchTask": "gdb-debug", 
            "postDebugTask": "stop emulation",
            "serverLaunchTimeout": 10000,
            "stopAtEntry": true,
            "program": "${workspaceFolder}/${workspaceFolderBasename}.elf",
            "MIMode": "gdb",
            "externalConsole": true,
            "cwd": "${workspaceFolder}",
            "targetArchitecture": "arm",
            "miDebuggerServerAddress": "localhost:2345",
            "windows": {
                "type": "cppdbg",
                "name": "(gdb) Launch Win",
                "request": "launch",
                "miDebuggerPath": "/opt/devkitpro/devkitARM/bin/arm-none-eabi-gdb",
                "setupCommands": [
                    {
                        "description": "Enable pretty-printing for gdb",
                        "ignoreFailures": true,
                        "text": "file ${workspaceFolder}/${workspaceFolderBasename}.elf -enable-pretty-printing"
                    }
                ]
            },
        }
    ]
}
```

`tasks.json`

```json
{
	"version": "2.0.0",
	"options": {
		"env": {
		  "DEVKITARM": "/opt/devkitpro/devkitARM"
		}
	 },
	"tasks": [
		{
            "label": "make release",
            "type": "process",
            "command": "make",
            "args": ["DEBUG=0"],
            "problemMatcher": [],
			"group": {
				"kind": "build",
			}
		},
		{
            "label": "make debug",
            "type": "process",
            "command": "make",
            "args": ["DEBUG=1"],
            "problemMatcher": [],
			"group": {
				"kind": "build",
				"isDefault": true
			}
		},
		{
            "label": "gdb-debug",
            "type": "shell",
            "dependsOn": ["make debug"],
            "isBackground": true,
            "command": "mgba -4 -g ${workspaceFolder}/${workspaceFolderBasename}.gba",
			"problemMatcher": [
				{
				  "pattern": [
					{
					  "regexp": ".",
					  "file": 1,
					  "location": 2,
					  "message": 3
					}
				  ],
				  "background": {
					"activeOnStart": true,	
					"beginsPattern": ".",
					"endsPattern": ".",
				  }
				}
			  ]
        },        
		{ 
            "label": "stop emulation",
            "type":"shell",
			"command":"killall mgba"
        },
	]
}
```
