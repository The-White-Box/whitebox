<?xml version="1.0" encoding="utf-8"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
  <!-- Name format company name.product name.application name. -->
  <assemblyIdentity
      version="@PROJECT_VERSION@"
      processorArchitecture="*"
      name="TheWhiteBoxAuthors.WhiteBox.@PROJECT_NAME@"
      type="win32"
  />
  <description>@PROJECT_DESCRIPTION@.</description>
  <dependency>
    <dependentAssembly>
      <!-- Enable usage of ComCtl32.dll version 6 visual styles.
           See https://docs.microsoft.com/en-us/windows/win32/controls/cookbook-overview -->
      <assemblyIdentity
          type="win32"
          name="Microsoft.Windows.Common-Controls"
          version="6.0.0.0"
          processorArchitecture="*"
          publicKeyToken="6595b64144ccf1df"
          language="*"
        />
    </dependentAssembly>
  </dependency>
</assembly>