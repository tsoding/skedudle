with import <nixpkgs> {}; rec {
  skedudleEnv = stdenv.mkDerivation {
    name = "skedudle-env";
    buildInputs = [ stdenv gcc gdb valgrind gnumake pkgconfig ];
  };
}
