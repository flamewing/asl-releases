# Source from the package directory under MSYS2 Bash, after setting executables
# and runtime_dlls. Copies toolchain DLLs and appends their names to runtime_dlls.
runtime_root=$(cygpath -m "$MSYSTEM_PREFIX")
declare -A seen_dlls=()
for executable in "${executables[@]}"; do
  dependencies=$(ntldd -R -D "$(cygpath -w "$MSYSTEM_PREFIX/lib")" "$executable")
  while IFS= read -r line; do
    if [[ "$line" =~ =[\>][[:space:]]+(.+)[[:space:]]+\(0x[[:xdigit:]]+\) ]]; then
      dependency=${BASH_REMATCH[1]//\\//}
      # Bundle toolchain DLLs, excluding Windows system libraries.
      if [[ "${dependency,,}" == "${runtime_root,,}/bin/"* || "${dependency,,}" == "${runtime_root,,}/lib/"* ]]; then
        dependency=$(cygpath -u "$dependency")
        dll=$(basename "$dependency")
        if [[ ! -v seen_dlls["${dll,,}"] ]]; then
          cp "$dependency" .
          runtime_dlls+=("$dll")
          seen_dlls["${dll,,}"]=1
        fi
      fi
    fi
  done <<< "$dependencies"
done
