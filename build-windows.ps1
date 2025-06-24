#!/usr/bin/env pwsh

<#
.SYNOPSIS
    Build script for Fidelity Engine on Windows
.DESCRIPTION
    This script configures and builds the Fidelity Engine project using CMake and Visual Studio.
    It supports both Debug and Release configurations with optional clean builds.
.PARAMETER Configuration
    Build configuration: Debug or Release (default: Release)
.PARAMETER Clean
    Perform a clean build by removing existing build directory
.PARAMETER Generator
    CMake generator to use (default: "Visual Studio 17 2022")
.PARAMETER Parallel
    Number of parallel build jobs (default: number of CPU cores)
.PARAMETER Verbose
    Enable verbose build output
.EXAMPLE
    .\build-windows.ps1
.EXAMPLE
    .\build-windows.ps1 -Configuration Debug -Clean
.EXAMPLE
    .\build-windows.ps1 -Configuration Release -Parallel 8 -Verbose
#>

[CmdletBinding()]
param(    [Parameter()]
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Configuration = "Release",
    
    [Parameter()]
    [switch]$Clean,
    
    [Parameter()]
    [string]$Generator = "Visual Studio 17 2022",
      [Parameter()]
    [int]$Parallel = [Environment]::ProcessorCount,
    
    [Parameter()]
    [switch]$RunTests,
    
    [Parameter()]
    [switch]$Package,
    
    [Parameter()]
    [switch]$Help
)

# Script configuration
$ErrorActionPreference = "Stop"
$InformationPreference = "Continue"

# Show help if requested
if ($Help) {
    Write-Host @"
Fidelity Engine - Windows Build Script

USAGE:
    .\build-windows.ps1 [OPTIONS]

OPTIONS:
    -Configuration <config>    Build configuration (Debug, Release, RelWithDebInfo, MinSizeRel)
                              Default: Release
    -Clean                    Clean build directory before building    -Generator <generator>    CMake generator (default: "Visual Studio 17 2022")
    -Parallel <jobs>          Number of parallel build jobs (default: CPU cores)
    -Verbose                  Enable verbose build output (built-in PowerShell parameter)
    -RunTests                 Run tests after successful build
    -Package                  Create installation package after build
    -Help                     Show this help message

EXAMPLES:
    .\build-windows.ps1
    .\build-windows.ps1 -Configuration Debug -Clean
    .\build-windows.ps1 -Configuration Release -RunTests -Package
    .\build-windows.ps1 -Clean -Verbose -Parallel 4

"@ -ForegroundColor Green
    exit 0
}

# Color output functions - Define these before any other code that might use them
function Write-Success {
    param([string]$Message)
    Write-Host "✓ $Message" -ForegroundColor Green
}

function Write-Info {
    param([string]$Message)
    Write-Host "ℹ $Message" -ForegroundColor Cyan
}

function Write-Warning {
    param([string]$Message)
    Write-Host "⚠ $Message" -ForegroundColor Yellow
}

function Write-ErrorMessage {
    param([string]$Message)
    Write-Host "✗ $Message" -ForegroundColor Red
}

# Project paths
$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"
$ConfigBuildDir = Join-Path $BuildDir $Configuration.ToLower()

function Test-Prerequisites {
    Write-Info "Checking build prerequisites..."
    
    # Check for CMake
    try {
        $cmakeVersion = cmake --version | Select-Object -First 1
        Write-Success "Found CMake: $cmakeVersion"
    }    catch {
        Write-ErrorMessage "CMake not found. Please install CMake and add it to PATH."
        exit 1
    }
    
    # Check for Visual Studio or Build Tools
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsInstances = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json | ConvertFrom-Json
        if ($vsInstances) {
            $vsInstance = $vsInstances | Select-Object -First 1
            Write-Success "Found Visual Studio: $($vsInstance.displayName) ($($vsInstance.installationVersion))"
        }
        else {
            Write-Warning "Visual Studio with C++ tools not found. Build may fail."
        }
    }
    else {
        Write-Warning "Visual Studio installer not found. Build may fail."
    }
    
    # Check for Vulkan SDK
    if ($env:VULKAN_SDK) {
        Write-Success "Found Vulkan SDK: $env:VULKAN_SDK"
    }
    else {
        Write-Warning "VULKAN_SDK environment variable not set. Vulkan features may be disabled."
    }
}

function Initialize-BuildDirectory {
    if ($Clean -and (Test-Path $BuildDir)) {
        Write-Info "Cleaning existing build directory..."
        Remove-Item $BuildDir -Recurse -Force
        Write-Success "Build directory cleaned"
    }
    
    if (-not (Test-Path $ConfigBuildDir)) {
        Write-Info "Creating build directory: $ConfigBuildDir"
        New-Item -ItemType Directory -Path $ConfigBuildDir -Force | Out-Null
        Write-Success "Build directory created"
    }
}

function Invoke-CMakeConfigure {
    Write-Info "Configuring CMake for $Configuration..."
    
    Push-Location $ConfigBuildDir
    try {
        $cmakeArgs = @(
            "-G", $Generator,
            "-A", "x64",
            "-DCMAKE_BUILD_TYPE=$Configuration",
            "-DCMAKE_CONFIGURATION_TYPES=Debug;Release",
            $ProjectRoot
        )
          if ($VerbosePreference -eq 'Continue') {
            $cmakeArgs += "--debug-output"
        }
        
        Write-Info "Running: cmake $($cmakeArgs -join ' ')"
        & cmake @cmakeArgs
        
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed with exit code $LASTEXITCODE"
        }
        
        Write-Success "CMake configuration completed successfully"
    }
    finally {
        Pop-Location
    }
}

function Invoke-CMakeBuild {
    Write-Info "Building project in $Configuration mode..."
    
    Push-Location $ConfigBuildDir
    try {
        $buildArgs = @(
            "--build", ".",
            "--config", $Configuration,
            "--parallel", $Parallel
        )
          if ($VerbosePreference -eq 'Continue') {
            $buildArgs += "--verbose"
        }
        
        Write-Info "Running: cmake $($buildArgs -join ' ')"
        & cmake @buildArgs
        
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed with exit code $LASTEXITCODE"
        }
        
        Write-Success "Build completed successfully"
    }
    finally {
        Pop-Location
    }
}

function Show-BuildSummary {
    Write-Info "=== Build Summary ==="
    Write-Info "Configuration: $Configuration"
    Write-Info "Build Directory: $ConfigBuildDir"
    Write-Info "Parallel Jobs: $Parallel"
    
    # List built executables
    $executablesDir = Join-Path $ConfigBuildDir "Source"
    if (Test-Path $executablesDir) {
        Write-Info "Built Executables:"
        Get-ChildItem -Path $executablesDir -Recurse -Include "*.exe" | ForEach-Object {
            $relPath = $_.FullName.Replace($ConfigBuildDir, "").TrimStart('\')
            Write-Success "  $relPath"
        }
    }
    
    Write-Success "Build completed successfully! 🎉"
}

function Invoke-Tests {
    if (-not $RunTests) { return }
    
    Write-Info "Running tests..."
    
    # Look for test executable
    $testExe = Get-ChildItem -Path $ConfigBuildDir -Recurse -Name "Tests.exe" | Select-Object -First 1
    if ($testExe) {
        $testPath = Join-Path $ConfigBuildDir $testExe
        Write-Info "Found test executable: $testPath"
        
        Push-Location (Split-Path $testPath -Parent)
        try {
            & $testPath
            if ($LASTEXITCODE -eq 0) {
                Write-Success "All tests passed!"
            } else {
                Write-Warning "Some tests failed (exit code: $LASTEXITCODE)"
            }
        }
        catch {
            Write-ErrorMessage "Failed to run tests: $($_.Exception.Message)"
        }
        finally {
            Pop-Location
        }
    } else {
        Write-Warning "Test executable not found"
    }
}

function Invoke-Package {
    if (-not $Package) { return }
    
    Write-Info "Creating installation package..."
    
    Push-Location $ConfigBuildDir
    try {
        & cmake --build . --config $Configuration --target package
        if ($LASTEXITCODE -eq 0) {
            Write-Success "Package created successfully"
            
            # List generated packages
            Get-ChildItem -Path . -Include "*.zip", "*.msi", "*.exe" | ForEach-Object {
                Write-Success "  Package: $($_.Name)"
            }
        } else {
            Write-Warning "Package creation failed or not configured"
        }
    }
    catch {
        Write-Warning "Package creation failed: $($_.Exception.Message)"
    }
    finally {
        Pop-Location
    }
}

function Copy-Resources {
    $resourcesSource = Join-Path $ProjectRoot "Resources"
    $resourcesTarget = Join-Path $ConfigBuildDir "Resources"
    
    if ((Test-Path $resourcesSource) -and (-not (Test-Path $resourcesTarget))) {
        Write-Info "Copying resources to build directory..."
        try {
            Copy-Item -Path $resourcesSource -Destination $resourcesTarget -Recurse -Force
            Write-Success "Resources copied successfully"
        }
        catch {
            Write-Warning "Failed to copy resources: $($_.Exception.Message)"
        }
    }
}

function Main {
    Write-Info "=== Fidelity Engine Build Script for Windows ==="
    Write-Info "Configuration: $Configuration"
    Write-Info "Generator: $Generator"
    Write-Info "Clean Build: $Clean"
    Write-Info "Parallel Jobs: $Parallel"
    Write-Info "Run Tests: $RunTests"
    Write-Info "Create Package: $Package"
    Write-Info ""
    
    $buildStartTime = Get-Date
    
    try {
        Test-Prerequisites
        Initialize-BuildDirectory
        Invoke-CMakeConfigure
        Invoke-CMakeBuild
        Copy-Resources
        Invoke-Tests
        Invoke-Package
        
        $buildEndTime = Get-Date
        $buildDuration = $buildEndTime - $buildStartTime
        
        Write-Info "Total build time: $([math]::Round($buildDuration.TotalSeconds, 2)) seconds"
        Show-BuildSummary
        
        Write-Info ""
        Write-Info "Next steps:"
        Write-Info "  • Run demos: Navigate to $ConfigBuildDir and run executables"
        Write-Info "  • Debug: Open $ConfigBuildDir/*.sln in Visual Studio"
        Write-Info "  • Package: Use -Package flag to create distribution"
        
    }
    catch {
        Write-ErrorMessage "Build failed: $($_.Exception.Message)"
        Write-Info "Check the output above for specific error details"
        exit 1
    }
}

# Run main function
Main
