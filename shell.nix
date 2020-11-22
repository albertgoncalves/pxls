with import <nixpkgs> {};
mkShell {
    buildInputs = [
        clang_10
        cppcheck
        linuxPackages.perf
        python3
        SDL2
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
