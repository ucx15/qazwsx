#setup
# Project Dir
#    └---> Obj
#    └---> Src
#    		└--> Include


# -------- BUILD SETTINGS --------
$C_FLAGS = "-Wall", "-Wextra", "-pedantic", "-std=c++20", "-masm=intel", "-Wsign-compare"

# $Optimization_flags = "-march=native"
$Optimization_flags = "-O3", "-mavx512f", "-march=native", "-s"
# $Optimization_flags = "-ggdb", "-g3"

$LINKER_FLAGS = "-lSDL3"


$buildAll = $false
# $scene_file = "Scenes/cube.json"
# $scene_file = "Scenes/sphere.json"
# $scene_file = "Scenes/myJube.json"
$scene_file = "Scenes/monkey.json"

$out_file = "qazwsx.exe"
$src_dir = "Src/"


$intermediate_dir = "Intermediate/"

# Include Directories
$include_dir = "Src/"
$sdl_inc_dir = "Libs/SDL3/include/"
$stb_inc_dir = "Libs/"

# Library Directories
$sdl_lib_dir = "Libs/SDL3/lib"


# -------- BUILD SCRIPT --------
$src_files = @()

Get-ChildItem -Recurse -Path $src_dir -Filter "*.cpp" | ForEach-Object {
	$src_files += $_.FullName
}

$C_FLAGS += $Optimization_flags

if (!(Test-Path("./Out"))) {
	mkdir Out
}

if (!(Test-Path($intermediate_dir))) {
	mkdir $intermediate_dir
}

# removing previous build
if (Test-Path ./$out_file) {
	Remove-Item $out_file
}


# Compiling
Write-Output "Building:"
foreach ($file in $src_files) {
	$relative_path = $file.Replace((Get-Location).Path + "\", "")
	$base_name = [System.IO.Path]::GetFileNameWithoutExtension($file)
	$obj_file = "$intermediate_dir/${base_name}.o"


	# Check if needs to be recompiled
	$modifyCondition = $true

	if (Test-Path $obj_file) {
		$targetModifiedDate = (Get-Item $obj_file).LastWriteTime
		$sourceModifiedDate = (Get-Item $file).LastWriteTime
		$modifyCondition = $sourceModifiedDate -gt $targetModifiedDate

		# Check corresponding header file if it exists
		$header_file = $file.Replace(".cpp", ".hpp")
		if (Test-Path $header_file) {
			$headerModifiedDate = (Get-Item $header_file).LastWriteTime
			$modifyCondition = $modifyCondition -or ($headerModifiedDate -gt $targetModifiedDate)
		}
	}

	if (($modifyCondition -eq $true) -or ($buildAll -eq $true)) {
		if (Test-Path $obj_file) {
			Remove-Item $obj_file
		}

		Write-Output "    $relative_path"
		g++ $C_FLAGS -I $include_dir -I $stb_inc_dir -I $sdl_inc_dir -o $obj_file -c $file
	}
}

# Linking
$obj_files = Get-ChildItem -Path $intermediate_dir -Filter "*.o"

Write-Output "Linking"
if ($obj_files.Count -gt 0) {
	g++ $obj_files.FullName $C_FLAGS -L $sdl_lib_dir $LINKER_FLAGS -o $out_file
} else {
	Write-Output "No object files found to link!"
}


# Running current build

if (Test-Path ./$out_file) {
	Write-Output "Build Successfully"
	Write-Output ""
	& ./$out_file $scene_file
}

else {
	Write-Output "ERROR in Building!"
}
