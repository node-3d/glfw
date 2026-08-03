#!/usr/bin/env bash
set -euo pipefail

target_arch="${1:-$(node -p 'process.arch')}"
brew_arch=()
brew_bin="brew"

if [[ "$target_arch" == "x64" ]]; then
	brew_arch=("arch" "-x86_64")
	brew_bin="/usr/local/bin/brew"

	if [[ ! -x "$brew_bin" ]]; then
		echo "[mac-gl] installing x86_64 Homebrew into /usr/local"
		"${brew_arch[@]}" /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
	fi
fi

echo "[mac-gl] target_arch=$target_arch"
echo "[mac-gl] brew=${brew_arch[*]:-native} $brew_bin"
"${brew_arch[@]}" "$brew_bin" --version
"${brew_arch[@]}" "$brew_bin" config

"${brew_arch[@]}" "$brew_bin" install mesa

mesa_prefix="$("${brew_arch[@]}" "$brew_bin" --prefix mesa)"
runtime_dir="$PWD/.glfw-runtime-lib"
mkdir -p "$runtime_dir"

link_lib() {
	local name="$1"
	local source="$mesa_prefix/lib/$name"

	if [[ -e "$source" ]]; then
		ln -sf "$source" "$runtime_dir/$name"
	fi
}

link_lib "libEGL.dylib"
link_lib "libEGL.1.dylib"
link_lib "libGLESv2.dylib"
link_lib "libGLESv2.2.dylib"
link_lib "libGL.dylib"
link_lib "libGL.1.dylib"

for target_dir in node_modules/@node-3d/deps-opengl/bin-darwin-* bin-darwin-*; do
	if [[ -d "$target_dir" ]]; then
		for dylib in "$runtime_dir"/*.dylib; do
			[[ -e "$dylib" ]] || continue
			ln -sf "$dylib" "$target_dir/$(basename "$dylib")"
		done
	fi
done

echo "[mac-gl] mesa=$mesa_prefix"
echo "[mac-gl] runtime=$runtime_dir"
find "$runtime_dir" -maxdepth 1 -type l -print -exec file {} \;
find "$mesa_prefix/lib" -maxdepth 1 -type f -name "lib*.dylib" -print -exec file {} \;

{
	echo "NODE_3D_GLFW_RUNTIME_LIB=$runtime_dir"
	echo "DYLD_LIBRARY_PATH=$runtime_dir:$mesa_prefix/lib:${DYLD_LIBRARY_PATH:-}"
	echo "DYLD_FALLBACK_LIBRARY_PATH=$runtime_dir:$mesa_prefix/lib:${DYLD_FALLBACK_LIBRARY_PATH:-}"
	echo "LIBGL_ALWAYS_SOFTWARE=1"
	echo "MESA_LOADER_DRIVER_OVERRIDE=llvmpipe"
} >> "$GITHUB_ENV"
