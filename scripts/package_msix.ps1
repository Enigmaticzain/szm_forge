<#
.SYNOPSIS
Packages SZM Forge into a Windows MSIX bundle.

.DESCRIPTION
This script sets up a staging directory, generates an AppxManifest.xml, 
and uses MakeAppx.exe and SignTool.exe to build and sign the MSIX package.
#>

$AppName = "SZMForge"
$Version = "1.0.0.0"
$Publisher = "CN=SZMForgePublisher"
$PublisherDisplayName = "SZM Engineering"
$StagingDir = ".\Staging"
$OutputDir = ".\"

Write-Host "=== Packaging SZM Forge as MSIX ===" -ForegroundColor Cyan

# 1. Clean Staging
if (Test-Path $StagingDir) { Remove-Item -Recurse -Force $StagingDir }
New-Item -ItemType Directory -Path "$StagingDir" | Out-Null
New-Item -ItemType Directory -Path "$StagingDir\bin" | Out-Null
New-Item -ItemType Directory -Path "$StagingDir\ui" | Out-Null
New-Item -ItemType Directory -Path "$StagingDir\ai_service" | Out-Null
New-Item -ItemType Directory -Path "$StagingDir\Assets" | Out-Null

# 2. Copy Assets
Write-Host "Copying assets to staging..."
# (Assuming build outputs exist relative to scripts directory)
if (Test-Path "..\build\Release\szm_forge.exe") {
    Copy-Item "..\build\Release\szm_forge.exe" -Destination "$StagingDir\bin\"
} else {
    Write-Warning "szm_forge.exe not found in build\Release. Proceeding without it (for testing only)."
}

if (Test-Path "..\new ui\dist\*") {
    Copy-Item "..\new ui\dist\*" -Destination "$StagingDir\ui\" -Recurse
}
if (Test-Path "..\ai_service\*") {
    Copy-Item "..\ai_service\*" -Destination "$StagingDir\ai_service\" -Recurse
}

# 3. Create AppxManifest.xml
Write-Host "Generating AppxManifest.xml..."
$ManifestXml = @"
<?xml version="1.0" encoding="utf-8"?>
<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
         xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10">
  <Identity Name="$AppName" Version="$Version" Publisher="$Publisher" ProcessorArchitecture="x64"/>
  <Properties>
    <DisplayName>SZM Forge</DisplayName>
    <PublisherDisplayName>$PublisherDisplayName</PublisherDisplayName>
    <Description>Physics-Driven Engineering Platform</Description>
    <Logo>Assets\StoreLogo.png</Logo>
  </Properties>
  <Resources>
    <Resource Language="en-US"/>
  </Resources>
  <Dependencies>
    <TargetDeviceFamily Name="Windows.Desktop" MinVersion="10.0.17763.0" MaxVersionTested="10.0.19041.0"/>
  </Dependencies>
  <Applications>
    <Application Id="App" Executable="bin\szm_forge.exe" EntryPoint="Windows.FullTrustApplication">
      <uap:VisualElements DisplayName="SZM Forge" Description="SZM Forge Engineering Suite"
                          BackgroundColor="#000000" Square150x150Logo="Assets\Square150x150Logo.png"
                          Square44x44Logo="Assets\Square44x44Logo.png">
        <uap:DefaultTile Wide310x150Logo="Assets\Wide310x150Logo.png" />
      </uap:VisualElements>
    </Application>
  </Applications>
  <Capabilities>
    <rescap:Capability Name="runFullTrust" xmlns:rescap="http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities"/>
  </Capabilities>
</Package>
"@
Set-Content -Path "$StagingDir\AppxManifest.xml" -Value $ManifestXml

# 4. Pack with MakeAppx (Assuming SDK is in path)
$MsixPath = "$OutputDir\$AppName.msix"
Write-Host "Packing MSIX using MakeAppx..."
& makeappx.exe pack /d $StagingDir /p $MsixPath /o

if ($LASTEXITCODE -eq 0) {
    Write-Host "=== Done! MSIX created successfully at $MsixPath ===" -ForegroundColor Green
    
    # Generate Cert (Optional, but useful for local testing)
    # $cert = New-SelfSignedCertificate -Type Custom -Subject $Publisher -KeyUsage DigitalSignature ...
    # & signtool.exe sign /fd SHA256 /a /f cert.pfx /p "password" $MsixPath
} else {
    Write-Host "MakeAppx failed. Please ensure the Windows 10 SDK is installed and MakeAppx.exe is in your PATH." -ForegroundColor Red
}
