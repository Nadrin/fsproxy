; FSproxy 1.0 RC2 NSIS Installer Script
; (C)2009 Micha³ Siejak
; Visit: http://fsproxy.masterm.org

!define PRODUCT_NAME "FSproxy"
!define PRODUCT_VERSION "1.0 RC2"
!define PRODUCT_PUBLISHER "Micha³ Siejak"
!define PRODUCT_WEB_SITE "http://masterm.org/fsproxy"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\fsproxy.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define FSPROXY_IP "192.168.255.1"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!define MUI_PAGE_CUSTOMFUNCTION_PRE CheckOS
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "license.txt"
; Components page
!insertmacro MUI_PAGE_COMPONENTS
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\fsproxy.exe"
!define MUI_FINISHPAGE_RUN_PARAMETERS "-show"
!define MUI_FINISHPAGE_RUN_TEXT "Configure ${PRODUCT_NAME}"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.txt"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Setup.exe"
InstallDir "$PROGRAMFILES\FSproxy"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Function CheckOS
   GetVersion::WindowsName
   Pop $R0
   StrCmp $R0 "2000" CheckOS_Unsupported
   StrCmp $R0 "CE" CheckOS_Unsupported
   StrCmp $R0 "NT" CheckOS_Unsupported
   StrCmp $R0 "ME" CheckOS_Unsupported
   StrCmp $R0 "98" CheckOS_Unsupported
   StrCmp $R0 "98 SE" CheckOS_Unsupported
   StrCmp $R0 "95" CheckOS_Unsupported
   StrCmp $R0 "95 OSR2" CheckOS_Unsupported
   StrCmp $R0 "Win32s" CheckOS_Unsupported
   GoTo CheckOS_Done

   CheckOS_Unsupported:
   MessageBox MB_OK|MB_ICONSTOP "At least Windows XP is required to run ${PRODUCT_NAME}!" IDOK
   Quit

   CheckOS_Done:
FunctionEnd

Section "FSproxy" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "files\bios.bin"
  File "files\qemu.bin"
  File "files\fsproxy.img"
  File "files\fsproxy.exe"
  File "files\readme.txt"
  File "license.txt"
  CreateDirectory "$SMPROGRAMS\FSproxy"
  CreateShortCut "$SMPROGRAMS\FSproxy\FSproxy.lnk" "$INSTDIR\fsproxy.exe"
  CreateShortCut "$SMPROGRAMS\FSproxy\Readme file.lnk" "$INSTDIR\readme.txt"
  CreateShortCut "$DESKTOP\FSproxy.lnk" "$INSTDIR\fsproxy.exe"
  
  DetailPrint "Writing fsproxy entry to hosts file."
  FileOpen $1 "$WINDIR\system32\drivers\etc\hosts" a
  FileSeek $1 0 END
  FileWrite $1 "$\r$\n ${FSPROXY_IP} fsproxy $\r$\n"
  FileClose $1

  WriteRegStr HKLM "SOFTWARE\${PRODUCT_NAME}" "autostart" "no"
  WriteRegStr HKLM "SOFTWARE\${PRODUCT_NAME}" "rw" "no"
  WriteRegStr HKLM "SOFTWARE\${PRODUCT_NAME}" "hdb" ""
  WriteRegStr HKLM "SOFTWARE\${PRODUCT_NAME}" "hdc" ""
  WriteRegStr HKLM "SOFTWARE\${PRODUCT_NAME}" "hdd" ""
  WriteRegStr HKLM "SOFTWARE\${PRODUCT_NAME}" "net_name" ""
  WriteRegStr HKLM "SOFTWARE\${PRODUCT_NAME}" "net_uid" ""
  WriteRegStr HKLM "SOFTWARE\${PRODUCT_NAME}" "path" "$INSTDIR"
  WriteRegStr HKLM "SOFTWARE\${PRODUCT_NAME}" "qemu" "$INSTDIR\qemu.bin"
SectionEnd

Section "Network TAP Driver" SEC02
  SetOutPath "$INSTDIR\tapdrv"
  SetOverwrite ifnewer

  DetailPrint "Installing TAP Network adapter..."
  GetVersion::WindowsPlatformArchitecture
  Pop $R0
  StrCmp $R0 "32" TAP_32
  StrCmp $R0 "64" TAP_64
  DetailPrint "Unsupported architecture detected!"
  MessageBox MB_OK|MB_ICONSTOP "Your architecture is not supported. Network TAP driver will NOT be installed!" IDOK
  GoTo TAP_Done
  
  TAP_32:
  DetailPrint "Detected 32 bit version of Windows."
  File "tapdrv\i386\OemWin2k.inf"
  File "tapdrv\i386\tap0901.cat"
  File "tapdrv\i386\tap0901.sys"
  File "tapdrv\i386\tapinstall.exe"
  GoTo TAP_Install
  
  TAP_64:
  DetailPrint "Detected 64 bit version of Windows."
  File "tapdrv\amd64\OemWin2k.inf"
  File "tapdrv\amd64\tap0901.cat"
  File "tapdrv\amd64\tap0901.sys"
  File "tapdrv\amd64\tapinstall.exe"
  GoTo TAP_Install
  
  TAP_Install:
  nsExec::ExecToStack '"$INSTDIR\tapdrv\tapinstall.exe" install OemWin2k.inf TAP0901'
  pop $R0
  StrCmp $R0 "error" TAP_Error
  IntCmp $R0 0 TAP_NoError TAP_Error TAP_Error
  TAP_NoError:
  DetailPrint "TAP Driver installation finished."
  GoTo TAP_Done
  TAP_Error:
  DetailPrint "TAP driver installation failed!"
  
  TAP_Done:
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\FSproxy\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\FSproxy\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\fsproxy.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\fsproxy.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "FSproxy server image and agent."
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "Network TAP driver V9 (by openvpn.net)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\bios.bin"
  Delete "$INSTDIR\fsproxy.exe"
  Delete "$INSTDIR\fsproxy.img"
  Delete "$INSTDIR\qemu.bin"
  Delete "$INSTDIR\vgabios.bin"
  Delete "$INSTDIR\readme.txt"
  Delete "$INSTDIR\license.txt"
  
  IfFileExists "$INSTDIR\tapdrv" TAP_AskRemove TAP_NoRemove
  
  TAP_AskRemove:
  MessageBox MB_ICONQUESTION|MB_YESNO "Do you wish to also remove network TAP driver?" IDYES TAP_Remove IDNO TAP_Keep
  TAP_Remove:
  DetailPrint "Removing Network TAP driver from the system..."
  nsExec::ExecToStack '"$INSTDIR\tapdrv\tapinstall.exe" remove TAP0901'
  Pop $R0
  StrCmp $R0 "error" TAP_RemoveError
  IntCmp $R0 0 TAP_Keep TAP_RemoveError TAP_RemoveError
  
  TAP_RemoveError:
  DetailPrint "Error occured during driver removal."
  
  TAP_Keep:
  Delete "$INSTDIR\tapdrv\OemWin2k.inf"
  Delete "$INSTDIR\tapdrv\tap0901.cat"
  Delete "$INSTDIR\tapdrv\tap0901.sys"
  Delete "$INSTDIR\tapdrv\tapinstall.exe"

  TAP_NoRemove:
  Delete "$SMPROGRAMS\FSproxy\Uninstall.lnk"
  Delete "$SMPROGRAMS\FSproxy\Website.lnk"
  Delete "$DESKTOP\FSproxy.lnk"
  Delete "$SMPROGRAMS\FSproxy\FSproxy.lnk"
  Delete "$SMPROGRAMS\FSproxy\Readme file.lnk"

  RMDir "$SMPROGRAMS\FSproxy"
  RMDir "$INSTDIR\tapdrv"
  RMDir "$INSTDIR"

  DeleteRegKey HKLM "SOFTWARE\${PRODUCT_NAME}"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd