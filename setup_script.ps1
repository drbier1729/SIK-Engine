#Required Libs
$libs_list = @(
    "FMOD"
    "glew-2.2.0"
    "glm"
    "imgui"
    "SDL2-2.24.0"
    "spdlog-1.10.0"
    "rapidjson"
    "stbi"
    "assimp"
    "lua"
    "sol"
    "freetype"
)

$curr_path = Get-Location;
$path_array = $curr_path.Path.split("\");

$root_dir = $curr_path;
if ($path_array[-1].Equals("Lua")) {
    #Script is being run during build from VS
    $root_dir = $path_array[0..($path_array.Length-3)] -join "\";
}

$solution_dir = $root_dir.ToString() + "\DTBB";
$libs_zip_path = $root_dir.ToString() + "\Libs.zip";
$libs_path = $root_dir.ToString() + "\Libs";

#Download libs zip by default
$download_libs_zip = $true;

#Check if Libs folder already exists
if (Test-Path $libs_path) {
    #Don't download libs zip since it exists
    $download_libs_zip = $false;

    Write-Host "====================================="
    Write-Host "Verifying Libs are up to date."
    Write-Host "====================================="

    #Listing contents of the Libs folder to verifying all the required libs are present
    $libs_found = Get-ChildItem -Path $libs_path -Name
    $not_found_libs = $libs_list | ?{$libs_found -notcontains $_}
    
    if ($not_found_libs){
        #Download libs zip since it's not up to date
        $download_libs_zip = $true;
        Write-Host "====================================="
        Write-Host "Unable to find :" $not_found_libs
        Write-Host "====================================="
    }
}

if ($download_libs_zip) {
    #Downloading latest libs
    Write-Host "====================================="
    Write-Host "Downloading Libs zip"
    Write-Host "====================================="
    $ProgressPreference = "silentlyContinue";
    wget "http://20.3.72.250:31415/StandardIssueKrab/Libs.zip" -OutFile $libs_zip_path;

    #Extracting libs
    Write-Host "====================================="
    Write-Host "Extracting Libs zip"
    Write-Host "====================================="
    Expand-Archive $libs_zip_path -DestinationPath $root_dir -ErrorAction SilentlyContinue;

    #Deleting the zip file
    Write-Host "====================================="
    Write-Host "Deleting Libs zip"
    Write-Host "====================================="
    Remove-Item $libs_zip_path
}

$resources_path = $root_dir.ToString() + "\Resources";
$resources_zip_path = $root_dir.ToString() + "\Resources.zip";
#Check if Resources folder already exists
if (Test-Path $resources_path) {
    Write-Host "====================================="
    Write-Host "Found Resources folder. Skipping Download."
    Write-Host "====================================="
}
else {
    #Downloading latest Resources
    Write-Host "====================================="
    Write-Host "Downloading Resources zip"
    Write-Host "====================================="
    $ProgressPreference = "silentlyContinue";
    wget "http://20.3.72.250:31415/StandardIssueKrab/Resources.zip" -OutFile $resources_zip_path;

    #Extracting libs
    Write-Host "====================================="
    Write-Host "Extracting Resources zip"
    Write-Host "====================================="
    Expand-Archive $resources_zip_path -DestinationPath $root_dir -ErrorAction SilentlyContinue;

    #Deleting the zip file
    Write-Host "====================================="
    Write-Host "Deleting Resources zip"
    Write-Host "====================================="
    Remove-Item $resources_zip_path
}

#Copying DLLs
Write-Host "====================================="
Write-Host "Copying DLLs to target location"
Write-Host "====================================="
$dll_destination_path = $solution_dir;
$dll_path_list = @(
    $libs_path + "\SDL2-2.24.0\lib\x64\SDL2.dll"
    $libs_path + "\glew-2.2.0\bin\Release\x64\glew32.dll"
    $libs_path + "\FMOD\api\core\lib\x64\fmod.dll"
    $libs_path + "\FMOD\api\core\lib\x64\fmodL.dll"
    $libs_path + "\assimp\bin\Release\assimp-vc143-mt.dll"
    $libs_path + "\freetype\lib\win64\freetype.dll"
)

foreach ($dll_path in $dll_path_list) {
    Copy-Item $dll_path -Destination $dll_destination_path -Force
}