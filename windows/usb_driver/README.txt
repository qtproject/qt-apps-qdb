To have a complete driver package, in addition to the .infs under this
directory the following steps are required:

- add WinUSB co-installer DLLs from Windows Driver Kit for each architecture for qdb_device.inf
- generate catalog files for the .infs
- sign the catalog files

-----------------------------------------------------------------------------

Add coinstallers from Windows Driver Kit as instructed in
https://msdn.microsoft.com/en-us/library/windows/hardware/ff540283(v=vs.85).aspx#howto

Example commands:

mkdir qdb_driver\x86
copy "C:\Program Files (x86)\Windows Kits\10\Redist\wdf\x86\winusbcoinstaller2.dll" qdb_driver\x86
copy "C:\Program Files (x86)\Windows Kits\10\Redist\wdf\x86\WdfCoInstaller01009.dll" qdb_driver\x86
mkdir qdb_driver\amd64
copy "C:\Program Files (x86)\Windows Kits\10\Redist\wdf\x64\winusbcoinstaller2.dll" qdb_driver\amd64
copy "C:\Program Files (x86)\Windows Kits\10\Redist\wdf\x64\WdfCoInstaller01009.dll" qdb_driver\amd64

Driver signing for Windows is documented at
https://msdn.microsoft.com/en-us/windows/hardware/drivers/install/driver-signing
but here's an abbreviated version:

The .cat files are catalog files, that are derived from the .infs (and the
files they reference). They are created with inf2cat from Windows Driver Kit.

Example commands for creating the catalog files:

"C:\Program Files (x86)\Windows Kits\10\bin\x86\inf2cat.exe" /driver:"C:\Users\kaoikari1\work\qdb\windows\usb_driver\qdb_driver" /os:8_X64,8_X86,Server8_X64,Server2008R2_X64,7_X64,7_X86,Server2008_X64,Server2008_X86,10_X86,10_X64,Server10_X64
"C:\Program Files (x86)\Windows Kits\10\bin\x86\inf2cat.exe" /driver:"C:\Users\kaoikari1\work\qdb\windows\usb_driver\rndis_driver" /os:8_X64,8_X86,Server8_X64,Server2008R2_X64,7_X64,7_X86,Server2008_X64,Server2008_X86,10_X86,10_X64,Server10_X64

Catalog files need to be signed in order for drivers to be installable.
It is done with signtool, which is included in Windows SDK. See the end of
this README for how to create a test certificate.

Example commands to sign with a test key "Qt.io(Test)" in PrivateCertStore:

"C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe" sign /v /s PrivateCertStore /n Qt.io(Test) /t http://timestamp.verisign.com/scripts/timstamp.dll qdb_driver\qdb_device.cat
"C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe" sign /v /s PrivateCertStore /n Qt.io(Test) /t http://timestamp.verisign.com/scripts/timstamp.dll rndis_driver\rndis_device.cat

Signtool can also be used to verify that driver is correctly signed:

"C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe" verify /pa /v /c qdb_driver\qdb_device.cat qdb_driver\qdb_device.inf
"C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe" verify /pa /v /c rndis_driver\rndis_device.cat rndis_driver\rndis_device.inf

The drivers can be installed with pnputil:

pnputil -a qdb_driver\qdb_device.inf
pnputil -a rndis_driver\rndis_device.inf

-----------------------------------------------------------------------------

Creating a test certificate:

makecert -r -pe -ss PrivateCertStore -n CN=Qt.io(Test) QtIoTest.cer

Test-signed  drivers can only be installed, if the computer is in test mode,
which can be enabled with:

bcdedit.exe -set testsigning on

The test certificate also needs to be added to trusted root certification
authorities:

certmgr.exe /add QtIoTest.cer /s /r localMachine root
