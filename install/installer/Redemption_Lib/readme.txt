To install Redemption either run install.exe (make sure that redemption.dll
is in the same directory) or run 

regsvr32.exe <fullpath>\redemption.dll

Redemption is a regular COM library and can be installed either with 
regsvr32.exe or using your favorite installer (you will need to mark
redemption.dll as self-registering).

To use Redemption in VB or .Net languages, add "Redemption" library to your project references.

To uninstall Redemption, run the following from the command line:

regsvr32.exe /u <fullpath>\redemption.dll