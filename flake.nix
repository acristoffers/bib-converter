{
  description = "Removes files like .DS_Store and Thumb.db from the disk";

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
        packages.default = packages.remove-trash;
        packages.remove-trash = pkgs.stdenv.mkDerivation
          {
            name = "remove-trash";
            version = (builtins.readFile ./resources/version);
            src = gitignoreSource ./.;
            nativeBuildInputs = with pkgs;[ cmake pkg-config ];
            enableParallelBuilding = true;
            inherit buildInputs;
            cmakeFlags = [
              "-DFETCHCONTENT_SOURCE_DIR_TREESITTER_BIBER=${tree-sitter-biber}"
            ];
            meta = {
              description = "Removes files like .DS_Store and Thumb.db from the disk";
              homepage = "https://github.com/acristoffers/remove-trash";
              license = pkgs.lib.licenses.mpl20;
            };
          };
        apps = rec {
          remove-trash = { type = "app"; program = "${packages.remove-trash}/bin/remove-trash"; };
          default = remove-trash;
        };
        devShell = pkgs.mkShell {
          buildInputs = with pkgs;[ cmake pkg-config stdenv git busybox ] ++ buildInputs;
          shellHook = ''export CMAKE_EXPORT_COMPILE_COMMANDS=1'';
        };
      }
    );
}
