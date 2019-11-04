$ErrorActionPreference = "Inquire"

$haxetoolsDir = "$Env:HAXEPATH"
$hashlinkDir = "$haxetoolsDir\lib\hashlink"
$scratchDir = "$Env:TEMP\HashLinkLibInstaller.TMP"


Remove-Item "$scratchDir" -Recurse -ErrorAction Ignore


New-Item -ItemType directory -Path "$scratchDir" | Out-Null


function Check-Install-Lib {
    Param($libName, $libDir)

    Write-Host "-------------------------------------------------------------"
    Write-Host "About to install includes for library: $libName"

    If ( Test-Path "$libDir" ) {
        $opt = $Host.UI.PromptForChoice(
            "",
            "There's already a directory at $sdlTargetPath",
            @("&Reinstall $libName", "&Keep current $libName"),
            0);
        
        if ($opt -ne 0) {
            return $false
        }

        Write-Host "Deleting $libDir"
        Remove-Item "$libDir" -Recurse
    }
    
    return $true
}

$sdlTargetPath = "$hashlinkDir\include\sdl"
if (Check-Install-Lib "SDL" "$sdlTargetPath") {
    $sdlVersion = "2.0.10"
    $sdlZip = "SDL2-devel-$sdlVersion-VC.zip"
    Invoke-WebRequest "http://libsdl.org/release/$sdlZip" -OutFile "$scratchDir\$sdlZip"
    Expand-Archive "$scratchDir\$sdlZip" -DestinationPath "$scratchDir"
    Move-Item -Verbose -Path "$scratchDir\SDL2-$sdlVersion" -Destination "$sdlTargetPath"   
    Copy-Item -Verbose -Path "$sdlTargetPath\lib\x86\SDL2.dll" -Destination "$haxetoolsDir"
    Copy-Item -Verbose -Path "$sdlTargetPath\lib\x64\SDL2.dll" -Destination "$haxetoolsDir\x64"
    Write-Host "SDL includes $sdlVersion installed."
}

$openalTargetPath = "$hashlinkDir\include\openal"
if (Check-Install-Lib "OpenAL" "$openalTargetPath") {
    #$openalVersion = "1.19.1"
    $openalVersion = "1.17.2"
    $openalZip = "openal-soft-$openalVersion-bin.zip"
    Invoke-WebRequest "https://openal-soft.org/openal-binaries/$openalZip" -OutFile "$scratchDir\$openalZip"
    Expand-Archive "$scratchDir\$openalZip" -DestinationPath "$scratchDir"
    Move-Item -Verbose -Path "$scratchDir\openal-soft-$openalVersion-bin" -Destination "$openalTargetPath"    
    Copy-Item -Verbose -Path "$openalTargetPath\bin\Win32\soft_oal.dll" -Destination "$haxetoolsDir\OpenAL32.dll"
    Copy-Item -Verbose -Path "$openalTargetPath\bin\Win64\soft_oal.dll" -Destination "$haxetoolsDir\x64\OpenAL32.dll"
    Write-Host "OpenAL $openalVersion installed."
}


Remove-Item "$scratchDir" -Recurse -ErrorAction Ignore
