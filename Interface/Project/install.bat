xcopy build\juma_sdk.lib ..\Output\ /F /Y

xcopy ..\..\Interface\Output\juma_sdk.lib  ..\..\Examples\lib\ /F /Y

xcopy ..\..\Interface\Source\juma_sdk.sct  ..\..\Examples\lib\ /F /Y

xcopy ..\..\Interface\Include\juma_sdk_types.h ..\..\Examples\include\  /F /Y

xcopy ..\..\Interface\Include\juma_sdk_api.h ..\..\Examples\include\  /F /Y
