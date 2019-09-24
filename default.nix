with import <nixpkgs> {}; rec {
  nodecEnv = stdenv.mkDerivation {
    name = "nodec-env";
    buildInputs = [ stdenv gcc gdb gnumake pkgconfig ];
  };
}
