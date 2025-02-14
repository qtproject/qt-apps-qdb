To have a complete driver package, in addition to the .infs under this
directory the following steps are required:

- Install Windows Driver Kit
- generate catalog files for the .infs
- sign the catalog files

-----------------------------------------------------------------------------

Driver signing for Windows is documented at
https://msdn.microsoft.com/en-us/windows/hardware/drivers/install/driver-signing
but here's an abbreviated version:

The .cat files are catalog files, that are derived from the .infs (and the
files they reference). They are created with inf2cat from Windows Driver Kit.

Example commands for creating the catalog files:

inf2cat /driver:qdb_driver /os:10_VB_X64,10_VB_ARM64,10_NI_X64,10_NI_ARM64,10_CO_X64,10_CO_ARM64
inf2cat /driver:rndis_driver /os:10_VB_X64,10_VB_ARM64,10_NI_X64,10_NI_ARM64,10_CO_X64,10_CO_ARM64

Catalog files need to be signed in order for drivers to be installable.
It is done with signtool, which is included in Windows SDK. See the end of
this README for how to create a test certificate.

Example commands to sign with a test key "Qt.io(Test)" in PrivateCertStore:

signtool sign /v /s PrivateCertStore /n Qt.io(Test) /fd SHA256 /t http://timestamp.digicert.com/?alg=sha1 qdb_driver\qdb_device.cat
signtool sign /v /s PrivateCertStore /n Qt.io(Test) /fd SHA256 /t http://timestamp.digicert.com/?alg=sha1 rndis_driver\rndis_device.cat

Signtool can also be used to verify that driver is correctly signed:

signtool verify /pa /v /c qdb_driver\qdb_device.cat qdb_driver\qdb_device.inf
signtool verify /pa /v /c rndis_driver\rndis_device.cat rndis_driver\rndis_device.inf

The drivers can be installed with pnputil:

pnputil -a qdb_driver\qdb_device.inf
pnputil -a rndis_driver\rndis_device.inf

-----------------------------------------------------------------------------

Creating a test certificate:

makecert -r -pe -ss PrivateCertStore -n CN=Qt.io(Test) QtIoTest.cer

Test-signed  drivers can only be installed, if the computer is in test mode,
which can be enabled with:

bcdedit -set testsigning on

The test certificate also needs to be added to trusted root certification
authorities:

certutil -addstore root QtIoTest.cer
