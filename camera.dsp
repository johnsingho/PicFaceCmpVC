# Microsoft Developer Studio Project File - Name="camera" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=camera - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "camera.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "camera.mak" CFG="camera - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "camera - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "camera - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "camera - Win32 Release"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W4 /GX /O2 /I "DirectShow/Include" /I "zbar/include" /I "opencv/include/cv" /I "opencv/include/cvaux" /I "opencv/include/cvcam" /I "opencv/include/cxcore" /I "opencv/include/highgui" /I "opencv/include/ml" /I "comm/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 cxcore.lib cv.lib cvaux.lib highgui.lib cvcam.lib ml.lib winmm.lib /nologo /subsystem:windows /map /debug /machine:I386 /out:"camera.exe" /libpath:"DirectShow/Lib" /libpath:"zbar/lib" /libpath:"opencv/lib"

!ELSEIF  "$(CFG)" == "camera - Win32 Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /Gm /GX /ZI /Od /I "DirectShow/Include" /I "zbar/include" /I "opencv/include/cv" /I "opencv/include/cvaux" /I "opencv/include/cvcam" /I "opencv/include/cxcore" /I "opencv/include/highgui" /I "opencv/include/ml" /I "comm/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "JOHN_DEBUG" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 cxcore.lib cv.lib cvaux.lib highgui.lib cvcam.lib ml.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /out:"cameraTest.exe" /pdbtype:sept /libpath:"DirectShow/Lib" /libpath:"zbar/lib" /libpath:"opencv/lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "camera - Win32 Release"
# Name "camera - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;asm"
# Begin Source File

SOURCE=.\camera.cpp
# End Source File
# Begin Source File

SOURCE=.\camera.rc
# End Source File
# Begin Source File

SOURCE=.\cameraDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CameraDS.cpp
# End Source File
# Begin Source File

SOURCE=.\Compare.cpp
# End Source File
# Begin Source File

SOURCE=.\DialogLockTest.cpp
# End Source File
# Begin Source File

SOURCE=.\IDStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\MyButton.cpp
# End Source File
# Begin Source File

SOURCE=.\MyStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc"
# Begin Source File

SOURCE=.\camera.h
# End Source File
# Begin Source File

SOURCE=.\cameraDlg.h
# End Source File
# Begin Source File

SOURCE=.\Compare.h
# End Source File
# Begin Source File

SOURCE=.\DialogLockTest.h
# End Source File
# Begin Source File

SOURCE=.\HWFaceRecSDK.h
# End Source File
# Begin Source File

SOURCE=.\IDStatic.h
# End Source File
# Begin Source File

SOURCE=.\MyButton.h
# End Source File
# Begin Source File

SOURCE=.\MyStatic.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\back.bmp
# End Source File
# Begin Source File

SOURCE=.\res\back2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\back3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\back4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bg.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\camera.exe.manifest
# End Source File
# Begin Source File

SOURCE=.\res\camera.ico
# End Source File
# Begin Source File

SOURCE=.\res\camera.rc2
# End Source File
# Begin Source File

SOURCE=.\res\IDBack.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pic.bmp
# End Source File
# Begin Source File

SOURCE=.\res\picIDCard.bmp
# End Source File
# Begin Source File

SOURCE=.\res\picRight.bmp
# End Source File
# Begin Source File

SOURCE=.\res\picTick.bmp
# End Source File
# Begin Source File

SOURCE=.\res\picWrong.bmp
# End Source File
# End Group
# Begin Group "comm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\comm\IdReadHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\comm\IdReadHelper.h
# End Source File
# Begin Source File

SOURCE=.\comm\MindVisionCam.cpp
# End Source File
# Begin Source File

SOURCE=.\comm\MindVisionCam.h
# End Source File
# Begin Source File

SOURCE=.\comm\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\comm\misc.h
# End Source File
# Begin Source File

SOURCE=.\comm\serial.cpp
# End Source File
# Begin Source File

SOURCE=.\comm\serial.h
# End Source File
# Begin Source File

SOURCE=.\comm\sw_lock03.cpp
# End Source File
# Begin Source File

SOURCE=.\comm\sw_lock03.h
# End Source File
# End Group
# Begin Group "exam"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\FaceExam.cpp
# End Source File
# Begin Source File

SOURCE=.\FaceExam.h
# End Source File
# End Group
# Begin Group "zbar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zbar\include\zbar.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
