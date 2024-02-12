{
  description = "Converts between bibtex and biblatex.";

  inputs =
    {
      nixpkgs.url = github:NixOS/nixpkgs/nixpkgs-unstable;

      flake-utils.url = github:numtide/flake-utils;

      gitignore.url = github:hercules-ci/gitignore.nix;
      gitignore.inputs.nixpkgs.follows = "nixpkgs";

      tree-sitter-biber.url = github:acristoffers/tree-sitter-biber;
      tree-sitter-biber.flake = false;
    };

  outputs = inputs:
    let
      inherit (inputs) nixpkgs gitignore flake-utils tree-sitter-biber;
      inherit (gitignore.lib) gitignoreSource;
    in
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        buildInputs = with pkgs;[
          glib
          libselinux # gio
          libsepol # libselinux
          pcre # libselinux
          pcre2 # gio
          tree-sitter
          util-linux # gio (libmount)
        ];
      in
      rec {
        formatter = pkgs.nixpkgs-fmt;
        packages.default = packages.bib-converter;
        packages.bib-converter = pkgs.stdenv.mkDerivation
          {
            name = "bib-converter";
            version = (builtins.readFile ./resources/version);
            src = gitignoreSource ./.;
            nativeBuildInputs = with pkgs;[ cmake pkg-config ];
            enableParallelBuilding = true;
            inherit buildInputs;
            cmakeFlags = [
              "-DFETCHCONTENT_SOURCE_DIR_TREESITTER_BIBER=${tree-sitter-biber}"
            ];
            meta = {
              description = "Converts between bibtex and biblatex.";
              homepage = "https://github.com/acristoffers/bib-converter";
              license = pkgs.lib.licenses.mpl20;
            };
          };
        apps = rec {
          bib-converter = { type = "app"; program = "${packages.bib-converter}/bin/bib-converter"; };
          default = bib-converter;
        };
        devShell = pkgs.mkShell {
          buildInputs = with pkgs;[ cmake pkg-config stdenv git busybox ] ++ buildInputs;
          shellHook = ''export CMAKE_EXPORT_COMPILE_COMMANDS=1'';
        };
      }
    );
}
