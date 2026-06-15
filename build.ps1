# Check if CMake is installed
where.exe cmake 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake is not installed or not in the PATH. Please install CMake and add it to your PATH."
    exit 1
}

# Initialize variables
$appName = $pwd.Path | Split-Path -Leaf
$platform = "Desktop"
$buildType = "Debug"
$buildPath = "build"
$installPath = "dist"
$shouldRun = $false
$shouldInstall = $false
$generator = "MinGW Makefiles"
$otherArgs = @()

# Process arguments
foreach ($arg in $args) {
    if ($arg -eq "--web") {
        where.exe em++ 2>$null
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Emscripten is not installed or not in the PATH. Please install Emscripten to build for Web."
            exit 1
        }
        $platform = "Web"
        $buildPath = "$buildPath-web"
        $installPath = "$installPath-web"
    } elseif ($arg -eq "--release") {
        $buildType = "Release"
    } elseif ($arg -eq "--run") {
        $shouldRun = $true
    } elseif ($arg -eq "--install") {
        $shouldInstall = $true
    } else {
        $otherArgs += $arg
    }
}

# Configure
if (-not (Test-Path $buildPath)) {
    New-Item -ItemType Directory -Path $buildPath | Out-Null

    if ($platform -eq "Web") {
        emcmake cmake -S . -B $buildPath -DCMAKE_BUILD_TYPE="$buildType" -DPLATFORM="$platform"
    } else {
        cmake -S . -B $buildPath -G $generator -DCMAKE_BUILD_TYPE="$buildType" -DPLATFORM="$platform"
    }
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "CONFIGURE ERROR"
    exit 1
}

# Generate
cmake $buildPath -DPLATFORM="$platform" -DCMAKE_BUILD_TYPE="$buildType"

if ($LASTEXITCODE -ne 0) {
    Write-Host "GENERATE ERROR"
    exit 1
}

# Build
if ($otherArgs.Count -gt 0) {
    cmake --build $buildPath $otherArgs
} else {
    cmake --build $buildPath
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "BUILD ERROR"
    exit 1
}

#Install
if ($shouldInstall) {
    cmake --install $buildPath --component Runtime
}

#Run
if ($shouldRun) {
    if($platform -eq "Web") {
        $appPath = "$buildPath\$appName.html"
        $serveDir = $buildPath
        if ($shouldInstall) {
            $appPath = "$installPath\$appName.html"
            $serveDir = $installPath
        }

        if (-not (Test-Path $appPath)) {
            Write-Host "LAUNCH ERROR: $appPath not found."
            exit 1
        }

        if($buildType -eq "Debug") {
            emrun $appPath
        } else {
            Write-Host "NOTE: Web release version does not use --emrun, open http://localhost:8000/$appName.html in browser."
            python -m http.server -d $serveDir
        }
    } else {
        $appPath = ".\$buildPath\$appName.exe"
        if ($shouldInstall) {
            $appPath = ".\$installPath\$appName.exe"
        }
        
        if (-not (Test-Path $appPath)) {
            Write-Host "LAUNCH ERROR: $appPath not found."
            exit 1
        }
        
        Invoke-Expression $appPath
    }
} 